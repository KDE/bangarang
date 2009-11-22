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

#include "playlist.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include "mediavocabulary.h"
#include <time.h>
#include <KUrl>
#include <KIcon>
#include <KDebug>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QDBusInterface>


Playlist::Playlist(QObject * parent, Phonon::MediaObject * mediaObject) : QObject(parent) 
{
    m_parent = parent;
    m_mediaObject = mediaObject;
    m_mediaController = new Phonon::MediaController(m_mediaObject);
    m_currentPlaylist = new MediaItemModel(this);
    m_nowPlaying = new MediaItemModel(this);
    m_queue = new MediaItemModel(this);   
    playWhenPlaylistChanges = false;
    m_mode = Playlist::Normal;
    m_repeat = false;
    m_queueDepth = 10;
    m_playlistFinished = false;
    m_loadingState = Playlist::LoadingComplete;
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
    
    connect(m_mediaObject, SIGNAL(aboutToFinish()), this, SLOT(queueNextPlaylistItem()));
    connect(m_mediaObject, SIGNAL(finished()), this, SLOT(confirmPlaylistFinished()));
    connect(m_mediaObject, SIGNAL(currentSourceChanged (const Phonon::MediaSource & )), this, SLOT(currentSourceChanged(const Phonon::MediaSource & )));
    connect(m_mediaObject, SIGNAL(stateChanged (Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(m_mediaController, SIGNAL(titleChanged (int)), this, SLOT(titleChanged(int)));
    connect(m_currentPlaylist, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    
}

Playlist::~Playlist()
{
    delete m_currentPlaylist;
    delete m_nowPlaying;
    delete m_queue;
    //delete m_mediaController;
}

MediaItemModel * Playlist::playlistModel()
{
    return m_currentPlaylist;
}

MediaItemModel * Playlist::queueModel()
{
    return m_queue;
}

MediaItemModel * Playlist::nowPlayingModel()
{
    return m_nowPlaying;
}

Phonon::MediaObject * Playlist::mediaObject()
{
    return m_mediaObject;
}

//----------------------------------------
//--- Primary playback control methods ---
//----------------------------------------
void Playlist::playItemAt(int row, int model)
{
    MediaItem nextMediaItem;
    if (model == Playlist::PlaylistModel) {
        //Get media item from playlist
        nextMediaItem = m_currentPlaylist->mediaItemAt(row);
        nextMediaItem.playlistIndex = row;
        nextMediaItem.nowPlaying = true;
        
        bool isNowPlaying = false;
        if (m_nowPlaying->rowCount() > 0) {
            if (nextMediaItem.url == m_nowPlaying->mediaItemAt(0).url) {
                isNowPlaying = true;
            }
        }
        
        //Play media Item
        m_mediaObject->clearQueue();
        if (nextMediaItem.fields["audioType"].toString() == "CD Track") {
            if (!isNowPlaying) {
                m_nowPlaying->loadMediaItem(nextMediaItem, true);
            }
            m_mediaObject->setCurrentSource(Phonon::Cd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
            m_mediaObject->play();
        } else if (nextMediaItem.fields["videoType"].toString() == "DVD Title") {
            if (!isNowPlaying) {
                m_nowPlaying->loadMediaItem(nextMediaItem, true);
            }
            m_mediaObject->setCurrentSource(Phonon::Dvd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
            m_mediaObject->play();
        } else {
            if (!isNowPlaying) {
                m_nowPlaying->loadMediaItem(nextMediaItem, true);
            }
            m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromPercentEncoding(nextMediaItem.url.toUtf8())));
            m_mediaObject->play();
            // - Get album artwork
            MediaItem itemWithArtwork = nextMediaItem;
            QPixmap artwork = Utilities::getArtworkFromMediaItem(nextMediaItem);
            if (!artwork.isNull()) {
                itemWithArtwork.artwork = KIcon(artwork);
                m_nowPlaying->replaceMediaItemAt(0, itemWithArtwork, false);
            }
        }
        m_playlistFinished = false;
        
        //Update Queue Model
        if (m_mode == Playlist::Normal) {
            //Just build a new queue from the specified row
            buildQueueFrom(row);
        } else if (m_mode == Playlist::Shuffle) {
            if (m_playlistIndicesHistory.indexOf(row) == -1) {
                //If item has not yet played move it to the front
                if (m_queue->rowOfUrl(nextMediaItem.url) != -1) {
                    QList<MediaItem> queueMediaList = m_queue->mediaList();
                    queueMediaList.move(m_queue->rowOfUrl(nextMediaItem.url), 0);
                    m_queue->clearMediaListData();
                    m_queue->loadMediaList(queueMediaList, true);
                } else {
                    QList<MediaItem> queueMediaList = m_queue->mediaList();
                    queueMediaList.insert(0, nextMediaItem);
                    if (queueMediaList.count() > m_queueDepth) {
                        queueMediaList.removeLast();
                    }
                    m_queue->clearMediaListData();
                    m_queue->loadMediaList(queueMediaList, true);
                }
            } else {
                //If item has already played remove from history and place at front of queue
                m_playlistIndicesHistory.removeAt(m_playlistIndicesHistory.indexOf(row));
                m_playlistUrlHistory.removeAt(m_playlistIndicesHistory.indexOf(row));
                QList<MediaItem> queueMediaList = m_queue->mediaList();
                queueMediaList.insert(0, nextMediaItem);
                queueMediaList.removeLast();
                m_queue->clearMediaListData();
                m_queue->loadMediaList(queueMediaList, true);
            }    
        }

    } else if (model == Playlist::QueueModel) {
        //Get media item from queue list
        nextMediaItem = m_queue->mediaItemAt(row);
        nextMediaItem.nowPlaying = true;
        
        //Play media Item
        m_mediaObject->clearQueue();
        if (nextMediaItem.fields["audioType"].toString() == "CD Track") {
            m_nowPlaying->loadMediaItem(nextMediaItem, true);
            m_mediaObject->setCurrentSource(Phonon::Cd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
        } else if (nextMediaItem.fields["videoType"].toString() == "DVD Title") {
            m_nowPlaying->loadMediaItem(nextMediaItem, true);
            m_mediaObject->setCurrentSource(Phonon::Dvd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
        } else {
            // - Get album artwork
            QPixmap artwork = Utilities::getArtworkFromMediaItem(nextMediaItem);
            if (!artwork.isNull()) {
                nextMediaItem.artwork = KIcon(artwork);
            }
            m_nowPlaying->loadMediaItem(nextMediaItem, true);
            m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromPercentEncoding(nextMediaItem.url.toUtf8())));
        }
        m_mediaObject->play();
        m_playlistFinished = false;
        
        //Update Queue Model
        if (m_mode == Playlist::Normal) {
            //Just build a new queue from the row of the item in the playlist
            buildQueueFrom(nextMediaItem.playlistIndex);
        } else if (m_mode == Playlist::Shuffle) {
            if (row > 0) {
                //Move item to front of queue
                QList<MediaItem> queueMediaList = m_queue->mediaList();
                queueMediaList.move(row, 0);
                m_queue->clearMediaListData();
                m_queue->loadMediaList(queueMediaList, true);
            }
        }
    }
    
    
}

void Playlist::playNext()
{
    if (m_queue->rowCount() > 1) {
        m_queue->removeMediaItemAt(0);
        playItemAt(0, Playlist::QueueModel);
    }
    addToQueue();
}

void Playlist::playPrevious()
{
    if (m_playlistIndicesHistory.count() > 0) {
        int previousRow = m_playlistIndicesHistory.last();
        int oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
        m_nowPlaying->removeMediaItemAt(0);
        if (oldItemRow != previousRow && oldItemRow >= 0 and oldItemRow < m_currentPlaylist->rowCount()) {
            //Cycle through true and false to ensure data change forces update
            m_currentPlaylist->item(oldItemRow,0)->setData(true, MediaItem::NowPlayingRole);
            m_currentPlaylist->item(oldItemRow,0)->setData(false, MediaItem::NowPlayingRole);
        }
        playItemAt(previousRow, Playlist::PlaylistModel);
    }
}

void Playlist::start()
{
    m_playlistIndices.clear();
    m_playlistIndicesHistory.clear();
    m_playlistUrlHistory.clear();
    m_queue->clearMediaListData();
    for (int i = 0; i < m_currentPlaylist->rowCount(); ++i) {
        m_playlistIndices.append(i);
    }
    if (m_mode == Playlist::Normal) {
        orderByPlaylist();
    } else if (m_mode == Playlist::Shuffle) {
        shuffle();
    }
    playItemAt(0, Playlist::QueueModel);
}

void Playlist::stop()
{
    m_mediaObject->stop();
    m_queue->clearMediaListData();
    if (m_nowPlaying->rowCount() > 0) {
        int oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
        m_nowPlaying->removeMediaItemAt(0, true);
        if ((oldItemRow >= 0) && (oldItemRow < m_currentPlaylist->rowCount())) {
            //Cycle through true and false to ensure data change forces update
            m_currentPlaylist->item(oldItemRow,0)->setData(true, MediaItem::NowPlayingRole);
            m_currentPlaylist->item(oldItemRow,0)->setData(false, MediaItem::NowPlayingRole);
        }
    }
    emit playlistFinished();
}

void Playlist::playMediaList(QList<MediaItem> mediaList)
{
    //Clear playlist
    clearPlaylist();
    
    
    //Load playlist with all media items
    //Note: Because playlist loads asynchronously we have to
    //wait for signal from playlist model that loading is 
    //complete (playlistChanged) before starting playback
    // - hence the use of playWhenPlaylistChanges.
    playWhenPlaylistChanges = true;
    m_loadingState = Playlist::Loading;
    emit loading();
    m_currentPlaylist->loadSources(mediaList); 
}

//----------------------------------------
//--- Playlist data control methods ---
//----------------------------------------

void Playlist::addMediaList(QList<MediaItem> mediaList)
{
    int startingRow = m_currentPlaylist->rowCount();
    for (int i = 0; i < mediaList.count(); ++i) {
        m_currentPlaylist->loadMediaItem(mediaList.at(i), true);
        m_playlistIndices.append(startingRow + i);
    }
}

void Playlist::addMediaItem(MediaItem mediaItem)
{
    int startingRow = m_currentPlaylist->rowCount();
    m_currentPlaylist->loadMediaItem(mediaItem, true);
    m_playlistIndices.append(startingRow);
}

void Playlist::removeMediaItemAt(int row)
{
    int foundAt = m_playlistIndices.indexOf(row);
    if (foundAt != -1) {
        m_playlistIndices.removeAt(foundAt);
    }
    foundAt = m_queue->rowOfUrl(m_currentPlaylist->mediaItemAt(row).url);
    if (foundAt != -1) {
        m_queue->removeMediaItemAt(foundAt);
    }
    foundAt = m_playlistIndicesHistory.indexOf(row);
    if (foundAt != -1) {
        m_playlistIndicesHistory.removeAt(foundAt);
        m_playlistUrlHistory.removeAt(foundAt);
    }
    m_currentPlaylist->removeMediaItemAt(row, true);
}

void Playlist::clearPlaylist()
{
    m_currentPlaylist->clearMediaListData(true);
    m_queue->clearMediaListData();
    m_nowPlaying->clearMediaListData();
    m_playlistIndices.clear();
    m_playlistIndicesHistory.clear();
    m_playlistUrlHistory.clear();
    m_mediaObject->stop();
}

void Playlist::setMode(int mode)
{
    if (mode <= 1) {
        m_mode = mode;
        if (m_currentPlaylist->rowCount() > 0) {
            if (m_mediaObject->state() == Phonon::PlayingState || m_mediaObject->state() == Phonon::PausedState || m_mediaObject->state() == Phonon::LoadingState) {
                //Rebuild queue after currently playing item
                if (m_mode == Playlist::Normal) {
                    if (m_queue->rowCount() > 1) {
                        buildQueueFrom(m_queue->mediaItemAt(0).playlistIndex);
                    } else {
                        orderByPlaylist();
                    }
                } else {
                    MediaItem nowPlayingItem;
                    if (m_queue->rowCount() > 1) {
                        m_queue->removeRows(1, m_queue->rowCount() - 1);
                        nowPlayingItem = m_queue->mediaItemAt(0);
                    }
                    m_playlistIndices.clear();
                    m_playlistIndicesHistory.clear();
                    m_playlistUrlHistory.clear();
                    for (int i = 0; i < m_currentPlaylist->rowCount(); ++i) {
                        if (nowPlayingItem.url.isEmpty() || m_currentPlaylist->mediaItemAt(i).url != nowPlayingItem.url) {
                            m_playlistIndices.append(i);
                        }
                    }
                    shuffle();
                }
            } else {
                //Rebuild queue from scratch
                m_queue->clearMediaListData();
                if (m_mode == Playlist::Normal) {
                    orderByPlaylist();
                } else if (m_mode == Playlist::Shuffle) {
                    shuffle();
                }
            }
        }
    }
}

int Playlist::mode()
{
    return m_mode;
}

void Playlist::setRepeat(bool repeat)
{
    m_repeat = repeat;
}

int Playlist::loadingState()
{
    return m_loadingState;
}


//----------------------------------------
//--- Media Object/Controller SLOTS    ---
//----------------------------------------

void Playlist::queueNextPlaylistItem() // connected to MediaObject::aboutToFinish()
{
    //NOTE:This is the handler for the aboutToFinish signal from the mediaObject.
    //The aboutToFinish signal is only emitted when setAutoplayTitles is false.
    //i.e. This slot is only called when the next mediaItem is not the next title
    // on a disc (a different title or a new url).
    
    addToQueue();
    if (m_queue->rowCount() == 1) {
        //Playlist is finished
        m_playlistFinished = true;        
    }
    if (m_queue->rowCount() > 1) {
        m_queue->removeMediaItemAt(0);
        //Load next queued item
        MediaItem nextMediaItem = m_queue->mediaItemAt(0);
        if (nextMediaItem.fields["audioType"].toString() == "CD Track") {
            QList<Phonon::MediaSource> queue;
            queue << Phonon::MediaSource(Phonon::Cd);
            m_mediaObject->setQueue(queue);
        } else if (nextMediaItem.fields["videoType"].toString() == "DVD Title") {
            QList<Phonon::MediaSource> queue;
            queue << Phonon::MediaSource(Phonon::Dvd);
            m_mediaObject->setQueue(queue);
        } else {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(nextMediaItem);
            if (!artwork.isNull()) {
                nextMediaItem.artwork = KIcon(artwork);
            }
            QList<QUrl> queue;
            queue << QUrl::fromPercentEncoding(nextMediaItem.url.toUtf8());
            m_mediaObject->setQueue(queue);
        }
        m_nowPlaying->loadMediaItem(nextMediaItem);
    }
    
}

void Playlist::currentSourceChanged(const Phonon::MediaSource & newSource) //connected to MediaObject::currentSourceChanged
{
    //Check next mediaItem to decide how to setAutoplayTitles
    if ((newSource.discType() == Phonon::Cd) || (newSource.discType() == Phonon::Dvd)){
        if (m_queue->rowCount() > 1) {
            if ((m_queue->mediaItemAt(1).url.startsWith("CDTRACK")) || (m_queue->mediaItemAt(1).url.startsWith("DVDTRACK"))) {
                if (m_queue->mediaItemAt(1).fields["trackNumber"].toInt() == m_mediaController->currentTitle() + 1) {
                   m_mediaController->setAutoplayTitles(true);
                } else {
                   m_mediaController->setAutoplayTitles(false);
                }
            }
        }
    } 
    if (newSource.discType() == Phonon::NoDisc) {
        updateNowPlaying();
    }
}

void Playlist::titleChanged(int newTitle) //connected to MediaController::titleChanged
{
    //TODO:Confirm that titleChanged signal is not emitted for first track on cd
    if ((m_queue->rowCount() > 1)) {
        MediaItem mediaItem = m_queue->mediaItemAt(1);
        if (mediaItem.fields["trackNumber"].toInt() == newTitle) {
            m_queue->removeMediaItemAt(0);
            m_nowPlaying->loadMediaItem(mediaItem);
        }
    }
    updateNowPlaying();
}

void Playlist::confirmPlaylistFinished() //connected to MediaObject::finished()
{
    if (m_playlistFinished) {
        //Refresh playlist model to ensure views get updated
        int row = -1;
        if (m_nowPlaying->rowCount() > 0) {
            row = m_nowPlaying->mediaItemAt(0).playlistIndex;
        }
        if ((row >= 0) && (row < m_currentPlaylist->rowCount())) {
            m_currentPlaylist->item(row,0)->setData(true, MediaItem::NowPlayingRole);
            m_currentPlaylist->item(row,0)->setData(false, MediaItem::NowPlayingRole);
        }
        
        //Clear nowPlaying and queue and emit playlistFinished
        m_nowPlaying->removeMediaItemAt(0);
        m_queue->removeMediaItemAt(0);
        emit playlistFinished();
    }
}

void Playlist::stateChanged(Phonon::State newstate, Phonon::State oldstate) {
	/* Change power profile only if the current media has video.
	 * Since this slot is called only if the state of the current
	 * media changes, it should be known whether it has video or not,
	 * so no connecting to Phonon::MediaObject::hasVideoChanged() is
	 * necessary.
	 */
	if (!m_mediaObject->hasVideo()) {
		return;
	}

    QDBusInterface iface(
    		"org.kde.kded",
    		"/modules/powerdevil",
    		"org.kde.PowerDevil");
	if ((newstate == Phonon::PlayingState || newstate == Phonon::PausedState)
			&& oldstate != Phonon::PlayingState
			&& oldstate != Phonon::PausedState) {

	    iface.call("setProfile", "Presentation");
	} else if (newstate == Phonon::StoppedState &&
			(oldstate == Phonon::PlayingState || oldstate == Phonon::PausedState)){
		/* There is no way to reset the profile to the last used one.
		 * We therefore set the profile always to performance and let the
		 * refreshStatus call handle the case when the computer runs on battery.
		 */
	    iface.call("setProfile", "Performance");
	    iface.call("refreshStatus");
	}
}


//--------------------------------
//--- MediaItemModel SLOTS     ---
//--------------------------------
void Playlist::playlistChanged()
{
    m_loadingState = Playlist::LoadingComplete;
    if (playWhenPlaylistChanges && m_currentPlaylist->rowCount() > 0) {
        //Start playing with clean playlist, queue and history
        start();
        playWhenPlaylistChanges = false;
    } else if (m_currentPlaylist->rowCount() > 0){
        //if playlist mode is normal (sequential)
        // - rebuild history to just before currently playing/paused url
        // - rebuild queue from currently playing item to queue depth
        // - rebuild playlist indices using remaining items
        if (m_mode == Playlist::Normal) {
            int currentRow = 0;
            if ((m_mediaObject->state() == Phonon::PlayingState) || (m_mediaObject->state() == Phonon::PausedState)) {
                //Starting with the currently playing item, check to see if item
                //is in the new playlist. If not continue through the existing queue
                //until an item is found in the playlist.
                for (int i = 0; i < m_queue->rowCount(); i++) {
                    QString url = m_queue->mediaItemAt(i).url;
                    currentRow = m_currentPlaylist->rowOfUrl(url);
                    if (currentRow != -1) {
                        break;
                    }
                }
            } else {
                currentRow = 0;
            }
            buildQueueFrom(currentRow);
        } else {
            //if playlist mode is shuffle
            // - remove from history any items NOT in the current playlistChanged
            // - remove from queue any items NOT in the current playlistChanged
            // - rebuild playlist indices using remaining items
            // - add items to queu to fill queue depth
            QList<QString> oldPlaylistUrlHistory = m_playlistUrlHistory;
            m_playlistIndicesHistory.clear();
            m_playlistUrlHistory.clear();
            for (int i = 0; i < oldPlaylistUrlHistory.count(); i++) {
                int rowOfUrl = m_currentPlaylist->rowOfUrl(oldPlaylistUrlHistory.at(i));
                if (rowOfUrl != -1) {
                    m_playlistIndicesHistory.append(rowOfUrl);
                    m_playlistUrlHistory.append(oldPlaylistUrlHistory.at(i));
                }
            }
            
            QList<MediaItem> newQueueMediaList;
            for (int i = 0; i < m_queue->rowCount(); i++) {
                MediaItem mediaItem = m_queue->mediaItemAt(i);
                QString urlToSearch = mediaItem.url;
                int rowOfUrl = m_currentPlaylist->rowOfUrl(urlToSearch);
                if (rowOfUrl != -1) {
                    newQueueMediaList.append(mediaItem);
                }
            }
            m_queue->clearMediaListData();
            m_queue->loadMediaList(newQueueMediaList);
            
            m_playlistIndices.clear();
            for (int i = 0; i < m_currentPlaylist->rowCount(); i++) {
                if (m_playlistIndicesHistory.indexOf(i) == -1) {
                    MediaItem mediaItem = m_queue->mediaItemAt(i);
                    QString urlToSearch = mediaItem.url;
                    int rowOfUrl = m_queue->rowOfUrl(urlToSearch);
                    if (rowOfUrl == -1) {
                        m_playlistIndices.append(rowOfUrl);
                    }
                }
            }
            
            for (int i = m_queue->rowCount(); i < m_queueDepth; i++) {
                addToQueue();
            }
            
        }
        
        //if currently playing url is not at front of queueMediaList
        // stop playing and play item at front of queue
        if ((m_mediaObject->state() == Phonon::PlayingState) || (m_mediaObject->state() == Phonon::PausedState)) {
            QString currentUrl;
            if (m_mediaObject->currentSource().discType() == Phonon::Cd) {
                currentUrl = QString("CDTRACK%1").arg(m_mediaController->currentTitle());
            } else if (m_mediaObject->currentSource().discType() == Phonon::Dvd) {
                currentUrl = QString("DVDTRACK%1").arg(m_mediaController->currentTitle());
            } else {
                currentUrl = m_mediaObject->currentSource().url().toString();
            }
            if (currentUrl != QUrl::fromPercentEncoding(m_queue->mediaItemAt(0).url.toUtf8())) {
                m_mediaObject->stop();
                playItemAt(0, Playlist::QueueModel);
            }
        }
    }
}

//--------------------------------
//--- Private Methods          ---
//--------------------------------

void Playlist::updateNowPlaying()
{
    //ACTUALLY UPDATE THE NOW PLAYING VIEW HERE!
    //THIS SHOULD BE THE ONLY PLACE THAT UPDATES THE NOW PLAYING VIEW!
    
    //Get row of previously playing item and add to history
    MediaItem mediaItem;
    bool itemIsMedia = false;
    if (m_nowPlaying->rowCount() > 0) {
        mediaItem = m_nowPlaying->mediaItemAt(0);
        if ((mediaItem.type == "Audio") || (mediaItem.type == "Video") || (mediaItem.type == "Image")) {
            itemIsMedia = true;
        }
    }
    
    bool itemIsStale = true;
    if (itemIsMedia) {
        if ((mediaItem.fields["audioType"].toString() == "CD Track") || (mediaItem.fields["videoType"].toString() == "DVD Title")) {
            if (mediaItem.fields["trackNumber"].toInt() == m_mediaController->currentTitle()) {
                itemIsStale = false;
            }
        } else {
            QUrl mediaItemUrl = QUrl::fromPercentEncoding(mediaItem.url.toUtf8());
            if (mediaItemUrl == m_mediaObject->currentSource().url()) {
                itemIsStale = false;
            }
        }
    }
        
    int oldItemRow = -1;
    if (itemIsStale) {
        if (itemIsMedia) {
            oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
            m_playlistIndicesHistory.append(oldItemRow);
            m_playlistUrlHistory.append(m_currentPlaylist->mediaItemAt(oldItemRow).url);
        }
        //Update Now Playing view
        m_nowPlaying->removeMediaItemAt(0, true); //remove old now playing item
    }
    
    //Refresh playlist model to ensure views get updated
    int row = -1;
    if (m_nowPlaying->rowCount() > 0) {
        row = m_nowPlaying->mediaItemAt(0).playlistIndex;
    }
    if ((row >= 0) && (row < m_currentPlaylist->rowCount())) {
        m_currentPlaylist->item(row,0)->setData(false, MediaItem::NowPlayingRole);
        m_currentPlaylist->item(row,0)->setData(true, MediaItem::NowPlayingRole);
    }
    if (oldItemRow != row && oldItemRow >= 0 and oldItemRow < m_currentPlaylist->rowCount()) {
        //Cycle through true and false to ensure data change forces update
        m_currentPlaylist->item(oldItemRow,0)->setData(true, MediaItem::NowPlayingRole);
        m_currentPlaylist->item(oldItemRow,0)->setData(false, MediaItem::NowPlayingRole);
    }
    
    if (m_mediaObject->currentSource().discType() == Phonon::Cd) {
        //m_mediaController->setCurrentTitle(m_nowPlaying->mediaItemAt(0).fields["trackNumber"].toInt());
    } else if (m_mediaObject->currentSource().discType() == Phonon::Dvd) {
        //m_mediaController->setCurrentTitle(m_nowPlaying->mediaItemAt(0).fields["trackNumber"].toInt());
    } else {
        //Update last played date and play count
        if (m_nepomukInited && m_nowPlaying->rowCount() > 0) {
            MediaVocabulary mediaVocabulary = MediaVocabulary();
            Nepomuk::Resource res(m_nowPlaying->mediaItemAt(0).url);
            if (res.exists()) {
                res.setProperty(mediaVocabulary.lastPlayed(), Nepomuk::Variant(QDateTime::currentDateTime()));
                int playCount = res.property(mediaVocabulary.playCount()).toInt();
                playCount = playCount + 1;
                res.setProperty(mediaVocabulary.playCount(), Nepomuk::Variant(playCount));        
            }
        }
    }
}

void Playlist::shuffle()
{
    srand((unsigned)time(0));
    int numberToAdd = m_queueDepth - m_queue->rowCount();
    for (int i = 0; i < qMin(numberToAdd, m_currentPlaylist->rowCount()); ++i) {
        addToQueue();
    }
}

void Playlist::orderByPlaylist()
{
    int numberToAdd = m_queueDepth - m_queue->rowCount();
    for (int i = 0; i < qMin(numberToAdd, m_currentPlaylist->rowCount()); ++i) {
        addToQueue();
    }
}

void Playlist::addToQueue()
{
    if ((m_playlistIndices.count() == 0) && (m_repeat)) {
        for (int i = 0; i < m_currentPlaylist->rowCount(); ++i) {
            m_playlistIndices.append(i);
        }
    }
    if (m_playlistIndices.count() > 0) {
        if (m_mode == Playlist::Normal) {
            int nextIndex = m_playlistIndices.takeAt(0);
            MediaItem nextMediaItem = m_currentPlaylist->mediaItemAt(nextIndex);
            nextMediaItem.playlistIndex = nextIndex;
            m_queue->loadMediaItem(nextMediaItem);
        } else if (m_mode == Playlist::Shuffle) {
            int nextIndex = m_playlistIndices.takeAt(rand()%m_playlistIndices.count());
            MediaItem nextMediaItem = m_currentPlaylist->mediaItemAt(nextIndex);
            nextMediaItem.playlistIndex = nextIndex;
            m_queue->loadMediaItem(nextMediaItem);
        }
    }
}

void Playlist::buildQueueFrom(int playlistRow)
{
    if (playlistRow < m_currentPlaylist->rowCount()) {
        m_playlistIndices.clear();
        m_playlistIndicesHistory.clear();
        m_playlistUrlHistory.clear();
        m_queue->clearMediaListData();
        for (int i = 0; i < playlistRow; ++i) {
            m_playlistIndicesHistory.append(i);
            m_playlistUrlHistory.append(m_currentPlaylist->mediaItemAt(i).url);
        }
        createUrlHistoryFromIndices();
        int lastRowOfQueue = 0;
        for (int j = playlistRow; j < qMin(playlistRow + m_queueDepth, m_currentPlaylist->rowCount()); ++j) {
            MediaItem nextMediaItem = m_currentPlaylist->mediaItemAt(j);
            nextMediaItem.playlistIndex = j;
            m_queue->loadMediaItem(nextMediaItem);
            lastRowOfQueue = j;
        }
        for (int k = lastRowOfQueue + 1; k < m_currentPlaylist->rowCount(); ++k) {
            m_playlistIndices.append(k);
        }
    }   
}

void Playlist::createUrlHistoryFromIndices()
{
    m_playlistUrlHistory.clear();
    for (int i = 0; i < m_playlistIndicesHistory.count(); i++) {
        m_playlistUrlHistory.append(m_currentPlaylist->mediaItemAt(m_playlistIndicesHistory.at(i)).url);
    }
}
