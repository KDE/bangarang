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

#ifndef MEDIAVOCABULARY_H
#define MEDIAVOCABULARY_H

#include <QtCore>

class MediaVocabulary {
    
    public:
        MediaVocabulary();
        ~MediaVocabulary();
        
        enum MediaVocabularyNamespace { xesam = Qt::UserRole + 1,
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
        QUrl typeVideoTVShow();
        QUrl typeImage();
        
        //Media-related types
        QUrl typeTVSeries();
        
        //These properties are applicable to all media types
        QUrl title();
        QUrl description();
        QUrl duration();
        QUrl lastPlayed();
        QUrl playCount();
        QUrl artwork();
        QUrl created();
        QUrl genre();
        QUrl releaseDate();
        
        QUrl musicArtist();
        QUrl musicArtistName();
        QUrl musicAlbum();
        QUrl musicAlbumName();
        QUrl musicAlbumYear();
        QUrl musicTrackNumber();
        QUrl musicGenre();
        
        QUrl videoGenre();
        QUrl videoSeries();
        QUrl videoSynopsis();
        QUrl videoSeason();
        QUrl videoEpisodeNumber();
        QUrl videoAudienceRating();
        QUrl videoWriter();
        QUrl videoDirector();
        QUrl videoAssistantDirector();
        QUrl videoProducer();
        QUrl videoActor();
        QUrl videoCinematographer();
        
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