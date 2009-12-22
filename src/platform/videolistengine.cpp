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
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/Vocabulary/NAO>
#include <QApplication>
#include <KIcon>
#include <KUrl>
#include <KLocale>
#include <taglib/fileref.h>
#include <QTime>
#include <nepomuk/variant.h>


VideoListEngine::VideoListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

VideoListEngine::~VideoListEngine()
{
}

MediaItem VideoListEngine::createMediaItem(Soprano::QueryResultIterator& it) {
    MediaItem mediaItem;
    QUrl url = it.binding("url").uri().isEmpty() ? 
                    it.binding("r").uri() :
                    it.binding("url").uri();
    mediaItem.url = url.toString();
    mediaItem.title = it.binding("title").literal().toString();
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
        }
    }
    int duration = it.binding("duration").literal().toInt();
    if (duration != 0) {
        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
    }

    QString seriesName = it.binding("seriesName").literal().toString();
    if (!seriesName.isEmpty()) {
        mediaItem.fields["seriesName"] = seriesName;
        mediaItem.subTitle = seriesName;
    }

    int season = it.binding("season").literal().toInt();
    if (season !=0 ) {
        mediaItem.fields["season"] = season;
        if (!mediaItem.subTitle.isEmpty()) {
            mediaItem.subTitle += " - ";
        }
        mediaItem.subTitle += QString("Season %1").arg(season);
    }

    int episode = it.binding("episode").literal().toInt();
    if (episode != 0) {
        mediaItem.fields["episode"] = episode;
        if (!mediaItem.subTitle.isEmpty()) {
        	mediaItem.subTitle += " - ";
        }
        mediaItem.subTitle += QString("Episode %1").arg(episode);
    }

    if (it.binding("created").isValid()) {
        QDate created = it.binding("created").literal().toDate();
        if (created.isValid()) {
            mediaItem.fields["year"] = created.year();
        }
    }
    mediaItem.type = "Video";
    mediaItem.nowPlaying = false;
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["title"] = it.binding("title").literal().toString();
    mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
    mediaItem.fields["description"] = it.binding("description").literal().toString();
    mediaItem.fields["genre"] = it.binding("genre").literal().toString();
    mediaItem.fields["artworkUrl"] = it.binding("artwork").uri().toString();
    mediaItem.fields["rating"] = it.binding("rating").literal().toInt();

    return mediaItem;
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
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectResource();
            videoQuery.selectTitle();
            videoQuery.selectDuration(true);
            videoQuery.selectSeason(true);
            videoQuery.selectDescription(true);
            videoQuery.isTVShow(false);
            videoQuery.isMovie(false);
            videoQuery.orderBy("?title");
            
            //Execute Query
            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);

                mediaItem.artwork = KIcon("video-x-generic");
                mediaItem.fields["videoType"] = "Video Clip";
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.name = i18n("Video Clips");
            m_mediaListProperties.summary = i18np("1 clip", "%1 clips", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        } else if (engineArg.toLower() == "tvshows") {
            VideoQuery query = VideoQuery(true);
            query.isTVShow(true);
            query.selectSeriesName();
            if (!genre.isEmpty()) {
                query.hasGenre(genre);
            }
            query.orderBy("?seriesName");
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            while( it.next() ) {
                QString seriesName = it.binding("seriesName").literal().toString();
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
                VideoQuery noSeriesQuery = VideoQuery();
                noSeriesQuery.isTVShow(true);
                noSeriesQuery.hasNoSeriesName();

                if(noSeriesQuery.executeAsk(m_mainModel)) {
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
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectSeason();
            videoQuery.isTVShow(true);
            if (!genre.isEmpty()) {
                if (genre == "~") genre = QString();
                videoQuery.hasGenre(genre);
            }
            if (!seriesName.isEmpty()) {
                if (seriesName != "~") {
                    videoQuery.hasSeriesName(seriesName);
                } else {
                    videoQuery.hasNoSeriesName();
                }
            }
            videoQuery.orderBy("?season");

            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);

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
            VideoQuery noSeasonsQuery = VideoQuery();
            noSeasonsQuery.isTVShow(true);
            noSeasonsQuery.hasSeriesName(seriesName);
            noSeasonsQuery.hasNoSeason();

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
            
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectResource();
            videoQuery.selectTitle();
            videoQuery.isTVShow(true);
            videoQuery.selectSeriesName(true);
            videoQuery.selectSeason(true);
            if (!genre.isEmpty()) {
                if (genre == "~") genre = QString();
                videoQuery.hasGenre(genre);
            }
            if (!seriesName.isEmpty()) {
                if (seriesName != "~") {
                    videoQuery.hasSeriesName(seriesName);
                } else {
                    videoQuery.hasNoSeriesName();
                }
            }
            if (season > 0) {
                videoQuery.hasSeason(season);
                hasSeason = true;
            } else if (season == -1) {
                videoQuery.hasNoSeason();
            }
            videoQuery.selectDuration(true);
            videoQuery.selectDescription(true);
            videoQuery.selectRating(true);
            videoQuery.selectEpisode(true);
            videoQuery.selectCreated(true);
            videoQuery.selectGenre(true);
            videoQuery.selectArtwork(true);
            videoQuery.orderBy("?seriesName ?season ?episode");


            //Execute Query
            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);

            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);

                mediaItem.artwork = KIcon("video-television");
                mediaItem.fields["videoType"] = "TV Show";
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
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectResource();
            videoQuery.selectTitle();
            videoQuery.isMovie(true);
            videoQuery.selectDescription(true);
            videoQuery.selectSeriesName(true);
            videoQuery.selectRating(true);
            videoQuery.selectDuration(true);
            videoQuery.selectCreated(true);
            videoQuery.selectGenre(true);
            videoQuery.selectArtwork(true);
            if (!genre.isEmpty()) {
                videoQuery.hasGenre(genre);
            }
            videoQuery.orderBy("?title ?created");

            //Execute Query
            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                mediaItem.artwork = KIcon("tool-animator");
                mediaItem.fields["videoType"] = "Movie";
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.name = i18n("Movies");
            if (!genre.isEmpty()) {
                m_mediaListProperties.name = i18n("Movies - %1", genre);
            }
            m_mediaListProperties.summary = i18np("1 movie", "%1 movies", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "genres") {
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectGenre();
            videoQuery.isMovie(true);
            if (!genre.isEmpty()) {
                videoQuery.hasGenre(genre);
            }
            videoQuery.orderBy("?genre");
            
            //Execute Query
            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);
            
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
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectResource();
            videoQuery.selectTitle(true);
            videoQuery.selectDescription(true);
            videoQuery.selectRating(true);
            videoQuery.selectDuration(true);
            videoQuery.selectSeriesName(true);
            videoQuery.selectSeason(true);
            videoQuery.selectEpisode(true);
            videoQuery.selectCreated(true);
            videoQuery.selectGenre(true);
            videoQuery.selectIsMovie(true);
            videoQuery.selectIsTVShow(true);
            videoQuery.selectArtwork(true);
            videoQuery.searchString(searchTerm);
            videoQuery.orderBy("?title ?seriesName ?season ?episode");
            
            //Execute Query
            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                if (it.binding("isMovie").literal().toBool()) {
                    mediaItem.artwork = KIcon("tool-animator");
                    mediaItem.fields["videoType"] = "Movie";
                } else if (it.binding("isTVShow").literal().toBool()) {
                    mediaItem.artwork = KIcon("video-television");
                    mediaItem.fields["videoType"] = "TV Show";
                } else {
                    mediaItem.artwork = KIcon("video-x-generic");
                    mediaItem.fields["videoType"] = "Video Clip";
                }
                mediaList.append(mediaItem);
            }
            
            /*if (mediaList.isEmpty()) {
                MediaItem noResults;
                noResults.url = "video://";
                noResults.title = "No results";
                noResults.type = "Message";
                mediaList << noResults;
            }*/
            
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "sources") {
            VideoQuery videoQuery = VideoQuery(true);
            videoQuery.selectResource();
            videoQuery.selectTitle(true);
            videoQuery.selectDescription(true);
            videoQuery.selectRating(true);
            videoQuery.selectDuration(true);
            videoQuery.selectSeriesName(true);
            videoQuery.selectSeason(true);
            videoQuery.selectEpisode(true);
            videoQuery.selectCreated(true);
            videoQuery.selectGenre(true);
            videoQuery.selectIsMovie(true);
            videoQuery.selectIsTVShow(true);
            videoQuery.selectArtwork(true);
            if (!searchTerm.isEmpty()) {
                videoQuery.searchString(searchTerm);
            }
            if (!genre.isEmpty()) {
                if (genre == "~") genre = QString();
                videoQuery.hasGenre(genre);
            }
            if (!seriesName.isEmpty()) {
                if (seriesName != "~") {
                    videoQuery.hasSeriesName(seriesName);
                } else {
                    videoQuery.hasNoSeriesName();
                }
            }
            if (season > 0) {
                videoQuery.hasSeason(season);
            } else if (season == -1) {
                videoQuery.hasNoSeason();
            }
            videoQuery.orderBy("?seriesName ?season ?episode ?title");
            
            //Execute Query
            Soprano::QueryResultIterator it = videoQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                if (it.binding("isMovie").literal().toBool()) {
                    mediaItem.artwork = KIcon("tool-animator");
                    mediaItem.fields["videoType"] = "Movie";
                } else if (it.binding("isTVShow").literal().toBool()) {
                    mediaItem.artwork = KIcon("video-television");
                    mediaItem.fields["videoType"] = "TV Show";
                } else {
                    mediaItem.artwork = KIcon("video-x-generic");
                    mediaItem.fields["videoType"] = "Video Clip";
                }
                mediaList.append(mediaItem);
            }
            
            /*if (mediaList.isEmpty()) {
                MediaItem noResults;
                noResults.url = "video://";
                noResults.title = "No results";
                noResults.type = "Message";
                mediaList << noResults;
            }*/
            
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



