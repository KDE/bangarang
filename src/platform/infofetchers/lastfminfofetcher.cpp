/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#include "lastfminfofetcher.h"
#include "../downloader.h"
#include "../mediaitemmodel.h"
#include "../utilities/utilities.h"

#include <KDebug>
#include <KGlobal>
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <Solid/Networking>

#include <QDomDocument>
#include <QFile>
#include <QTextEdit>

LastfmInfoFetcher::LastfmInfoFetcher(QObject *parent) :
        InfoFetcher(parent)
{
    m_name = i18n("Last.fm");
    m_icon = KIcon("bangarang-lastfm");
    m_url = KUrl("http://last.fm");
    m_about = i18n("Note: This fetcher uses the Last.fm API but is not endorsed or certified by Last.fm.");

    QString lang = KGlobal::locale()->language();
    if (lang.size() > 2) {
        lang = lang.left(2);
    }

    //NOTE: The API key below must be used only in Bangarang.  Please do not use this key in other applications.
    //      API keys can be requested from last.fm.
    m_apiKey = "07066aba654ada984ea6b45032f510c0";
    m_artistSearchAPI = QString("http://ws.audioscrobbler.com/2.0/?method=artist.search&api_key=%1&limit=%2&artist=%3").arg(m_apiKey).arg("%1").arg("%2");
    m_artistInfoAPI = QString("http://ws.audioscrobbler.com/2.0/?method=artist.getInfo&autocorrect=1&api_key=%1&lang=%2&artist=%3").arg(m_apiKey).arg(lang).arg("%1");
    m_albumInfoAPI = QString("http://ws.audioscrobbler.com/2.0/?method=album.getInfo&autocorrect=1&api_key=%1&lang=%2&album=%3&artist=%4").arg(m_apiKey).arg(lang).arg("%1").arg("%2");
    m_trackInfoAPI = QString("http://ws.audioscrobbler.com/2.0/?method=track.getInfo&autocorrect=1&api_key=%1&lang=%2&track=%3&artist=%4").arg(m_apiKey).arg(lang).arg("%1").arg("%2");
    m_timeout = 20000;

    m_downloader = new Downloader(this);
    connect(this, SIGNAL(download(KUrl,KUrl)), m_downloader, SLOT(download(KUrl,KUrl)));
    connect(m_downloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(gotLastfmInfo(KUrl,KUrl)));

    //Define fetchable fields
    m_fetchableFields["Music"] = QStringList() << "artwork" << "title" << "description" << "artist" << "album" << "trackNumber" << "genre" << "year" << "duration";
    m_fetchableFields["Artist"] = QStringList() << "artwork" << "title" << "description";
    //m_fetchableFields["Album"] = QStringList() << "artwork" << "title" << "description";

    //Define required fields
    m_requiredFields["Music"] = QStringList() << "title" << "artist";
    m_requiredFields["Artist"] = QStringList() << "title";
    //m_requiredFields["Album"] = QStringList() << "title" << "artist";

    m_lastRequestTime = QDateTime::currentDateTime();

}

bool LastfmInfoFetcher::available(const QString &subType)
{
    /*
     * We check if the network is connected. As some distributions use ifup to set up their connection
     * Solid::Networking may return Unknown, as it only checks NetworkManager, not ifup.
     * A real check to the Lastfm server would be overkill and take too long here.
     * In general this fetcher should not be available if the Lastfm server cannot be reached.
     * Maybe we get an idea to handle this properly later on, but there's currently the following problem:
     * A connection test would take some, so maybe the user wouldn't see this infofetcher directly or
     * even worse this test would block the whole application.
     */
    Solid::Networking::Status state = Solid::Networking::status();
    bool networkConnected = ( state == Solid::Networking::Connected ||
                              state == Solid::Networking::Unknown );
    bool handlesType = (m_requiredFields[subType].count() > 0);
    return (networkConnected && handlesType);
}

void LastfmInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    //Wait 5 seconds before processing next request
    //NOTE: This is intended to prevent abuse of the last.fm api during autofetches.
    while (m_lastRequestTime.secsTo(QDateTime::currentDateTime()) < 5) {
        sleep(1);
    }

    m_updateRequiredFields = updateRequiredFields;
    m_mediaList.clear();
    m_requestKeys.clear();
    m_fetchedMatches.clear();
    m_thumbnailKeys.clear();
    m_moreInfoKeys.clear();
    m_updateArtwork = updateArtwork;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        QString LastfmUrlStr;
        if (mediaItem.subType() == "Artist") {
            m_requestType = ArtistRequest;
            QString searchTerm = Utilities::titleForRequest(mediaItem.fields["title"].toString());
            LastfmUrlStr = m_artistSearchAPI.arg(maxMatches).arg(searchTerm);
        } else if (mediaItem.subType() == "Album") {
            m_requestType = AlbumRequest;
            QString album = mediaItem.fields["title"].toString();
            QString artist = mediaItem.fields["artist"].toString();
            LastfmUrlStr = m_albumInfoAPI.arg(album).arg(artist);
        } else if (mediaItem.subType() == "Music") {
            m_requestType = TrackRequest;
            QString title = mediaItem.fields["title"].toString();
            QString artist = mediaItem.fields["artist"].toString();
            LastfmUrlStr = m_trackInfoAPI.arg(title).arg(artist);
        }

        KUrl LastfmUrl(LastfmUrlStr);
        if (!LastfmUrl.isEmpty()) {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(LastfmUrl.prettyUrl());

            //Retrieve Lastfm info
            QString LastfmTargetFile = QString("bangarang/temp/Lastfm%1.xml").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzzz"));
            KUrl LastfmTargetUrl = KUrl(KStandardDirs::locateLocal("data", LastfmTargetFile, true));
            QFile LastfmTarget(LastfmTargetUrl.path());
            LastfmTarget.remove();
            setFetching();
            emit download(LastfmUrl, LastfmTargetUrl);
        }
    }
}

void LastfmInfoFetcher::gotLastfmInfo(const KUrl &from, const KUrl &to)
{
    m_lastRequestTime = QDateTime::currentDateTime();

    //Process original requests
    processOriginalRequest(from, to);

    //Process thumbnails
    processThumbnails(from, to);

    //Process more info
    processMoreInfo(from, to);

    //Determine if all info for each match set is fetched
    for (int i = 0; i < m_requestKeys.count(); i++) {
        //Check if original request is done
        bool originalRequestDone = (m_requestKeys.at(i) == "done");
        if (!originalRequestDone) {
            continue;
        }

        //Check if thumbnail requests for this request are done
        int thumbnailsRequested = m_thumbnailKeys.keys().count();
        QStringList thumbnailValues =  m_thumbnailKeys.values();
        int thumbnailsRetrieved = thumbnailValues.filter(QRegExp("^done")).count();
        bool thumbnailsDone = (thumbnailsRetrieved == thumbnailsRequested) ? true : false;
        if (!thumbnailsDone) {
            continue;
        }

        //Check if more info requests for this request are done
        int moreInfosRequested = m_moreInfoKeys.keys().count();
        QStringList moreInfoValues =  m_moreInfoKeys.values();
        int moreInfosRetrieved = moreInfoValues.filter(QRegExp("^done")).count();
        bool moreInfosDone = (moreInfosRetrieved == moreInfosRequested) ? true : false;
        if (!moreInfosDone) {
            continue;
        }

        //If we got here then it means all data for this request has been fetched
        QList<MediaItem> matches = m_fetchedMatches.value(i);
        emit infoFetched(matches);

        m_requestKeys.replace(i, "alldone");
    }

    //Check if all requests are complete
    bool allDone =(m_requestKeys.filter(QRegExp("^alldone")).count() == m_requestKeys.count());
    if (allDone) {
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
            emit fetchComplete(this);
        }
    } else if (!m_timeout){
        m_timer->start(m_timeoutLength);
    }
}

