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
#include "mediavocabulary.h"
#include "utilities.h"
#include <KIcon>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/variant.h>
#include <QApplication>
#include <QTime>
#include <taglib/fileref.h>

MusicListEngine::MusicListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

MusicListEngine::~MusicListEngine()
{
}

MediaItem MusicListEngine::createMediaItem(Soprano::QueryResultIterator& it) {
    MediaItem mediaItem;
    QUrl url = it.binding("url").uri().isEmpty() ? 
                    it.binding("r").uri() :
                    it.binding("url").uri();
    mediaItem.url = url.toString();
    mediaItem.title = it.binding("title").literal().toString();
    mediaItem.fields["title"] = it.binding("title").literal().toString();
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
        }
    }

    QString artist = it.binding("artist").literal().toString();
    if (!artist.isEmpty()) {
        mediaItem.fields["artist"] = artist;
        mediaItem.subTitle = artist;
    }
    
    QString album = it.binding("album").literal().toString();
    if (!album.isEmpty()) {
        mediaItem.fields["album"] = album;
        if (!artist.isEmpty()) {
            mediaItem.subTitle += QString(" - %1").arg(album);
        } else {
            mediaItem.subTitle = album;
        }
    }
    
    int duration = it.binding("duration").literal().toInt();
    if (duration != 0) {
        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
        mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
    }
    
    if (it.binding("created").isValid()) {
        QDate created = it.binding("created").literal().toDate();
        if (created.isValid()) {
            mediaItem.fields["year"] = created.year();
        }
    }
    
    int trackNumber = it.binding("trackNumber").literal().toInt();
    if (trackNumber != 0) {
        mediaItem.fields["trackNumber"] = trackNumber;
    }
    
    mediaItem.type = "Audio";
    mediaItem.nowPlaying = false;
    mediaItem.artwork = KIcon("audio-mpeg");
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["genre"] = it.binding("genre").literal().toString();
    mediaItem.fields["rating"] = it.binding("rating").literal().toInt();
    mediaItem.fields["description"] = it.binding("description").literal().toString();
    mediaItem.fields["artworkUrl"] = it.binding("artwork").uri().toString();
    mediaItem.fields["audioType"] = "Music";

    return mediaItem;
}