VideoQuery::VideoQuery(bool distinct) :
		m_distinct(distinct),
		m_selectResource(false),
		m_selectSeason(false),
		m_selectSeriesName(false),
		m_selectTitle(false),
		m_selectDuration(false),
		m_selectEpisode(false),
		m_selectDescription(false),
		m_selectIsTVShow(false),
		m_selectIsMovie(false) {
}

void VideoQuery::selectResource() {
	m_selectResource = true;
}

void VideoQuery::selectSeason(bool optional) {
	m_selectSeason = true;
    m_seasonCondition = addOptional(optional,
    		QString("?r <%1> ?season . ")
    		.arg(MediaVocabulary().videoSeriesSeason().toString()));
}

void VideoQuery::selectSeriesName(bool optional) {
	m_selectSeriesName = true;
    m_seriesNameCondition = addOptional(optional,
    		QString("?r <%1> ?seriesName . ")
    		.arg(MediaVocabulary().videoSeriesName().toString()));
}

void VideoQuery::selectTitle(bool optional) {
	m_selectTitle = true;
    m_titleCondition = addOptional(optional,
    		QString("?r <%1> ?title . ")
    		.arg(MediaVocabulary().title().toString()));
}

void VideoQuery::selectDuration(bool optional) {
	m_selectDuration = true;
    m_durationCondition = addOptional(optional,
    		QString("?r <%1> ?duration . ")
    		.arg(MediaVocabulary().duration().toString()));
}

