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
#include "../utilities/utilities.h"
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <KDebug>
#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Solid/Networking>
#include <platform/utilities/general.h>

DBPediaInfoFetcher::DBPediaInfoFetcher(QObject * parent) : InfoFetcher(parent)
{
    m_name = i18n("DBPedia");
    m_icon = KIcon("bangarang-dbpedia");
    m_url = KUrl("http://dbpedia.org");
    m_about = i18n("This fetcher gets information from DBPedia.org.");

    m_dbPediaQuery = new DBPediaQuery(this);
    connect (m_dbPediaQuery, SIGNAL(gotArtistInfo(bool,QList<Soprano::BindingSet>,QString)), this, SLOT(gotPersonInfo(bool,QList<Soprano::BindingSet>,QString)));
    connect (m_dbPediaQuery, SIGNAL(gotActorInfo(bool,QList<Soprano::BindingSet>,QString)), this, SLOT(gotPersonInfo(bool,QList<Soprano::BindingSet>,QString)));
    connect (m_dbPediaQuery, SIGNAL(gotDirectorInfo(bool,QList<Soprano::BindingSet>,QString)), this, SLOT(gotPersonInfo(bool,QList<Soprano::BindingSet>,QString)));
    connect (m_dbPediaQuery, SIGNAL(gotMovieInfo(bool,QList<Soprano::BindingSet>,QString)), this, SLOT(gotMovieInfo(bool,QList<Soprano::BindingSet>,QString)));

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
    /*
     * We check if the network is connected. As some distributions use ifup to set up their connection
     * Solid::Networking may return Unknow, as it only checks NetworkManager, not ifup.
     * A real check to the dbpedia server would be overkill and take too long here.
     * In general this fetcher should not be available if the dbpedia server cannot be reached.
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

void DBPediaInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    Q_UNUSED(maxMatches);
    m_updateRequiredFields = updateRequiredFields;
    m_mediaList.clear();
    m_requestKeys.clear();
    m_thumbnailKeys.clear();
    m_fetchedMatches.clear();
    m_indexesWithThumbnail.clear();
    m_updateArtwork = updateArtwork;

    //Build request keys (basically the search string for DBPediaQuery)
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        QString keyPrefix;
        QString keySuffix;
        if (mediaItem.subType() == "Artist") {
            keyPrefix = "Artist:";
            keySuffix = mediaItem.fields["title"].toString().remove("'");
        }
        if (mediaItem.subType() == "Actor") {
            keyPrefix = "Actor:";
            keySuffix = mediaItem.fields["title"].toString().remove("'");
        }
        if (mediaItem.subType() == "Director") {
            keyPrefix = "Director:";
            keySuffix = mediaItem.fields["title"].toString().remove("'");
        }
        if (mediaItem.subType() == "Movie") {
            keyPrefix = "Movie:";
            mediaItem.title = Utilities::titleForRequest(mediaItem.title);
            keySuffix = mediaItem.title;
            keySuffix.remove("'");
        }
        if (!keyPrefix.isEmpty()) {
            m_mediaList.append(mediaItem);
            int keyIndex = m_requestKeys.lastIndexOf(QRegExp(QString("%1*").arg(keyPrefix)
                                                         ,Qt::CaseSensitive,
                                                         QRegExp::Wildcard));
            if (keyIndex == -1) {
                QString requestKey = QString("%1'%2'").arg(keyPrefix).arg(keySuffix);
                m_requestKeys.append(requestKey);
            } else {
                QString requestKey = m_requestKeys.at(keyIndex);
                requestKey.append(QString(" OR '%1'").arg(keySuffix));
                m_requestKeys.replace(keyIndex, requestKey);
            }
        }
    }

    //Launch requests
    if (!m_requestKeys.isEmpty()) {
        for (int i = 0; i < m_requestKeys.count(); i++) {
            if (m_requestKeys.at(i).startsWith("Artist:")) {
                setFetching();
                m_dbPediaQuery->getArtistInfo(m_requestKeys.at(i).mid(7));
            }
            if (m_requestKeys.at(i).startsWith("Actor:")) {
                setFetching();
                m_dbPediaQuery->getActorInfo(m_requestKeys.at(i).mid(6));
            }
            if (m_requestKeys.at(i).startsWith("Director:")) {
                setFetching();
                m_dbPediaQuery->getDirectorInfo(m_requestKeys.at(i).mid(9));
            }
            if (m_requestKeys.at(i).startsWith("Movie:")) {
                setFetching();
                m_dbPediaQuery->getMovieInfo(m_requestKeys.at(i).mid(6));
            }
        }
    }
}

void DBPediaInfoFetcher::gotPersonInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
{
    m_requestKeys.removeAll(requestKey);
    if (!successful || m_timeout) {
        checkComplete();
        emit noResults(this);
        return;
    }

    if (results.count() == 0) {
        checkComplete();
        emit noResults(this);
        return;
    }

    QString subType = requestKey.left(requestKey.indexOf(":"));
    QString lastThumbnailUrl;
    for (int i = 0; i < results.count(); i++) {
        Soprano::BindingSet binding = results.at(i);
        QString name = binding.value("name").literal().toString().trimmed();

        //Find item corresponding to info
        MediaItem mediaItem;
        int foundIndex = -1;
        for (int j = 0; j < m_mediaList.count(); j++) {
            MediaItem item = m_mediaList.at(j);
            if (item.subType() == subType && name.contains(item.fields["title"].toString())) {
                mediaItem = item;
                foundIndex = j;
                break;
            }
        }

        if (foundIndex == -1) {
            continue;
        }

        //Create new match item based on result and add to matches
        QList<MediaItem> matches = m_fetchedMatches[foundIndex];
        if (!name.isEmpty()) {
            MediaItem match = mediaItem;

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
            //Get Thumbnail
            if (m_updateArtwork) {
                QString thumbnailUrlString = binding.value("thumbnail").uri().toString();
                KUrl thumbnailUrl = KUrl(thumbnailUrlString);
                if (thumbnailUrl.isValid() && thumbnailUrlString != lastThumbnailUrl) {
                    QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-%2-%3")
                                                  .arg(subType)
                                                  .arg(name)
                                                  .arg(thumbnailUrl.fileName());
                    KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
                    QFile downloadTarget(thumbnailTargetUrl.path());
                    downloadTarget.remove();
                    m_thumbnailKeys[QString("%1,%2").arg(foundIndex).arg(matches.count())] = thumbnailUrl.prettyUrl();
                    emit download(thumbnailUrl, thumbnailTargetUrl);
                    lastThumbnailUrl = thumbnailUrlString;
                }
            }
            match = Utilities::makeSubtitle(match);

            matches.append(match);
        }
        m_fetchedMatches[foundIndex] = matches;
    }
}

void DBPediaInfoFetcher::gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
{
    m_requestKeys.removeAll(requestKey);
    if (!successful || m_timeout) {
        checkComplete();
        emit noResults(this);
        return;
    }

    if (results.count() == 0) {
        checkComplete();
        emit noResults(this);
        return;
    }

    //Process results
    QString subType = requestKey.left(requestKey.indexOf(":"));
    QString lastThumbnailUrl;
    for (int i = 0; i < results.count(); i++) {
        Soprano::BindingSet binding = results.at(i);
        QString title = binding.value("title").literal().toString().trimmed();

        //Find item corresponding to info
        MediaItem mediaItem;
        int foundIndex = -1;
        for (int j = 0; j < m_mediaList.count(); j++) {
            MediaItem item = m_mediaList.at(j);
            if (item.subType() == subType && title.contains(item.title)) {
                mediaItem = item;
                foundIndex = j;
                break;
            }
        }

        if (foundIndex == -1) {
            continue;
        }

        //Create new match item based on result and add to matches
        QList<MediaItem> matches = m_fetchedMatches[foundIndex];
        if (!title.isEmpty()) {
            //Find item in matches
            int matchIndex = -1;
            for (int j = 0; j < matches.count(); j++) {
                MediaItem matchItem = matches.at(j);
                if (matchItem.title == title) {
                    matchIndex = j;
                    break;
                }
            }
            MediaItem match = mediaItem;
            if (matchIndex != -1) {
                match = matches.at(matchIndex);
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
                match.duration = Utilities::durationString(duration);
                match.fields["duration"] = duration;
            }
            //Set releaseDate
            QDate releaseDate = binding.value("releaseDate").literal().toDate();
            if (releaseDate.isValid()) {
                match.fields["releaseDate"] = releaseDate;
                match.fields["year"] = releaseDate.year();
            }

            //Set Actors
            QStringList actors = match.fields["actor"].toStringList();
            QString actor = binding.value("actor").literal().toString().trimmed();
            if (actors.indexOf(actor) == -1 && !actor.isEmpty()) {
                actors.append(actor);
            }
            match.fields["actor"] = actors;

            //Set Directors
            QStringList directors = match.fields["director"].toStringList();
            QString director = binding.value("director").literal().toString().trimmed();
            if (directors.indexOf(director) == -1 && !director.isEmpty()) {
                directors.append(director);
            }
            match.fields["director"] = directors;

            //Set Writers
            QStringList writers = match.fields["writer"].toStringList();
            QString writer = binding.value("writer").literal().toString().trimmed();
            if (writers.indexOf(writer) == -1) {
                writers.append(writer);
            }
            match.fields["writer"] = writers;

            //Set Producers
            QStringList producers = match.fields["producer"].toStringList();
            QString producer = binding.value("producer").literal().toString().trimmed();
            if (producers.indexOf(producer) == -1) {
                producers.append(producer);
            }
            match.fields["producer"] = producers;

            //Get Thumbnail
            if (m_updateArtwork) {
                QString thumbnailUrlString = binding.value("thumbnail").uri().toString();
                thumbnailUrlString.replace("/wikipedia/commons/", "/wikipedia/en/"); //Wikipedia appears to be storing posters here instead
                KUrl thumbnailUrl = KUrl(thumbnailUrlString);
                if (thumbnailUrl.isValid() && thumbnailUrlString != lastThumbnailUrl &&
                    !m_timeout) {
                    lastThumbnailUrl = thumbnailUrlString;
                    if (m_indexesWithThumbnail.indexOf(foundIndex) == -1) {
                        m_indexesWithThumbnail.append(foundIndex);
                    }
                    if (matchIndex == -1) {
                        m_thumbnailKeys[QString("%1,%2").arg(foundIndex).arg(matches.count())] = thumbnailUrl.prettyUrl();
                    } else {
                        m_thumbnailKeys[QString("%1,%2").arg(foundIndex).arg(matchIndex)] = thumbnailUrl.prettyUrl();
                    }
                    QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-%2-%3")
                                                  .arg(subType)
                                                  .arg(title)
                                                  .arg(thumbnailUrl.fileName());
                    KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
                    QFile downloadTarget(thumbnailTargetUrl.path());
                    downloadTarget.remove();
                    m_timer->start(2000*m_indexesWithThumbnail.count());  // additional 2 seconds to get thumbnail
                    emit download(thumbnailUrl, thumbnailTargetUrl);
                }
            }
            match = Utilities::makeSubtitle(match);

            if (matchIndex == -1) {
                matches.append(match);
            } else {
                matches.replace(matchIndex, match);
            }
        }
        m_fetchedMatches[foundIndex] = matches;

        if (m_timeout) {
            break;
        }
    }

    //For matches with no thumbnails return fetched info
    QList<int> indexes = m_fetchedMatches.keys();
    for (int i = 0; i < indexes.count(); i++) {
        int foundIndex = indexes.at(i);
        if (m_indexesWithThumbnail.indexOf(foundIndex) == -1) {
            QList<MediaItem> matches = m_fetchedMatches.take(foundIndex);
            emit infoFetched(matches);
        }
    }
}


void DBPediaInfoFetcher::gotThumbnail(const KUrl &from, const KUrl &to)
{
    QString requestKey = m_thumbnailKeys.key(from.prettyUrl());
    m_thumbnailKeys.remove(requestKey);
    if (requestKey.isEmpty()) {
        return;
    }

    int foundIndex = requestKey.split(",").at(0).toInt();
    int matchesIndex = requestKey.split(",").at(1).toInt();

    if (!m_fetchedMatches.contains(foundIndex)) {
        return;
    }

    QList<MediaItem> matches = m_fetchedMatches[foundIndex];
    MediaItem match = matches.at(matchesIndex);
    QString thumbnailFile = to.path();
    QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!thumbnail.isNull()) {
        match.artwork = QIcon(thumbnail);
        match.fields["artworkUrl"] = to.prettyUrl();
        match.hasCustomArtwork = true;
        matches.replace(matchesIndex, match);
        m_fetchedMatches[foundIndex] = matches;
    }

    //When all thumbnails are retrieved return fetched info
    if (allThumbnailsFetchedForIndex(foundIndex)) {
        QList<MediaItem> matches = m_fetchedMatches.take(foundIndex);
        emit infoFetched(matches);
    }
    if (m_thumbnailKeys.isEmpty()) {
        checkComplete();
    }

}

bool DBPediaInfoFetcher::allThumbnailsFetchedForIndex(int index)
{
    bool thumbnailsFetched = true;
    foreach(QString requestKey, m_thumbnailKeys) {
        int foundIndex = requestKey.split(",").at(0).toInt();
        if (foundIndex == index) {
            thumbnailsFetched = false;
            break;
        }
    }
    return thumbnailsFetched;
}

void DBPediaInfoFetcher::checkComplete()
{
    if (m_requestKeys.count() == 0) {
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
            emit fetchComplete(this);
        }
    }
}

void DBPediaInfoFetcher::timeout()
{
    //Return any remaining matches that have not yet been returned
    if (!m_fetchedMatches.isEmpty()) {
        foreach (QList<MediaItem> matches, m_fetchedMatches) {
            emit infoFetched(matches);
        }
    }

    InfoFetcher::timeout();
}
