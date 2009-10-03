#include "playlist.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include "mediavocabulary.h"
#include <time.h>
#include <KUrl>
#include <KIcon>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>


Playlist::Playlist(QObject * parent, Phonon::MediaObject * mediaObject) : QObject(parent) 
{
    m_parent = parent;
    m_mediaObject = mediaObject;
    m_currentPlaylist = new MediaItemModel(this);
    m_nowPlaying = new MediaItemModel(this);
    m_queue = new MediaItemModel(this);   
    playWhenPlaylistChanges = false;
    m_mode = Playlist::Normal;
    m_repeat = false;
    m_queueDepth = 10;
    m_playlistFinished = false;
    
    connect(m_mediaObject, SIGNAL(aboutToFinish()), this, SLOT(queueNextPlaylistItem()));
    connect(m_mediaObject, SIGNAL(currentSourceChanged (const Phonon::MediaSource & )), this, SLOT(updateNowPlaying(const Phonon::MediaSource & )));
    connect(m_currentPlaylist, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    
}

Playlist::~Playlist()
{
    delete m_currentPlaylist;
    delete m_nowPlaying;
    delete m_queue;
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

void Playlist::playItemAt(int row, int model)
{
    MediaItem nextMediaItem;
    if (model == Playlist::PlaylistModel) {
        //Get media item from playlist
        nextMediaItem = m_currentPlaylist->mediaItemAt(row);
        nextMediaItem.playlistIndex = row;
        nextMediaItem.nowPlaying = true;
        
        //Play media Item
        m_mediaObject->clearQueue();
        m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl(nextMediaItem.url)));
        m_mediaObject->play();
        m_playlistFinished = false;
        
        
        if (m_mode == Playlist::Normal) {
            //Just build a new queue from the specified row
            buildQueueFrom(row);
        } else if (m_mode == Playlist::Shuffle) {
            if ((m_playlistIndicesHistory.indexOf(row) == -1) && (m_nowPlaying->mediaItemAt(0).url != nextMediaItem.url)) {
                //If item has not yet played move it to the front
                if (m_queue->rowOfUrl(nextMediaItem.url) != -1) {
                    QList<MediaItem> queueMediaList = m_queue->mediaList();
                    queueMediaList.move(m_queue->rowOfUrl(nextMediaItem.url), 0);
                    m_queue->clearMediaListData();
                    m_queue->loadMediaList(queueMediaList, true);
                } else {
                    QList<MediaItem> queueMediaList = m_queue->mediaList();
                    queueMediaList.insert(0, nextMediaItem);
                    queueMediaList.removeLast();
                    m_queue->clearMediaListData();
                    m_queue->loadMediaList(queueMediaList, true);
                }
            } else if (m_playlistIndicesHistory.indexOf(row) != -1) {
                //If item has already played remove from history and place at front of queue
                m_playlistIndicesHistory.removeAt(m_playlistIndicesHistory.indexOf(row));
                QList<MediaItem> queueMediaList = m_queue->mediaList();
                queueMediaList.insert(0, nextMediaItem);
                queueMediaList.removeLast();
                m_queue->clearMediaListData();
                m_queue->loadMediaList(queueMediaList, true);
            }    
        }
            
        //Get row of previously playing item
        int oldItemRow = -1;
        if (m_nowPlaying->rowCount() > 0) {
            oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
        }
        
        //Update Now Playing view
        m_nowPlaying->clearMediaListData();
        // - Get album artwork
        QPixmap artwork = Utilities::getArtworkFromTag(nextMediaItem.url);
        if (!artwork.isNull()) {
            nextMediaItem.artwork = KIcon(artwork);
        }
        m_nowPlaying->loadMediaItem(nextMediaItem, true);
        
        //Refresh playlist to show currently playing item
        m_currentPlaylist->item(row,0)->setData(true, MediaItem::NowPlayingRole);
        if ((oldItemRow != -1) && (oldItemRow != row) && (oldItemRow < m_currentPlaylist->rowCount())) {
            m_currentPlaylist->item(oldItemRow, 0)->setData(false, MediaItem::NowPlayingRole);
        }
    } else if (model == Playlist::QueueModel) {
        //Get media item from playlist
        nextMediaItem = m_queue->mediaItemAt(row);
        nextMediaItem.nowPlaying = true;
        
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
        m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl(nextMediaItem.url)));
        m_mediaObject->play();
        m_playlistFinished = false;
        
        //Get row of previously playing item
        int oldItemRow = -1;
        if (m_nowPlaying->rowCount() > 0) {
            oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
        }
        
        //Update Now Playing view
        m_nowPlaying->clearMediaListData();       
        // - Get album artwork
        QPixmap artwork = Utilities::getArtworkFromTag(nextMediaItem.url);
        if (!artwork.isNull()) {
            nextMediaItem.artwork = KIcon(artwork);
        }
        m_nowPlaying->loadMediaItem(nextMediaItem, true);

        //Refresh playlist model to ensure views get updated
        int row = m_nowPlaying->mediaItemAt(0).playlistIndex;
        if (row >= 0) m_currentPlaylist->item(row,0)->setData(true, MediaItem::NowPlayingRole);
        if (oldItemRow != row and oldItemRow >= 0 and oldItemRow < m_currentPlaylist->rowCount()) {
            m_currentPlaylist->item(oldItemRow, 0)->setData(false, MediaItem::NowPlayingRole);
        }
    }
    
    
}

void Playlist::playNext()
{
    addToQueue();
    if (m_queue->rowCount() > 1) {
        m_queue->removeMediaItemAt(0);
        int oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
        m_playlistIndicesHistory.append(oldItemRow);
        playItemAt(0, Playlist::QueueModel);
    }
}