void VideoQuery::selectEpisode(bool optional) {
	m_selectEpisode = true;
    m_episodeCondition = addOptional(optional,
    		QString("?r <%1> ?episode . ")
    		.arg(MediaVocabulary().videoSeriesEpisode().toString()));
}

void VideoQuery::selectDescription(bool optional) {
	m_selectDescription = true;
    m_descriptionCondition = addOptional(optional,
    		QString("?r <%1> ?description . ")
    		.arg(MediaVocabulary().description().toString()));
}

void VideoQuery::selectCreated(bool optional) {
    m_selectCreated = true;
    m_createdCondition = addOptional(optional,
                                         QString("?r <%1> ?created . ")
                                         .arg(MediaVocabulary().created().toString()));
}
void VideoQuery::selectGenre(bool optional) {
    m_selectGenre = true;
    m_genreCondition = addOptional(optional,
                                         QString("?r <%1> ?genre . ")
                                         .arg(MediaVocabulary().genre().toString()));
}

void VideoQuery::selectArtwork(bool optional) {
    m_selectArtwork = true;
    m_artworkCondition = addOptional(optional,
                                         QString("?r <%1> ?artwork . ")
                                         .arg(MediaVocabulary().artwork().toString()));
}

void VideoQuery::selectRating(bool optional) {
    m_selectRating = true;
    m_ratingCondition = addOptional(optional,
                                    QString("?r <%1> ?rating . ")
                                    .arg(Soprano::Vocabulary::NAO::numericRating().toString()));
}

