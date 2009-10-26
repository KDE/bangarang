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

#include <Soprano/QueryResultIterator>
//#include <Soprano/Query>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KIcon>
#include <KUrl>
#include <taglib/fileref.h>
#include <QTime>
#include <nepomuk/variant.h>

VideoListEngine::VideoListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;    
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        //resource manager inited successfully
    } else {
        //no resource manager
    };
    
    m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    
}

VideoListEngine::~VideoListEngine()
{
}

void VideoListEngine::run()
{
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    if (engineArg.toLower() == "clips") {
        QString videoQuery;
        
        //Build clips query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX nmm: <%3> "
        "PREFIX xls: <%4> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg("http://www.semanticdesktop.org/ontologies/nmm#")
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?duration ?season ?description ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type <%1> . "
        "?r <%2> ?title . ")
        .arg(mediaVocabulary.typeVideo().toString())
        .arg(mediaVocabulary.title().toString());
        QString whereOptionalConditions = QString("OPTIONAL { ?r <%1> ?duration } "
        "OPTIONAL { ?r <%2> ?season } "
        "OPTIONAL { ?r <%3> ?description } "
        "OPTIONAL { ?r rdf:type <%4> . "
        "?r rdf:type ?typeMovie } "
        "OPTIONAL { ?r rdf:type <%5> . "
        "?r rdf:type ?typeSeries } ")
        .arg(mediaVocabulary.duration().toString())
        .arg(mediaVocabulary.videoSeriesSeason().toString())
        .arg(mediaVocabulary.description().toString())
        .arg(mediaVocabulary.typeVideoMovie().toString())
        .arg(mediaVocabulary.typeVideoSeries().toString());
        QString filter = QString("FILTER ( !bound(?typeMovie) && !bound(?typeSeries)) ");
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?title ");

        videoQuery = prefix + select + whereConditions + whereOptionalConditions + filter + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( videoQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            MediaItem mediaItem;
            mediaItem.url = it.binding("r").uri().toString();
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
            mediaItem.type = "Video";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("video-x-generic");
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
            mediaItem.fields["description"] = it.binding("description").literal().toString();
            mediaItem.fields["videoType"] = "Video Clip";
            Nepomuk::Resource res(mediaItem.url);
            if (res.exists()) {
                mediaItem.fields["rating"] = res.rating();
            }
            mediaList.append(mediaItem);
            ++i;
        }
        
        m_mediaListProperties.name = QString("Video Clips");
        m_mediaListProperties.type = QString("Sources");
        
    } else if (engineArg.toLower() == "series") {
        QString videoQuery;
        
        //Build clips query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX nmm: <%3> "
        "PREFIX xls: <%4> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg("http://www.semanticdesktop.org/ontologies/nmm#")
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?duration ?season ?episode ?description ?seriesName ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type <%1> . "
        "?r <%2> ?title . ")
        .arg(mediaVocabulary.typeVideoSeries().toString())
        .arg(mediaVocabulary.title().toString());
        QString whereOptionalConditions = QString("OPTIONAL { ?r <%1> ?duration } "
        "OPTIONAL { ?r <%2> ?season } "
        "OPTIONAL { ?r <%3> ?episode } "
        "OPTIONAL { ?r <%4> ?seriesName } "
        "OPTIONAL { ?r <%5> ?description } ")
        .arg(mediaVocabulary.duration().toString())
        .arg(mediaVocabulary.videoSeriesSeason().toString())
        .arg(mediaVocabulary.videoSeriesEpisode().toString())
        .arg(mediaVocabulary.videoSeriesName().toString())
        .arg(mediaVocabulary.description().toString());
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?title ");
        
        videoQuery = prefix + select + whereConditions + whereOptionalConditions + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( videoQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            MediaItem mediaItem;
            mediaItem.url = it.binding("r").uri().toString();
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
            int season = it.binding("season").literal().toInt();
            if (season != 0) {
                mediaItem.fields["season"] = season;
                mediaItem.subTitle = QString("Season %1 ").arg(season);
            }
            int episode = it.binding("episode").literal().toInt();
            if (episode != 0) {
                mediaItem.fields["episode"] = episode;
                mediaItem.subTitle = mediaItem.subTitle + QString("Episode %1").arg(episode);
            }
            QString seriesName = it.binding("seriesName").literal().toString();
            if (!seriesName.isEmpty()) {
                mediaItem.fields["seriesName"] = seriesName;
                mediaItem.subTitle = seriesName + " - " + mediaItem.subTitle;
            }

            mediaItem.type = "Video";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("video-television");
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
            mediaItem.fields["description"] = it.binding("description").literal().toString();
            mediaItem.fields["videoType"] = "Series";
            Nepomuk::Resource res(mediaItem.url);
            if (res.exists()) {
                mediaItem.fields["rating"] = res.rating();
            }
            mediaList.append(mediaItem);
            ++i;
        }
        
        m_mediaListProperties.name = QString("Series");
        m_mediaListProperties.type = QString("Sources");
        
    } else if (engineArg.toLower() == "movies") {
        QString videoQuery;
        
        //Build clips query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX nmm: <%3> "
        "PREFIX xls: <%4> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg("http://www.semanticdesktop.org/ontologies/nmm#")
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?duration ?description ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type <%1> . "
        "?r <%2> ?title . ")
        .arg(mediaVocabulary.typeVideoMovie().toString())
        .arg(mediaVocabulary.title().toString());
        QString whereOptionalConditions = QString("OPTIONAL { ?r <%1> ?duration } "
        "OPTIONAL { ?r <%2> ?description } ")
        .arg(mediaVocabulary.duration().toString())
        .arg(mediaVocabulary.description().toString());
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?title ");
        
        videoQuery = prefix + select + whereConditions + whereOptionalConditions + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( videoQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            MediaItem mediaItem;
            mediaItem.url = it.binding("r").uri().toString();
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
            mediaItem.type = "Video";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("tool-animator");
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
            mediaItem.fields["description"] = it.binding("description").literal().toString();
            mediaItem.fields["videoType"] = "Movie";
            Nepomuk::Resource res(mediaItem.url);
            if (res.exists()) {
                mediaItem.fields["rating"] = res.rating();
            }
            mediaList.append(mediaItem);
            ++i;
        }
        
        m_mediaListProperties.name = QString("Movies");
        m_mediaListProperties.type = QString("Sources");
        
    } else if (engineArg.toLower() == "search") {
        QString videoQuery;
        
        //Build search query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?description ?duration ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type <%1> . ")
        .arg(mediaVocabulary.typeVideo().toString());
        QString whereOptionalConditions = QString("OPTIONAL {?r xesam:title ?title } "
        "OPTIONAL { ?r <%1> ?description } "
        "OPTIONAL { ?r <%2> ?duration } ")
        .arg(mediaVocabulary.description().toString())
        .arg(mediaVocabulary.duration().toString());
        QString searchCondition = QString("FILTER (regex(str(?title),\"%1\",\"i\") || "
        "regex(str(?description),\"%1\",\"i\")) ")
        .arg(engineFilter);
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?title ");
        videoQuery = prefix + select + whereConditions + whereOptionalConditions + searchCondition + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( videoQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            MediaItem mediaItem;
            mediaItem.url = it.binding("r").uri().toString();
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
            mediaItem.type = "Video";
            mediaItem.nowPlaying = false;
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["description"] = it.binding("description").literal().toString();
            mediaItem.fields["duration"] = it.binding("duration").literal().toString();
            Nepomuk::Resource res(mediaItem.url);
            if (res.exists()) {
                if (res.hasType(mediaVocabulary.typeVideoMovie())) {
                    mediaItem.artwork = KIcon("tool-animator");
                    mediaItem.fields["videoType"] = "Movie";
                } else if (res.hasType(mediaVocabulary.typeVideoSeries())) {
                    mediaItem.artwork = KIcon("video-television");
                    mediaItem.fields["videoType"] = "Series";
                    int season = res.property(mediaVocabulary.videoSeriesSeason()).toInt();
                    if (season !=0 ) {
                        mediaItem.fields["season"] = season;
                        mediaItem.subTitle = QString("Season %1 ").arg(season);
                    }
                    int episode = res.property(mediaVocabulary.videoSeriesEpisode()).toInt();
                    if (episode !=0 ) {
                        mediaItem.fields["episode"] = episode;
                        mediaItem.subTitle = mediaItem.subTitle + QString("Episode %1").arg(episode);
                    }
                } else {
                    mediaItem.artwork = KIcon("video-x-generic");
                    mediaItem.fields["videoType"] = "Video Clip";
                }
                mediaItem.fields["rating"] = res.rating();
            }
            mediaList.append(mediaItem);
            ++i;
        }
        
        if (mediaList.count() == 0) {
            MediaItem noResults;
            noResults.url = "video://";
            noResults.title = "No results";
            noResults.type = "Message";
            mediaList << noResults;
        }
        
        m_mediaListProperties.type = QString("Sources");
        
    }
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    //exec();
    
}

void VideoListEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties VideoListEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void VideoListEngine::setFilterForSources(QString engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("video://?%1").arg(engineFilter);
}

void VideoListEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void VideoListEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void VideoListEngine::activateAction()
{
    
}
