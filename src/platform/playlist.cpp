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
#include "utilities/utilities.h"
#include "mediavocabulary.h"
#include "mediaindexer.h"
#include <time.h>
#include <KApplication>
#include <KUrl>
#include <KIcon>
#include <KDebug>
#include <KStandardDirs>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QDBusInterface>
#include <Solid/Device>
#include <Solid/Block>
#include <Solid/PowerManagement>


Playlist::Playlist(QObject * parent, Phonon::MediaObject * mediaObject) : QObject(parent) 
{
    m_parent = parent;
    m_mediaController = NULL;
    m_currentPlaylist = new MediaItemModel(this);
    m_currentPlaylist->setSuppressNoResultsMessage(true);
    m_nowPlaying = new MediaItemModel(this);
    m_nowPlaying->setSuppressTooltip(true);
    m_nowPlaying->setSuppressNoResultsMessage(true);
    m_queue = new MediaItemModel(this);
    m_queue->setSuppressNoResultsMessage(true);
    playWhenPlaylistChanges = false;
    m_shuffle = false;
    m_repeat = false;
    m_queueDepth = 10;
    m_state = Playlist::Finished;
    m_hadVideo = false;
    m_notificationRestrictions = 0;
    m_filterProxyModel = new MediaSortFilterProxyModel();
    m_playbackInfoChecks = 0;
    m_powerManagementCookie = -1;
    
    setMediaObject(mediaObject);

    m_nepomukInited = Utilities::nepomukInited();
    if (m_nepomukInited) {
        m_mediaIndexer = new MediaIndexer(this);
    }
    
    connect(m_currentPlaylist, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    connect(m_currentPlaylist, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(playlistModelItemChanged(QStandardItem*)));
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
void Playlist::playItemAt(int row, Model model)
{
    bool isQueue = (model == QueueModel);
    MediaItem nextMediaItem = isQueue ? m_queue->mediaItemAt(row) :
                                        m_currentPlaylist->mediaItemAt(row);
    if (!isQueue) {
        nextMediaItem.playlistIndex = row;
    }
    nextMediaItem.nowPlaying = true;
    
    //Update Queue Model
    if (!m_shuffle) {
        //Just build a new queue from the row of the item in the playlist
        buildQueueFrom(nextMediaItem.playlistIndex);
    } else {
        int rowInQueue = isQueue ? row : m_queue->rowOfUrl(nextMediaItem.url);

        //Add currently playing item to history
        if (rowInQueue > 0 && m_nowPlaying->rowCount() > 0) {
            if (m_nowPlaying->mediaItemAt(0).type == "Audio" || m_nowPlaying->mediaItemAt(0).type == "Video") {
                int nowPlayingIndex = m_nowPlaying->mediaItemAt(0).playlistIndex;
                m_playlistIndicesHistory.append(nowPlayingIndex);
                m_playlistUrlHistory.append(m_nowPlaying->mediaItemAt(0).url);
                if (m_queue->rowCount() > 1) {
                    m_queue->removeMediaItemAt(0);
                    rowInQueue--;
                }
            }
        }

        //Remove requested item from history
        bool inHistory = (m_playlistIndicesHistory.indexOf(nextMediaItem.playlistIndex) != -1);
        if ( inHistory ) { //remove from history
            int idx = m_playlistIndicesHistory.indexOf(row);
            m_playlistIndicesHistory.removeAt(idx);
            m_playlistUrlHistory.removeAt(idx);
        }

        //Place requested item at front of queue
        QList<MediaItem> queueMediaList = m_queue->mediaList();
        if ( rowInQueue > 0 ) { //in queue, but not at first place, so move it
            queueMediaList.move(rowInQueue, 0);
        } else if (rowInQueue < 0) { //not in queue, so add it at first place
            queueMediaList.insert(0, nextMediaItem);
            if (queueMediaList.count() > m_queueDepth) {
                queueMediaList.removeLast();
            }
        } //else it is already at first place in the queue
        m_queue->clearMediaListData();
        m_queue->loadMediaList(queueMediaList, true);

        //Fill out queue
        shuffle();
    }
    
    //Play media Item
    m_mediaObject->clearQueue();
    m_currentStream.clear();
    QString subType;
    if (nextMediaItem.type == "Audio") {
        subType = nextMediaItem.fields["audioType"].toString();
    } else if(nextMediaItem.type == "Video") {
        subType = nextMediaItem.fields["videoType"].toString();
    }
    m_currentUrl = nextMediaItem.url;
    bool isDiscTitle = Utilities::isDisc( nextMediaItem.url );
    if (isDiscTitle) {
        Solid::Device device = Solid::Device( Utilities::deviceUdiFromUrl(nextMediaItem.url) );
        if (!device.isValid()) {
            stop();
            return;
        }
        const Solid::Block* block = device.as<const Solid::Block>();
        Phonon::DiscType discType = (subType == "CD Track") ? Phonon::Cd : Phonon::Dvd;
        Phonon::MediaSource src = Phonon::MediaSource(discType, block->device());
        int title = nextMediaItem.fields["trackNumber"].toInt();
        if (m_mediaObject->currentSource().discType() != src.discType() ||
            m_mediaObject->currentSource().deviceName() != src.deviceName()) {
            m_mediaObject->setCurrentSource(src);
        }
        if (title != -1) {
            m_mediaController->setCurrentTitle(title);
            m_mediaController->setAutoplayTitles(true);
        }
        m_mediaObject->play();
    } else if (subType == "Audio Stream") {
        m_currentStream = nextMediaItem.url;
        m_streamListUrls.clear();
        if (Utilities::isPls(nextMediaItem.url) || Utilities::isM3u(nextMediaItem.url)) {
            QList<MediaItem> streamList = Utilities::mediaListFromSavedList(nextMediaItem);
            for (int i = 0; i < streamList.count(); i++) {
                m_streamListUrls << streamList.at(i).url;
                if (i == 0) {
                    m_currentUrl = streamList.at(i).url;
                } else {
                    m_mediaObject->enqueue(Phonon::MediaSource(QUrl::fromPercentEncoding(streamList.at(i).url.toUtf8())));
                }
            }
        } else {
            m_streamListUrls << nextMediaItem.url;
        }
        m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromPercentEncoding(m_currentUrl.toUtf8())));
        m_mediaObject->play();
    } else {
        m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromEncoded(m_currentUrl.toUtf8())));
        m_mediaObject->play();
    }
    m_state = Playlist::Playing;
}

