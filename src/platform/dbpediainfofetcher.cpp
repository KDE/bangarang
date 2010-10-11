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

#include "dbpediainfofetcher.h"
#include "dbpediaquery.h"
#include "mediaitemmodel.h"
#include "downloader.h"
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <KDebug>
#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Solid/Networking>

DBPediaInfoFetcher::DBPediaInfoFetcher(QObject * parent) : InfoFetcher(parent)
{
    m_name = i18n("DBPedia");
    //m_icon = KIcon("dbpedia");
    m_timeout = false;
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_dbPediaQuery = new DBPediaQuery(this);
    connect (m_dbPediaQuery, SIGNAL(gotArtistInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
    connect (m_dbPediaQuery, SIGNAL(gotActorInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
    connect (m_dbPediaQuery, SIGNAL(gotDirectorInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));

    m_downloader = new Downloader(this);
    connect(this, SIGNAL(download(KUrl,KUrl)), m_downloader, SLOT(download(KUrl,KUrl)));
    connect(m_downloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(gotThumbnail(KUrl,KUrl)));

    //Define fetchable fields
    m_fetchableFields["Artist"] = QStringList() << "artwork" << "title" << "description";
    m_fetchableFields["Actor"] = QStringList() << "artwork" << "title" << "description";
    m_fetchableFields["Director"] = QStringList() << "artwork" << "title" << "description";

    //Define required fields
    m_requiredFields["Artist"] = QStringList() << "title";
    m_requiredFields["Actor"] = QStringList() << "title";
    m_requiredFields["Director"] = QStringList() << "title";
}

DBPediaInfoFetcher::~DBPediaInfoFetcher()
{
}

QStringList DBPediaInfoFetcher::fetchableFields(const QString &subType)
{
    return m_fetchableFields[subType];
}

QStringList DBPediaInfoFetcher::requiredFields(const QString &subType)
{
    return m_requiredFields[subType];
}

bool DBPediaInfoFetcher::available(const QString &subType)
{
    //Available if connected to network
    bool networkConnected = (Solid::Networking::status() == Solid::Networking::Connected);
    bool handlesType = (m_requiredFields[subType].count() > 0);
    return (networkConnected && handlesType);
}

void DBPediaInfoFetcher::fetchInfo(QList<MediaItem> mediaList, bool updateRequiredFields, bool updateArtwork)
{
    m_updateRequiredFields = updateRequiredFields;
    m_mediaList.clear();
    m_requestKeys.clear();
    m_updateArtwork = updateArtwork;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        if (mediaItem.subType() == "Artist") {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(QString("%1:%2").arg(mediaItem.subType()).arg(mediaItem.fields["title"].toString()));
            connect (m_dbPediaQuery,
                     SIGNAL(gotArtistInfo(bool , const QList<Soprano::BindingSet>, const QString)),
                     this,
                     SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
            setFetching();
            m_dbPediaQuery->getArtistInfo(mediaItem.fields["title"].toString());
        }
        if (mediaItem.subType() == "Actor") {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(QString("%1:%2").arg(mediaItem.subType()).arg(mediaItem.fields["title"].toString()));
            connect (m_dbPediaQuery,
                     SIGNAL(gotActorInfo(bool , const QList<Soprano::BindingSet>, const QString)),
                     this,
                     SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
            setFetching();
            m_dbPediaQuery->getActorInfo(mediaItem.fields["title"].toString());
        }
        if (mediaItem.subType() == "Director") {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(QString("%1:%2").arg(mediaItem.subType()).arg(mediaItem.fields["title"].toString()));
            connect (m_dbPediaQuery,
                     SIGNAL(gotDirectorInfo(bool , const QList<Soprano::BindingSet>, const QString)),
                     this,
                     SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
            setFetching();
            m_dbPediaQuery->getDirectorInfo(mediaItem.fields["title"].toString());
        }
        /*if (mediaItem.type == "Video") {
            QString subType = mediaItem.fields["videoType"].toString();
            if (subType == "Movie") {
                m_mediaList.append(mediaItem);
                kDebug() << "getting movie info:" << mediaList.count();
                m_dbPediaQuery->getMovieInfo(mediaItem.title);
            }
        }*/
    }
}

void DBPediaInfoFetcher::gotPersonInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
{
    //Find corresponding MediaItem
    m_requestKeys.removeAll(requestKey);
    if (successful & !m_timeout) {
        //Find item corresponding to fetched info
        QString subType = requestKey.left(requestKey.indexOf(":"));
        QString title = requestKey.mid(requestKey.indexOf(":") + 1);
        MediaItem mediaItem;
        int foundIndex = -1;
        for (int i = 0; i < m_mediaList.count(); i++) {
            MediaItem item = m_mediaList.at(i);
            if (item.subType() == subType && item.fields["title"].toString() == title) {
                mediaItem = item;
                foundIndex = i;
                break;
            }
        }
        if (foundIndex != -1 && results.count() > 0) {
            Soprano::BindingSet binding = results.at(0);
            //Get Thumbnail
            if (m_updateArtwork) {
                KUrl thumbnailUrl = KUrl(binding.value("thumbnail").uri());
                if (thumbnailUrl.isValid()) {
                    QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-%2-%3")
                                                  .arg(subType)
                                                  .arg(title)
                                                  .arg(thumbnailUrl.fileName());
                    KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
                    QFile downloadTarget(thumbnailTargetUrl.path());
                    downloadTarget.remove();
                    m_thumbnailKeys[QString("%1:%2").arg(subType).arg(title)] = thumbnailUrl.prettyUrl();
                    emit download(thumbnailUrl, thumbnailTargetUrl);
                }
            }

            //Set Title
            QString title = binding.value("title").literal().toString().trimmed();
            if (!title.isEmpty() && m_updateRequiredFields) {
                mediaItem.title = title;
                mediaItem.fields["title"] = mediaItem.title;
            }

            //Set Description
            QString description = binding.value("description").literal().toString().trimmed();
            if (!description.isEmpty()) {
                mediaItem.fields["description"] = description;
            }

            m_mediaList.replace(foundIndex, mediaItem);
            emit infoFetched(mediaItem);
        }
    }
    if (m_requestKeys.count() == 0) {
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
        }
    } else if (!m_timeout){
        m_timer->start(6000);
    }
}

void DBPediaInfoFetcher::gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
{
    int index = -1;
    for (int i = 0; i< m_mediaList.count(); i++) {
        QString keyForCurrentItem = "Movie:" + m_mediaList.at(i).title;
        if (keyForCurrentItem == requestKey) {
            index = i;
            break;
        }
    }

    kDebug() << "Got movie info for " << requestKey << " index:" << index << " successful:" << successful;
    
    if (index != -1 && successful) {
        if (results.count() > 0) {
            MediaItem mediaItem = m_mediaList.at(index);
            
            Soprano::BindingSet binding = results.at(0);
            
            //Get Thumbnail
            KUrl thumbnailUrl = KUrl(binding.value("thumbnail").uri());
            if (thumbnailUrl.isValid()) {
                //getThumbnail(thumbnailUrl);
            }
            
            //Set Title
            mediaItem.title = binding.value("title").literal().toString().trimmed();
            mediaItem.fields["title"] = mediaItem.title;
            
            //Set Description
            mediaItem.fields["description"] = binding.value("description").literal().toString().trimmed();
            
            m_mediaList.replace(index, mediaItem);
            emit infoFetched(mediaItem);
        }
    }
}


void DBPediaInfoFetcher::gotThumbnail(const KUrl &from, const KUrl &to)
{
    QString thumbnailFile = to.path();
    QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!thumbnail.isNull())  {
        //Find item corresponding to fetched info
        QString requestKey = m_thumbnailKeys.key(from.prettyUrl());
        QString subType = requestKey.left(requestKey.indexOf(":"));
        QString title = requestKey.mid(requestKey.indexOf(":") + 1);
        MediaItem mediaItem;
        int foundIndex = -1;
        for (int i = 0; i < m_mediaList.count(); i++) {
            MediaItem item = m_mediaList.at(i);
            if (item.subType() == subType && item.fields["title"].toString() == title) {
                mediaItem = item;
                foundIndex = i;
                break;
            }
        }

        //Update artwork
        if (foundIndex != -1) {
            mediaItem.artwork = QIcon(thumbnail);
            mediaItem.fields["artworkUrl"] = to.prettyUrl();
            m_mediaList.replace(foundIndex, mediaItem);
            emit infoFetched(mediaItem);
        }
    }
}

void DBPediaInfoFetcher::setFetching()
{
    m_isFetching = true;
    m_timer->start(6000);
    emit fetching();
}

void DBPediaInfoFetcher::timeout()
{
    kDebug() << "TIMEOUT";
    m_timeout = true;
    m_isFetching = false;
    emit fetchComplete();
}
