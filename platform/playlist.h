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

class Playlist : public QObject
{
    Q_OBJECT
    
    public:
        enum Mode { Normal = 0, Shuffle = 1};
        enum Model { PlaylistModel = 0, QueueModel = 1};
        enum State { LoadingComplete = 0, Loading = 1};
        Playlist(QObject * parent, Phonon::MediaObject * mediaObject);
        ~Playlist();
        MediaItemModel * playlistModel();
        MediaItemModel * queueModel();
        MediaItemModel * nowPlayingModel();
        Phonon::MediaObject * mediaObject();
        void start();
        void playItemAt(int row, int model = 0);
        void stop();
        void playMediaList(QList<MediaItem> mediaList);
        void addMediaList(QList<MediaItem> mediaList);
        void addMediaItem(MediaItem mediaItem);
        void removeMediaItemAt(int row);
        void clearPlaylist();
        void setMode(int mode);
        int mode();
        void shuffle();
        void orderByPlaylist();
        void addToQueue();
        void setRepeat(bool repeat);
        void buildQueueFrom(int playlistRow);
        int loadingState();
        
    private:
        QObject * m_parent;
        MediaItemModel * m_currentPlaylist;
        MediaItemModel * m_nowPlaying;
        MediaItemModel * m_queue;
        int m_mode;
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
        int m_loadingState;
        
    public slots:
        void playNext();
        void playPrevious();
        
    private slots:
        void currentSourceChanged(const Phonon::MediaSource & newSource);
        void titleChanged(int newTitle);
        void playlistChanged();
        void queueNextPlaylistItem();
        void confirmPlaylistFinished();
        void stateChanged(Phonon::State newstate, Phonon::State oldstate);
        
    Q_SIGNALS:
        void playlistFinished();
        void loading();
        
};
#endif // PLAYLIST_H
