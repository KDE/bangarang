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

#include "musiclistengine.h"
#include "mediaitemmodel.h"
#include "listenginefactory.h"
#include <KIcon>
#include <KUrl>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/variant.h>
#include <QApplication>
#include <QTime>
#include <taglib/fileref.h>

MusicListEngine::MusicListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;
    
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        //resource manager inited successfully
    } else {
        //no resource manager
    };
    
    m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    
    //m_mediaListProperties.dataEngine = "music://";
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    
}

MusicListEngine::~MusicListEngine()
{
}

void MusicListEngine::run()
{
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    if (engineArg.toLower() == "artists") {
        //Build songs query
        QString songQuery = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> "
        "SELECT DISTINCT ?artist "
        "WHERE { "
        "?r rdf:type xesam:Music . "
        "?r xesam:artist ?artist . "
        "} "
        "ORDER BY ?artist ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( songQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            QString artist = it.binding("artist").literal().toString().trimmed();
            if (!artist.isEmpty()) {
                MediaItem mediaItem;
                mediaItem.url = QString("music://songs?%1||%2").arg(artist, QString());
                mediaItem.title = artist;
                mediaItem.type = QString("Category");
                mediaItem.nowPlaying = false;
                mediaItem.artwork = KIcon("system-users");
                mediaList.append(mediaItem);
            }
            ++i;
        }
        m_mediaListProperties.name = QString("Artists");
        m_mediaListProperties.type = QString("Categories");
        
    } else if (engineArg.toLower() == "albums") {
        //Build songs query
        QString songQuery = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> "
        "SELECT DISTINCT ?album ?artist "
        "WHERE { "
        "?r rdf:type xesam:Music . "
        "?r xesam:album ?album . "
        "?r xesam:artist ?artist . "
        "} "
        "ORDER BY ?album ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( songQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            QString album = it.binding("album").literal().toString().trimmed();
            QString artist = it.binding("artist").literal().toString();
            if (!album.isEmpty()) {
                MediaItem mediaItem;
                mediaItem.url = QString("music://songs?%1||%2").arg(artist, album);
                mediaItem.title = album;
                mediaItem.subTitle = artist;
                mediaItem.type = QString("Category");
                mediaItem.nowPlaying = false;
                mediaItem.artwork = KIcon("media-optical-audio");
                mediaList.append(mediaItem);
            }
            ++i;
        }
        m_mediaListProperties.name = QString("Albums");
        m_mediaListProperties.type = QString("Categories");
        
    } else if (engineArg.toLower() == "songs") {
        QString songQuery;
        QString artist;
        QString album;
        
        //Parse filter
        if (!engineFilter.isNull()) {
            QStringList argList = engineFilter.split("||");
            artist = argList.at(0);
            album = argList.at(1);
        }
        
        //Build songs query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?artist ?album ?duration ?trackNumber ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type xesam:Music . "
        "?r xesam:title ?title . "
        "?r xesam:artist ?artist . ");
        QString whereOptionalConditions = QString("OPTIONAL { ?r xesam:album ?album } "
        "OPTIONAL { ?r xesam:mediaDuration ?duration } "
        "OPTIONAL { ?r xesam:genre ?genre } "
        "OPTIONAL { ?r xesam:trackNumber ?trackNumber } ");
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?artist ?album ?trackNumber ");
        QString artistCondition;
        if (!artist.isEmpty()) {
            artistCondition = QString("?r xesam:artist %1 . ").arg(Soprano::Node::literalToN3(artist));
        }
        QString albumCondition;
        if (!album.isEmpty()) {
            albumCondition = QString("?r xesam:album %1 . ").arg(Soprano::Node::literalToN3(album));
            //albumCondition = QString("OPTIONAL { ?r xesam:album %1 } ").arg(Soprano::Node::literalToN3(album));
        }
        songQuery = prefix + select + whereConditions + artistCondition + albumCondition + whereOptionalConditions + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( songQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        //QString lastUrl;
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
            mediaItem.subTitle = it.binding( "artist" ).literal().toString() + QString(" - ") + it.binding( "album" ).literal().toString();
            if (mediaItem.subTitle == QString(" - ")) {
                mediaItem.subTitle = QString();
            }
            int duration = it.binding("duration").literal().toInt();
            mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
            mediaItem.type = "Audio";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("audio-mpeg");
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["artist"] = it.binding("artist").literal().toString();
            mediaItem.fields["album"] = it.binding("album").literal().toString();
            mediaItem.fields["genre"] = it.binding("genre").literal().toString();
            mediaItem.fields["trackNumber"] = it.binding("trackNumber").literal().toInt();
            mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
            //mediaItem.fields["rating"] = (it.binding("rating").literal().toDouble() + 0.5)/2.0;
            Nepomuk::Resource res(mediaItem.url);
            if (res.exists()) {
                mediaItem.fields["rating"] = res.rating();
            }
            mediaItem.fields["audioType"] = "Music";
            //FIXME: Do not update the RDF store synchronously.  Provide UI for asynchronous update
            /*Nepomuk::Resource song(it["r"].uri());
            if (duration == 0) {
                TagLib::FileRef file(KUrl(mediaItem.url).path().toUtf8());
                duration = file.audioProperties()->length();
                mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                if (song.exists()) {
                    song.setProperty(Soprano::Vocabulary::Xesam::mediaDuration(), Nepomuk::Variant(duration));
                }
            }
            if (mediaItem.url == lastUrl) {
                //FIXME: for some reason removing the resource causes the thread to hang
                //Remove duplicate from RDF store
                Nepomuk::Variant title = song.property(Soprano::Vocabulary::Xesam::title());
                if (song.exists()) {
                    //song.remove();
                    mediaItem.duration = "-";
                }
            }
            lastUrl = mediaItem.url;*/
            mediaList.append(mediaItem);
            ++i;
        }
        
        // Name the newly created media
        m_mediaListProperties.name = album + QString(" - ") + artist;
        if (!album.isEmpty() && !artist.isEmpty()) {
            m_mediaListProperties.name = album + QString(" - ") + artist;
        } else if (!album.isEmpty()) {
            m_mediaListProperties.name = album;
        } else if (!artist.isEmpty()) {
            m_mediaListProperties.name = artist;
        } else {
            m_mediaListProperties.name = QString("Songs");
        }
        m_mediaListProperties.type = QString("Sources");
        
    } else if (engineArg.toLower() == "search") {
        QString songQuery;
        
        //Build songs query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?artist ?album ?duration ?trackNumber ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type xesam:Music . ");
        QString whereOptionalConditions = QString("OPTIONAL {?r xesam:title ?title } "
        "OPTIONAL { ?r xesam:album ?album } "
        "OPTIONAL { ?r xesam:artist ?artist } "
        "OPTIONAL { ?r xesam:genre ?genre } "
        "OPTIONAL { ?r xesam:mediaDuration ?duration } "
        "OPTIONAL { ?r xesam:trackNumber ?trackNumber } ");
        QString searchCondition = QString("FILTER (regex(str(?artist),\"%1\",\"i\") || " 
        "regex(str(?album),\"%1\",\"i\") || "
        "regex(str(?title),\"%1\",\"i\")) ")
        .arg(engineFilter);
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?artist ?album ?trackNumber ");
        songQuery = prefix + select + whereConditions + whereOptionalConditions + searchCondition + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( songQuery,                                   Soprano::Query::QueryLanguageSparql );
        
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
            mediaItem.subTitle = it.binding( "artist" ).literal().toString() + QString(" - ") + it.binding( "album" ).literal().toString();
            if (mediaItem.subTitle == QString(" - ")) {
                mediaItem.subTitle = QString();
            }
            int duration = it.binding("duration").literal().toInt();
            mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
            mediaItem.type = "Audio";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("audio-mpeg");
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["artist"] = it.binding("artist").literal().toString();
            mediaItem.fields["album"] = it.binding("album").literal().toString();
            mediaItem.fields["genre"] = it.binding("genre").literal().toString();
            mediaItem.fields["trackNumber"] = it.binding("trackNumber").literal().toInt();
            mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
            //mediaItem.fields["rating"] = (it.binding("rating").literal().toDouble() + 0.5)/2.0;
            Nepomuk::Resource res(mediaItem.url);
            if (res.exists()) {
                mediaItem.fields["rating"] = res.rating();
            }
            mediaItem.fields["audioType"] = "Music";
            mediaList.append(mediaItem);
            ++i;
        }
        
        if (mediaList.count() == 0) {
            MediaItem noResults;
            noResults.url = "music://";
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

void MusicListEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties MusicListEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void MusicListEngine::setFilterForSources(QString engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("music://songs?%1").arg(engineFilter);
}

void MusicListEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void MusicListEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void MusicListEngine::activateAction()
{
    
}