void LastfmInfoFetcher::processOriginalRequest(const KUrl &from, const KUrl to)
{

    //Determine if we got original request and process
    int originalRequestIndex = m_requestKeys.indexOf(from.prettyUrl());
    bool isOriginalRequest = (originalRequestIndex != -1);
    if (!isOriginalRequest) {
        return;
    }

    //Process results
    MediaItem mediaItem = m_mediaList.at(originalRequestIndex);

    QFile file(to.path());
    QDomDocument LastfmDoc("Lastfm");
    LastfmDoc.setContent(&file);

    QHash<int, KUrl> thumbnailUrls;
    QHash<int, KUrl> moreInfoUrls;
    QList<MediaItem> fetchedMatches;

    //Read Artist results
    if (m_requestType == ArtistRequest) {
        //Iterate through item nodes of the Lastfm XML document
        QDomNodeList artists = LastfmDoc.elementsByTagName("artist");
        for (int i = 0; i < artists.count(); i++) {
            MediaItem match = mediaItem;

            //Clear artwork
            match.artwork = Utilities::defaultArtworkForMediaItem(match);
            match.fields["artworkUrl"] = QString("");

            QDomNodeList nodes = artists.at(i).childNodes();
            bool gotThumbnailUrl = false;
            for (int j = 0; j < nodes.count(); j++) {
                if (!nodes.at(j).isElement()) {
                    continue;
                }
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "name") {
                    match.title = element.text();
                    match.fields["title"] = match.title;
                    QString artistInfoUrlStr = m_artistInfoAPI.arg(match.title);
                    moreInfoUrls.insert(fetchedMatches.count(), artistInfoUrlStr);
                } else if (element.tagName() == "url") {
                    match.fields["relatedTo"] = QStringList(element.text());
                } else if (element.tagName() == "image" && element.attribute("size") == "large") {
                    if (gotThumbnailUrl || !m_updateArtwork) {
                        continue;
                    }
                    QString imageUrl = element.text();
                    KUrl thumbnailUrl = KUrl(imageUrl);
                    if (thumbnailUrl.isValid()) {
                        thumbnailUrls.insert(fetchedMatches.count(), thumbnailUrl);
                        gotThumbnailUrl = true;
                        break;
                    }
                }
            }
            fetchedMatches.append(match);
        }
    }

    //Read Album results
    if (m_requestType == AlbumRequest) {
        //Iterate through item nodes of the Lastfm XML document
        QDomNodeList albums = LastfmDoc.elementsByTagName("album");
        for (int i = 0; i < albums.count(); i++) {
            MediaItem match = mediaItem;

            //Clear artwork
            match.artwork = Utilities::defaultArtworkForMediaItem(match);
            match.fields["artworkUrl"] = QString("");

            QDomNodeList nodes = albums.at(i).childNodes();
            bool gotThumbnailUrl = false;
            for (int j = 0; j < nodes.count(); j++) {
                if (!nodes.at(j).isElement()) {
                    continue;
                }
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "name") {
                    match.title = element.text();
                    match.fields["title"] = match.title;
                } else if (element.tagName() == "wiki") {
                    QDomElement descriptionElement = nodes.at(j).firstChildElement("summary");
                    match.fields["description"] = descriptionElement.text();
                } else if (element.tagName() == "url") {
                    match.fields["relatedTo"] = QStringList(element.text());
                }  else if (element.tagName() == "image" && element.attribute("size") == "large") {
                    if (gotThumbnailUrl || !m_updateArtwork) {
                        continue;
                    }
                    QString imageUrl = element.text();
                    KUrl thumbnailUrl = KUrl(imageUrl);
                    if (thumbnailUrl.isValid()) {
                        thumbnailUrls.insert(fetchedMatches.count(), thumbnailUrl);
                        gotThumbnailUrl = true;
                        break;
                    }
                }
            }
            fetchedMatches.append(match);
        }
    }

    if (m_requestType == TrackRequest) {
        //Iterate through item nodes of the Lastfm XML document
        QDomNodeList tracks = LastfmDoc.elementsByTagName("track");
        for (int i = 0; i < tracks.count(); i++) {
            MediaItem match = mediaItem;

            QDomNodeList nodes = tracks.at(i).childNodes();
            bool gotThumbnailUrl = false;
            QStringList artists;
            QStringList genres;
            for (int j = 0; j < nodes.count(); j++) {
                if (!nodes.at(j).isElement()) {
                    continue;
                }
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "name") {
                    match.title = element.text();
                    match.fields["title"] = match.title;
                } else if (element.tagName() == "wiki") {
                    QDomElement descriptionElement = nodes.at(j).firstChildElement("summary");
                    match.fields["description"] = descriptionElement.text();
                } else if (element.tagName() == "artist") {
                    QDomElement artistNameElement = nodes.at(j).firstChildElement("name");
                    artists.append(artistNameElement.text());
                    match.fields["artist"] = artists;
                } else if (element.tagName() == "album") {
                    QDomElement albumTitleElement = nodes.at(j).firstChildElement("title");
                    match.fields["album"] = albumTitleElement.text();
                    match.fields["trackNumber"] = element.attribute("position").toInt();
                    QDomNodeList albumNodes = nodes.at(j).childNodes();
                    for (int k = 0; k < albumNodes.count(); k++) {
                        if (gotThumbnailUrl || !m_updateArtwork) {
                            continue;
                        }
                        QDomElement albumElement = albumNodes.at(k).toElement();
                        if (albumElement.tagName() == "image" &&
                            albumElement.attribute("size") == "large") {
                            QString imageUrl = albumElement.text();
                            KUrl thumbnailUrl = KUrl(imageUrl);
                            if (thumbnailUrl.isValid()) {
                                thumbnailUrls.insert(fetchedMatches.count(), thumbnailUrl);
                                gotThumbnailUrl = true;
                                break;
                            }
                        }
                    }
                } else if (element.tagName() == "duration") {
                    int duration = element.text().toInt();
                    if (duration > 0) {
                        match.fields["duration"] = duration;
                        match.duration = Utilities::durationString(duration);
                    }
                } else if (element.tagName() == "url") {
                    match.fields["relatedTo"] = QStringList(element.text());
                } else if (element.tagName() == "toptags") {
                    QDomNodeList tagNodes = nodes.at(j).childNodes();
                    for (int k = 0; k < qMin(3, tagNodes.count()); k++) {
                        QDomElement tagElement = tagNodes.at(k).firstChildElement("name");
                        QString genre = Utilities::capitalize(tagElement.text());
                        genres.append(genre);
                        match.fields["genre"] = genres;
                    }
                }
            }
            fetchedMatches.append(match);
        }
    }

    QFile(to.path()).remove();


    //Add fetched matches to collection of fetched matches
    m_fetchedMatches.insert(originalRequestIndex, fetchedMatches);

    //Launch Thumbnail requests
    for (int i = 0; i < thumbnailUrls.keys().count(); i++) {
        int matchIndex = thumbnailUrls.keys().at(i);
        MediaItem match = fetchedMatches.at(matchIndex);
        KUrl thumbnailUrl = thumbnailUrls.value(matchIndex);
        QString key = QString("%1,%2").arg(originalRequestIndex).arg(matchIndex);
        m_thumbnailKeys.insert(key, thumbnailUrl.prettyUrl());
        QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-Lastfm-%2-%3")
                                      .arg(match.subType())
                                      .arg(match.fields["LastfmID"].toString())
                                      .arg(thumbnailUrl.fileName());
        KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
        QFile downloadTarget(thumbnailTargetUrl.path());
        downloadTarget.remove();
        m_timer->start(m_timeoutLength);
        emit download(thumbnailUrl, thumbnailTargetUrl);

    }

    //Launch More info requests
    for (int i = 0; i < moreInfoUrls.count(); i++) {
        int matchIndex = moreInfoUrls.keys().at(i);
        KUrl moreInfoUrl = moreInfoUrls.value(matchIndex);
        QString key = QString("%1,%2").arg(originalRequestIndex).arg(matchIndex);
        m_moreInfoKeys.insert(key,moreInfoUrl.prettyUrl());
        QString moreInfoTargetFile = QString("bangarang/temp/LastfmInfo-%1-%2.xml")
                                     .arg(key)
                                     .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        KUrl moreInfoTargetUrl = KUrl(KStandardDirs::locateLocal("data", moreInfoTargetFile, true));
        QFile moreInfoTarget(moreInfoTargetUrl.path());
        moreInfoTarget.remove();
        m_timer->start(m_timeoutLength);
        emit download(moreInfoUrl, moreInfoTargetUrl);
    }

    //Mark request done
    m_requestKeys.replace(originalRequestIndex, "done");
}

