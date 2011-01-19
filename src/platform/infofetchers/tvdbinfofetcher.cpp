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

#include "tvdbinfofetcher.h"
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

TVDBInfoFetcher::TVDBInfoFetcher(QObject *parent) :
        InfoFetcher(parent)
{
    m_name = i18n("TVDB");
    m_icon = KIcon("bangarang-tvdb");
    m_url = KUrl("http://thetvdb.com");
    m_about = i18n("Note: This fetcher uses theTVDB.com API. Please help improve theTVDB.com information by clicking the link.");

    QString lang = KGlobal::locale()->language();
    if (lang.size() > 2) {
        lang = lang.left(2);
    }

    //NOTE: The API key below must be used only in Bangarang.  Please do not use this key in other applications.
    //      API keys can be requested from thetvdb.org.
    m_apiKey = "60DD172338AFC0A1";
    m_seriesSearchAPI = QString("http://www.thetvdb.com/api/GetSeries.php?seriesname=%2&language=%1").arg(lang).arg("%1");
    m_seriesInfoAPI = QString("%3/api/%1/series/%4/all/%2.xml").arg(m_apiKey).arg(lang).arg("%1").arg("%2");
    m_updateRequestAPI = QString("http://www.thetvdb.com/api/Updates.php?type=series&time=%1");
    m_mirrorRequestAPI = QString("http://www.thetvdb.com/api/%1/mirrors.xml").arg(m_apiKey);
    m_serverTimeRequestAPI = QString("http://www.thetvdb.com/api/Updates.php?type=none");
    m_timeout = 20000;

    m_downloader = new Downloader(this);
    connect(this, SIGNAL(download(KUrl,KUrl)), m_downloader, SLOT(download(KUrl,KUrl)));
    connect(m_downloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(gotTVDBInfo(KUrl,KUrl)));

    //Define fetchable fields
    m_fetchableFields["TV Series"] = QStringList() << "artwork" << "title" << "description" << "actor" << "director" << "writer" << "audienceRating";
    m_fetchableFields["TV Show"] = QStringList() << "artwork" << "title" << "description";

    //Define required fields
    m_requiredFields["TV Series"] = QStringList() << "title";
    m_requiredFields["TV Show"] = QStringList() << "seriesName" << "season" << "episodeNumber";

    m_lastRequestTime = QDateTime::currentDateTime();
    m_mirror = "http://thetvdb.com";

}

