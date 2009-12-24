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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <Phonon/MediaObject>
#include <Phonon/MediaController>

class MediaItemModel;
class MediaItem;
class MediaIndexer;

/**
 * This class provides MediaItemModels for a playlist and a queue.
 * It additionally provides an interface to playback MediaItems
 * in the playlist or the queue, shuffle and repeat.  It also
 * provides a MediaItemModel containing the currently playing
 * MediaItem.
 *
 * The queue MediaItemModel contains MediaItems in the order they 
 * will be played.
 * The playlist MediaItemModel is the model from which MediaItems
 * are added to the queue according to the Playlist::Mode.
 */
class Playlist : public QObject
{
    Q_OBJECT
    
    public:
        enum Mode { Normal = 0, Shuffle = 1};
        enum Model { PlaylistModel = 0, QueueModel = 1};
        enum State { Finished = 0, Loading = 1, Playing = 2};
        
        /**
         * Constructor
         *
         * @param mediaObject MediaObject the Playlist should use for playback
         */
        Playlist(QObject * parent, Phonon::MediaObject * mediaObject);
        
        /**
         * Destructor
         */
        ~Playlist();
        
        /**
         * Adds a list of MediaItems to the playlist
         *
         * @param mediaList List of MediaItems to add
         */
        void addMediaList(QList<MediaItem> mediaList);
        
        /**
         * Adds a MediaItem to playlist.
         *
         * @param mediaItem MediaItem to add.
         */
        void addMediaItem(MediaItem mediaItem);
        
        /**
         * Clears the playlist
         */
        void clearPlaylist();
        
        /**
         * Returns the loading state of the Playlist.
         *
         * @returns Playlist::State (see enum)
         */
        Playlist::State state();
        
        /**
         * Returns the Phonon::MediaObject used by Playlist.
         */
        Phonon::MediaObject * mediaObject();
        
        /**
         * Returns the current Playlist mode.
         *
         * @returns Playlist::Mode (see enum)
         */
        Playlist::Mode mode();
        
        /**
         * Returns the MediaItemModel containing the currently
         * playing MediaItem.
         */
        MediaItemModel * nowPlayingModel();
        
        /**
         * Plays item at the specified row of the specified model
         *
         * @param row row of the specified model
         * @param model either Playlist::PlaylistModel or Playlist::QueueModel
         */
        void playItemAt(int row, Playlist::Model model = Playlist::PlaylistModel);
        
        /**
         * Returns the MediaItemModel containing the list of MediaItems
         * in the playlist.
         */
        MediaItemModel * playlistModel();
        
        /**
         * Plays the specified list of MediaItems
         *
         * @param mediaList list of MediaItems to play
         */
        void playMediaList(QList<MediaItem> mediaList);
        
        /**
         * Returns the MediaItemModel containing the list of MediaItems
         * in the queue.
         */
        MediaItemModel * queueModel();
        
        /**
         * Removes MediaItem corresponding to the specified row in the 
         * playlist model. 
         *
         * @param row row of playlist model
         */
        void removeMediaItemAt(int row);
        
        /**
         * Sets the Playlist queuing mode
         *
         * @param mode mode to add MediaItems to the queue. 
         *             Either Playlist::Normal or Playlist::Shuffle.
         */
        void setMode(Playlist::Mode mode);
        
        /**
         * Sets whether or playback should repeat after playing
         * all items in the playlist.
         *
         * @param repeat true to repeat, false to end playback
         *               when all items in the playlist has 
         *               been played.
         */
        void setRepeat(bool repeat);
        
    public slots:
        /**
         * Play next MediaItem in queue.
         */
        void playNext();
        
        /**
         * Play previous MediaItem in queue.
         */
        void playPrevious();
        
        /**
        * Start playback.
        */
        void start();
        
        /**
        * Stop playback
        */
        void stop();
        
    Q_SIGNALS:
        /**
        * Emitted when loading MediaItems into the playlist.
        */
        void loading();
        
        /**
         * Emitting when all items in playlist have been played.
         */
        void playlistFinished();
        
    private:
        QObject * m_parent;
        MediaItemModel * m_currentPlaylist;
        MediaItemModel * m_nowPlaying;
        MediaItemModel * m_queue;
        Playlist::Mode m_mode;
        int m_repeat;
        int m_queueDepth;
        int m_oldPlaylistLength;
        QList<int> m_playlistIndices;
        QList<int> m_playlistIndicesHistory;
        QList<QString> m_playlistUrlHistory;
        Phonon::MediaObject * m_mediaObject;
        Phonon::MediaController * m_mediaController;
        bool playWhenPlaylistChanges;
        bool m_playlistFinished;
        void createUrlHistoryFromIndices();
        void updateNowPlaying();
        bool m_nepomukInited;
        Playlist::State m_state;
        MediaIndexer * m_mediaIndexer;
        bool m_playbackInfoWritten;
        void buildQueueFrom(int playlistRow);
        void shuffle();
        void orderByPlaylist();
        void addToQueue();
        bool m_hadVideo;
        
    private slots:
        void currentSourceChanged(const Phonon::MediaSource & newSource);
        void titleChanged(int newTitle);
        void playlistChanged();
        void queueNextPlaylistItem();
        void confirmPlaylistFinished();
        void stateChanged(Phonon::State newstate, Phonon::State oldstate);
        void updatePlaybackInfo(qint64 time);
        void metaDataChanged();
        
};
#endif // PLAYLIST_H
