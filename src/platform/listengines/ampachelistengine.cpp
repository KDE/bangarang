/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)
* <http://gitorious.org/bangarang>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../mediaitemmodel.h"
#include "ampachelistengine.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

#include <QApplication>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>
#include <KWallet/Wallet>
#include <QFile>
#include <QCryptographicHash>
#include <Nepomuk/ResourceManager>

AmpacheListEngine::AmpacheListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
    m_fetchingThumbnails = false;
    m_token = QString();
}

AmpacheListEngine::~AmpacheListEngine()
{

}

void AmpacheListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    if (m_updateSourceInfo || m_removeSourceInfo) {
        return;
    }


    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;

    QString engineArg = m_mediaListProperties.engineArg();
    QStringList engineFilterList = m_mediaListProperties.engineFilterList();

    if (!engineFilterList.isEmpty()) {
        QString server = m_mediaListProperties.filterFieldValue("server");
        QString request = m_mediaListProperties.filterFieldValue("request");
        m_pendingRequest["server"] = server;
        m_pendingRequest["request"] = request;

        if (request == "root") {
            QString username = m_mediaListProperties.filterFieldValue("username");
            QString key = m_mediaListProperties.filterFieldValue("key");

            //Handshake
            m_pendingRequest["username"] = username;
            m_pendingRequest["key"] = key;
            sendHandshake(server, username, key);

        } else {
            QString requestUrlStr = server + m_mediaListProperties.filterFieldValue("requestPath");
            KUrl requestUrl(requestUrlStr);
            if (!requestUrl.isEmpty()) {
                QString targetFile = QString("bangarang/temp/%1").arg(QString("ampacheResult-%1-%2").arg(m_requestSignature, m_subRequestSignature));
                KUrl targetUrl = KUrl(KStandardDirs::locateLocal("data", targetFile, true));
                connectDownloader();
                emit download(requestUrl, targetUrl);
            }
        }

        //Start event loop to wait for feed results
        exec();
    } else {
        //Return no results
        m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
        m_mediaListProperties.type = QString("Categories");
        emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    }

}