void MusicListEngine::run()
{
    QThread::setTerminationEnabled(true);

    if (m_updateSourceInfo || m_removeSourceInfo) {
        
        if (m_updateSourceInfo) {
            //Update to file metadata as well
            QList<MediaItem> mediaList = m_mediaItemsInfoToUpdate;
            for (int i = 0; i < mediaList.count(); i++) {
                MediaItem mediaItem = mediaList.at(i);
                if ((mediaItem.type == "Audio") && (mediaItem.fields["audioType"] == "Music")) {
                    if (Utilities::isMusic(mediaList.at(i).url)) {
                        TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toLocal8Bit());
                        if (!file.isNull()) {
                            QString title = mediaItem.title;
                            if (!title.isEmpty()) {
                                TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
                                file.tag()->setTitle(tTitle);
                            }
                            QUrl url = mediaItem.fields["artworkUrl"].toString();
                            if (!url.isEmpty()) {
                                //FIXME: Can't understand why this doesn't work.
                                Utilities::saveArtworkToTag(mediaList.at(i).url, url.toString());
                            }
                            QString artist = mediaItem.fields["artist"].toString();
                            if (!artist.isEmpty()) {
                                TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
                                file.tag()->setArtist(tArtist);
                            }
                            QString album = mediaItem.fields["album"].toString();
                            if (!album.isEmpty()) {
                                TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
                                file.tag()->setAlbum(tAlbum);
                            }
                            int year = mediaItem.fields["year"].toInt();
                            if (year != 0) {
                                file.tag()->setYear(year);
                            }
                            int trackNumber = mediaItem.fields["trackNumber"].toInt();
                            if (trackNumber != 0) {
                                file.tag()->setTrack(trackNumber);
                            }
                            QString genre = mediaItem.fields["genre"].toString();
                            if (!genre.isEmpty()) {
                                TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
                                file.tag()->setGenre(tGenre);
                            }
                            file.save();
                        }
                    }
                }
            }
        }
        
        NepomukListEngine::run();
        return;
    }

    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QString artist;
    QString album;
    QString genre;
    
    //Parse filter
    if (!engineFilter.isNull()) {
        QStringList argList = engineFilter.split("||");
        artist = argList.at(0);
        if (argList.count() >= 2) {
            album = argList.at(1);
        }
        if (argList.count() >= 3) {
            genre = argList.at(2);
        }
    }
    
    if (m_nepomukInited) {
        if (engineArg.toLower() == "artists") {
            MusicQuery query = MusicQuery(true);
            query.selectArtist();
            if (!artist.isEmpty()) {
                query.hasArtist(artist);
            }
            if (!album.isEmpty()) {
                query.hasAlbum(album);
            }
            if (!genre.isEmpty()) {
                query.hasGenre(genre);
            }
            query.orderBy("?artist");
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            int i = 0;
            while( it.next() ) {
                QString artist = it.binding("artist").literal().toString().trimmed();
                if (!artist.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://songs?%1||%2||%3").arg(artist, album, genre);
                    mediaItem.title = artist;
                    mediaItem.type = QString("Category");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("system-users");
                    mediaList.append(mediaItem);
                }
                ++i;
            }
            m_mediaListProperties.name = i18n("Artists");
            if (!genre.isEmpty()) {
                m_mediaListProperties.name = i18n("Artists - %1", genre);
            }
            m_mediaListProperties.summary = i18np("1 artist", "%1 artists", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
        } else if (engineArg.toLower() == "albums") {
            MusicQuery query = MusicQuery(true);
            query.selectAlbum(true);
            query.selectArtist();
            if (!artist.isEmpty()) {
                query.hasArtist(artist);
            }
            if (!album.isEmpty()) {
                query.hasAlbum(album);
            }
            if (!genre.isEmpty()) {
                query.hasGenre(genre);
            }
            query.orderBy("?album");
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            int i = 0;
            while( it.next() ) {
                QString artist = it.binding("artist").literal().toString().trimmed();
                QString album = it.binding("album").literal().toString().trimmed();
                if (!album.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://songs?%1||%2||%3").arg(artist, album, genre);
                    mediaItem.title = album;
                    mediaItem.subTitle = artist;
                    mediaItem.type = QString("Category");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("media-optical-audio");
                    mediaList.append(mediaItem);
                }
                ++i;
            }
            m_mediaListProperties.name = i18n("Albums");
            if (!artist.isEmpty()) {
                m_mediaListProperties.name = i18n("Albums - %1", artist);
            }
            if (!genre.isEmpty()) {
                m_mediaListProperties.name = i18n("Albums - %1", genre);
            }
            if (!artist.isEmpty() && !genre.isEmpty()) {
                m_mediaListProperties.name = i18n("Albums - %1 - %2", album, genre);
            }
            
            m_mediaListProperties.summary = i18np("1 album", "%1 albums", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
        } else if (engineArg.toLower() == "genres") {
            MusicQuery query = MusicQuery(true);
            query.selectGenre();
            if (!artist.isEmpty()) {
                query.hasArtist(artist);
            }
            if (!album.isEmpty()) {
                query.hasAlbum(album);
            }
            if (!genre.isEmpty()) {
                query.hasGenre(genre);
            }
            query.orderBy("?genre");
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            int i = 0;
            while( it.next() ) {
                QString genre = it.binding("genre").literal().toString().trimmed();
                if (!genre.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://artists?%1||%2||%3").arg(artist, album, genre);
                    mediaItem.title = genre;
                    mediaItem.type = QString("Category");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("flag-blue");
                    mediaList.append(mediaItem);
                }
                ++i;
            }
            m_mediaListProperties.name = i18n("Genres");
            m_mediaListProperties.summary = i18np("1 genre", "%1 genres", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
        } else if (engineArg.toLower() == "songs") {
            
            MusicQuery musicQuery = MusicQuery(true);
            musicQuery.selectResource();
            musicQuery.selectTitle();
            musicQuery.selectArtist();
            musicQuery.selectAlbum(true);
            musicQuery.selectTrackNumber(true);
            musicQuery.selectDuration(true);
            musicQuery.selectCreated(true);
            musicQuery.selectRating(true);
            musicQuery.selectDescription(true);
            musicQuery.selectArtwork(true);
            musicQuery.selectGenre(true);
            if (!artist.isEmpty()) {
                musicQuery.hasArtist(artist);
            }
            if (!album.isEmpty()) {
                musicQuery.hasAlbum(album);
            }
            if (!genre.isEmpty()) {
                musicQuery.hasGenre(genre);
            }
            musicQuery.orderBy("?artist ?created ?album ?trackNumber");
            
            //Execute Query
            Soprano::QueryResultIterator it = musicQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                mediaList.append(mediaItem);
            }
            
            // Name the newly created media list
            m_mediaListProperties.name = album + QString(" - ") + artist;
            if (!album.isEmpty() && !artist.isEmpty()) {
                m_mediaListProperties.name = QString("%1 - %2").arg(album).arg(artist);
            } else if (!album.isEmpty()) {
                m_mediaListProperties.name = QString("%1").arg(album);
            } else if (!artist.isEmpty()) {
                m_mediaListProperties.name = QString("%1").arg(artist);
            } else {
                m_mediaListProperties.name = i18n("Songs");
            }
            m_mediaListProperties.summary = i18np("1 song", "%1 songs", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "search") {
            MusicQuery musicQuery = MusicQuery(true);
            musicQuery.selectResource();
            musicQuery.selectTitle();
            musicQuery.selectArtist();
            musicQuery.selectAlbum(true);
            musicQuery.selectTrackNumber(true);
            musicQuery.selectDuration(true);
            musicQuery.selectCreated(true);
            musicQuery.selectRating(true);
            musicQuery.selectDescription(true);
            musicQuery.selectArtwork(true);
            musicQuery.selectGenre(true);
            musicQuery.searchString(engineFilter);
            musicQuery.orderBy("?artist ?created ?album ?trackNumber");
            
            //Execute Query
            Soprano::QueryResultIterator it = musicQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 song", "%1 songs", mediaList.count());
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
}

void MusicListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("music://songs?%1").arg(engineFilter);
}

MusicQuery::MusicQuery(bool distinct) :
m_distinct(distinct),
m_selectResource(false),
m_selectArtist(false),
m_selectAlbum(false),
m_selectTitle(false),
m_selectDuration(false),
m_selectTrackNumber(false),
m_selectGenre(false)
{
}

void MusicQuery::selectResource() {
    m_selectResource = true;
}

void MusicQuery::selectArtist(bool optional) {
    m_selectArtist = true;
    m_artistCondition = addOptional(optional,
                                    QString("?r <%1> ?artist . ")
                                    .arg(MediaVocabulary().musicArtist().toString()));
}

void MusicQuery::selectAlbum(bool optional) {
    m_selectAlbum = true;
    m_albumCondition = addOptional(optional,
                                        QString("?r <%1> ?album . ")
                                        .arg(MediaVocabulary().musicAlbum().toString()));
}

void MusicQuery::selectTitle(bool optional) {
    m_selectTitle = true;
    m_titleCondition = addOptional(optional,
                                   QString("?r <%1> ?title . ")
                                   .arg(MediaVocabulary().title().toString()));
}

void MusicQuery::selectDuration(bool optional) {
    m_selectDuration = true;
    m_durationCondition = addOptional(optional,
                                      QString("?r <%1> ?duration . ")
                                      .arg(MediaVocabulary().duration().toString()));
}

void MusicQuery::selectCreated(bool optional) {
    m_selectCreated = true;
    m_createdCondition = addOptional(optional,
                                      QString("?r <%1> ?created . ")
                                      .arg(MediaVocabulary().created().toString()));
}

void MusicQuery::selectTrackNumber(bool optional) {
    m_selectTrackNumber = true;
    m_trackNumberCondition = addOptional(optional,
                                     QString("?r <%1> ?trackNumber . ")
                                     .arg(MediaVocabulary().musicTrackNumber().toString()));
}

void MusicQuery::selectGenre(bool optional) {
    m_selectGenre = true;
    m_genreCondition = addOptional(optional,
                                         QString("?r <%1> ?genre . ")
                                         .arg(MediaVocabulary().genre().toString()));
}

void MusicQuery::selectRating(bool optional) {
    m_selectRating = true;
    m_ratingCondition = addOptional(optional,
                                   QString("?r <%1> ?rating . ")
                                   .arg(Soprano::Vocabulary::NAO::numericRating().toString()));
}

void MusicQuery::selectDescription(bool optional) {
    m_selectDescription = true;
    m_descriptionCondition = addOptional(optional,
                                    QString("?r <%1> ?description . ")
                                    .arg(MediaVocabulary().description().toString()));
}

void MusicQuery::selectArtwork(bool optional) {
    m_selectArtwork = true;
    m_artworkCondition = addOptional(optional,
                                         QString("?r <%1> ?artwork . ")
                                         .arg(MediaVocabulary().artwork().toString()));
}

void MusicQuery::hasArtist(QString artist) {
    m_artistCondition = QString("?r <%1> ?artist . "
                                "?r <%1> %2 . ")
    .arg(MediaVocabulary().musicArtist().toString())
    .arg(Soprano::Node::literalToN3(artist));
}

void MusicQuery::hasNoArtist() {
    m_artistCondition = QString(
    "OPTIONAL { ?r <%1> ?artist  } "
    "FILTER (!bound(?artist) || regex(str(?artist), \"^$\")) ")
    .arg(MediaVocabulary().musicArtist().toString());
}

void MusicQuery::hasAlbum(QString album) {
    m_albumCondition = QString("?r <%1> ?album . "
                                "?r <%1> %2 . ")
    .arg(MediaVocabulary().musicAlbum().toString())
    .arg(Soprano::Node::literalToN3(album));
}

void MusicQuery::hasNoAlbum() {
    m_albumCondition = QString(
    "OPTIONAL { ?r <%1> ?album  } "
    "FILTER (!bound(?album) || regex(str(?album), \"^$\")) ")
    .arg(MediaVocabulary().musicAlbum().toString());
}

void MusicQuery::hasGenre(QString genre) {
    m_genreCondition = QString("?r <%1> ?genre . "
    "?r <%1> %2 . ")
    .arg(MediaVocabulary().genre().toString())
    .arg(Soprano::Node::literalToN3(genre));
}

void MusicQuery::hasNoGenre() {
    m_genreCondition = QString(
    "OPTIONAL { ?r <%1> ?genre  } "
    "FILTER (!bound(?genre) || regex(str(?genre), \"^$\")) ")
    .arg(MediaVocabulary().genre().toString());
}

void MusicQuery::searchString(QString str) {
    if (! str.isEmpty()) {
        m_searchCondition = QString(
        "FILTER (regex(str(?artist),\"%1\",\"i\") || " 
        "regex(str(?album),\"%1\",\"i\") || "
        "regex(str(?title),\"%1\",\"i\")) ")
        .arg(str);
    }
}


void MusicQuery::orderBy(QString var) {
    if (!var.isEmpty()) {
        m_order = "ORDER BY " + var;
    }
}


QString MusicQuery::addOptional(bool optional, QString str) {
    if (optional) {
        return QString("OPTIONAL { ") + str + "} . ";
    } else {
        return str;
    }
}

QString MusicQuery::getPrefix() {
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

Soprano::QueryResultIterator MusicQuery::executeSelect(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += "SELECT ";
    
    if (m_distinct)
        queryString += "DISTINCT ";
    if (m_selectResource)
        queryString += "?r ?url";
    if (m_selectArtist)
        queryString += "?artist ";
    if (m_selectAlbum)
        queryString += "?album ";
    if (m_selectTitle)
        queryString += "?title ";
    if (m_selectDuration)
        queryString += "?duration ";
    if (m_selectCreated)
        queryString += "?created ";
    if (m_selectTrackNumber)
        queryString += "?trackNumber ";
    if (m_selectGenre)
        queryString += "?genre ";
    if (m_selectRating)
        queryString += "?rating ";
    if (m_selectDescription)
        queryString += "?description ";
    if (m_selectArtwork)
        queryString += "?artwork ";
    
    //NOTE: nie:url is not in any released nie ontology that I can find.
    //      In future KDE will use nfo:fileUrl so this will need to be changed.
    queryString += QString("WHERE { ?r rdf:type <%1> . OPTIONAL { ?r nie:url ?url } . ")
    .arg(MediaVocabulary().typeAudioMusic().toString());
    
    queryString += m_artistCondition;
    queryString += m_albumCondition;
    queryString += m_titleCondition;
    queryString += m_durationCondition;
    queryString += m_createdCondition;
    queryString += m_genreCondition;
    queryString += m_trackNumberCondition;
    queryString += m_ratingCondition;
    queryString += m_searchCondition;
    queryString += m_descriptionCondition;
    queryString += m_artworkCondition;
    
    queryString += "} ";
    
    queryString += m_order;
    
    return model->executeQuery(queryString,
                               Soprano::Query::QueryLanguageSparql);
}

bool MusicQuery::executeAsk(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += QString("ASK { ?r rdf:type <%1> . ")
    .arg(MediaVocabulary().typeAudioMusic().toString());
    
    queryString += m_artistCondition;
    queryString += m_albumCondition;
    queryString += m_titleCondition;
    queryString += m_durationCondition;
    queryString += m_createdCondition;
    queryString += m_trackNumberCondition;
    queryString += m_genreCondition;
    queryString += m_ratingCondition;
    queryString += m_searchCondition;
    queryString += m_descriptionCondition;
    
    queryString += "} ";
    
    return model->executeQuery(queryString,
                               Soprano::Query::QueryLanguageSparql)
                               .boolValue();
}
