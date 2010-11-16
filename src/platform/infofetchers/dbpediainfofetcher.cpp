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
#include "../dbpediaquery.h"
#include "../mediaitemmodel.h"
#include "../downloader.h"
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
    m_fetchableFields["Movie"] = QStringList() << "artwork" << "title" << "description" << "actor" << "director" << "writer";

    //Define required fields
    m_requiredFields["Artist"] = QStringList() << "title";
    m_requiredFields["Actor"] = QStringList() << "title";
    m_requiredFields["Director"] = QStringList() << "title";
    m_requiredFields["Movie"] = QStringList() << "title";
}

DBPediaInfoFetcher::~DBPediaInfoFetcher()
{
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
    m_fetchedMatches.clear();
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
        if (mediaItem.subType() == "Movie") {
            QString title = mediaItem.title;
            if (title.lastIndexOf(".") == title.length() - 4) { //chop file extension
                title.chop(4);
                mediaItem.title = title;
            }
            m_mediaList.append(mediaItem);
            m_requestKeys.append(QString("%1:%2").arg(mediaItem.subType()).arg(mediaItem.title));
            connect (m_dbPediaQuery,
                     SIGNAL(gotMovieInfo(bool , const QList<Soprano::BindingSet>, const QString)),
                     this,
                     SLOT(gotMovieInfo(bool , const QList<Soprano::BindingSet>, const QString)));
            setFetching();
            m_dbPediaQuery->getMovieInfo(mediaItem.title);
        }
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
            for (int i = 0; i < results.count(); i++) {
                Soprano::BindingSet binding = results.at(i);
                QString name = binding.value("name").literal().toString().trimmed();
                bool newName = true;

                //Check if this result is for a new title
                if (!m_fetchedMatches.isEmpty()) {
                    for (int j = 0; j < m_fetchedMatches.count(); j++) {
                        if (m_fetchedMatches.at(j).title == name) {
                            newName = false;
                        }
                    }
                }

                //Create new match item based on result and add to matches
                if (newName && !name.isEmpty()) {
                    MediaItem match = mediaItem;

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
                            m_thumbnailKeys[QString("%1:%2").arg(subType).arg(name)] = thumbnailUrl.prettyUrl();
                            emit download(thumbnailUrl, thumbnailTargetUrl);
                        }
                    }

                    //Set Title
                    match.title = name;
                    if (m_updateRequiredFields) {
                        match.fields["title"] = match.title;
                    }

                    //Set Description
                    QString description = binding.value("description").literal().toString().trimmed();
                    if (!description.isEmpty()) {
                        match.fields["description"] = description;
                    }
                    m_fetchedMatches.append(match);
                }
            }
            if (!m_fetchedMatches.isEmpty()) {
                emit infoFetched(m_fetchedMatches);
            }
        }
    }
    if (m_requestKeys.count() == 0) {
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
        }
    } else if (!m_timeout){
        m_timer->start(m_timeoutLength);
    }
}

void DBPediaInfoFetcher::gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
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
            if (item.subType() == subType && item.title == title) {
                mediaItem = item;
                foundIndex = i;
                break;
            }
        }
        if (foundIndex != -1 && results.count() > 0) {
            for (int i = 0; i < results.count(); i++) {
                Soprano::BindingSet binding = results.at(i);
                QString title = binding.value("title").literal().toString().trimmed();
                bool newTitle = true;

                //Check if this result is for a new title
                if (!m_fetchedMatches.isEmpty()) {
                    for (int j = 0; j < m_fetchedMatches.count(); j++) {
                        if (m_fetchedMatches.at(j).title == title) {
                            newTitle = false;
                        }
                    }
                }

                //Create new match item based on result and add to matches
                if (newTitle && !title.isEmpty()) {
                    MediaItem match = mediaItem;

                    //Get Thumbnail
                    if (m_updateArtwork) {
                        QString thumbnailUrlString = binding.value("thumbnail").uri().toString();
                        thumbnailUrlString.replace("/wikipedia/commons/", "/wikipedia/en/"); //Wikipedia appears to be storing posters here instead
                        KUrl thumbnailUrl = KUrl(thumbnailUrlString);
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
                    match.title = title;
                    if (m_updateRequiredFields) {
                        match.fields["title"] = title;
                    }

                    //Set Description
                    QString description = binding.value("description").literal().toString().trimmed();
                    if (!description.isEmpty()) {
                        match.fields["description"] = description;
                    }

                    //Set Duration
                    int duration = binding.value("duration").literal().toInt();
                    if (duration != 0) {
                        match.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                        match.fields["duration"] = duration;
                    }
                    //Set releaseDate
                    QDate releaseDate = binding.value("releaseDate").literal().toDate();
                    if (releaseDate.isValid()) {
                        match.fields["releaseDate"] = releaseDate;
                        match.fields["year"] = releaseDate.year();
                    }

                    //Set Actors, Directors, Writers
                    QStringList actors;
                    QStringList directors;
                    QStringList writers;
                    QStringList producers;
                    for (int j = 0; j < results.count(); j++) {
                        binding = results.at(j);
                        if (match.title == binding.value("title").literal().toString().trimmed()) {
                            QString actor = binding.value("actor").literal().toString().trimmed();
                            if (actors.indexOf(actor) == -1) {
                                actors.append(actor);
                            }
                            QString director = binding.value("director").literal().toString().trimmed();
                            if (directors.indexOf(director) == -1) {
                                directors.append(director);
                            }
                            QString writer = binding.value("writer").literal().toString().trimmed();
                            if (writers.indexOf(writer) == -1) {
                                writers.append(writer);
                            }
                            QString producer = binding.value("producer").literal().toString().trimmed();
                            if (producers.indexOf(producer) == -1) {
                                producers.append(producer);
                            }
                        }
                    }
                    if (!actors.isEmpty()) {
                        match.fields["actor"] = actors;
                    }
                    if (!directors.isEmpty()) {
                        match.fields["director"] = directors;
                    }
                    if (!writers.isEmpty()) {
                        match.fields["writer"] = writers;
                    }
                    if (!producers.isEmpty()) {
                        match.fields["producer"] = producers;
                    }
                    m_fetchedMatches.append(match);
                }

            }
            if (!m_fetchedMatches.isEmpty()) {
                emit infoFetched(m_fetchedMatches);
            }

        }
    }
    if (m_requestKeys.count() == 0) {
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
        }
    } else if (!m_timeout){
        m_timer->start(m_timeoutLength);
    }
}


void DBPediaInfoFetcher::gotThumbnail(const KUrl &from, const KUrl &to)
{
    //Find item corresponding to fetched thumbnail
    QString requestKey = m_thumbnailKeys.key(from.prettyUrl());
    QString subType = requestKey.left(requestKey.indexOf(":"));
    QString title = requestKey.mid(requestKey.indexOf(":") + 1);
    MediaItem match;
    int foundIndex = -1;
    for (int i = 0; i < m_fetchedMatches.count(); i++) {
        MediaItem item = m_fetchedMatches.at(i);
        if (item.subType() == subType && item.title == title) {
            match = item;
            foundIndex = i;
            break;
        }
    }

    //Update artwork
    if (foundIndex != -1) {
        QString thumbnailFile = to.path();
        QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!thumbnail.isNull()) {
            match.artwork = QIcon(thumbnail);
            match.fields["artworkUrl"] = to.prettyUrl();
            match.hasCustomArtwork = true;
            m_fetchedMatches.replace(foundIndex, match);
            emit updateFetchedInfo(foundIndex, match);
        }
    }
}