void AmpacheListEngine::downloadComplete(const KUrl &from, const KUrl &to)
{
    if (!m_fetchingThumbnails) {
        m_mediaList.clear();
        m_artworkUrlList.clear();
        QFile file(to.path());
        QDomDocument feedDoc("feed");
        feedDoc.setContent(&file);

        bool errorExists = false;
        if (!feedDoc.elementsByTagName("error").isEmpty()) {
            errorExists = true;
            QDomElement errorElement = feedDoc.elementsByTagName("error").at(0).toElement();
            if (errorElement.attribute("code") == "400") {
                 errorExists = true;
            }
        }

        if (!feedDoc.elementsByTagName("auth").isEmpty()) {
            QDomElement authElement = feedDoc.elementsByTagName("auth").at(0).toElement();
            m_token = authElement.text();
        }

        if (errorExists) {
            QDomElement errorElement = feedDoc.elementsByTagName("error").at(0).toElement();
            MediaItem errorMessage;
            errorMessage.title = errorElement.text();
            errorMessage.type = "Message";
            m_mediaList.append(errorMessage);
        } else if (m_pendingRequest["request"] == "root") {
            QString server = m_pendingRequest["server"];
            MediaItem mediaItem;
            QString requestPath = QString("/server/xml.server.php?action=artists&auth=%2").arg(m_token);
            mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=artists").arg(server, requestPath);
            mediaItem.title = i18n("Artists");
            mediaItem.type = QString("Category");
            mediaItem.artwork = KIcon("system-users");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            m_mediaList.append(mediaItem);
            requestPath = QString("/server/xml.server.php?action=albums&auth=%2").arg(m_token);
            mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=albums").arg(server, requestPath);
            mediaItem.title = i18n("Albums");
            mediaItem.type = QString("Category");
            mediaItem.artwork = KIcon("media-optical");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            m_mediaList.append(mediaItem);
            requestPath = QString("/server/xml.server.php?action=genres&auth=%2").arg(m_token);
            mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=genres").arg(server, requestPath);
            mediaItem.title = i18n("Genres");
            mediaItem.type = QString("Category");
            mediaItem.artwork = KIcon("flag-blue");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            m_mediaList.append(mediaItem);
            requestPath = QString("/server/xml.server.php?action=playlists&auth=%2").arg(m_token);
            mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=playlists").arg(server, requestPath);
            mediaItem.title = i18n("Playlists");
            mediaItem.type = QString("Category");
            mediaItem.artwork = KIcon("view-media-playlist");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            m_mediaList.append(mediaItem);
            m_pendingRequest.clear();

            m_mediaListProperties.summary = i18np("1 item", "%1 items", m_mediaList.count());
            m_mediaListProperties.type = QString("Categories");

        } else if (m_pendingRequest["request"] == "artists")  {
            QString server = m_pendingRequest["server"];
            //Iterate through item nodes of the XML document
            QDomNodeList items = feedDoc.elementsByTagName("artist");
            for (int i = 0; i < items.count(); i++) {
                MediaItem mediaItem;
                QString id = items.at(i).toElement().attribute("id");
                QString requestPath = QString("/server/xml.server.php?action=artist_songs&auth=%2&filter=%3").arg(m_token, id);
                mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=songs").arg(server, requestPath);
                mediaItem.type = QString("Category");
                mediaItem.artwork = KIcon("system-users");
                QDomNodeList childNodes = items.at(i).childNodes();
                for (int j = 0; j < childNodes.count(); j++) {
                    QDomElement element = childNodes.at(j).toElement();
                    if (element.tagName() == "name") {
                        mediaItem.title = element.text();
                        mediaItem.fields["title"] = mediaItem.title;
                    }
                }
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                m_mediaList.append(mediaItem);
            }
            m_mediaListProperties.summary = i18np("1 artist", "%1 artists", m_mediaList.count());
            m_mediaListProperties.type = QString("Categories");

        } else if (m_pendingRequest["request"] == "albums")  {
            QString server = m_pendingRequest["server"];
            //Iterate through item nodes of the XML document
            QDomNodeList items = feedDoc.elementsByTagName("album");
            for (int i = 0; i < items.count(); i++) {
                MediaItem mediaItem;
                QString id = items.at(i).toElement().attribute("id");
                QString requestPath = QString("/server/xml.server.php?action=album_songs&auth=%2&filter=%3").arg(m_token, id);
                mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=songs").arg(server, requestPath);
                mediaItem.type = QString("Category");
                mediaItem.artwork = KIcon("media-optical");
                QString artist;
                QDomNodeList childNodes = items.at(i).childNodes();
                for (int j = 0; j < childNodes.count(); j++) {
                    QDomElement element = childNodes.at(j).toElement();
                    if (element.tagName() == "name") {
                        mediaItem.title = element.text();
                        mediaItem.fields["title"] = mediaItem.title;
                    }
                    if (element.tagName() == "artist") {
                        artist = element.text();
                    }
                }
                mediaItem.subTitle = artist;
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                m_mediaList.append(mediaItem);
            }
            m_mediaListProperties.summary = i18np("1 album", "%1 albums", m_mediaList.count());
            m_mediaListProperties.type = QString("Categories");

        } else if (m_pendingRequest["request"] == "genres")  {
            QString server = m_pendingRequest["server"];
            //Iterate through item nodes of the XML document
            QDomNodeList items = feedDoc.elementsByTagName("tag");
            for (int i = 0; i < items.count(); i++) {
                MediaItem mediaItem;
                QString id = items.at(i).toElement().attribute("id");
                QString requestPath = QString("/server/xml.server.php?action=genre_songs&auth=%2&filter=%3").arg(m_token, id);
                mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||genre=songs").arg(server, requestPath);
                mediaItem.type = QString("Category");
                mediaItem.artwork = KIcon("flag-blue");
                QDomNodeList childNodes = items.at(i).childNodes();
                for (int j = 0; j < childNodes.count(); j++) {
                    QDomElement element = childNodes.at(j).toElement();
                    if (element.tagName() == "name") {
                        mediaItem.title = element.text();
                        mediaItem.fields["title"] = mediaItem.title;
                    }
                }
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                m_mediaList.append(mediaItem);
            }
            m_mediaListProperties.summary = i18np("1 genre", "%1 genres", m_mediaList.count());
            m_mediaListProperties.type = QString("Categories");

        } else if (m_pendingRequest["request"] == "playlists")  {
            QString server = m_pendingRequest["server"];
            //Iterate through item nodes of the XML document
            QDomNodeList items = feedDoc.elementsByTagName("playlist");
            for (int i = 0; i < items.count(); i++) {
                MediaItem mediaItem;
                QString id = items.at(i).toElement().attribute("id");
                QString requestPath = QString("/server/xml.server.php?action=playlist_songs&auth=%2&filter=%3").arg(m_token, id);
                mediaItem.url = QString("ampache://audio?server=%1||requestPath=%2||request=songs").arg(server, requestPath);
                mediaItem.type = QString("Category");
                mediaItem.artwork = KIcon("view-media-playlist");
                QDomNodeList childNodes = items.at(i).childNodes();
                for (int j = 0; j < childNodes.count(); j++) {
                    QDomElement element = childNodes.at(j).toElement();
                    if (element.tagName() == "name") {
                        mediaItem.title = element.text();
                        mediaItem.fields["title"] = mediaItem.title;
                    }
                }
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                m_mediaList.append(mediaItem);
            }
            m_mediaListProperties.summary = i18np("1 playlist", "%1 playlists", m_mediaList.count());
            m_mediaListProperties.type = QString("Categories");

        }  else if (m_pendingRequest["request"] == "songs")  {
            //Iterate through item nodes of the XML document
            QDomNodeList items = feedDoc.elementsByTagName("song");
            for (int i = 0; i < items.count(); i++) {
                MediaItem mediaItem;
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Stream";
                mediaItem.artwork = KIcon("text-html");
                QDomNodeList childNodes = items.at(i).childNodes();
                QString artist;
                QString album;
                for (int j = 0; j < childNodes.count(); j++) {
                    QDomElement element = childNodes.at(j).toElement();
                    if (element.tagName() == "title") {
                        mediaItem.title = element.text();
                        mediaItem.fields["title"] = mediaItem.title;
                    }
                    if (element.tagName() == "artist") {
                        artist = element.text();
                    }
                    if (element.tagName() == "album") {
                        album = element.text();
                    }
                    if (element.tagName() == "track") {
                        mediaItem.fields["trackNumber"] = element.text();
                    }
                    if (element.tagName() == "url") {
                        mediaItem.url = element.text();
                        mediaItem.fields["url"] = mediaItem.url;
                    }
                    if (element.tagName() == "time") {
                        int duration = element.text().trimmed().toInt();
                        mediaItem.fields["duration"] = duration;
                        mediaItem.duration = Utilities::durationString(duration);
                    }
                }
                mediaItem.subTitle = QString("%1 - %2").arg(artist, album);
                mediaItem.fields["description"] = mediaItem.subTitle;
                m_mediaList.append(mediaItem);
            }
            m_mediaListProperties.summary = i18np("1 song", "%1 songs", m_mediaList.count());
            m_mediaListProperties.type = QString("Sources");

        }

        //Return results
        emit results(m_requestSignature, m_mediaList, m_mediaListProperties, true, m_subRequestSignature);

        //Consume results file
        file.remove();

        //Launch thumbnail downloads
//        for (int i = 0; i < m_artworkUrlList.count(); i++) {
//            KUrl artworkUrl = m_artworkUrlList.at(i);
//            if (!artworkUrl.isEmpty()) {
//                m_fetchingThumbnails = true;
//                QString artworkTargetFile = QString("bangarang/thumbnails/%1").arg(artworkUrl.fileName());
//                KUrl artworkTargetUrl = KUrl(KStandardDirs::locateLocal("data", artworkTargetFile, true));
//                emit download(artworkUrl, artworkTargetUrl);
//            }
//        }

//    } else {
//        //Update feed item artwork with thumbnail downloads
//        int index = m_artworkUrlList.indexOf(from);
//        if (index != -1) {
//            MediaItem mediaItem = m_mediaList.at(index);
//            mediaItem.fields["artworkUrl"] = to.prettyUrl();
//            QImage artwork = Utilities::getArtworkImageFromMediaItem(mediaItem);
//            if (!artwork.isNull()) {
//                mediaItem.hasCustomArtwork = true;
//                emit updateArtwork(artwork, mediaItem);
//            }
//            m_mediaList.removeAt(index);
//            m_artworkUrlList.removeAt(index);
//            if (m_mediaList.count() == 0) {
//                m_fetchingThumbnails = false;
//                disconnectDownloader();
//            }
//        }
    }

    if (!m_fetchingThumbnails) {
        //Exit event loop
        quit();
    }

    Q_UNUSED(from);
}

void AmpacheListEngine::sendHandshake(const QString server, const QString username, const QString key)
{
    QString timestamp = QString::number(QDateTime::currentDateTime().toTime_t());
    QString passPhrase = Utilities::sha256Of(timestamp + key);
    QString serverUrlStr = QString("%1/server/xml.server.php?action=handshake&auth=%2&timestamp=%3&version=350001&user=%4")
                                  .arg(server)
                                  .arg(passPhrase)
                                  .arg(timestamp)
                                  .arg(username);
    KUrl serverUrl(serverUrlStr);
    if (!serverUrl.isEmpty()) {
        QString targetFile = QString("bangarang/temp/%1").arg(QString("ampacheResult"));
        KUrl targetUrl = KUrl(KStandardDirs::locateLocal("data", targetFile, true));
        connectDownloader();
        emit download(serverUrl, targetUrl);
    }
}
