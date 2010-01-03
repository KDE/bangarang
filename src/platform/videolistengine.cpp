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

#include "mediaitemmodel.h"
#include "videolistengine.h"
#include "listenginefactory.h"
#include "mediavocabulary.h"
#include "utilities.h"

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
    //Engine filter format:
    // searchTerm||genre||seriesName||season||episode
    QString engineFilter = m_mediaListProperties.engineFilter();
    QString searchTerm;
    QString genre;
    QString seriesName;
    int season = 0;
    int episode = 0;
    
    if (!engineFilter.isEmpty()) {
        QStringList argList = engineFilter.split("||");
        searchTerm = argList.at(0);
        if (argList.count() >= 2) {
            genre = argList.at(1);
        }
        if (argList.count() >= 3) {
            seriesName = argList.at(2);
        }
        if (argList.count() >= 4) {
            season = argList.at(3).toInt();
        }
        if (argList.count() >= 5) {
            episode = argList.at(4).toInt();
        }
    }
    
    if (m_nepomukInited) {
        if (engineArg.toLower() == "clips") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            //bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Video Clip"));
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.name = i18n("Video Clips");
            m_mediaListProperties.summary = i18np("1 clip", "%1 clips", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        } else if (engineArg.toLower() == "tvshows") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            while( it.next() ) {
                QString seriesName = it.binding(mediaVocabulary.videoSeriesTitleBinding()).literal().toString();
                if (!seriesName.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("video://seasons?||%1||%2").arg(genre).arg(seriesName);
                    mediaItem.title = seriesName;
                    mediaItem.type = QString("Category");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("video-television");
                    mediaList.append(mediaItem);
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
                    mediaItem.url = QString("video://episodes?||||~");
                    mediaItem.title = i18n("Uncategorized TV Shows");
                    mediaItem.type = QString("Category");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("video-television");
                    mediaList.append(mediaItem);
                }
            }

            m_mediaListProperties.name = i18n("TV Shows");
            m_mediaListProperties.summary = i18np("1 show", "%1 shows", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
        } else if (engineArg.toLower() == "seasons") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Required));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
            if (!seriesName.isEmpty()) {
                if (seriesName != "~") {
                    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required,
                                                            seriesName,
                                                            MediaQuery::Equal));;
                } else {
                    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
                    query.startFilter();
                    query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), QString(), MediaQuery::NotBound);
                    query.addFilterOr();
                    query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), "^$", MediaQuery::Contains);
                    query.endFilter();
                }
            } else {
                query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            }
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            

            //Build media list from results
            while( it.next() ) {
                int season = it.binding("season").literal().toInt();
                MediaItem mediaItem;
                mediaItem.url = QString("video://episodes?||%1||%2||%3")
                                    .arg(genre).arg(seriesName).arg(season);
                mediaItem.title = seriesName;
                mediaItem.subTitle = i18n("Season %1", season);
                mediaItem.type = QString("Category");
                mediaItem.nowPlaying = false;
                mediaItem.artwork = KIcon("video-television");
                mediaList.append(mediaItem);
            }

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
                mediaItem.url = QString("video://episodes?||%1||%2||-1").arg(genre).arg(seriesName);
                mediaItem.title = seriesName;
                mediaItem.subTitle = i18n("Uncategorized seasons");
                mediaItem.type = QString("Category");
                mediaItem.nowPlaying = false;
                mediaItem.artwork = KIcon("video-television");
                mediaList.append(mediaItem);
            }

            m_mediaListProperties.name = i18n("Seasons - %1", seriesName);
            m_mediaListProperties.summary = i18np("1 season", "%1 seasons", mediaList.count());
            
            m_mediaListProperties.type = QString("Categories");
        } else if (engineArg.toLower() == "episodes") {
            bool hasSeason = false;
            
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
            bindings.append(mediaVocabulary.videoSynopsisBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.genreBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.videoWriterBinding());
            bindings.append(mediaVocabulary.videoDirectorBinding());
            bindings.append(mediaVocabulary.videoAssistantDirectorBinding());
            bindings.append(mediaVocabulary.videoProducerBinding());
            bindings.append(mediaVocabulary.videoActorBinding());
            bindings.append(mediaVocabulary.videoCinematographerBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSynopsis(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAssistantDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoCinematographer(MediaQuery::Optional));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
            if (!seriesName.isEmpty()) {
                if (seriesName != "~") {
                    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required,
                                                                           seriesName,
                                                                           MediaQuery::Equal));
                } else {
                    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
                    query.startFilter();
                    query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), QString(), MediaQuery::NotBound);
                    query.addFilterOr();
                    query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), "^$", MediaQuery::Contains);
                    query.endFilter();
                }
            } else {
                query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            }
            if (season > 0) {
                query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Required,
                                                                       season,
                                                                       MediaQuery::Equal));
                hasSeason = true;
            } else if (season == -1) {
                query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
                query.startFilter();
                query.addFilterConstraint(mediaVocabulary.videoSeasonBinding(), QString(), MediaQuery::NotBound);
                query.endFilter();
            } else {
                query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
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
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("TV Show"));
                mediaList.append(mediaItem);
            }
            
            if (seriesName == "~") {
                m_mediaListProperties.name = i18n("Uncategorized TV Shows");
            } else if (hasSeason) {
                m_mediaListProperties.name = i18n("%1 - Season %2", seriesName, season);
            } else {
                m_mediaListProperties.name = i18n("%1 - Uncategorized Seasons", seriesName);
            }
            m_mediaListProperties.summary = i18np("1 episode", "%1 episodes", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "movies") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.videoSynopsisBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.videoAudienceRatingBinding());
            bindings.append(mediaVocabulary.genreBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.videoWriterBinding());
            bindings.append(mediaVocabulary.videoDirectorBinding());
            bindings.append(mediaVocabulary.videoAssistantDirectorBinding());
            bindings.append(mediaVocabulary.videoProducerBinding());
            bindings.append(mediaVocabulary.videoActorBinding());
            bindings.append(mediaVocabulary.videoCinematographerBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoMovie(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSynopsis(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAssistantDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoCinematographer(MediaQuery::Optional));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            orderByBindings.append(mediaVocabulary.releaseDateBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Movie"));
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.name = i18n("Movies");
            if (!genre.isEmpty()) {
                m_mediaListProperties.name = i18n("Movies - %1", genre);
            }
            m_mediaListProperties.summary = i18np("1 movie", "%1 movies", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "genres") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required));
            }
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                QString genre = it.binding("genre").literal().toString().trimmed();
                if (!genre.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("video://sources?||%1").arg(genre);
                    mediaItem.title = genre;
                    mediaItem.type = QString("Category");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("flag-green");
                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.name = i18n("Genres");
            m_mediaListProperties.summary = i18np("1 genre", "%1 genres", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
        } else if (engineArg.toLower() == "search") {
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
            bindings.append(mediaVocabulary.videoSynopsisBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.videoAudienceRatingBinding());
            bindings.append(mediaVocabulary.genreBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.videoWriterBinding());
            bindings.append(mediaVocabulary.videoDirectorBinding());
            bindings.append(mediaVocabulary.videoAssistantDirectorBinding());
            bindings.append(mediaVocabulary.videoProducerBinding());
            bindings.append(mediaVocabulary.videoActorBinding());
            bindings.append(mediaVocabulary.videoCinematographerBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSynopsis(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAssistantDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoCinematographer(MediaQuery::Optional));
            query.startFilter();
            query.addFilterConstraint(mediaVocabulary.titleBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.descriptionBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoSynopsisBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoWriterBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoDirectorBinding(), searchTerm,  MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoAssistantDirectorBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoProducerBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoActorBinding(), searchTerm, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.videoCinematographerBinding(), searchTerm, MediaQuery::Contains);
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
            while( it.next() ) {
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
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, type);
                
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "sources") {
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
            bindings.append(mediaVocabulary.videoSynopsisBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.releaseDateBinding());
            bindings.append(mediaVocabulary.videoAudienceRatingBinding());
            bindings.append(mediaVocabulary.genreBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.videoWriterBinding());
            bindings.append(mediaVocabulary.videoDirectorBinding());
            bindings.append(mediaVocabulary.videoAssistantDirectorBinding());
            bindings.append(mediaVocabulary.videoProducerBinding());
            bindings.append(mediaVocabulary.videoActorBinding());
            bindings.append(mediaVocabulary.videoCinematographerBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSynopsis(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoAssistantDirector(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoCinematographer(MediaQuery::Optional));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
            if (!seriesName.isEmpty()) {
                if (seriesName != "~") {
                    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required,
                                                                           seriesName,
                                                                           MediaQuery::Equal));
                } else {
                    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
                    query.startFilter();
                    query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), QString(), MediaQuery::NotBound);
                    query.addFilterOr();
                    query.addFilterConstraint(mediaVocabulary.videoSeriesTitleBinding(), "^$", MediaQuery::Contains);
                    query.endFilter();
                }
            } else {
                query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            }
            if (season > 0) {
                query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Required,
                                                                  season,
                                                                  MediaQuery::Equal));
            } else if (season == -1) {
                query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
                query.startFilter();
                query.addFilterConstraint(mediaVocabulary.videoSeasonBinding(), QString(), MediaQuery::NotBound);
                query.endFilter();
            } else {
                query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            }
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.videoSeriesTitleBinding());
            orderByBindings.append(mediaVocabulary.videoSeasonBinding());
            orderByBindings.append(mediaVocabulary.videoEpisodeNumberBinding());
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            
            //Build media list from results
            while( it.next() ) {
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
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, type);
                
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        }
    }
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        model()->updateMediaItems(mediaItems);
    }
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    //exec();
    
}

void VideoListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("video://sources?%1").arg(engineFilter);
}

