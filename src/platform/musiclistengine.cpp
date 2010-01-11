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
#include "mediaquery.h"
#include "utilities.h"
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
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            if (!artist.isEmpty()) { 
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required,
                                                                      artist,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required));
            }
            if (!album.isEmpty()) {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required,
                                                                      album,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            }
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
            int i = 0;
            while( it.next() ) {
                QString artist = it.binding(mediaVocabulary.musicArtistNameBinding()).literal().toString().trimmed();
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
                m_mediaListProperties.name = i18nc("%1=Name of Genre", "Artists - %1", genre);
            }
            m_mediaListProperties.summary = i18np("1 artist", "%1 artists", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
        } else if (engineArg.toLower() == "albums") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.musicAlbumTitleBinding());
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            if (!album.isEmpty()) {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required, 
                                                                      album,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required));
            }
            if (!artist.isEmpty()) { 
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required,
                                                                      artist,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            }
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
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
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required));
            }
            if (!artist.isEmpty()) { 
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required,
                                                                      artist,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            }
            if (!album.isEmpty()) {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required,
                                                                      album,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            }
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
            bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicTrackNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDuration(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumYear(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            if (!artist.isEmpty()) { 
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required,
                                                                      artist,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            }
            if (!album.isEmpty()) {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required,
                                                                      album,
                                                                      MediaQuery::Equal));
            } else {
                query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            }
            if (!genre.isEmpty()) {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                            genre,
                                                            MediaQuery::Equal));;
            } else {
                query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
            }
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
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Music"));
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
            bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasMusicTrackNumber(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Optional));
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
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Music"));
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 song", "%1 songs", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        }
    }
    
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