void VideoQuery::selectIsTVShow(bool optional) {
	m_selectIsTVShow = true;
    m_TVShowCondition = addOptional(optional,
    		QString("?r <%1> ?isTVShow . ")
    		.arg(MediaVocabulary().videoIsTVShow().toString()));
}

void VideoQuery::selectIsMovie(bool optional) {
	m_selectIsMovie = true;
    m_movieCondition = addOptional(optional,
    		QString("?r <%1> ?isMovie . ")
    		.arg(MediaVocabulary().videoIsMovie().toString()));
}

void VideoQuery::isTVShow(bool flag) {
	if (flag) {
		m_TVShowCondition = QString("?r <%1> %2 . ")
	    		.arg(MediaVocabulary().videoIsTVShow().toString())
	    		.arg(Soprano::Node::literalToN3(true));
	} else {
		m_TVShowCondition = QString(
				"OPTIONAL { ?r <%1> ?isTVShow } "
				"FILTER ( !bound(?isTVShow) || !?isTVShow ) ")
				.arg(MediaVocabulary().videoIsTVShow().toString());
	}

}

void VideoQuery::isMovie(bool flag) {
	if (flag) {
		m_movieCondition = QString("?r <%1> %2 . ")
	    		.arg(MediaVocabulary().videoIsMovie().toString())
	    		.arg(Soprano::Node::literalToN3(true));
	} else {
		m_movieCondition = QString(
				"OPTIONAL { ?r <%1> ?isMovie } "
				"FILTER ( !bound(?isMovie) || !?isMovie ) ")
				.arg(MediaVocabulary().videoIsMovie().toString());
	}
}

void VideoQuery::hasSeason(int season) {
	m_seasonCondition = QString("?r <%1> %2 . "
                                "?r <%1> ?season . ")
    		.arg(MediaVocabulary().videoSeriesSeason().toString())
    		.arg(Soprano::Node::literalToN3(season));
}

void VideoQuery::hasNoSeason() {
	m_seasonCondition = QString(
			"OPTIONAL { ?r <%1> ?season  } "
			"FILTER ( !bound(?season) ) ")
			.arg(MediaVocabulary().videoSeriesSeason().toString());
}

void VideoQuery::hasSeriesName(QString seriesName) {
	m_seriesNameCondition = QString("?r <%1> %2 . "
                                    "?r <%1> ?seriesName . ")
    		.arg(MediaVocabulary().videoSeriesName().toString())
    		.arg(Soprano::Node::literalToN3(seriesName));
}

void VideoQuery::hasGenre(QString genre) {
    m_genreCondition = QString("?r <%1> %2 . "
                               "?r <%1> ?genre . ")
    .arg(MediaVocabulary().genre().toString())
    .arg(Soprano::Node::literalToN3(genre));
}

