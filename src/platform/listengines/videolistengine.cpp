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
#include "videolistengine.h"
#include "listenginefactory.h"
#include "../mediavocabulary.h"
#include "../utilities/utilities.h"

#include <Soprano/QueryResultIterator>
#include <KIcon>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <QTime>
#include <nepomuk/variant.h>


VideoListEngine::VideoListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

VideoListEngine::~VideoListEngine()
{
}

void VideoListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
    mediaVocabulary.setVideoVocabulary(MediaVocabulary::nmm);
    
    QString engineArg = m_mediaListProperties.engineArg();

    //Parse filter
    QString engineFilter = m_mediaListProperties.engineFilter();
    QStringList engineFilterList = m_mediaListProperties.engineFilterList();
    QString searchTerm;
    if (engineFilterList.count() > 0) {
        searchTerm = engineFilterList.at(0);
    }
    QString genre = m_mediaListProperties.filterFieldValue("genre");
    QString genreFilter = m_mediaListProperties.filterForField("genre");
    QString seriesName = m_mediaListProperties.filterFieldValue("seriesName");
    QString seriesNameFilter = m_mediaListProperties.filterForField("seriesName");
    int season = m_mediaListProperties.filterFieldValue("season").trimmed().toInt();
    QString seasonFilter = m_mediaListProperties.filterForField("season");
    
    if (m_nepomukInited) {
        
        // Retrieve Movies
        if (engineArg.toLower() == "movies") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            bindings.append(mediaVocabulary.lastPlayedBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoMovie(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(mediaVocabulary.mediaResourceBinding(), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            orderByBindings.append(mediaVocabulary.releaseDateBinding());
            query.orderBy(orderByBindings);

            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            QStringList urls;
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Movie"), m_mediaListProperties.lri);
                if (urls.indexOf(mediaItem.url) == -1) {
                    if (!mediaItem.url.startsWith("nepomuk:/")) {
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaList.append(mediaItem);
                        urls.append(mediaItem.url);
                    }
                } else {
                    //Update multivalue fields for existing media item
                    mediaItem = mediaList.at(urls.indexOf(mediaItem.url));
                    relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                    mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                    mediaList.replace(urls.indexOf(mediaItem.url), mediaItem);
                }
                kDebug() << mediaItem.fields["duration"].toInt();
                kDebug() << mediaItem.duration;
            }
            
            m_mediaListProperties.name = i18n("Movies");
            if (!genre.isEmpty()) {
                m_mediaListProperties.name = i18nc("%1=Genre of the movie", "Movies - %1", genre);
            }
            m_mediaListProperties.summary = i18np("1 movie", "%1 movies", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } 
        
        //Retrieve Video Clips
        if (engineArg.toLower() == "clips") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            bindings.append(mediaVocabulary.lastPlayedBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Video Clip"), m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.name = i18n("Video Clips");
            m_mediaListProperties.summary = i18np("1 clip", "%1 clips", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        } 
        
        //Retrieve TV Series
        if (engineArg.toLower() == "tvshows") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(MediaVocabulary::resourceBindingForCategory("TV Series"));
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            bindings.append(mediaVocabulary.videoSeriesDescriptionBinding());
            bindings.append(mediaVocabulary.videoSeriesArtworkBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeriesDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeriesArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(MediaVocabulary::resourceBindingForCategory("TV Series"), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings(mediaVocabulary.videoSeriesTitleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            QStringList urls;
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString seriesName = it.binding(mediaVocabulary.videoSeriesTitleBinding()).literal().toString();
                if (!seriesName.isEmpty()) {
                    QString lri = QString("video://seasons?||seriesName=%1||%2").arg(seriesName).arg(genreFilter);
                    if (urls.indexOf(lri) == -1) {
                        MediaItem mediaItem;
                        mediaItem.url = lri;
                        mediaItem.title = seriesName;
                        mediaItem.type = QString("Category");
                        mediaItem.fields["categoryType"] = QString("TV Series");
                        mediaItem.fields["title"] = seriesName;
                        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                        mediaItem.fields["description"] = it.binding(mediaVocabulary.videoSeriesDescriptionBinding()).literal().toString().trimmed();
                        mediaItem.fields["artworkUrl"] = it.binding(mediaVocabulary.videoSeriesArtworkBinding()).uri().toString();
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaItem.nowPlaying = false;
                        mediaItem.artwork = KIcon("video-television");

                        //Provide context info for TV series
                        mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
                        mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
                        mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));

                        mediaList.append(mediaItem);
                        urls.append(lri);
                    } else {
                        //Update multivalue fields for existing media item
                        MediaItem mediaItem = mediaList.at(urls.indexOf(lri));
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaList.replace(urls.indexOf(lri), mediaItem);
                    }
                }
            }


            /* Check, whether there are videos which have the TV show flag set,
             * but no series name is entered. If so, add an entry which allows
             * access to those files. Only do so if no category (Genre) is
             * specified.
             */
            if (genre.isEmpty()) {
                MediaQuery query;
                query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
                query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
                query.startFilter();
                query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), QString(), MediaQuery::NotBound);
                query.addFilterOr();
                query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), "^$", MediaQuery::Contains);
                query.endFilter();
                
                if(query.executeAsk(m_mainModel)) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("video://episodes?||seriesName=~");
                    mediaItem.title = i18n("Uncategorized TV Shows");
                    mediaItem.fields["title"] = i18n("Uncategorized TV Shows");
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("Basic+Artwork");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("video-television");
                    mediaList.append(mediaItem);
                }
            }
            
            if(mediaList.count() != 1) {
                m_mediaListProperties.name = i18n("TV Shows");
                m_mediaListProperties.summary = i18np("1 show", "%1 shows", mediaList.count());
                m_mediaListProperties.type = QString("Categories");
            } else {
                //engineArg = "episodes";
                if (mediaList.at(0).url == "video://episodes?||seriesName=~") {
                    seriesName = "~";
                    engineArg = "episodes";
                } else {
                    seriesName = mediaList.at(0).title;
                    engineArg = "seasons";
                }
                seriesNameFilter = QString("seriesName=%1").arg(seriesName);
                mediaList.clear();
            }
        } 
        
        //Retrieve TV Seasons
        if (engineArg.toLower() == "seasons") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            if (seriesName == "~") {
                query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
                query.startFilter();
                query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), QString(), MediaQuery::NotBound);
                query.addFilterOr();
                query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), "^$", MediaQuery::Contains);
                query.endFilter();
            }
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            int lastSeason = -1;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                int season = it.binding("season").literal().toInt();
                MediaItem mediaItem;
                mediaItem.url = QString("video://episodes?||season=%1||%2||%3")
                                    .arg(season).arg(seriesNameFilter).arg(genreFilter);
                mediaItem.title = seriesName;
                mediaItem.fields["title"] = mediaItem.title;
                mediaItem.fields["season"] = season;
                mediaItem.fields["seriesName"] = seriesName;
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                mediaItem.type = QString("Category");
                mediaItem.fields["categoryType"] = QString("TV Season");
                mediaItem.nowPlaying = false;
                mediaItem.artwork = KIcon("video-television");
                mediaItem = Utilities::makeSubtitle(mediaItem);

                //Provide context info for genre
                mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
                mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
                mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));

                mediaList.append(mediaItem);
                lastSeason = season;
            }

            int totalSeasons = mediaList.count();

            /* Check, whether there are TV shows, which have no series entered.
             * If so, add an entry which allows access to those files.
             */
            MediaQuery noSeasonsQuery;
            noSeasonsQuery.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            noSeasonsQuery.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required,
                                                                   seriesName,
                                                                   MediaQuery::Equal));
            noSeasonsQuery.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            noSeasonsQuery.startFilter();
            noSeasonsQuery.addFilterConstraint(mediaVocabulary.videoSeasonBinding(), QString(), MediaQuery::NotBound);
            noSeasonsQuery.endFilter();
            
            if(noSeasonsQuery.executeAsk(m_mainModel)) {
                MediaItem mediaItem;
                mediaItem.url = QString("video://episodes?||season=-1||%1||%2").arg(genreFilter).arg(seriesNameFilter);
                mediaItem.title = seriesName;
                mediaItem.fields["title"] = seriesName;
                mediaItem.fields["seriesName"] = seriesName;
                mediaItem.subTitle = i18n("Uncategorized seasons");
                mediaItem.type = QString("Category");
                mediaItem.fields["categoryType"] = QString("Basic+Artwork");
                mediaItem.nowPlaying = false;
                mediaItem.artwork = KIcon("video-television");
                mediaList.append(mediaItem);
            }

            if (totalSeasons != 1) {
                m_mediaListProperties.name = i18nc("%1=Name of the Series", "Seasons - %1", seriesName);
                m_mediaListProperties.summary = i18np("1 season", "%1 seasons", totalSeasons);
                m_mediaListProperties.type = QString("Categories");
            } else {
                engineArg = "episodes";
                if (mediaList.at(0).url == QString("video://episodes?||season=-1||%1||%2").arg(genreFilter).arg(seriesNameFilter)) {
                    season = -1;
                } else {
                    season = lastSeason;
                }
                seasonFilter = QString("season=%1").arg(season);
                mediaList.clear();
            }
        }
        
        //Retrieve TV Show Episodes
        if (engineArg.toLower() == "episodes") {

            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            bindings.append(mediaVocabulary.lastPlayedBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            if (seriesName == "~") {
                query.startFilter();
                query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), QString(), MediaQuery::NotBound);
                query.addFilterOr();
                query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), "^$", MediaQuery::Contains);
                query.endFilter();
            }
            if (season == -1) {
                query.startFilter();
                query.addFilterConstraint(mediaVocabulary.videoSeasonBinding(), QString(), MediaQuery::NotBound);
                query.endFilter();
            }
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.videoSeriesTitleBinding());
            orderByBindings.append(mediaVocabulary.videoSeasonBinding());
            orderByBindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("TV Show"), m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    mediaList.append(mediaItem);
                }
            }
            
            if (seriesName == "~") {
                m_mediaListProperties.name = i18n("Uncategorized TV Shows");
            } else if (season != -1) {
                m_mediaListProperties.name = i18nc("%1=Name of the series, %2=Number of the Season", "%1 - Season %2", seriesName, season);
            } else {
                m_mediaListProperties.name = i18nc("%1=Name of the Series", "%1 - Uncategorized Seasons", seriesName);
            }
            m_mediaListProperties.summary = i18np("1 episode", "%1 episodes", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } 
        
        //Retrieve Genres
        if (engineArg.toLower() == "genres") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString genre = it.binding("genre").literal().toString().trimmed();
                if (!genre.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("video://sources?||genre=%1").arg(genre);
                    mediaItem.title = genre;
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("VideoGenre");
                    mediaItem.fields["title"] = genre;
                    mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("flag-green");

                    mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||genre=%1").arg(genre));
                    mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||genre=%1").arg(genre));
                    mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||genre=%1").arg(genre));

                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.name = i18n("Genres");
            m_mediaListProperties.summary = i18np("1 genre", "%1 genres", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
        }
        
        //Retrieve Actors
        if (engineArg.toLower() == "actors") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(MediaVocabulary::resourceBindingForCategory("Actor"));
            bindings.append(mediaVocabulary.videoActorBinding());
            bindings.append(mediaVocabulary.videoActorDescriptionBinding());
            bindings.append(mediaVocabulary.videoActorArtworkBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoActorDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoActorArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(MediaVocabulary::resourceBindingForCategory("Actor"), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings(mediaVocabulary.videoActorBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            QStringList urls;
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString actor = it.binding(mediaVocabulary.videoActorBinding()).literal().toString().trimmed();
                if (!actor.isEmpty()) {
                    QString lri = QString("video://sources?||actor=%1").arg(actor);;
                    if (urls.indexOf(lri) == -1) {
                        MediaItem mediaItem;
                        mediaItem.url = lri;
                        mediaItem.title = actor;
                        mediaItem.type = QString("Category");
                        mediaItem.fields["categoryType"] = QString("Actor");
                        mediaItem.fields["title"] = actor;
                        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                        mediaItem.fields["description"] = it.binding(mediaVocabulary.videoActorDescriptionBinding()).literal().toString().trimmed();
                        mediaItem.fields["artworkUrl"] = it.binding(mediaVocabulary.videoActorArtworkBinding()).uri().toString();
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaItem.nowPlaying = false;
                        mediaItem.artwork = KIcon("view-media-artist");

                        mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||actor=%1").arg(actor));
                        mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||actor=%1").arg(actor));
                        mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||actor=%1").arg(actor));

                        mediaList.append(mediaItem);
                        urls.append(lri);
                    } else {
                        //Update multivalue fields for existing media item
                        MediaItem mediaItem = mediaList.at(urls.indexOf(lri));
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaList.replace(urls.indexOf(lri), mediaItem);
                    }
                }
            }
            
            m_mediaListProperties.name = i18n("Actors");
            m_mediaListProperties.summary = i18np("1 actor", "%1 actors", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
        }
        
        //Retrieve Directors
        if (engineArg.toLower() == "directors") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(MediaVocabulary::resourceBindingForCategory("Director"));
            bindings.append(mediaVocabulary.videoDirectorBinding());
            bindings.append(mediaVocabulary.videoDirectorDescriptionBinding());
            bindings.append(mediaVocabulary.videoDirectorArtworkBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoDirectorDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoDirectorArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(MediaVocabulary::resourceBindingForCategory("Director"), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings(mediaVocabulary.videoDirectorBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            QStringList urls;
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString director = it.binding(mediaVocabulary.videoDirectorBinding()).literal().toString().trimmed();
                if (!director.isEmpty()) {
                    QString lri = QString("video://sources?||director=%1").arg(director);
                    if (urls.indexOf(lri) == -1) {
                        MediaItem mediaItem;
                        mediaItem.url = lri;
                        mediaItem.title = director;
                        mediaItem.type = QString("Category");
                        mediaItem.fields["categoryType"] = QString("Director");
                        mediaItem.fields["title"] = director;
                        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                        mediaItem.fields["description"] = it.binding(mediaVocabulary.videoDirectorDescriptionBinding()).literal().toString().trimmed();
                        mediaItem.fields["artworkUrl"] = it.binding(mediaVocabulary.videoDirectorArtworkBinding()).uri().toString();
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaItem.nowPlaying = false;
                        mediaItem.artwork = KIcon("view-media-artist");

                        mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||director=%1").arg(director));
                        mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||director=%1").arg(director));
                        mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||director=%1").arg(director));

                        mediaList.append(mediaItem);
                        urls.append(lri);
                    } else {
                        //Update multivalue fields for existing media item
                        MediaItem mediaItem = mediaList.at(urls.indexOf(lri));
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaList.replace(urls.indexOf(lri), mediaItem);
                    }
                }
            }
            
            m_mediaListProperties.name = i18n("Directors");
            m_mediaListProperties.summary = i18np("1 director", "%1 directors", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
        }
        
        //Retrieve Search Results
        if (engineArg.toLower() == "search") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            bindings.append(mediaVocabulary.lastPlayedBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(mediaVocabulary.mediaResourceBinding(), MediaQuery::Optional));
            query.startFilter();
            query.addFilterConstraint(mediaVocabulary.titleBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.descriptionBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoWriterBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoDirectorBinding(), searchTerm,  MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoProducerBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoActorBinding(), searchTerm, MediaQuery::Contains);
            query.endFilter();
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            orderByBindings.append(mediaVocabulary.videoSeriesTitleBinding());
            orderByBindings.append(mediaVocabulary.videoSeasonBinding());
            orderByBindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QUrl url = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().isEmpty() ?
                it.binding(MediaVocabulary::mediaResourceBinding()).uri() :
                it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri();
                Nepomuk::Resource res(url);
                QString type = "Video Clip";
                if (res.exists()) {
                    if (res.hasType(mediaVocabulary.typeVideoMovie())) {
                        type = "Movie";
                    }
                    if (res.hasType(mediaVocabulary.typeVideoTVShow())) {
                        type = "TV Show";
                    }
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, type, m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                    mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } 
        
        //Retrieve Video sources of any type
        if (engineArg.toLower() == "sources") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.videoAudienceRatingBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            bindings.append(mediaVocabulary.lastPlayedBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(mediaVocabulary.mediaResourceBinding(), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.videoSeriesTitleBinding());
            orderByBindings.append(mediaVocabulary.videoSeasonBinding());
            orderByBindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QUrl url = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().isEmpty() ?
                it.binding(MediaVocabulary::mediaResourceBinding()).uri() :
                it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri();
                Nepomuk::Resource res(url);
                QString type = "Video Clip";
                if (res.exists()) {
                    if (res.hasType(mediaVocabulary.typeVideoMovie())) {
                        type = "Movie";
                    }
                    if (res.hasType(mediaVocabulary.typeVideoTVShow())) {
                        type = "TV Show";
                    }
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, type, m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                    mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        }
    }
    
    //Return results
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        emit updateMediaItems(mediaItems);
    } else {
        //Get any remaining metadata
        if (m_nepomukInited) {
            for (int i = 0; i < mediaList.count(); i++) {
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::completeMediaItem(mediaList.at(i));
                emit updateMediaItem(mediaItem);
            }
        }
    }
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}

void VideoListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return videos
    m_mediaListProperties.lri = QString("video://sources?%1").arg(engineFilter);
}
