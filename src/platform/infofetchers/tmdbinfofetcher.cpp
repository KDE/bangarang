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

#include "tmdbinfofetcher.h"
#include "../downloader.h"
#include "../mediaitemmodel.h"
#include "../utilities/utilities.h"

#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <Solid/Networking>

#include <QDomDocument>
#include <QFile>

TMDBInfoFetcher::TMDBInfoFetcher(QObject *parent) :
        InfoFetcher(parent)
{
    m_name = i18n("TMDb");
    m_icon = KIcon("bangarang-tmdb");
    m_url = KUrl("http://themoviedb.org");
    m_about = i18n("Note: This fetcher uses the TMDb API but is not endorsed or certified by TMDb.");

    QString lang = KGlobal::locale()->language();
    if (lang.size() > 2) {
        lang = lang.left(2);
    }

    //NOTE: The API key below must be used only in Bangarang.  Please do not use this key in other applications.
    //      API keys can be requested from themoviedb.org.
    m_apiKey = "efa7a3197ca2ab2b9af306580c42075c";
    m_movieSearchAPI = QString("http://api.themoviedb.org/2.1/Movie.search/%1/xml/%2/%3").arg(lang).arg(m_apiKey).arg("%1");
    m_movieInfoAPI = QString("http://api.themoviedb.org/2.1/Movie.getInfo/%1/xml/%2/%3").arg(lang).arg(m_apiKey).arg("%1");
    m_personSearchAPI = QString("http://api.themoviedb.org/2.1/Person.search/%1/xml/%2/%3").arg(lang).arg(m_apiKey).arg("%1");
    m_timeout = 20000;

    m_downloader = new Downloader(this);
    connect(this, SIGNAL(download(KUrl,KUrl)), m_downloader, SLOT(download(KUrl,KUrl)));
    connect(m_downloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(gotTMDBInfo(KUrl,KUrl)));

    //Define fetchable fields
    m_fetchableFields["Movie"] = QStringList() << "artwork" << "title" << "description" << "actor" << "director" << "writer" << "audienceRating";
    m_fetchableFields["Actor"] = QStringList() << "artwork" << "title" << "description";
    m_fetchableFields["Director"] = QStringList() << "artwork" << "title" << "description";
    m_fetchableFields["Writer"] = QStringList() << "artwork" << "title" << "description";
    m_fetchableFields["Producer"] = QStringList() << "artwork" << "title" << "description";

    //Define required fields
    m_requiredFields["Movie"] = QStringList() << "title";
    m_requiredFields["Actor"] = QStringList() << "title";
    m_requiredFields["Director"] = QStringList() << "title";
    m_requiredFields["Writer"] = QStringList() << "title";
    m_requiredFields["Producer"] = QStringList() << "title";

    m_lastRequestTime = QDateTime::currentDateTime();

}