void VideoQuery::hasNoSeriesName() {
	m_seriesNameCondition = QString(
			"OPTIONAL { ?r <%1> ?seriesName  } "
			"FILTER (!bound(?seriesName) || regex(str(?seriesName), \"^$\")) ")
			.arg(MediaVocabulary().videoSeriesName().toString());
}

void VideoQuery::searchString(QString str) {
	if (! str.isEmpty()) {
		m_searchCondition = QString(
				"FILTER (regex(str(?title),\"%1\",\"i\") || "
			    "regex(str(?description),\"%1\",\"i\")) ")
				.arg(str);
	}
}


void VideoQuery::orderBy(QString var) {
	if (!var.isEmpty()) {
		m_order = "ORDER BY " + var;
	}
}


QString VideoQuery::addOptional(bool optional, QString str) {
	if (optional) {
		return QString("OPTIONAL { ") + str + "} . ";
 	} else {
		return str;
 	}
}

QString VideoQuery::getPrefix() {
    return QString("PREFIX xesam: <%1> "
			"PREFIX rdf: <%2> "
			"PREFIX nmm: <%3> "
            "PREFIX xls: <%4> "
            "PREFIX nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> ")
		.arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
		.arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
		.arg("http://www.semanticdesktop.org/ontologies/nmm#")
		.arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
}

Soprano::QueryResultIterator VideoQuery::executeSelect(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += "SELECT ";

    if (m_distinct)
    	queryString += "DISTINCT ";
    if (m_selectResource)
    	queryString += "?r ?url ";
    if (m_selectSeason)
    	queryString += "?season ";
    if (m_selectSeriesName)
    	queryString += "?seriesName ";
    if (m_selectTitle)
    	queryString += "?title ";
    if (m_selectDuration)
    	queryString += "?duration ";
    if (m_selectEpisode)
    	queryString += "?episode ";
    if (m_selectDescription)
    	queryString += "?description ";
    if (m_selectCreated)
        queryString += "?created ";
    if (m_selectGenre)
        queryString += "?genre ";
    if (m_selectRating)
        queryString += "?rating ";
    if (m_selectIsTVShow)
    	queryString += "?isTVShow ";
    if (m_selectIsMovie)
    	queryString += "?isMovie ";
    if (m_selectArtwork)
        queryString += "?artwork ";
    
    //NOTE: nie:url is not in any released nie ontology that I can find.
    //      In future KDE will use nfo:fileUrl so this will need to be changed.
    queryString += QString("WHERE { ?r rdf:type <%1> . OPTIONAL { ?r nie:url ?url } . ")
			.arg(MediaVocabulary().typeVideo().toString());

    queryString += m_seasonCondition;
    queryString += m_seriesNameCondition;
    queryString += m_titleCondition;
    queryString += m_durationCondition;
    queryString += m_episodeCondition;
    queryString += m_descriptionCondition;
    queryString += m_createdCondition;
    queryString += m_genreCondition;
    queryString += m_ratingCondition;
    queryString += m_TVShowCondition;
    queryString += m_movieCondition;
    queryString += m_searchCondition;
    queryString += m_artworkCondition;

    queryString += "} ";

    queryString += m_order;

    return model->executeQuery(queryString,
    		Soprano::Query::QueryLanguageSparql);
}

bool VideoQuery::executeAsk(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += QString("ASK { ?r rdf:type <%1> . ")
			.arg(MediaVocabulary().typeVideo().toString());

    queryString += m_seasonCondition;
    queryString += m_seriesNameCondition;
    queryString += m_titleCondition;
    queryString += m_durationCondition;
    queryString += m_episodeCondition;
    queryString += m_descriptionCondition;
    queryString += m_createdCondition;
    queryString += m_genreCondition;
    queryString += m_TVShowCondition;
    queryString += m_movieCondition;
    queryString += m_artworkCondition;
    queryString += "} ";

    return model->executeQuery(queryString,
    		Soprano::Query::QueryLanguageSparql)
    		.boolValue();
}
