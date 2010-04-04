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
    QStringList engineFilterList = m_mediaListProperties.engineFilterList();
    QString artist = m_mediaListProperties.filterFieldValue("artist");
    QString artistFilter = m_mediaListProperties.filterForField("artist");
    QString album = m_mediaListProperties.filterFieldValue("album");
    QString albumFilter = m_mediaListProperties.filterForField("album");
    QString genre = m_mediaListProperties.filterFieldValue("genre");
    QString genreFilter = m_mediaListProperties.filterForField("genre");
    
    if (m_nepomukInited) {
        if (engineArg.toLower() == "artists") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.musicArtistNameBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
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
                    mediaItem.url = QString("music://songs?artist=%1||%2||%3").arg(artist, albumFilter, genreFilter);
                    mediaItem.title = artist;
                    mediaItem.type = QString("Category");
                    mediaItem.fields["categoryType"] = QString("Artist");
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("system-users");
                    mediaItem.fields["title"] = artist;
                    
                    //Provide context info for artist
                    QStringList contextTitles;
                    contextTitles << i18n("Recently Played") << i18n("Highest Rated") << i18n("Frequently Played");
                    QStringList contextLRIs;
                    contextLRIs << QString("semantics://recent?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    contextLRIs << QString("semantics://highest?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    contextLRIs << QString("semantics://frequent?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    mediaItem.fields["contextTitles"] = contextTitles;
                    mediaItem.fields["contextLRIs"] = contextLRIs;
                    
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
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("media-optical-audio");

                    //Provide context info for album
                    QStringList contextTitles;
                    contextTitles << i18n("Recently Played") << i18n("Highest Rated") << i18n("Frequently Played");
                    QStringList contextLRIs;
                    contextLRIs << QString("semantics://recent?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    contextLRIs << QString("semantics://highest?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    contextLRIs << QString("semantics://frequent?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    mediaItem.fields["contextTitles"] = contextTitles;
                    mediaItem.fields["contextLRIs"] = contextLRIs;

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
                    mediaItem.fields["categoryType"] = QString("MusicGenre");
                    mediaItem.fields["title"] = genre;
                    mediaItem.nowPlaying = false;
                    mediaItem.artwork = KIcon("flag-blue");
                    
                    //Provide context info for genre
                    QStringList contextTitles;
                    contextTitles << i18n("Recently Played") << i18n("Highest Rated") << i18n("Frequently Played");
                    QStringList contextLRIs;
                    contextLRIs << QString("semantics://recent?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    contextLRIs << QString("semantics://highest?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    contextLRIs << QString("semantics://frequent?audio||limit=5||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre);
                    mediaItem.fields["contextTitles"] = contextTitles;
                    mediaItem.fields["contextLRIs"] = contextLRIs;
                    
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
    
    //Get local album artwork
    if (m_nepomukInited && engineArg.toLower() == "albums") {
        for (int i = 0; i < mediaList.count(); i++) {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required, mediaList.at(i).title, MediaQuery::Equal));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.endWhere();
            query.addLimit(5);
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
            while( it.next() ) {
                MediaItem resourceMediaItem = Utilities::mediaItemFromIterator(it, QString("Music"));
                QImage artwork = Utilities::getArtworkImageFromMediaItem(resourceMediaItem);
                if (!artwork.isNull()) {
                    MediaItem mediaItem = mediaList.at(i);
                    emit updateArtwork(artwork, mediaItem);
                    break;
                }
            }
        }
    }
        
    
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

