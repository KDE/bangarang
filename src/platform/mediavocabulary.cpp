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

#include "mediavocabulary.h"
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>


MediaVocabulary::MediaVocabulary()
{
    m_vocabulary = MediaVocabulary::xesam;
    m_audioVocabulary = MediaVocabulary::nie;
    m_musicVocabulary = MediaVocabulary::xesam;
    m_videoVocabulary = MediaVocabulary::nmm;
}

MediaVocabulary::~MediaVocabulary() 
{
}

void MediaVocabulary::setVocabulary(int vocabulary)
{
    if (vocabulary == MediaVocabulary::xesam) {
        m_vocabulary = MediaVocabulary::xesam;
    } else if (vocabulary == MediaVocabulary::nie) {
        m_vocabulary = MediaVocabulary::nie;
    } else if (vocabulary == MediaVocabulary::nmm) {
        m_vocabulary = MediaVocabulary::nmm;
    }
}

void MediaVocabulary::setAudioVocabulary(int vocabulary)
{
    if (vocabulary == MediaVocabulary::xesam) {
        m_audioVocabulary = MediaVocabulary::xesam;
    } else if (vocabulary == MediaVocabulary::nie) {
        m_audioVocabulary = MediaVocabulary::nie;
    } else if (vocabulary == MediaVocabulary::nmm) {
        m_audioVocabulary = MediaVocabulary::nmm;
    }
}

void MediaVocabulary::setMusicVocabulary(int vocabulary)
{
    if (vocabulary == MediaVocabulary::xesam) {
        m_musicVocabulary = MediaVocabulary::xesam;
    } else if (vocabulary == MediaVocabulary::nid3) {
        m_vocabulary = MediaVocabulary::nid3;
    } else if (vocabulary == MediaVocabulary::nmm) {
        m_musicVocabulary = MediaVocabulary::nmm;
    }
}

void MediaVocabulary::setVideoVocabulary(int vocabulary)
{
    if (vocabulary == MediaVocabulary::xesam) {
        m_videoVocabulary = MediaVocabulary::xesam;
    } else if (vocabulary == MediaVocabulary::nie) {
        m_videoVocabulary = MediaVocabulary::nie;
    } else if (vocabulary == MediaVocabulary::nmm) {
        m_videoVocabulary = MediaVocabulary::nmm;
    }
}

int MediaVocabulary::vocabulary()
{
    return m_vocabulary;
}

int MediaVocabulary::audioVocabulary()
{
    return m_audioVocabulary;
}

int MediaVocabulary::musicVocabulary()
{
    return m_musicVocabulary;
}

int MediaVocabulary::videoVocabulary()
{
    return m_videoVocabulary;
}

QUrl MediaVocabulary::mediaNamespace(int vocabulary)
{
    QUrl returnUrl = QUrl();
    if (vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::xesamNamespace();
    } else if (vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#");
    } else if (vocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#");
    } else if (vocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudio()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Audio();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Audio");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Audio");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudioMusic()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Music();
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#ID3Audio");
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#MusicPiece");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudioStream()
{
    QUrl returnUrl = QUrl();
    if (m_audioVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Audio();
    } else if (m_audioVocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#MediaStream");
    } else if (m_audioVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#DigitalRadio");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeVideo()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Video();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Video");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Bangarang uses nfo:Video not nmm:Video - can't see semantic benefit
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Video");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeImage()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Image();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Image");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Image");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::title()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::title();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#title");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#title");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::description()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::description();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#description");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#description");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::duration()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::mediaDuration();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#duration");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#duration");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::lastPlayed()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::lastUsed();
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::playCount()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::useCount();
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::artwork()
{
    QUrl returnUrl = QUrl();
    
    //Bangarang extension to nmm ontology draft
    returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#artwork");
    
    return returnUrl;
}

QUrl MediaVocabulary::created()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::contentCreated();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#contentCreated");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#contentCreated");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::genre()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#genre");
    }
    return returnUrl;
}

QUrl MediaVocabulary::musicArtist()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::artist();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#Artist");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#leadArtist");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicArtistName()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::artist();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#Artist");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nco#fullName");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicAlbum()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::album();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#musicAlbum");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#albumTitle");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicAlbumName()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::album();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#albumTitle");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#albumTitle");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicAlbumYear()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::contentCreated();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nie#contentCreated");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#recordingYear");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicTrackNumber()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::trackNumber();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#trackNumber");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#trackNumber");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicGenre()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#genre");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nid3#contentType");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoIsMovie()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        //Bangarang extension to nmm ontology draft
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#isMovie");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoIsTVShow()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        //Bangarang extension to nmm ontology draft
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#isTVShow");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoGenre()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#genre");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeriesName()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        //Bangarang extension of nmm ontology draft
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#seriesName");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeriesSeason()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#season");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeriesEpisode()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#episodeNumber");
    }
    
    return returnUrl;
}