bool TVDBInfoFetcher::available(const QString &subType)
{
    /*
     * We check if the network is connected. As some distributions use ifup to set up their connection
     * Solid::Networking may return Unknown, as it only checks NetworkManager, not ifup.
     * A real check to the TVDB server would be overkill and take too long here.
     * In general this fetcher should not be available if the TVDB server cannot be reached.
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

void TVDBInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    //Wait 5 seconds before processing next request
    //NOTE: This is intended to prevent abuse of the TVDB api during autofetches.
    while (m_lastRequestTime.secsTo(QDateTime::currentDateTime()) < 5) {
        sleep(1);
    }

    m_updateRequiredFields = updateRequiredFields;
    m_maxMatches = maxMatches;
    m_mediaList.clear();
    m_requestKeys.clear();
    m_fetchedMatches.clear();
    m_seriesInfoKeys.clear();;
    m_thumbnailKeys.clear();
    m_updateArtwork = updateArtwork;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        QString searchTerm;
        QString TVDBUrlStr;
        if (mediaItem.subType() == "TV Series") {
            m_requestType = SeriesRequest;
            searchTerm = Utilities::titleForRequest(mediaItem.fields["title"].toString());
            TVDBUrlStr = m_seriesSearchAPI.arg(searchTerm);
        } else if (mediaItem.subType() == "TV Show") {
            m_requestType = EpisodeRequest;
            searchTerm = Utilities::titleForRequest(mediaItem.fields["seriesName"].toString());
            TVDBUrlStr = m_seriesSearchAPI.arg(searchTerm);
        }

        KUrl TVDBUrl(TVDBUrlStr);
        if (!TVDBUrl.isEmpty()) {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(TVDBUrl.prettyUrl());

            //Retrieve TVDB info
            QString TVDBTargetFile = QString("bangarang/temp/tvdb%1.xml").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzzz"));
            KUrl TVDBTargetUrl = KUrl(KStandardDirs::locateLocal("data", TVDBTargetFile, true));
            QFile TVDBTarget(TVDBTargetUrl.path());
            TVDBTarget.remove();
            setFetching();
            emit download(TVDBUrl, TVDBTargetUrl);
        }
    }
}

void TVDBInfoFetcher::gotTVDBInfo(const KUrl &from, const KUrl &to)
{
    m_lastRequestTime = QDateTime::currentDateTime();

    //Process original requests
    processOriginalRequest(from, to);

    //Process original requests
    processSeriesInfoRequest(from, to);

    //Process thumbnails
    processThumbnails(from, to);

    //Determine if all info for each match set is fetched
    for (int i = 0; i < m_requestKeys.count(); i++) {
        //Check if original request is done
        bool originalRequestDone = (m_requestKeys.at(i) == "done");
        if (!originalRequestDone) {
            continue;
        }

        //Check if more info requests for this request are done
        int seriesInfosRequested = m_seriesInfoKeys.keys().count();
        QStringList seriesInfoValues =  m_seriesInfoKeys.values();
        int seriesInfosRetrieved = seriesInfoValues.filter(QRegExp("^done")).count();
        bool seriesInfosDone = (seriesInfosRetrieved == seriesInfosRequested) ? true : false;
        if (!seriesInfosDone) {
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

void TVDBInfoFetcher::processOriginalRequest(const KUrl &from, const KUrl to)
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
    QDomDocument TVDBDoc("TVDB");
    TVDBDoc.setContent(&file);

    QHash<int, KUrl> seriesInfoUrls;
    QList<MediaItem> fetchedMatches;

    //Read Series Search results
    //Iterate through item nodes of the TVDB XML document
    QDomNodeList series = TVDBDoc.elementsByTagName("Series");
    for (int i = 0; i < qMin(series.count(), m_maxMatches); i++) {
        MediaItem match = mediaItem;

        //Clear artwork
        match.artwork = Utilities::defaultArtworkForMediaItem(match);
        match.fields["artworkUrl"] = QString("");

        QDomNodeList nodes = series.at(i).childNodes();
        for (int j = 0; j < nodes.count(); j++) {
            if (!nodes.at(j).isElement()) {
                continue;
            }
            QDomElement element = nodes.at(j).toElement();
            if (element.tagName() == "SeriesName") {
                match.title = element.text();
                match.fields["title"] = element.text();
            } else if (element.tagName() == "seriesid") {
                QString id = element.text();
                match.fields["TVDBID"] = id;
                QString seriesInfoUrlStr = m_seriesInfoAPI.arg(m_mirror).arg(id);
                seriesInfoUrls.insert(fetchedMatches.count(), seriesInfoUrlStr);
            }
        }
        fetchedMatches.append(match);
    }
    QFile(to.path()).remove();

    //Add fetched matches to collection of fetched matches
    m_fetchedMatches.insert(originalRequestIndex, fetchedMatches);

    //Launch series info request
    for (int i = 0; i < seriesInfoUrls.keys().count(); i++) {
        int matchIndex = seriesInfoUrls.keys().at(i);
        KUrl seriesInfoUrl = seriesInfoUrls.value(matchIndex);
        QString key = QString("%1,%2").arg(originalRequestIndex).arg(matchIndex);
        m_seriesInfoKeys.insert(key,seriesInfoUrl.prettyUrl());
        QString seriesInfoTargetFile = QString("bangarang/temp/TVDBInfo-%1-%2.xml")
                                     .arg(key)
                                     .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        KUrl seriesInfoTargetUrl = KUrl(KStandardDirs::locateLocal("data", seriesInfoTargetFile, true));
        QFile seriesInfoTarget(seriesInfoTargetUrl.path());
        seriesInfoTarget.remove();
        m_timer->start(m_timeoutLength);
        emit download(seriesInfoUrl, seriesInfoTargetUrl);
    }

    //Mark request done
    m_requestKeys.replace(originalRequestIndex, "done");
}

void TVDBInfoFetcher::processSeriesInfoRequest(const KUrl &from, const KUrl to)
{
    kDebug() << "GOT SERIES INFO";
    //Determine we got a series info request
    QString seriesInfoKey = m_seriesInfoKeys.key(from.prettyUrl());
    bool isSeriesInfoRequest = !seriesInfoKey.isEmpty();

    if (!isSeriesInfoRequest) {
        return;
    }

    //Mark series info request done
    m_seriesInfoKeys.insert(seriesInfoKey, "done");

    //Process results
    int originalRequestIndex = seriesInfoKey.split(",").at(0).toInt();
    int matchesIndex = seriesInfoKey.split(",").at(1).toInt();

    QList<MediaItem> matches = m_fetchedMatches.value(originalRequestIndex);
    MediaItem match = matches.at(matchesIndex);

    QFile file(to.path());
    QDomDocument TVDBDoc("TVDB");
    TVDBDoc.setContent(&file);

    QHash<int, KUrl> thumbnailUrls;
    bool gotThumbnailUrl = false;

    //Read Series Info results
    //Iterate through item nodes of the TVDB XML document
    if (m_requestType == SeriesRequest) {
        QDomNodeList series = TVDBDoc.elementsByTagName("Series");
        if (series.count() == 0) {
            return;
        }

        //Clear artwork
        match.artwork = Utilities::defaultArtworkForMediaItem(match);
        match.fields["artworkUrl"] = QString("");

        QDomNodeList nodes = series.at(0).childNodes();
        for (int j = 0; j < nodes.count(); j++) {
            if (!nodes.at(j).isElement()) {
                continue;
            }
            QDomElement element = nodes.at(j).toElement();
            if (element.tagName() == "SeriesName") {
                match.title = element.text();
                match.fields["title"] = element.text();
            } else if (element.tagName() == "Overview") {
                match.fields["description"] = element.text();
            } else if (element.tagName() == "poster") {
                QString imageUrl = QString("http://thetvdb.com/banners/%1").arg(element.text());
                KUrl thumbnailUrl = KUrl(imageUrl);
                if (thumbnailUrl.isValid()) {
                    thumbnailUrls.insert(matchesIndex, thumbnailUrl);
                    gotThumbnailUrl = true;
                }
            }
        }
        matches.replace(matchesIndex, match);
        m_fetchedMatches[originalRequestIndex] = matches;
    }

    //Read Episode Info results
    //Iterate through item nodes of the TVDB XML document
    if (m_requestType == EpisodeRequest) {
        QDomNodeList series = TVDBDoc.elementsByTagName("Series");
        if (series.count() == 0) {
            return;
        }

        QString seriesArtworkUrl;
        QDomNodeList nodes = series.at(0).childNodes();
        for (int j = 0; j < nodes.count(); j++) {
            if (!nodes.at(j).isElement()) {
                continue;
            }
            QDomElement element = nodes.at(j).toElement();
            if (element.tagName() == "Actors") {
                match.fields["actor"] = element.text().split("|", QString::SkipEmptyParts);
            } else if (element.tagName() == "Genre") {
                match.fields["genre"] = element.text().split("|", QString::SkipEmptyParts);
            } else if (element.tagName() == "poster") {
                seriesArtworkUrl = QString("http://thetvdb.com/banners/%1").arg(element.text());
            }
        }

        QDomNodeList episodes = TVDBDoc.elementsByTagName("Episode");
        if (episodes.count() == 0) {
            return;
        }
        int seasonNumber = match.fields["season"].toInt();
        int episodeNumber = match.fields["episodeNumber"].toInt();
        if (seasonNumber <= 0 || episodeNumber <= 0) {
            return;
        }

        //Clear artwork
        match.artwork = Utilities::defaultArtworkForMediaItem(match);
        match.fields["artworkUrl"] = QString("");

        for (int i = 0; i < episodes.count(); i++) {
            QDomNodeList nodes = episodes.at(i).childNodes();
            bool matchedSeason = false;
            bool matchedEpisode = false;
            for (int j = 0; j < nodes.count(); j++) {
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "SeasonNumber" && element.text().toInt() == seasonNumber) {
                    matchedSeason = true;
                }
                if (element.tagName() == "EpisodeNumber" && element.text().toInt() == episodeNumber) {
                    matchedEpisode = true;
                }
            }
            if (!matchedSeason || !matchedEpisode) {
                continue;
            }
            for (int j = 0; j < nodes.count(); j++) {
                if (!nodes.at(j).isElement()) {
                    continue;
                }
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "EpisodeName") {
                    match.title = element.text();
                    match.fields["title"] = element.text();
                } else if (element.tagName() == "Overview") {
                    match.fields["description"] = element.text();
                } else if (element.tagName() == "Director") {
                    match.fields["director"] = element.text().split("|", QString::SkipEmptyParts);
                } else if (element.tagName() == "Writer") {
                    match.fields["writer"] = element.text().split("|", QString::SkipEmptyParts);
                } else if (element.tagName() == "filename") {
                    QString imageUrl = QString("http://thetvdb.com/banners/%1").arg(element.text());
                    KUrl thumbnailUrl = KUrl(imageUrl);
                    if (thumbnailUrl.isValid()) {
                        thumbnailUrls.insert(matchesIndex, thumbnailUrl);
                        gotThumbnailUrl = true;
                    }
                }
            }
            if (!gotThumbnailUrl) {
                KUrl thumbnailUrl = KUrl(seriesArtworkUrl);
                if (thumbnailUrl.isValid()) {
                    thumbnailUrls.insert(matchesIndex, thumbnailUrl);
                    gotThumbnailUrl = true;
                }
            }
            break;
        }
        matches.replace(matchesIndex, match);
        m_fetchedMatches[originalRequestIndex] = matches;
    }
    QFile(to.path()).remove();

    //Launch Thumbnail requests
    for (int i = 0; i < thumbnailUrls.keys().count(); i++) {
        int matchIndex = thumbnailUrls.keys().at(i);
        MediaItem match = matches.at(matchIndex);
        KUrl thumbnailUrl = thumbnailUrls.value(matchIndex);
        QString key = QString("%1,%2").arg(originalRequestIndex).arg(matchIndex);
        m_thumbnailKeys.insert(key, thumbnailUrl.prettyUrl());
        QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-TVDB-%2")
                                      .arg(match.subType())
                                      .arg(thumbnailUrl.fileName());
        KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
        QFile downloadTarget(thumbnailTargetUrl.path());
        downloadTarget.remove();
        m_timer->start(m_timeoutLength);
        emit download(thumbnailUrl, thumbnailTargetUrl);
    }

}

void TVDBInfoFetcher::processThumbnails(const KUrl &from, const KUrl to)
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

void TVDBInfoFetcher::timeout()
{
    //Return any remaining matches that have not yet been returned
    if (!m_fetchedMatches.isEmpty()) {
        foreach (QList<MediaItem> matches, m_fetchedMatches) {
            emit infoFetched(matches);
        }
    }

    InfoFetcher::timeout();
}
