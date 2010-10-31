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
#include "../mediaitemmodel.h"
#include "listenginefactory.h"
#include "../mediavocabulary.h"
#include "../mediaquery.h"
#include "../utilities/utilities.h"
#include <KIcon>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/variant.h>
#include <QApplication>
#include <QTime>
#include <QTextStream>
#include <QFile>
#include <taglib/fileref.h>

MusicListEngine::MusicListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

MusicListEngine::~MusicListEngine()
{
}

void MusicListEngine::run()
{
    QThread::setTerminationEnabled(true);
    
    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }

    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QStringList engineFilterList = m_mediaListProperties.engineFilterList();
    QString artist = m_mediaListProperties.filterFieldValue("artist");
    QString artistFilter = m_mediaListProperties.filterForField("artist");
    QString album = m_mediaListProperties.filterFieldValue("album");
    QString albumFilter = m_mediaListProperties.filterForField("album");
    QString genre = m_mediaListProperties.filterFieldValue("genre");
    QString genreFilter = m_mediaListProperties.filterForField("genre");
    
    if (m_nepomukInited) {
        //Retrieve Artists
        if (engineArg.toLower() == "artists") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            bindings.append(mediaVocabulary.musicArtistDescriptionBinding());
            bindings.append(mediaVocabulary.musicArtistArtworkBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicArtistArtwork(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = QStringList(mediaVocabulary.musicArtistNameBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            int i = 0;
            while( it.next() ) {
                QString artist = it.binding(mediaVocabulary.musicArtistNameBinding()).literal().toString().trimmed();
                if (!artist.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://albums?artist=%1||%2||%3").arg(artist, albumFilter, genreFilter);
                    mediaItem.title = artist;
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("Artist");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("system-users");
                    mediaItem.fields["title"] = artist;
                    mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                    mediaItem.fields["description"] = it.binding(mediaVocabulary.musicArtistDescriptionBinding()).literal().toString().trimmed();
                    mediaItem.fields["artworkUrl"] = it.binding(mediaVocabulary.musicArtistArtworkBinding()).uri().toString();

                    //Provide context info for artist
                    mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    
                    mediaList.append(mediaItem);
                }
                ++i;
            }
            if (mediaList.count() != 1)  {
                m_mediaListProperties.name = i18n("Artists");
                if (!genre.isEmpty()) {
                    QStringList genreList = genre.split("|OR|");
                    QString singleGenreName = Utilities::genreFromRawTagGenre(genreList.at(0));
                    m_mediaListProperties.name = i18nc("%1=Name of Genre", "Artists - %1", singleGenreName);

                    //Add an additional item to show all songs for all artists in this genre
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://songs?%1||%2||%3").arg(albumFilter, artistFilter, genreFilter);
                    mediaItem.title = i18n("All songs");
                    mediaItem.fields["title"] = singleGenreName;
                    mediaItem.subTitle = QString("%1").arg(singleGenreName);
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("AudioGenre");
                    mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                    mediaItem.fields["artworkUrl"] = Utilities::getGenreArtworkUrl(singleGenreName);
                    mediaItem.artwork = KIcon("audio-x-monkey");

                    mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));

                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.summary = i18np("1 artist", "%1 artists", mediaList.count());
                m_mediaListProperties.type = QString("Categories");
            } else {
                engineArg = "songs";
                artist = mediaList.at(0).title;
                artistFilter = QString("artist=%1").arg(artist);
                m_mediaListProperties.category = mediaList.at(0);
                mediaList.clear();
            }
        }
        
        //Retrieve Albums
        if (engineArg.toLower() == "albums") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.musicAlbumTitleBinding());
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings(mediaVocabulary.musicAlbumTitleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            int i = 0;
            while( it.next() ) {
                QString artist = it.binding(mediaVocabulary.musicArtistNameBinding()).literal().toString().trimmed();
                QString album = it.binding(mediaVocabulary.musicAlbumTitleBinding()).literal().toString().trimmed();
                if (!album.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://songs?album=%1||%2||%3").arg(album, artistFilter, genreFilter);
                    mediaItem.title = album;
                    mediaItem.subTitle = artist;
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("Album");
                    mediaItem.fields["title"] = album;
                    mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("media-optical-audio");

                    //Provide context info for album
                    mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));

                    mediaList.append(mediaItem);
                }
                ++i;
            }
            
            if (mediaList.count() != 1) {
                m_mediaListProperties.name = i18n("Albums");
                if (!artist.isEmpty()) {
                    m_mediaListProperties.name = i18n("Albums - %1", artist);
                    
                    //Add an additional item to show all songs on all albums for artist
                    MediaItem mediaItem = Utilities::getArtistCategoryItem(artist);
                    mediaItem.url = QString("music://songs?%1||%2||%3").arg(albumFilter, artistFilter, genreFilter);
                    mediaItem.title = QString( "  " ) + i18n("All songs");
                    mediaItem.subTitle = QString("  %1").arg(artist);
                    mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                    mediaItem.artwork = KIcon("audio-x-monkey");

                    mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));

                    mediaList.append(mediaItem);
                }
                if (!genre.isEmpty()) {
                    QStringList genreList = genre.split("|OR|");
                    QString singleGenreName = Utilities::genreFromRawTagGenre(genreList.at(0));
                    m_mediaListProperties.name = i18n("Albums - %1", singleGenreName);
                }
                if (!artist.isEmpty() && !genre.isEmpty()) {
                    QStringList genreList = genre.split("|OR|");
                    QString singleGenreName = Utilities::genreFromRawTagGenre(genreList.at(0));
                    m_mediaListProperties.name = i18n("Albums - %1 - %2", album, genre);
                }
                
                m_mediaListProperties.summary = i18np("1 album", "%1 albums", mediaList.count());
                m_mediaListProperties.type = QString("Categories");
            } else {
                engineArg = "songs";
                album = mediaList.at(0).title;
                albumFilter = QString("album=%1").arg(album);
                mediaList.clear();
            }
            
            
        } 
        
        //Retrieve Genres
        if (engineArg.toLower() == "genres") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            int i = 0;
            while( it.next() ) {
                QString genre = it.binding(mediaVocabulary.genreBinding()).literal().toString().trimmed();
                if (!genre.isEmpty()) {
                    MediaItem mediaItem;
                    mediaItem.url = QString("music://artists?genre=%1||%2||%3").arg(genre, artistFilter, albumFilter);
                    mediaItem.title = genre;
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("AudioGenre");
                    mediaItem.fields["title"] = genre;
                    mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                    mediaItem.nowPlaying = false;
                    mediaItem.fields["artworkUrl"] = Utilities::getGenreArtworkUrl(genre);
                    mediaItem.artwork = KIcon("flag-blue");

                    //Provide context info for genre
                    mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                    mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));

                    mediaList.append(mediaItem);
                }
                ++i;
            }
            if (mediaList.count() != 1) {
                mediaList = Utilities::sortMediaList(Utilities::mergeGenres(mediaList));
                m_mediaListProperties.name = i18n("Genres");
                m_mediaListProperties.summary = i18np("1 genre", "%1 genres", mediaList.count());
                m_mediaListProperties.type = QString("Categories");
            } else {
                engineArg = "songs";
                genre = mediaList.at(0).title;
                genreFilter = QString("genre=%1").arg(genre);
                mediaList.clear();
            }
            
        } 
        
        //Retrieve Songs
        if (engineArg.toLower() == "songs") {
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
            m_mediaListProperties.type = QString("Sources");

            //Create and execute query
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            bindings.append(mediaVocabulary.musicAlbumTitleBinding());
            bindings.append(mediaVocabulary.musicTrackNumberBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.musicAlbumYearBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicTrackNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumYear(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.musicArtistNameBinding());
            orderByBindings.append(mediaVocabulary.musicAlbumYearBinding());
            orderByBindings.append(mediaVocabulary.musicAlbumTitleBinding());
            orderByBindings.append(mediaVocabulary.musicTrackNumberBinding());
            query.orderBy(orderByBindings);
            
            QStringList urls;
            int limit = 100;
            int resultSetCount = limit;
            int resultCount = 0;
            int offset = 0;
            query.addLimit(limit);
            while (resultSetCount >= limit) {
                
                //Execute Query
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                resultSetCount = 0;
                while( it.next() ) {
                    KUrl url = it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri().isEmpty() ?
                    it.binding(mediaVocabulary.mediaResourceBinding()).uri() :
                    it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri();
                    QString urlString = url.prettyUrl();
                    if (urls.indexOf(urlString) == -1) {
                        //Only create new mediaItem if url is new
                        MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Music"), m_mediaListProperties.lri);
                        mediaList.append(mediaItem);
                        urls.append(urlString);
                    }
                    resultSetCount++;
                }
                
                //Emit current result set and increment offset
                resultCount += mediaList.count();
                m_mediaListProperties.summary = i18np("1 song", "%1 songs", resultCount);
                if (resultSetCount >= limit) {
                    emit results(m_requestSignature, mediaList, m_mediaListProperties, false, m_subRequestSignature);
                    mediaList.clear();
                    offset += limit;
                    query.addOffset(offset);
                }
            }
        } 
        
        //Retrieve Search Results
        if (engineArg.toLower() == "search") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            bindings.append(mediaVocabulary.musicAlbumTitleBinding());
            bindings.append(mediaVocabulary.musicTrackNumberBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.musicAlbumYearBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicTrackNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumYear(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.startFilter();
            query.addFilterConstraint(mediaVocabulary.titleBinding(), engineFilter, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.musicArtistNameBinding(), engineFilter, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.musicAlbumTitleBinding(), engineFilter, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.descriptionBinding(), engineFilter, MediaQuery::Contains);
            query.endFilter();
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.musicArtistNameBinding());
            orderByBindings.append(mediaVocabulary.musicAlbumYearBinding());
            orderByBindings.append(mediaVocabulary.musicAlbumTitleBinding());
            orderByBindings.append(mediaVocabulary.musicTrackNumberBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Music"), m_mediaListProperties.lri);
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 song", "%1 songs", mediaList.count());
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
        //Get local album artwork
        if (m_nepomukInited) {
            MediaVocabulary mediaVocabulary;
            for (int i = 0; i < mediaList.count(); i++) {
                MediaItem mediaItem = mediaList.at(i);
                if (mediaItem.fields["categoryType"].toString() == "Album") {
                    QImage artwork = Utilities::getAlbumArtwork(mediaItem.title);
                    if (!artwork.isNull()) {
                        mediaItem.hasCustomArtwork = true;
                        emit updateArtwork(artwork, mediaItem);
                    }
                }
                if (mediaItem.fields["categoryType"].toString() == "AudioGenre") {
                    if (!mediaItem.fields["artworkUrl"].toString().isEmpty()) {
                        QImage artwork = Utilities::getArtworkImageFromMediaItem(mediaItem);
                        mediaItem.hasCustomArtwork = true;
                        if (!artwork.isNull()) {
                            emit updateArtwork(artwork, mediaItem);
                        }
                    }
                }
            }
        }
    }

    m_requestSignature = QString();
    m_subRequestSignature = QString();
}

void MusicListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("music://songs?%1").arg(engineFilter);
}