void LastfmInfoFetcher::processThumbnails(const KUrl &from, const KUrl to)
{
    //Determine we got a thumbnail request
    QString requestKey = m_thumbnailKeys.key(from.prettyUrl());
    if (requestKey.isEmpty()) {
        return;
    }

    //Mark thumbnail request done
    m_thumbnailKeys.insert(requestKey, "done");

    //Process results
    int originalRequestIndex = requestKey.split(",").at(0).toInt();
    int matchesIndex = requestKey.split(",").at(1).toInt();

    if (!m_fetchedMatches.contains(originalRequestIndex)) {
        return;
    }

    QList<MediaItem> matches = m_fetchedMatches.value(originalRequestIndex);
    MediaItem match = matches.at(matchesIndex);
    QString thumbnailFile = to.path();
    QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!thumbnail.isNull()) {
        match.artwork = QIcon(thumbnail);
        match.fields["artworkUrl"] = to.prettyUrl();
        match.hasCustomArtwork = true;
    } else {
        match.artwork = Utilities::defaultArtworkForMediaItem(match);
        match.fields["artworkUrl"] = QString("");
        match.hasCustomArtwork = false;
    }
    matches.replace(matchesIndex, match);
    m_fetchedMatches[originalRequestIndex] = matches;
}

void LastfmInfoFetcher::processMoreInfo(const KUrl &from, const KUrl to)
{
    //Determine if we got original request and process
    QString requestKey = m_moreInfoKeys.key(from.prettyUrl());
    if (requestKey.isEmpty()) {
        return;
    }

    //Mark more info request done
    m_moreInfoKeys.insert(requestKey, "done");

    //Process results
    int originalRequestIndex = requestKey.split(",").at(0).toInt();
    int matchesIndex = requestKey.split(",").at(1).toInt();

    if (!m_fetchedMatches.contains(originalRequestIndex)) {
        return;
    }

    QList<MediaItem> matches = m_fetchedMatches.value(originalRequestIndex);
    MediaItem match = matches.at(matchesIndex);

    QFile file(to.path());
    QDomDocument LastfmDoc("LastfmInfo");
    LastfmDoc.setContent(&file);

    kDebug() << "GOT MORE INFO!!!!";
    if (m_requestType == ArtistRequest) {
        //Iterate through item nodes of the Lastfm XML document
        QDomNodeList artists = LastfmDoc.elementsByTagName("artist");
        if (artists.count() == 0) {
            return;
        }
        match.fields["description"] = QString();
        QDomNodeList nodes = artists.at(0).childNodes();
        for (int j = 0; j < nodes.count(); j++) {
            if (!nodes.at(j).isElement()) {
                continue;
            }
            QDomElement element = nodes.at(j).toElement();
            if (element.tagName() == "bio") {
                QDomElement descriptionElement = nodes.at(j).firstChildElement("summary");
                QTextEdit converter;
                converter.setAcceptRichText(true);
                converter.setHtml(descriptionElement.text());
                QString description = converter.toPlainText();
                match.fields["description"] = description;
            }
        }
    }
    QFile(to.path()).remove();

    matches.replace(matchesIndex, match);
    m_fetchedMatches[originalRequestIndex] = matches;
}

void LastfmInfoFetcher::timeout()
{
    //Return any remaining matches that have not yet been returned
    if (!m_fetchedMatches.isEmpty()) {
        foreach (QList<MediaItem> matches, m_fetchedMatches) {
            emit infoFetched(matches);
        }
    }

    InfoFetcher::timeout();
}