void Playlist::playNext()
{
    if (m_mediaObject->state() == Phonon::PlayingState || m_mediaObject->state() == Phonon::PausedState || m_mediaObject->state() == Phonon::LoadingState || m_mediaObject->state() == Phonon::ErrorState) {
        //Add currently playing item to history
        if (m_queue->rowCount() > 1) {
            playItemAt(1, Playlist::QueueModel);
        }
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
            MediaItem previousItem = m_currentPlaylist->mediaItemAt(previousRow);
            previousItem.playlistIndex = previousRow;
            m_queue->insertMediaItemAt(0, previousItem);
            
            playItemAt(0, Playlist::QueueModel);
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
    if (!m_shuffle) {
        orderByPlaylist();
    } else {
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
    m_currentPlaylist->loadSources(mediaList);
}

void Playlist::addMediaItem(const MediaItem &mediaItem)
{
    QList<MediaItem> mediaList;
    mediaList.append(mediaItem);
    m_currentPlaylist->loadSources(mediaList);
}

void Playlist::removeMediaItemAt(int row, bool emitMediaListChange)
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
    m_currentPlaylist->removeMediaItemAt(row, emitMediaListChange);
}

void Playlist::removeMediaListItems(const QList< MediaItem >& list)
{
    int count = list.count();
    for (int i = 0; i < count; i++) {
        const MediaItem &item = list.at(i);
        if (Utilities::isMedia(item.type)) {
            int playlistRow = m_currentPlaylist->rowOfUrl(item.url);
            if (playlistRow != -1) {
                removeMediaItemAt(playlistRow, (i == count -1 ) );
            }
        }
    }
}

void Playlist::insertMediaItemAt(int row, Model model, const MediaItem &mediaItem)
{
    if (model == PlaylistModel) {
        if (row >= m_currentPlaylist->rowCount()) {
            return;
        }
        m_currentPlaylist->insertMediaItemAt(row, mediaItem, true);
    } else {
        if (row >= m_queue->rowCount()) {
            return;
        }
        //Update history
        int playlistIndex = m_currentPlaylist->rowOfUrl(mediaItem.url);
        if (m_playlistIndicesHistory.contains(playlistIndex)) {
            m_playlistIndicesHistory.removeAll(playlistIndex);
            m_playlistUrlHistory.removeAll(mediaItem.url);
        }
        if (m_playlistIndices.contains(playlistIndex)) {
            m_playlistIndices.removeAll(playlistIndex);
        }
        //Update Playlist
        int playlistRow = -1;
        if (m_shuffle) {
            m_currentPlaylist->loadMediaItem(mediaItem, false);
            playlistRow = m_currentPlaylist->rowCount() - 1;
        } else {
            MediaItem itemAtRow = m_queue->mediaItemAt(row);
            playlistRow = m_currentPlaylist->rowOfUrl(itemAtRow.url);
            if (playlistRow != -1) {
                m_currentPlaylist->insertMediaItemAt(playlistRow, mediaItem, false);
            }
        }
        //Update queue
        MediaItem queueItem = mediaItem;
        queueItem.playlistIndex = playlistRow;
        m_queue->insertMediaItemAt(row, queueItem);
        if (m_queue->rowCount() > m_queueDepth) {
            m_queue->removeMediaItemAt(m_queue->rowCount() - 1);
        }
        if (row == 0 && (m_mediaObject->state() == Phonon::PlayingState ||
                         m_mediaObject->state() == Phonon::PausedState)) {
            playItemAt(0, QueueModel);
        }
    }
}

void Playlist::insertMediaListAt(int row, Model model, const QList<MediaItem> &mediaList)
{
    for (int i = mediaList.count()-1; i >= 0; i--) {
        insertMediaItemAt(row, model, mediaList.at(i));
    }
}

bool Playlist::isInPlaylist(const MediaItem &mediaItem)
{
    int row = m_currentPlaylist->rowOfUrl(mediaItem.url);
    if (row != -1) {
        return true;
    } else {
        return false;
    }

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
    if (m_mediaController != NULL) {
        delete m_mediaController;
    }
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

QSortFilterProxyModel * Playlist::filterProxyModel()
{
    return m_filterProxyModel;
}

void Playlist::setShuffleMode(bool shuffleMode)
{
    m_shuffle = shuffleMode;
    if (m_currentPlaylist->rowCount() > 0) {
        if (m_mediaObject->state() == Phonon::PlayingState || m_mediaObject->state() == Phonon::PausedState || m_mediaObject->state() == Phonon::LoadingState) {
            //Rebuild queue after currently playing item
            if (!m_shuffle) {
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
            if (!m_shuffle) {
                orderByPlaylist();
            } else {
                shuffle();
            }
        }
    }
    emit shuffleModeChanged(m_shuffle);
}

bool Playlist::shuffleMode()
{
    return m_shuffle;
}

void Playlist::setRepeatMode(bool repeat)
{
    bool wasRepeat = m_repeat;
    m_repeat = repeat;

    //If switching from repeat to not-repeat make sure queue is only built to end of playlist.
    //NOTE: In shuffle mode repeat the end of the playlist is ambiguous so no need to alter queue.
    if (wasRepeat && !repeat &&
        !m_shuffle &&
        m_queue->rowCount() > 0) {
        buildQueueFrom(m_queue->mediaItemAt(0).playlistIndex);
    }

    if (!m_shuffle) {
        orderByPlaylist();
    } else {
        shuffle();
    }
    emit repeatModeChanged(m_repeat);
}

bool Playlist::repeatMode()
{
    return m_repeat;
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
    
    if (m_queue->rowCount() == 1 && !m_repeat) {
        //Playlist is finished
        m_state = Playlist::Finished;
    } else {
        if (m_queue->rowCount() > 0 && Utilities::isAudioStream(m_queue->mediaItemAt(0).fields["audioType"].toString())){
            return;
        }
        m_queue->removeMediaItemAt(0);
        addToQueue();
        if (m_queue->rowCount() > 0) {
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
}

void Playlist::currentSourceChanged(const Phonon::MediaSource & newSource) //connected to MediaObject::currentSourceChanged
{
    //Update currentUrl and check next mediaItem to decide how to setAutoplayTitles
    if (newSource.type() == Phonon::MediaSource::Disc) {
        if (m_queue->rowCount() > 0) {
            if (Utilities::isDisc(m_queue->mediaItemAt(0).url)) {
                m_currentUrl = m_queue->mediaItemAt(0).url;
                if (m_queue->rowCount() >1) {
                    m_mediaController->setAutoplayTitles((m_queue->mediaItemAt(1).fields["trackNumber"].toInt() == m_mediaController->currentTitle() + 1));
                }
            }
        }
    } else {
        m_currentUrl = newSource.url().toString();
    }
    updateNowPlaying();
}

void Playlist::titleChanged(int newTitle) //connected to MediaController::titleChanged
{
    if ((m_queue->rowCount() > 1)) {
        MediaItem mediaItem = m_queue->mediaItemAt(1);
        if ((mediaItem.fields["trackNumber"].toInt() == newTitle) && (m_mediaObject->currentSource().type() == Phonon::MediaSource::Disc)) {
            m_currentUrl = mediaItem.url;
            m_queue->removeMediaItemAt(0);
        }
    }
    updateNowPlaying();
}

void Playlist::confirmPlaylistFinished() //connected to MediaObject::finished()
{
    if (m_state == Playlist::Finished) {
        m_mediaObject->stop();
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

    //If a pls or m3u based stream starts playing and there are other streams in the playlist
    //clear the queue to allow the other playlist streams to accessed.
    if (newstate == Phonon::PlayingState && nowPlayingModel()->rowCount() > 0) {
        if (Utilities::isAudioStream(nowPlayingModel()->mediaItemAt(0).fields["audioType"].toString())) {
            m_mediaObject->clearQueue();
            m_streamListUrls.clear();
        }
    }

    if (newstate == Phonon::PlayingState || newstate == Phonon::PausedState) {
        m_hadVideo = m_mediaObject->hasVideo();
        
        /*  The commented code below will be used to test phonon external subtitle support
         *  when phonon is updated to provide this function.
        QString directoryPath = KUrl(m_nowPlaying->mediaItemAt(0).url).directory();
        kDebug() << directoryPath;
        QDir dir(directoryPath);
        dir.setNameFilters(QStringList("*.srt"));
        QStringList dirList = dir.entryList(QDir::Files, QDir::Name);;
        if (dirList.count() > 0) {
            kDebug() << dirList.at(0);
            QHash<QByteArray, QVariant> properties;
            properties.insert("type", "file");
            properties.insert("name", directoryPath + QString("/") +dirList.at(0));
            int newSubtitleId = m_mediaController->availableSubtitles().count();
            Phonon::SubtitleDescription newSubtitle(newSubtitleId, properties);
            m_mediaController->setCurrentSubtitle(newSubtitle);
        }    
        
        QList<Phonon::SubtitleDescription> subtitles = m_mediaController->availableSubtitles();
        foreach (Phonon::SubtitleDescription cur, subtitles) {
            kDebug() << cur.name();
        }*/
        
    }
    
    //NOTE: In KDE 4.6, below is not the correct way to disable power saving.
    //TODO: Update to use new Solid power status api in KDE 4.6 and later.
    bool isKDE46OrGreater = false;
    if ((KDE::versionMinor() >= 6)) {
        isKDE46OrGreater = true;
    }
    
    if ((newstate == Phonon::PlayingState || newstate == Phonon::PausedState)
        && oldstate != Phonon::PlayingState
                && oldstate != Phonon::PausedState) {

        if (isKDE46OrGreater) {
            m_powerManagementCookie = Solid::PowerManagement::beginSuppressingScreenPowerManagement(i18n("Video Playback"));
        } else {
            QDBusInterface iface(
                        "org.kde.kded",
                        "/modules/powerdevil",
                        "org.kde.PowerDevil");
            iface.call("setProfile", "Presentation");
        }
        //Disable screensaver
        delete m_notificationRestrictions; //just to make sure more than one KNotificationRestrictions isn't created.
        m_notificationRestrictions = new KNotificationRestrictions(KNotificationRestrictions::ScreenSaver);

    } else if (newstate == Phonon::StoppedState &&
               (oldstate == Phonon::PlayingState || oldstate == Phonon::PausedState)){
        if (isKDE46OrGreater) {
            Solid::PowerManagement::stopSuppressingScreenPowerManagement(m_powerManagementCookie);
        } else {
            QDBusInterface iface(
                        "org.kde.kded",
                        "/modules/powerdevil",
                        "org.kde.PowerDevil");
            iface.call("refreshStatus");
        }
        if (m_notificationRestrictions) {
            delete m_notificationRestrictions;
            m_notificationRestrictions = 0;
        }
    }
}

void Playlist::updatePlaybackInfo(qint64 time)
{
    if (m_playbackInfoWritten) {
        return;
    }

    //Check to make sure playback passed through both the 5 second and 10 second mark.
    if (time >= 5000 && time <= 6000 && m_playbackInfoChecks == 0) {
        m_playbackInfoChecks++;
    } else if (time >= 10000 && time <= 11000 &&  m_playbackInfoChecks == 1) {
        m_playbackInfoChecks++;
    }

    if (m_playbackInfoChecks == 2) {
        //Update last played date and play count after 10 seconds
        if (m_nepomukInited && m_nowPlaying->rowCount() > 0) {
            MediaItem nowPlayingItem = m_nowPlaying->mediaItemAt(0);
            Nepomuk::Resource res(nowPlayingItem.url);
            nowPlayingItem.fields["playCount"] = nowPlayingItem.fields["playCount"].toInt() + 1;
            nowPlayingItem.fields["lastPlayed"] = QDateTime::currentDateTime();
            m_nowPlaying->replaceMediaItemAt(0, nowPlayingItem);
            if (res.exists()) {
                m_mediaIndexer->updatePlaybackInfo(m_nowPlaying->mediaItemAt(0).fields["resourceUri"].toString(), true, nowPlayingItem.fields["lastPlayed"].toDateTime());
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
        if (!m_shuffle) {
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
            if (m_currentUrl != QUrl::fromEncoded(m_queue->mediaItemAt(0).url.toUtf8()).toString()) {
                m_mediaObject->stop();
                playItemAt(0, Playlist::QueueModel);
            }
        }
    } else { //playlist was cleared
        stop();
    }
}

//--------------------------------
//--- Private Methods          ---
//--------------------------------

void Playlist::updateNowPlaying()
{
    //ACTUALLY UPDATE THE NOW PLAYING MODEL HERE!
    //THIS SHOULD BE THE ONLY PLACE THAT UPDATES THE NOW PLAYING MODEL!
    
    //Get row and url of previously playing item and add to history
    int oldItemRow = -1;
    if (m_nowPlaying->rowCount() > 0) {
        oldItemRow = m_nowPlaying->mediaItemAt(0).playlistIndex;
    }
    
    //Find matching item in queue
    int queueRow = -1;
    for (int i = 0; i < m_queue->rowCount(); i++) {
        if (Utilities::isAudioStream(m_queue->mediaItemAt(i).fields["audioType"].toString())) {
            if (m_currentStream == m_queue->mediaItemAt(i).url) {
                queueRow = i;
                break;
            }
        } else {
            if (m_currentUrl == QUrl::fromEncoded(m_queue->mediaItemAt(i).url.toUtf8()).toString()) {
                queueRow = i;
                break;
            }
        }
    }


    //Update Now Playing model
    MediaItem nowPlayingItem;
    if (queueRow != -1) {
        nowPlayingItem = m_queue->mediaItemAt(queueRow);

        //Get artwork
        QPixmap artwork = Utilities::getArtworkFromMediaItem(nowPlayingItem);
        if (!artwork.isNull()) {
            nowPlayingItem.artwork = KIcon(artwork);
        }
        //Create a file containing artwork so that it can, for example,
        //be exposed through the MPRIS dbus interface.
        if (nowPlayingItem.fields["artworkUrl"].toString().isEmpty()) {
            QString thumbnailTargetFile = QString("bangarang/thumbnails/nowPlaying-artwork.png");
            KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
            nowPlayingItem.artwork.pixmap(200,200).save(thumbnailTargetUrl.path(), "PNG");
            nowPlayingItem.fields["artworkUrl"] = thumbnailTargetUrl.path();
        }

        //Update model
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
    if ((oldItemRow != row && oldItemRow >= 0) && (oldItemRow < m_currentPlaylist->rowCount())) {
        //Cycle through true and false to ensure data change forces update
        m_currentPlaylist->item(oldItemRow,0)->setData(true, MediaItem::NowPlayingRole);
        m_currentPlaylist->item(oldItemRow,0)->setData(false, MediaItem::NowPlayingRole);
    }
    
    if (!Utilities::isDisc(nowPlayingItem.url)) {
        //Update last played date and play count
        if (m_nepomukInited && m_nowPlaying->rowCount() > 0) {
            m_playbackInfoChecks = 0;
            m_playbackInfoWritten = false; // Written 10 seconds later with updatePlaybackInfo()
        }
    }
}

void Playlist::shuffle()
{
    srand((unsigned)time(0));
    int numberToAdd = m_queueDepth - m_queue->rowCount();
    for (int i = 0; i < numberToAdd; ++i) {
        addToQueue();
    }
}

void Playlist::orderByPlaylist()
{
    int numberToAdd = m_queueDepth - m_queue->rowCount();
    for (int i = 0; i < numberToAdd; ++i) {
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
        int nextIndex;
        if (!m_shuffle) {
            nextIndex = m_playlistIndices.takeAt(0);
        } else {
            nextIndex = m_playlistIndices.takeAt(rand()%m_playlistIndices.count());
        }
        MediaItem nextMediaItem = m_currentPlaylist->mediaItemAt(nextIndex);
        nextMediaItem.playlistIndex = nextIndex;
        m_queue->loadMediaItem(nextMediaItem);
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
        for (int j = playlistRow; j < m_currentPlaylist->rowCount(); ++j) {
            m_playlistIndices.append(j);
        }

        orderByPlaylist();
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

void Playlist::playlistModelItemChanged(QStandardItem *item)
{
    //Update corresponding items in queue and now playing models
    MediaItem changedMediaItem = m_currentPlaylist->mediaItemAt(item->row());
    int rowInQueue = m_queue->rowOfUrl(changedMediaItem.url);
    if (rowInQueue != -1) {
        changedMediaItem.playlistIndex = item->row();
        m_queue->replaceMediaItemAt(rowInQueue,changedMediaItem);
    }
    int rowInNowPlaying = m_nowPlaying->rowOfUrl(changedMediaItem.url);
    if (rowInNowPlaying != -1) {
        QPixmap artwork = Utilities::getArtworkFromMediaItem(changedMediaItem);
        if (!artwork.isNull()) {
            changedMediaItem.artwork = KIcon(artwork);
        }
        m_nowPlaying->replaceMediaItemAt(rowInNowPlaying, changedMediaItem);
    }
}