bool TMDBInfoFetcher::available(const QString &subType)
{
    /*
     * We check if the network is connected. As some distributions use ifup to set up their connection
     * Solid::Networking may return Unknown, as it only checks NetworkManager, not ifup.
     * A real check to the TMDb server would be overkill and take too long here.
     * In general this fetcher should not be available if the TMDb server cannot be reached.
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

void TMDBInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    //Wait 5 seconds before processing next request
    //NOTE: This is intended to prevent abuse of the tmdb api during autofetches.
    while (m_lastRequestTime.secsTo(QDateTime::currentDateTime()) < 5) {
        sleep(1);
    }

    m_updateRequiredFields = updateRequiredFields;
    m_maxMatches = maxMatches;
    m_mediaList.clear();
    m_requestKeys.clear();
    m_fetchedMatches.clear();
    m_thumbnailKeys.clear();
    m_moreInfoKeys.clear();
    m_updateArtwork = updateArtwork;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        QString searchTerm;
        QString TMDBUrlStr;
        if (mediaItem.subType() == "Movie") {
            m_requestType = MovieRequest;
            searchTerm = Utilities::titleForRequest(mediaItem.fields["title"].toString());
            TMDBUrlStr = m_movieSearchAPI.arg(searchTerm);
        } else if (mediaItem.subType() == "Actor" ||
                   mediaItem.subType() == "Writer" ||
                   mediaItem.subType() == "Director" ||
                   mediaItem.subType() == "Producer") {
            m_requestType = PersonRequest;
            searchTerm = mediaItem.fields["title"].toString();
            TMDBUrlStr = m_personSearchAPI.arg(searchTerm);
        }

        KUrl TMDBUrl(TMDBUrlStr);
        if (!TMDBUrl.isEmpty()) {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(TMDBUrl.prettyUrl());

            //Retrieve TMDB info
            QString TMDBTargetFile = QString("bangarang/temp/TMDB%1.xml").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzzz"));
            KUrl TMDBTargetUrl = KUrl(KStandardDirs::locateLocal("data", TMDBTargetFile, true));
            QFile TMDBTarget(TMDBTargetUrl.path());
            TMDBTarget.remove();
            setFetching();
            emit download(TMDBUrl, TMDBTargetUrl);
        }
    }
}

void TMDBInfoFetcher::gotTMDBInfo(const KUrl &from, const KUrl &to)
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

void TMDBInfoFetcher::processOriginalRequest(const KUrl &from, const KUrl to)
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
    QDomDocument TMDBDoc("TMDB");
    TMDBDoc.setContent(&file);

    QHash<int, KUrl> thumbnailUrls;
    QHash<int, KUrl> moreInfoUrls;
    QList<MediaItem> fetchedMatches;

    //Read Movie results
    if (m_requestType == MovieRequest) {
        //Iterate through item nodes of the TMDB XML document
        QDomNodeList movies = TMDBDoc.elementsByTagName("movie");
        for (int i = 0; i < qMin(movies.count(), m_maxMatches); i++) {
            MediaItem match = mediaItem;

            //Clear artwork
            match.artwork = Utilities::defaultArtworkForMediaItem(match);
            match.fields["artworkUrl"] = QString("");

            QDomNodeList nodes = movies.at(i).childNodes();
            bool gotThumbnailUrl = false;
            for (int j = 0; j < nodes.count(); j++) {
                if (!nodes.at(j).isElement()) {
                    continue;
                }
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "name") {
                    match.title = element.text();
                    match.fields["title"] = match.title;
                } else if (element.tagName() == "overview") {
                    match.fields["description"] = element.text();
                } else if (element.tagName() == "released") {
                    QDate releaseDate = QDate::fromString(element.text(), "yyyy-MM-dd");
                    if (releaseDate.isValid()) {
                        match.fields["releaseDate"] = releaseDate;
                        match.fields["year"] = releaseDate.year();
                    }
                } else if (element.tagName() == "url") {
                    match.fields["relatedTo"] = QStringList(element.text());
                } else if (element.tagName() == "certification") {
                    match.fields["audienceRating"] = element.text();
                } else if (element.tagName() == "id") {
                    QString id = element.text();
                    match.fields["TMDBID"] = id;
                    QString movieInfoUrlStr = m_movieInfoAPI.arg(id);
                    moreInfoUrls.insert(fetchedMatches.count(), movieInfoUrlStr);
                } else if (element.tagName() == "images") {
                    QDomNodeList imageNodes = nodes.at(j).childNodes();
                    for (int k = 0; k < imageNodes.count(); k++) {
                        QDomElement imageElement = imageNodes.at(k).toElement();
                        if (!(imageElement.tagName() == "image" &&
                              imageElement.attribute("type") == "poster" &&
                              imageElement.attribute("size") == "cover")) {
                            continue;
                        }
                        if (gotThumbnailUrl || !m_updateArtwork) {
                            continue;
                        }
                        QString imageUrl = imageElement.attribute("url");
                        KUrl thumbnailUrl = KUrl(imageUrl);
                        if (thumbnailUrl.isValid()) {
                            thumbnailUrls.insert(fetchedMatches.count(), thumbnailUrl);
                            gotThumbnailUrl = true;
                            break;
                        }
                    }
                }
            }
            fetchedMatches.append(match);
        }
    }

    //Read Person results
    if (m_requestType == PersonRequest) {
        //Iterate through item nodes of the TMDB XML document
        QDomNodeList movies = TMDBDoc.elementsByTagName("person");
        for (int i = 0; i < movies.count(); i++) {
            MediaItem match = mediaItem;

            //Clear artwork
            match.artwork = Utilities::defaultArtworkForMediaItem(match);
            match.fields["artworkUrl"] = QString("");

            QDomNodeList nodes = movies.at(i).childNodes();
            bool gotThumbnailUrl = false;
            for (int j = 0; j < nodes.count(); j++) {
                if (!nodes.at(j).isElement()) {
                    continue;
                }
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "name") {
                    match.title = element.text();
                    match.fields["title"] = match.title;
                } else if (element.tagName() == "biography") {
                    match.fields["description"] = element.text();
                } else if (element.tagName() == "url") {
                    match.fields["relatedTo"] = QStringList(element.text());
                } else if (element.tagName() == "images") {
                    QDomNodeList imageNodes = nodes.at(j).childNodes();
                    for (int k = 0; k < imageNodes.count(); k++) {
                        QDomElement imageElement = imageNodes.at(k).toElement();
                        if (!(imageElement.tagName() == "image" &&
                              imageElement.attribute("type") == "profile" &&
                              imageElement.attribute("size") == "profile")) {
                            continue;
                        }
                        if (gotThumbnailUrl || !m_updateArtwork) {
                            continue;
                        }
                        QString imageUrl = imageElement.attribute("url");
                        KUrl thumbnailUrl = KUrl(imageUrl);
                        if (thumbnailUrl.isValid()) {
                            thumbnailUrls.insert(fetchedMatches.count(), thumbnailUrl);
                            gotThumbnailUrl = true;
                            break;
                        }
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
        QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-TMDB-%2-%3")
                                      .arg(match.subType())
                                      .arg(match.fields["TMDBID"].toString())
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
        QString moreInfoTargetFile = QString("bangarang/temp/TMDBInfo-%1-%2.xml")
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

void TMDBInfoFetcher::processThumbnails(const KUrl &from, const KUrl to)
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

void TMDBInfoFetcher::processMoreInfo(const KUrl &from, const KUrl to)
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
    QDomDocument TMDBDoc("TMDBInfo");
    TMDBDoc.setContent(&file);

    //Iterate through item nodes of the TMDB XML document
    QDomNodeList movies = TMDBDoc.elementsByTagName("movie");
    if (movies.count() == 0) {
        return;
    }

    QDomNodeList nodes = movies.at(0).childNodes();
    for (int j = 0; j < nodes.count(); j++) {
        if (!nodes.at(j).isElement()) {
            continue;
        }
        QDomElement element = nodes.at(j).toElement();
        if (element.tagName() == "runtime") {
            int duration = element.text().toInt()*60; //tmdb duration is in minutes
            match.fields["duration"] = duration;
            match.duration = Utilities::durationString(duration);
        } else if (element.tagName() == "categories") {
            QDomNodeList genreNodes = nodes.at(j).childNodes();
            QStringList genres;
            for (int k = 0; k < genreNodes.count(); k++) {
                QDomElement genreElement = genreNodes.at(k).toElement();
                if (!(genreElement.tagName() == "category" && genreElement.attribute("type").toLower() == "genre")) {
                    continue;
                }
                QString genre = genreElement.attribute("name").trimmed();
                if (!genre.isEmpty()) {
                    genres.append(genre);
                }
            }
            match.fields["genre"] = genres;
        }  else if (element.tagName() == "cast") {
            QDomNodeList castNodes = nodes.at(j).childNodes();
            QStringList actors;
            QStringList directors;
            QStringList writers;
            QStringList producers;
            for (int k = 0; k < castNodes.count(); k++) {
                QDomElement castElement = castNodes.at(k).toElement();
                QString job = castElement.attribute("job").toLower();
                if (!(castElement.tagName() == "person" &&
                      (job == "actor" ||
                       job == "screenplay" ||
                       job == "writer" ||
                       job == "director" ||
                       job == "executive producer" ||
                       job == "producer"))) {
                    continue;
                }
                if (job == "actor") {
                    QString actor = castElement.attribute("name");
                    if (!actor.isEmpty()) {
                        actors.append(actor);
                    }
                }
                if (job == "screenplay" || job == "writer") {
                    QString writer = castElement.attribute("name");
                    if (!writer.isEmpty()) {
                        writers.append(writer);
                    }
                }
                if (job == "director") {
                    QString director = castElement.attribute("name");
                    if (!director.isEmpty()) {
                        directors.append(director);
                    }
                }
                if (job == "execute producer" || job == "producer") {
                    QString producer = castElement.attribute("name");
                    if (!producer.isEmpty()) {
                        producers.append(producer);
                    }
                }
            }
            match.fields["actor"] = actors;
            match.fields["director"] = directors;
            match.fields["writer"] = writers;
            match.fields["producer"] = producers;
        }
    }
    QFile(to.path()).remove();

    matches.replace(matchesIndex, match);
    m_fetchedMatches[originalRequestIndex] = matches;
}

void TMDBInfoFetcher::timeout()
{
    //Return any remaining matches that have not yet been returned
    if (!m_fetchedMatches.isEmpty()) {
        foreach (QList<MediaItem> matches, m_fetchedMatches) {
            emit infoFetched(matches);
        }
    }

    InfoFetcher::timeout();
}
