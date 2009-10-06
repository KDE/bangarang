#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <Phonon>

class MediaItemModel;
class MediaItem;

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
        void buildQueueFrom(int playlistRow);
        
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
        
    private slots:
        void currentSourceChanged(const Phonon::MediaSource & newSource);
        void titleChanged(int newTitle);
        void playlistChanged();
        void queueNextPlaylistItem();
        void confirmPlaylistFinished();
        
    Q_SIGNALS:
        void playlistFinished();
        
};
#endif // PLAYLIST_H
