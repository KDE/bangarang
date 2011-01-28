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
    m_stop = false;
    
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
            bindings.append(MediaVocabulary::resourceBindingForCategory("Artist"));
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            bindings.append(mediaVocabulary.musicArtistDescriptionBinding());
            bindings.append(mediaVocabulary.musicArtistArtworkBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicAnyArtistName(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicAnyArtistDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicArtistArtwork(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(mediaVocabulary.artistResourceBinding(), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = QStringList(mediaVocabulary.musicArtistNameBinding());
            query.orderBy(orderByBindings);

            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            QStringList urls;
            QHash<QString, QStringList> relatedTos;
            int i = 0;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString artist = it.binding(mediaVocabulary.musicArtistNameBinding()).literal().toString().trimmed();
                if (!artist.isEmpty()) {
                    QString lri = QString("music://albums?artist=%1||%2||%3").arg(artist, albumFilter, genreFilter);
                    if (urls.indexOf(lri) == -1) {
                        //Create new media item
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
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);

                        //Provide context info for artist
                        mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                        mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                        mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));

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
                ++i;
            }
            int totalArtists = mediaList.count();
            if (totalArtists != 1)  {
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
                m_mediaListProperties.summary = i18np("1 artist", "%1 artists", totalArtists);
                m_mediaListProperties.type = QString("Categories");
            } else {
                engineArg = "albums";
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
            bindings.append(MediaVocabulary::resourceBindingForCategory("Album"));
            bindings.append(mediaVocabulary.musicAlbumTitleBinding());
            bindings.append(mediaVocabulary.artistResourceBinding());
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicAnyArtistName(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(mediaVocabulary.albumResourceBinding(), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings(mediaVocabulary.musicAlbumTitleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            QStringList urls;
            QHash<QString, QStringList> relatedTos;
            int i = 0;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                //TODO: For some reason virtuoso SPARQL corrupts string variable non-ascii characters when a filter is specified
                //WORKAROUND: Prefer artist title using artist resource for now
                QString artist = Nepomuk::Resource(it.binding(mediaVocabulary.artistResourceBinding()).uri()).property(mediaVocabulary.musicArtistName()).toString();
                QString album = it.binding(mediaVocabulary.musicAlbumTitleBinding()).literal().toString().trimmed();
                if (!album.isEmpty()) {
                    QString lri = QString("music://songs?album=%1||%2||%3").arg(album, artistFilter, genreFilter);
                    if (urls.indexOf(lri) == -1) {
                        MediaItem mediaItem;
                        mediaItem.url = lri;
                        mediaItem.title = album;
                        mediaItem.type = QString("Category");
                        mediaItem.fields["categoryType"] = QString("Album");
                        mediaItem.fields["title"] = album;
                        mediaItem.fields["artist"] = artist;
                        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaItem.nowPlaying = false;
                        mediaItem.artwork = KIcon("media-optical-audio");
                        mediaItem = Utilities::makeSubtitle(mediaItem);

                        //Provide context info for album
                        mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                        mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
                        mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));

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
                ++i;
            }
            int totalAlbmus = mediaList.count();
            if (totalAlbmus != 1) {
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
                    m_mediaListProperties.name = i18n("Albums - %1 - %2", artist, singleGenreName);
                }
                
                m_mediaListProperties.summary = i18np("1 album", "%1 albums", totalAlbmus);
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
                if (m_stop) {
                    return;
                }
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
            bindings.append(mediaVocabulary.albumResourceBinding());
            bindings.append(mediaVocabulary.musicAlbumTitleBinding());
            bindings.append(mediaVocabulary.musicTrackNumberBinding());
            bindings.append(mediaVocabulary.durationBinding());
            bindings.append(mediaVocabulary.musicAlbumYearBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            bindings.append(mediaVocabulary.playCountBinding());
            bindings.append(mediaVocabulary.relatedToBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            if (artistFilter.isEmpty()) {
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            }
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicTrackNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumYear(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRelatedTo(mediaVocabulary.mediaResourceBinding(), MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.musicArtistNameBinding());
            orderByBindings.append(mediaVocabulary.musicAlbumTitleBinding());
            orderByBindings.append(mediaVocabulary.musicTrackNumberBinding());
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);

            QStringList urls;
            QList<MediaItem> fullMediaList;
            int limit = 100;
            int resultSetCount = limit;
            int resultCount = 0;
            int offset = 0;
            query.addLimit(limit);
            QHash<QString, QStringList> relatedTos;
            while (resultSetCount >= limit) {
                if (m_stop) {
                    return;
                }

                //Execute Query
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                resultSetCount = 0;
                while( it.next() ) {
                    if (m_stop) {
                        return;
                    }
                    KUrl url = it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri().isEmpty() ?
                    it.binding(mediaVocabulary.mediaResourceBinding()).uri() :
                    it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri();
                    int urlsIndex = urls.indexOf(url.prettyUrl());
                    if (urlsIndex == -1) {
                        //Only create new mediaItem if url is new
                        MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Music"), m_mediaListProperties.lri);
                        if (!mediaItem.url.startsWith("nepomuk:/")) {
                            relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                            mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                            mediaList.append(mediaItem);
                            fullMediaList.append(mediaItem);
                            urls.append(mediaItem.url);
                        }
                    } else {
                        MediaItem mediaItem = fullMediaList.at(urlsIndex);
                        relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                        mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                        mediaList.append(mediaItem);
                        fullMediaList.replace(urlsIndex, mediaItem);
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
            bindings.append(mediaVocabulary.albumResourceBinding());
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
            orderByBindings.append(mediaVocabulary.musicAlbumTitleBinding());
            orderByBindings.append(mediaVocabulary.musicTrackNumberBinding());
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            //Build media list from results
            QHash<QString, QStringList> relatedTos;
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Music"), m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    relatedTos = Utilities::multiValueAppend(relatedTos, mediaItem.url, it.binding(mediaVocabulary.relatedToBinding()).uri().toString());
                    mediaItem.fields["relatedTo"] = relatedTos.value(mediaItem.url);
                    mediaList.append(mediaItem);
                }
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
    }

    m_requestSignature = QString();
    m_subRequestSignature = QString();
}

void MusicListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("music://songs?%1").arg(engineFilter);
}
