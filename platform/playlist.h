#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <Phonon>
#include "mediaitemmodel.h"

class Playlist : public QObject
{
    Q_OBJECT
    
    public:
        enum Mode { Normal = 0, Shuffle = 1};
        enum Model { PlaylistModel = 0, QueueModel = 1};
        Playlist(QObject * parent, Phonon::MediaObject * mediaObject);
        ~Playlist();
        MediaItemModel * playlistModel();
        MediaItemModel * queueModel();
        MediaItemModel * nowPlayingModel();
        void start();
        void playItemAt(int row, int model = 0);
        void playNext();
        void playPrevious();
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
        Phonon::MediaObject * m_mediaObject;
        bool playWhenPlaylistChanges;
        bool m_playlistFinished;
        
    private slots:
        void updateNowPlaying(const Phonon::MediaSource & newSource);
        void playlistChanged();
        void queueNextPlaylistItem();
        
};
#endif // PLAYLIST_H