void Playlist::playPrevious()
{
    if (m_playlistIndicesHistory.count() > 0) {
        int previousRow = m_playlistIndicesHistory.last();
        playItemAt(previousRow, Playlist::PlaylistModel);
    }
}

void Playlist::start()
{
    m_playlistIndices.clear();
    m_playlistIndicesHistory.clear();
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
}

void Playlist::playMediaList(QList<MediaItem> mediaList)
{
    //Clear playlist and set to play
    m_currentPlaylist->clearMediaListData();
    playWhenPlaylistChanges = true;
    
    //Load playlist with all media items
    m_currentPlaylist->loadSources(mediaList); 
}

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
    }
    m_currentPlaylist->removeMediaItemAt(row);
}

void Playlist::clearPlaylist()
{
    m_currentPlaylist->clearMediaListData();
    m_queue->clearMediaListData();
    m_nowPlaying->clearMediaListData();
    m_playlistIndices.clear();
    m_playlistIndicesHistory.clear();
    m_mediaObject->stop();
}

void Playlist::queueNextPlaylistItem()
{
    addToQueue();
    if (m_queue->rowCount() == 1) {
        //remove old queue item
        m_playlistFinished = true;
        m_queue->removeMediaItemAt(0);
    }
    if (m_queue->rowCount() > 1) {
        m_queue->removeMediaItemAt(0);
        //Load next queued item
        MediaItem nextMediaItem = m_queue->mediaItemAt(0);
        QPixmap artwork = Utilities::getArtworkFromTag(nextMediaItem.url);
        if (!artwork.isNull()) {
            nextMediaItem.artwork = KIcon(artwork);
        }
        QList<QUrl> queue;
        queue << QUrl(nextMediaItem.url);
        m_mediaObject->setQueue(queue);
        m_nowPlaying->loadMediaItem(nextMediaItem);
    }
    
    //Get row of previously playing item and add to history
    int oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
    m_playlistIndicesHistory.append(oldItemRow);
    
    //Refresh playlist model to ensure views get updated
    int row = -1;
    m_nowPlaying->removeMediaItemAt(0, true); //remove old now playing item
    if (m_nowPlaying->rowCount() > 0) {
        row = m_nowPlaying->mediaItemAt(0).playlistIndex;
    }
    if ((row >= 0) && (row < m_currentPlaylist->rowCount())) {
        m_currentPlaylist->item(row,0)->setData(true, MediaItem::NowPlayingRole);
    }
    if (oldItemRow != row && oldItemRow >= 0) {
        //Cycle through true and false to ensure data change forces update
        m_currentPlaylist->item(oldItemRow,0)->setData(true, MediaItem::NowPlayingRole);
        m_currentPlaylist->item(oldItemRow,0)->setData(false, MediaItem::NowPlayingRole);
    }
    
}

void Playlist::updateNowPlaying(const Phonon::MediaSource & newSource)
{
    if (m_nowPlaying->rowCount() > 0) {
        if (newSource.url().toString() == m_nowPlaying->mediaItemAt(0).url) {
        }
    }
    
    //Update last played date and play count
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    Nepomuk::Resource res(newSource.url());
    if (res.exists()) {
        res.setProperty(mediaVocabulary.lastPlayed(), Nepomuk::Variant(QDateTime::currentDateTime()));
        int playCount = res.property(mediaVocabulary.playCount()).toInt();
        playCount = playCount + 1;
        res.setProperty(mediaVocabulary.playCount(), Nepomuk::Variant(playCount));        
    }
}

void Playlist::playlistChanged()
{
    if (playWhenPlaylistChanges && m_currentPlaylist->rowCount() > 0) {
        start();
        playWhenPlaylistChanges = false;
    } else if (m_currentPlaylist->rowCount() > 0){
        m_playlistIndices.clear();
        for (int i = 0; i < m_currentPlaylist->rowCount(); ++i) {
            m_playlistIndices.append(i);
        }
        for (int j = 0; j < m_playlistIndicesHistory.count(); ++j) {
            m_playlistIndices.removeAt(m_playlistIndicesHistory.at(j));
        }
        for (int k = 0; k < m_queue->rowCount(); ++k) {
            m_playlistIndices.removeAt(m_playlistIndices.indexOf(m_queue->mediaItemAt(k).playlistIndex));
        }
        for (int l = 0; l < qMin(m_queueDepth, m_playlistIndices.count()); ++l) {
            addToQueue();
        }
    }
}

void Playlist::setMode(int mode)
{
    if (mode <= 1) {
        m_mode = mode;
        m_playlistIndices.clear();
        m_playlistIndicesHistory.clear();
        for (int i = 0; i < m_currentPlaylist->rowCount(); ++i) {
            m_playlistIndices.append(i);
        }
        if (m_mode == Playlist::Normal) {
            orderByPlaylist();
        } else if (m_mode == Playlist::Shuffle) {
            shuffle();
        }
        if (m_mediaObject->state() == Phonon::PlayingState) {
            playItemAt(0, Playlist::QueueModel);
        } else {
            m_mediaObject->stop();
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

void Playlist::shuffle()
{
    m_queue->clearMediaListData();
    srand((unsigned)time(0));
    for (int i = 0; i < qMin(m_queueDepth, m_currentPlaylist->rowCount()); ++i) {
        addToQueue();
    }
}

void Playlist::orderByPlaylist()
{
    m_queue->clearMediaListData();
    for (int i = 0; i < qMin(m_queueDepth, m_currentPlaylist->rowCount()); ++i) {
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
        m_queue->clearMediaListData();
        for (int i = 0; i < playlistRow; ++i) {
            m_playlistIndicesHistory.append(i);
        }
        int lastRowOfQueue;
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