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
#include "mediaindexer.h"
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
    m_state = Playlist::Finished;
    m_hadVideo = false;
    m_notificationRestrictions = 0;
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
        m_mediaIndexer = new MediaIndexer(this);
    } else {
        m_nepomukInited = false; //no resource manager
    }
    
    connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(updatePlaybackInfo(qint64)));
    connect(m_mediaObject, SIGNAL(aboutToFinish()), this, SLOT(queueNextPlaylistItem()));
    connect(m_mediaObject, SIGNAL(finished()), this, SLOT(confirmPlaylistFinished()));
    connect(m_mediaObject, SIGNAL(currentSourceChanged (const Phonon::MediaSource & )), this, SLOT(currentSourceChanged(const Phonon::MediaSource & )));
    connect(m_mediaObject, SIGNAL(stateChanged (Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(m_mediaObject, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
    connect(m_mediaController, SIGNAL(titleChanged (int)), this, SLOT(titleChanged(int)));
    connect(m_currentPlaylist, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    
}

Playlist::~Playlist()
{
    delete m_notificationRestrictions;
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
void Playlist::playItemAt(int row, Playlist::Model model)
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

        //Play media Item
        m_mediaObject->clearQueue();
        if (nextMediaItem.fields["audioType"].toString() == "CD Track") {
            m_mediaObject->setCurrentSource(Phonon::Cd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
            m_mediaObject->play();
        } else if (nextMediaItem.fields["videoType"].toString() == "DVD Title") {
            m_mediaObject->setCurrentSource(Phonon::Dvd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
            m_mediaObject->play();
        } else {
            m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromPercentEncoding(nextMediaItem.url.toUtf8())));
            m_mediaObject->play();
        }
        m_state = Playlist::Playing;
        

    } else if (model == Playlist::QueueModel) {
        //Get media item from queue list
        nextMediaItem = m_queue->mediaItemAt(row);
        nextMediaItem.nowPlaying = true;
        
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
        
        //Play media Item
        m_mediaObject->clearQueue();
        if (nextMediaItem.fields["audioType"].toString() == "CD Track") {
            m_mediaObject->setCurrentSource(Phonon::Cd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
        } else if (nextMediaItem.fields["videoType"].toString() == "DVD Title") {
            m_mediaObject->setCurrentSource(Phonon::Dvd);
            m_mediaController->setAutoplayTitles(false);
            m_mediaController->setCurrentTitle(nextMediaItem.fields["trackNumber"].toInt());
        } else {
            m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromPercentEncoding(nextMediaItem.url.toUtf8())));
        }
        m_mediaObject->play();
        m_state = Playlist::Playing;
    }
    
    
}

void Playlist::playNext()
{
    if (m_mediaObject->state() == Phonon::PlayingState || m_mediaObject->state() == Phonon::PausedState || m_mediaObject->state() == Phonon::LoadingState || m_mediaObject->state() == Phonon::ErrorState) {
        //Add currently playing item to history
        if (m_nowPlaying->rowCount() > 0) {
            if (m_nowPlaying->mediaItemAt(0).type == "Audio" || m_nowPlaying->mediaItemAt(0).type == "Video") {
                int row = m_nowPlaying->mediaItemAt(0).playlistIndex;
                m_playlistIndicesHistory.append(row);
                m_playlistUrlHistory.append(m_nowPlaying->mediaItemAt(0).url);
            }
        }
        
        if (m_queue->rowCount() > 1) {
            m_queue->removeMediaItemAt(0);
            playItemAt(0, Playlist::QueueModel);
        }
        addToQueue();
    }
}

void Playlist::playPrevious()
{
    if (m_mediaObject->state() == Phonon::PlayingState || m_mediaObject->state() == Phonon::PausedState || m_mediaObject->state() == Phonon::LoadingState) {
        if (m_playlistIndicesHistory.count() > 0) {
            //Get previously played item and remove from history
            int previousRow = m_playlistIndicesHistory.last();
            m_playlistIndicesHistory.removeLast();
            m_playlistUrlHistory.removeLast();
            
            playItemAt(previousRow, Playlist::PlaylistModel);
        }
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
    m_hadVideo = false;
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
    m_state = Playlist::Finished;
    emit playlistFinished();
}

void Playlist::playMediaList(const QList<MediaItem> &mediaList)
{
    //Clear playlist
    clearPlaylist();
    
    //Load playlist with all media items
    //Note: Because playlist loads asynchronously we have to
    //wait for signal from playlist model that loading is 
    //complete (playlistChanged) before starting playback
    // - hence the use of playWhenPlaylistChanges.
    playWhenPlaylistChanges = true;
    m_state = Playlist::Loading;
    emit loading();
    m_currentPlaylist->loadSources(mediaList); 
}

//----------------------------------------
//--- Playlist data control methods ---
//----------------------------------------

void Playlist::addMediaList(const QList<MediaItem> &mediaList)
{
    int startingRow = m_currentPlaylist->rowCount();
    for (int i = 0; i < mediaList.count(); ++i) {
        m_currentPlaylist->loadMediaItem(mediaList.at(i), true);
        m_playlistIndices.append(startingRow + i);
    }
}

void Playlist::addMediaItem(const MediaItem &mediaItem)
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
    stop();
    m_currentPlaylist->clearMediaListData(true);
    m_queue->clearMediaListData();
    m_nowPlaying->clearMediaListData();
    m_playlistIndices.clear();
    m_playlistIndicesHistory.clear();
    m_playlistUrlHistory.clear();
}

void Playlist::setMediaObject(Phonon::MediaObject *mediaObject)
{
    //NOTE:Not disconnecting signals here.  Calling routing is responsible for deletion/disconnection of old media object.  If object is not deleted/disconnected, playlist slot will continue to respond to old media object signals.
    delete m_mediaController;
    m_mediaObject = mediaObject;
    m_mediaController = new Phonon::MediaController(m_mediaObject);

    connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(updatePlaybackInfo(qint64)));
    connect(m_mediaObject, SIGNAL(aboutToFinish()), this, SLOT(queueNextPlaylistItem()));
    connect(m_mediaObject, SIGNAL(finished()), this, SLOT(confirmPlaylistFinished()));
    connect(m_mediaObject, SIGNAL(currentSourceChanged (const Phonon::MediaSource & )), this, SLOT(currentSourceChanged(const Phonon::MediaSource & )));
    connect(m_mediaObject, SIGNAL(stateChanged (Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    connect(m_mediaObject, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
    connect(m_mediaController, SIGNAL(titleChanged (int)), this, SLOT(titleChanged(int)));
    
}

void Playlist::setMode(Playlist::Mode mode)
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

Playlist::Mode Playlist::mode()
{
    return m_mode;
}

void Playlist::setRepeat(bool repeat)
{
    m_repeat = repeat;
}

Playlist::State Playlist::state()
{
    return m_state;
}

int Playlist::rowOfNowPlaying()
{
    int row  = -1;
    if (m_nowPlaying->rowCount() > 0) {
        MediaItem nowPlayingItem = m_nowPlaying->mediaItemAt(0);
        if (nowPlayingItem.type == "Audio" || nowPlayingItem.type == "Video") {
            row = nowPlayingItem.playlistIndex;
        }
    }
    return row;
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
        m_state = Playlist::Finished;
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
            QList<QUrl> queue;
            queue << QUrl::fromPercentEncoding(nextMediaItem.url.toUtf8());
            m_mediaObject->setQueue(queue);
        }
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
    updateNowPlaying();
}

void Playlist::titleChanged(int newTitle) //connected to MediaController::titleChanged
{
    if ((m_queue->rowCount() > 1)) {
        MediaItem mediaItem = m_queue->mediaItemAt(1);
        if (mediaItem.fields["trackNumber"].toInt() == newTitle) {
            m_queue->removeMediaItemAt(0);
        }
    }
    updateNowPlaying();
}

void Playlist::confirmPlaylistFinished() //connected to MediaObject::finished()
{
    if (m_state == Playlist::Finished) {
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
        m_playlistIndicesHistory.clear();
        m_playlistUrlHistory.clear();
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
        //Re-enable screensaver
        delete m_notificationRestrictions;
        m_notificationRestrictions = 0;
		return;
	}
    
    if (m_hadVideo && m_mediaObject->hasVideo()) {
        return;
    }
    if (newstate == Phonon::PlayingState || newstate == Phonon::PausedState) {
        m_hadVideo = m_mediaObject->hasVideo();
    }
    
    
    QDBusInterface iface(
    		"org.kde.kded",
    		"/modules/powerdevil",
    		"org.kde.PowerDevil");
	if ((newstate == Phonon::PlayingState || newstate == Phonon::PausedState)
			&& oldstate != Phonon::PlayingState
			&& oldstate != Phonon::PausedState) {

	    iface.call("setProfile", "Presentation");
        //Disable screensaver
        delete m_notificationRestrictions; //just to make sure more than one KNotificationRestrictions isn't created.
        m_notificationRestrictions = new KNotificationRestrictions(KNotificationRestrictions::ScreenSaver);
    
	} else if (newstate == Phonon::StoppedState &&
			(oldstate == Phonon::PlayingState || oldstate == Phonon::PausedState)){
		/* There is no way to reset the profile to the last used one.
		 * We therefore set the profile always to performance and let the
		 * refreshStatus call handle the case when the computer runs on battery.
		 */
	    //iface.call("setProfile", "Performance");
	    iface.call("refreshStatus");
	}
}

void Playlist::updatePlaybackInfo(qint64 time)
{
    if (time >= 10000 && !m_playbackInfoWritten) {
        //Update last played date and play count after 10 seconds
        if (m_nepomukInited && m_nowPlaying->rowCount() > 0) {
            Nepomuk::Resource res(m_nowPlaying->mediaItemAt(0).url);
            if (res.exists()) {
                m_mediaIndexer->updatePlaybackInfo(m_nowPlaying->mediaItemAt(0).url, true, QDateTime::currentDateTime());
            }
        }
        m_playbackInfoWritten = true;
    }
}


//--------------------------------
//--- MediaItemModel SLOTS     ---
//--------------------------------
void Playlist::playlistChanged()
{
    m_state = Playlist::Finished;
    if (playWhenPlaylistChanges && m_currentPlaylist->rowCount() > 0) {
        //Start playing with clean playlist, queue and history
        playWhenPlaylistChanges = false;
        if (m_currentPlaylist->mediaItemAt(0).type == "Audio" ||
            m_currentPlaylist->mediaItemAt(0).type == "Video") {
            start();
        } else {
            if (m_currentPlaylist->mediaItemAt(0).fields["messageType"].toString() == "No Results") {
                m_currentPlaylist->removeMediaItemAt(0,false);
            }
            m_mediaObject->stop();
            emit playlistFinished();
        }
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
            // - remove from history any items NOT in the current playlist
            // - remove from queue any items NOT in the current playlist
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
                    MediaItem mediaItem = m_currentPlaylist->mediaItemAt(i);
                    QString urlToSearch = mediaItem.url;
                    int rowInQueue = m_queue->rowOfUrl(urlToSearch);
                    if (rowInQueue == -1) {
                        m_playlistIndices.append(i);
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
    
    //Get row and url of previously playing item and add to history
    int oldItemRow = -1;
    if (m_nowPlaying->rowCount() > 0) {
        oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
    }
    
    //Find matching item in queue
    int queueRow = -1;
    for (int i = 0; i < m_queue->rowCount(); i++) {
        QString currentUrl;
        if (m_mediaObject->currentSource().discType() == Phonon::Cd) {
            currentUrl = QString("CDTRACK%1").arg(m_mediaController->currentTitle());
        } else if (m_mediaObject->currentSource().discType() == Phonon::Dvd) {
            currentUrl = QString("DVDTRACK%1").arg(m_mediaController->currentTitle());
        } else {
            currentUrl = m_mediaObject->currentSource().url().toString();
        }    
        if ((currentUrl == QUrl::fromPercentEncoding(m_queue->mediaItemAt(i).url.toUtf8()))) {
            queueRow = i;
            break;
        }
    }
    
    //Update Now Playing view
    if (queueRow != -1) {
        MediaItem nowPlayingItem = m_queue->mediaItemAt(queueRow);
        QPixmap artwork = Utilities::getArtworkFromMediaItem(nowPlayingItem);
        if (!artwork.isNull()) {
            nowPlayingItem.artwork = KIcon(artwork);
        }
        if (m_nowPlaying->rowCount() > 0) {
            m_nowPlaying->replaceMediaItemAt(0, nowPlayingItem, true);
        } else {
            m_nowPlaying->loadMediaItem(nowPlayingItem, true);
        }
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
    
    if ((m_mediaObject->currentSource().discType() != Phonon::Cd) && (m_mediaObject->currentSource().discType() != Phonon::Dvd)) {
        //Update last played date and play count
        if (m_nepomukInited && m_nowPlaying->rowCount() > 0) {
            m_playbackInfoWritten = false; // Written 10 seconds later with updatePlaybackInfo()
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

void Playlist::metaDataChanged()
{
    if (m_nowPlaying->rowCount()>0) {
        MediaItem mediaItem = m_nowPlaying->mediaItemAt(0);
        if ((mediaItem.type == "Audio") && (mediaItem.fields["audioType"].toString() == "Audio Stream")) {
            
            mediaItem.subTitle = m_mediaObject->metaData("TITLE").join(" ");
            m_nowPlaying->replaceMediaItemAt(0, mediaItem, true);
        }
    }
}
