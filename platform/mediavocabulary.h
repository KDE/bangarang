#ifndef MEDIAVOCABULARY_H
#define MEDIAVOCABULARY_H

#include <QtCore>

class MediaVocabulary {
    
    public:
        MediaVocabulary();
        ~MediaVocabulary();
        
        enum MediaVocabularies { xesam = Qt::UserRole + 1,
        nie = Qt::UserRole + 2,
        nmm = Qt::UserRole + 3,
        nid3 = Qt::UserRole + 4};
        
        void setVocabulary(int vocabulary);
        void setAudioVocabulary(int vocabulary);
        void setMusicVocabulary(int vocabulary);
        void setVideoVocabulary(int vocabulary);
        int vocabulary();
        int audioVocabulary();
        int musicVocabulary();
        int videoVocabulary();
        QUrl mediaNamespace(int vocabulary);
        
        // Media types 
        QUrl typeAudio();
        QUrl typeAudioMusic();
        QUrl typeAudioStream();
        QUrl typeVideo();
        QUrl typeVideoMovie();
        QUrl typeVideoSeries();
        
        //These properties are applicable to all media types
        QUrl title();
        QUrl description();
        QUrl duration();
        QUrl lastPlayed();
        QUrl playCount();
        
        QUrl musicArtist();
        QUrl musicArtistName();
        QUrl musicAlbum();
        QUrl musicAlbumName();
        QUrl musicAlbumYear();
        QUrl musicTrackNumber();
        QUrl musicGenre();
        
        QUrl videoGenre();
        QUrl videoSeriesName();
        QUrl videoSeriesSeason();
        QUrl videoSeriesEpisode();
        
    private:
        int m_vocabulary;
        int m_audioVocabulary;
        int m_videoVocabulary;
        int m_musicVocabulary;
        
        QUrl m_namespace;
        QUrl m_audioNameSpace;
        QUrl m_musicNameSpace;
        QUrl m_videoNameSpace;
                
};
        
#endif // MEDIAVOCABULARY_H