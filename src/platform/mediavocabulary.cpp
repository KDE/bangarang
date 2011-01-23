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
#include <kdeversion.h>
#include <KDebug>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>

MediaVocabulary::MediaVocabulary()
{
    if ((KDE::versionMinor() <= 3) && (KDE::versionRelease() < 83)) {
        m_vocabulary = MediaVocabulary::xesam;
        m_audioVocabulary = MediaVocabulary::nie;
        m_musicVocabulary = MediaVocabulary::xesam;
    } else {
        m_vocabulary = MediaVocabulary::nmm;
        m_audioVocabulary = MediaVocabulary::nmm;
        m_musicVocabulary = MediaVocabulary::nmm;
    }
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
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#");
    } else if (vocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#");
    } else if (vocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudio()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Audio();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Audio");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Audio");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudioMusic()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Music();
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#ID3Audio");
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#MusicPiece");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudioStream()
{
    QUrl returnUrl = QUrl();
    if (m_audioVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Audio();
    } else if (m_audioVocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#MediaStream");
    } else if (m_audioVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#DigitalRadio");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeAudioFeed()
{
    QUrl returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#AudioFeed");
    return returnUrl;
}

QUrl MediaVocabulary::typeVideo()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Video();
    } else if (m_videoVocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Video");
    } else if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#Video");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeVideoMovie()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#Movie");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeVideoTVShow()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#TVShow");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeVideoFeed()
{
    QUrl returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#VideoFeed");
    return returnUrl;
}

QUrl MediaVocabulary::typeTVSeries()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#TVSeries");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeImage()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Image();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeMusicArtist()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeMusicAlbum()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#MusicAlbum");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeNCOContact()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact");
}

QUrl MediaVocabulary::title()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::title();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::tag()
{
  QUrl returnUrl = QUrl();
  returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasTag");
  return returnUrl;
}

QUrl MediaVocabulary::relatedTo()
{
  return QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#relatedTo");
}

QUrl MediaVocabulary::description()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::description();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::duration()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::mediaDuration();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::lastPlayed()
{
    QUrl returnUrl = QUrl();
    //TODO:Waiting for Nepmouk ontology for useCount and lastUsed
    //if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2010/01/25/nuao#lastUsage");
    //}
    
    return returnUrl;
}

QUrl MediaVocabulary::playCount()
{
    QUrl returnUrl = QUrl();
    //TODO:Waiting for Nepmouk ontology for useCount and lastUsed
    //if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2010/01/25/nuao#usageCount");
    //}
    
    return returnUrl;
}

QUrl MediaVocabulary::artwork()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasLogicalPart");
}

QUrl MediaVocabulary::created()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::contentCreated();
    } else if (m_vocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated");
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::releaseDate()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::contentCreated();
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        //Draft nmm ontology is extension of nie
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#releaseDate");
    }
    
    return returnUrl;
    
}

QUrl MediaVocabulary::genre()
{
    QUrl returnUrl = QUrl();
    if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_vocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre");
    }
    return returnUrl;
}

QUrl MediaVocabulary::rating()
{
    return Soprano::Vocabulary::NAO::numericRating();
    
}

QUrl MediaVocabulary::ncoFullname()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname");
}

QUrl MediaVocabulary::musicArtist()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::artist();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#leadArtist");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicPerformer()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::artist();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#leadArtist");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicComposer()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::artist();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#composer");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#leadArtist");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicArtistName()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::artist();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicAlbum()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::album();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#albumTitle");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicAlbumName()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::album();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#albumTitle");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicAlbumYear()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::contentCreated();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#releaseDate");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#recordingYear");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicTrackNumber()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::trackNumber();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#trackNumber");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::musicGenre()
{
    QUrl returnUrl = QUrl();
    if (m_musicVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_musicVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre");
    } else if (m_musicVocabulary == MediaVocabulary::nid3) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#contentType");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoGenre()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeries()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#series");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeriesTitle()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSynopsis()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::genre();
    } else if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeason()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#season");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoEpisodeNumber()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#episodeNumber");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoAudienceRating()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#audienceRating");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoWriter()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#writer");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoDirector()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoAssistantDirector()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#assistantDirector");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoProducer()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#producer");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoActor()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#actor");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoCinematographer()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#cinematographer");
    }
    
    return returnUrl;
}

QString MediaVocabulary::hasTypeAudio(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeAudio());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
         statement = MediaQuery::addOptional(statement);
    }
    statement += MediaQuery::excludeType(resourceBinding, typeAudioMusic());
    statement += MediaQuery::excludeType(resourceBinding, typeAudioStream());
    statement += MediaQuery::excludeType(resourceBinding, typeAudioFeed());
    return statement;
    
}

QString MediaVocabulary::hasTypeAudioMusic(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeAudioMusic());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
         statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeAudioStream(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeAudioStream());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
         statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeAudioFeed(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeAudioFeed());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeAnyAudio(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = QString("{%1} UNION  {%2} UNION {%3} ")
                        .arg(MediaQuery::hasType(resourceBinding, typeAudio()))
                        .arg(MediaQuery::hasType(resourceBinding, typeAudioMusic()))
                        .arg(MediaQuery::hasType(resourceBinding, typeAudioStream()));
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeVideo(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = QString("{%1} UNION  {%2} ")
                        .arg(MediaQuery::hasType(resourceBinding, typeVideo()))
                        .arg(MediaQuery::hasType(resourceBinding, QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Video")));
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    statement += MediaQuery::excludeType(resourceBinding, typeVideoMovie());
    statement += MediaQuery::excludeType(resourceBinding, typeVideoTVShow());
    statement += MediaQuery::excludeType(resourceBinding, typeVideoFeed());
    return statement;
}

QString MediaVocabulary::hasTypeVideoMovie(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeVideoMovie());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeVideoTVShow(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeVideoTVShow());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeVideoFeed(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeVideoFeed());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeAnyVideo(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = QString("{%1} UNION  {%2} UNION {%3} ")
                        .arg(MediaQuery::hasType(resourceBinding, typeVideo()))
                        .arg(MediaQuery::hasType(resourceBinding, typeVideoMovie()))
                        .arg(MediaQuery::hasType(resourceBinding, typeVideoTVShow()));
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeImage(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeImage());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeTVSeries(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeTVSeries());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeMusicArtist(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeMusicArtist());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTypeMusicAlbum(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = MediaQuery::hasType(resourceBinding, typeMusicAlbum());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}


QString MediaVocabulary::hasResource(const QString &uri)
{
    QString resourceBinding = mediaResourceBinding();
    QString statement = QString("?%1 ?dummyPredicate ?dummyObject ").arg(resourceBinding);
    statement += QString("FILTER (?%1 = <%2> ) ").arg(resourceBinding)
                                                .arg(uri);
    statement += fileUrl(resourceBinding);
    return statement;
}

QString MediaVocabulary::hasUrl(MediaQuery::Match match, 
                                  const QString &url)
{
    QString statement = QString("?%1 nie:url ?%2 . ")
                        .arg(mediaResourceBinding())
                        .arg(mediaResourceUrlBinding());
    statement += QString("FILTER (?%1 = <%2> ) ")
                 .arg(mediaResourceUrlBinding())
                 .arg(url);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTitle(MediaQuery::Match match, 
                                  const QString &title, 
                                  MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = titleBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::title(), propertyBinding);
    if (!title.isEmpty()) {
        QStringList titles = title.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < titles.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, titles.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasTag(MediaQuery::Match match,
                                const QString &tag,
                                MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString tagResourceBinding = "tr";
    QString propertyBinding = tagBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::tag(), tagResourceBinding);
    statement += QString("?%1 nao:prefLabel ?%2 . ").arg(tagResourceBinding).arg(propertyBinding);
    if (!tag.isEmpty()) {
        QStringList tags = tag.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < tags.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, tags.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
      statement = MediaQuery::addOptional(statement);
    }
    
    return statement;
}

QString MediaVocabulary::hasDescription(MediaQuery::Match match, 
                                        const QString &description, 
                                        MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = descriptionBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::description(), propertyBinding);
    if (!description.isEmpty()) {
        QStringList descriptions = description.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < descriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, descriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasDuration(MediaQuery::Match match, 
                                     int duration,
                                     MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = durationBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::duration(), propertyBinding);
    if (duration != -2) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, duration, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasLastPlayed(MediaQuery::Match match, 
                      const QDateTime &lastPlayed, 
                      MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = lastPlayedBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::lastPlayed(), propertyBinding);
    if (lastPlayed.isValid()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, lastPlayed, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasPlayCount(MediaQuery::Match match, 
                     int playCount, 
                     MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = playCountBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::playCount(), propertyBinding);
    bool forceOptional = false;
    QString filter;
    if (playCount == 0 && (constraint == MediaQuery::Equal ||
                           constraint == MediaQuery::LessThanOrEqual ||
                           constraint == MediaQuery::LessThan ||
                           constraint == MediaQuery::GreaterThanOrEqual)) {
        filter = QString("FILTER ((bound(?%1) && %2) || !bound(?%1)) ")
                     .arg(propertyBinding)
                     .arg(MediaQuery::filterConstraint(propertyBinding, playCount, constraint));
        forceOptional = true;
    } else if (playCount > 0){
        filter = QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, playCount, constraint);
    }
    if (match == MediaQuery::Optional || forceOptional) {
        statement = MediaQuery::addOptional(statement);
    }
    statement += filter;
    return statement;
}

QString MediaVocabulary::hasArtwork(MediaQuery::Match match)
{
    QString resourceBinding = mediaResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = artworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    
    return statement;
}

QString MediaVocabulary::hasCreated(MediaQuery::Match match, 
                   const QDate &created, 
                   MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = createdBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::created(), propertyBinding);
    if (created.isValid()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, created, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasGenre(MediaQuery::Match match, 
              const QString &genre, 
              MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = genreBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::genre(), propertyBinding);
    if (!genre.isEmpty()) {
        QStringList genres = genre.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < genres.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, genres.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasReleaseDate(MediaQuery::Match match, 
                                        const QDate &releaseDate, 
                                        MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = releaseDateBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::releaseDate(), propertyBinding);
    if (releaseDate.isValid()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, releaseDate, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasRating(MediaQuery::Match match, 
                  int rating, 
                  MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = ratingBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::rating(), propertyBinding);
    bool forceOptional = false;
    QString filter;
    if (rating == 0 && (constraint == MediaQuery::Equal ||
                           constraint == MediaQuery::LessThanOrEqual ||
                           constraint == MediaQuery::LessThan ||
                           constraint == MediaQuery::GreaterThanOrEqual)) {
        filter = QString("FILTER ((bound(?%1) && %2) || !bound(?%1)) ")
                     .arg(propertyBinding)
                     .arg(MediaQuery::filterConstraint(propertyBinding, rating, constraint));
        forceOptional = true;
    } else if (rating > 0){
        filter = QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, rating, constraint);
    }
    if (match == MediaQuery::Optional || forceOptional) {
        statement = MediaQuery::addOptional(statement);
    }
    statement += filter;
    return statement;
}

QString MediaVocabulary::hasRelatedTo(const QString &resourceBinding, MediaQuery::Match match, const QUrl &related, MediaQuery::Constraint constraint)
{
    //QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = relatedToBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::relatedTo(), propertyBinding);
    if (related.isValid()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, related, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicAnyArtistName(MediaQuery::Match match,
                                            const QString &artistName,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicArtistNameBinding();
    QString statement;
    if (m_musicVocabulary == MediaVocabulary::nmm) {
        statement = QString("{%1} UNION  {%2} UNION {%3} ")
        .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtist(), artistResourceBinding()))
        .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicPerformer(), artistResourceBinding()))
        .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicComposer(), artistResourceBinding()));
        statement += MediaQuery::hasProperty(artistResourceBinding(), MediaVocabulary::musicArtistName(), propertyBinding);
    } else {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtistName(), propertyBinding);
    }

    if (!artistName.isEmpty()) {
        QStringList artistNames = artistName.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < artistNames.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, artistNames.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicAnyArtistDescription(MediaQuery::Match match,
                                            const QString &artistDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicArtistDescriptionBinding();
    QString statement;
    statement = QString("{%1} UNION  {%2} UNION {%3} ")
                .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtist(), artistResourceBinding()))
                .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicPerformer(), artistResourceBinding()))
                .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicComposer(), artistResourceBinding()));
    statement += MediaQuery::hasProperty(artistResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!artistDescription.isEmpty()) {
        QStringList artistDescriptions = artistDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < artistDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, artistDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicArtistName(MediaQuery::Match match,
                                            const QString &artistName, 
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicArtistNameBinding();
    QString statement;
    if (m_musicVocabulary == MediaVocabulary::nmm) {
        statement = QString("{%1} UNION  {%2} ")
                .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtist(), artistResourceBinding()))
                .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicPerformer(), artistResourceBinding()));
        statement += MediaQuery::hasProperty(artistResourceBinding(), MediaVocabulary::musicArtistName(), propertyBinding);
    } else {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtistName(), propertyBinding);
    }
        
    if (!artistName.isEmpty()) {
        QStringList artistNames = artistName.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < artistNames.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, artistNames.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicArtistDescription(MediaQuery::Match match,
                                            const QString &artistDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicArtistDescriptionBinding();
    QString statement;
    statement = QString("{%1} UNION  {%2} ")
            .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtist(), artistResourceBinding()))
            .arg(MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicPerformer(), artistResourceBinding()));
    statement += MediaQuery::hasProperty(artistResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!artistDescription.isEmpty()) {
        QStringList artistDescriptions = artistDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < artistDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, artistDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicArtistArtwork(MediaQuery::Match match)
{
    QString resourceBinding = artistResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = musicArtistArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasMusicComposerName(MediaQuery::Match match,
                                            const QString &composerName,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicComposerNameBinding();
    QString statement;
    if (m_musicVocabulary == MediaVocabulary::nmm) {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicComposer(), composerResourceBinding());
        statement += MediaQuery::hasProperty(composerResourceBinding(), MediaVocabulary::musicArtistName(), propertyBinding);
    } else {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtistName(), propertyBinding);
    }

    if (!composerName.isEmpty()) {
        QStringList composerNames = composerName.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < composerNames.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, composerNames.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicComposerDescription(MediaQuery::Match match,
                                            const QString &composerDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicComposerDescriptionBinding();
    QString statement;
    statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicComposer(), composerResourceBinding());
    statement += MediaQuery::hasProperty(composerResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!composerDescription.isEmpty()) {
        QStringList composerDescriptions = composerDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < composerDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, composerDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicComposerArtwork(MediaQuery::Match match)
{
    QString resourceBinding = composerResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = musicArtistArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasMusicAlbumTitle(MediaQuery::Match match,
                                            const QString &albumTitle,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicAlbumTitleBinding();
    QString statement;
    if (m_musicVocabulary == MediaVocabulary::nmm) {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicAlbum(), albumResourceBinding());
        statement += MediaQuery::hasProperty(albumResourceBinding(), MediaVocabulary::musicAlbumName(), propertyBinding);
    } else {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicAlbumName(), propertyBinding);
    }
    
    if (!albumTitle.isEmpty()) {
        QStringList albumTitles = albumTitle.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < albumTitles.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, albumTitles.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicAlbumYear(MediaQuery::Match match,
                                           int year, 
                                           MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicAlbumYearBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicAlbumYear(), propertyBinding);
    if (year != 0) {
        QDate yearDate = QDate(year, 1, 1);
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, yearDate, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasMusicTrackNumber(MediaQuery::Match match,
                                             int trackNumber, 
                                             MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = musicTrackNumberBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicTrackNumber(), propertyBinding);
    if (trackNumber != 0) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, trackNumber, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoSeriesTitle(MediaQuery::Match match,
                                             const QString &seriesTitle,
                                             MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoSeriesTitleBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoSeries(), videoSeriesResourceBinding());
    statement += MediaQuery::hasProperty(videoSeriesResourceBinding(), MediaVocabulary::videoSeriesTitle(), propertyBinding);
    if (!seriesTitle.isEmpty()) {
        QStringList seriesTitles = seriesTitle.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < seriesTitles.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, seriesTitles.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoSeriesDescription(MediaQuery::Match match,
                                            const QString &seriesDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoSeriesDescriptionBinding();
    QString seriesResourceBinding = videoSeriesResourceBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoSeries(), seriesResourceBinding);
    statement += MediaQuery::hasProperty(seriesResourceBinding, MediaVocabulary::description(), propertyBinding);
    if (!seriesDescription.isEmpty()) {
        QStringList seriesDescriptions = seriesDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < seriesDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, seriesDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoSeriesArtwork(MediaQuery::Match match)
{
    QString resourceBinding = videoSeriesResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = videoSeriesArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasVideoSynopsis(MediaQuery::Match match,
                                          const QString &synopsis,
                                          MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoSynopsisBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoSynopsis(), propertyBinding);
    if (!synopsis.isEmpty()) {
        QStringList synopses = synopsis.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < synopses.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, synopses.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}
                                              
QString MediaVocabulary::hasVideoSeason(MediaQuery::Match match,
                                        int season, 
                                        MediaQuery::Constraint constraint)
{
    //TODO:Modify for new 0.3+ sdo nmm ontology with Series class
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoSeasonBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoSeason(), propertyBinding);
    if (season != 0) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, season, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoEpisodeNumber(MediaQuery::Match match,
                                               int episodeNumber, 
                                               MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoEpisodeNumberBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoEpisodeNumber(), propertyBinding);
    if (episodeNumber != 0) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, episodeNumber, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoAudienceRating(MediaQuery::Match match,
                                                const QString &audienceRating, 
                                                MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoAudienceRatingBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoAudienceRating(), propertyBinding);
    if (!audienceRating.isEmpty()) {
        QStringList audienceRatings = audienceRating.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < audienceRatings.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, audienceRatings.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoWriter(MediaQuery::Match match,
                                        const QString &writer, 
                                        MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoWriterBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoWriter(), writerResourceBinding());
    statement += MediaQuery::hasProperty(writerResourceBinding(), MediaVocabulary::ncoFullname(), propertyBinding);
    if (!writer.isEmpty()) {
        QStringList writers = writer.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < writers.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, writers.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoWriterDescription(MediaQuery::Match match,
                                            const QString &writerDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoWriterDescriptionBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoWriter(), writerResourceBinding());
    statement += MediaQuery::hasProperty(writerResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!writerDescription.isEmpty()) {
        QStringList writerDescriptions = writerDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < writerDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, writerDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoWriterArtwork(MediaQuery::Match match)
{
    QString resourceBinding = writerResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = videoWriterArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasVideoDirector(MediaQuery::Match match,
                                          const QString &director, 
                                          MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoDirectorBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoDirector(), directorResourceBinding());
    statement += MediaQuery::hasProperty(directorResourceBinding(), MediaVocabulary::ncoFullname(), propertyBinding);
    if (!director.isEmpty()) {
        QStringList directors = director.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < directors.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, directors.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoDirectorDescription(MediaQuery::Match match,
                                            const QString &directorDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoDirectorDescriptionBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoDirector(), directorResourceBinding());
    statement += MediaQuery::hasProperty(directorResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!directorDescription.isEmpty()) {
        QStringList directorDescriptions = directorDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < directorDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, directorDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoDirectorArtwork(MediaQuery::Match match)
{
    QString resourceBinding = directorResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = videoDirectorArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasVideoProducer(MediaQuery::Match match,
                                          const QString &producer, 
                                          MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoProducerBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoProducer(), producerResourceBinding());
    statement += MediaQuery::hasProperty(producerResourceBinding(), MediaVocabulary::ncoFullname(), propertyBinding);
    if (!producer.isEmpty()) {
        QStringList producers = producer.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < producers.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, producers.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoProducerDescription(MediaQuery::Match match,
                                            const QString &producerDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoProducerDescriptionBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoProducer(), producerResourceBinding());
    statement += MediaQuery::hasProperty(producerResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!producerDescription.isEmpty()) {
        QStringList producerDescriptions = producerDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < producerDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, producerDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoProducerArtwork(MediaQuery::Match match)
{
    QString resourceBinding = producerResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = videoProducerArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasVideoActor(MediaQuery::Match match,
                                       const QString &actor, 
                                       MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoActorBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoActor(), actorResourceBinding());
    statement += MediaQuery::hasProperty(actorResourceBinding(), MediaVocabulary::ncoFullname(), propertyBinding);
    if (!actor.isEmpty()) {
        QStringList actors = actor.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < actors.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, actors.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoActorDescription(MediaQuery::Match match,
                                            const QString &actorDescription,
                                            MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoActorDescriptionBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoActor(), actorResourceBinding());
    statement += MediaQuery::hasProperty(actorResourceBinding(), MediaVocabulary::description(), propertyBinding);
    if (!actorDescription.isEmpty()) {
        QStringList actorDescriptions = actorDescription.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < actorDescriptions.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, actorDescriptions.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }

    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoActorArtwork(MediaQuery::Match match)
{
    QString resourceBinding = actorResourceBinding();
    QString artworkResourceBinding = "artworkResource";
    QString propertyBinding = videoActorArtworkBinding();
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::artwork(), artworkResourceBinding);
    statement += QString("?%1 nie:url ?%2 . ").arg(artworkResourceBinding).arg(propertyBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }

    return statement;
}

QString MediaVocabulary::hasVideoAssistantDirector(MediaQuery::Match match,
                                                   const QString &assistantDirector,
                                                   MediaQuery::Constraint constraint)

{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoAssistantDirectorBinding();
    QString contactResourceBinding = "assistantDirectorResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoAssistantDirector(), contactResourceBinding);
    statement += MediaQuery::hasProperty(contactResourceBinding, MediaVocabulary::ncoFullname(), propertyBinding);
    if (!assistantDirector.isEmpty()) {
        QStringList assistantDirectors = assistantDirector.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < assistantDirectors.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, assistantDirectors.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::hasVideoCinematographer(MediaQuery::Match match,
                                                 const QString &cinematographer, 
                                                 MediaQuery::Constraint constraint)
{
    QString resourceBinding = mediaResourceBinding();
    QString propertyBinding = videoCinematographerBinding();
    QString contactResourceBinding = "cinematographerResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoCinematographer(), contactResourceBinding);
    statement += MediaQuery::hasProperty(contactResourceBinding, MediaVocabulary::ncoFullname(), propertyBinding);
    if (!cinematographer.isEmpty()) {
        QStringList cinematographers = cinematographer.split("|OR|");
        statement += QString("FILTER (");
        for (int i = 0; i < cinematographers.count(); i++) {
            statement += QString("%1 || ").arg(MediaQuery::filterConstraint(propertyBinding, cinematographers.at(i), constraint));
        }
        statement.chop(4);
        statement += QString(") ");
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
    return statement;
}

QString MediaVocabulary::mediaResourceBinding()
{
    return "r";
}

QString MediaVocabulary::artistResourceBinding()
{
    return "artistResource";
}

QString MediaVocabulary::albumResourceBinding()
{
    return "albumResource";
}

QString MediaVocabulary::videoSeriesResourceBinding()
{
    return "videoSeriesResource";
}

QString MediaVocabulary::writerResourceBinding()
{
    return "writerResource";
}

QString MediaVocabulary::directorResourceBinding()
{
    return "directorResource";
}

QString MediaVocabulary::producerResourceBinding()
{
    return "producerResource";
}

QString MediaVocabulary::actorResourceBinding()
{
    return "actorResource";
}

QString MediaVocabulary::composerResourceBinding()
{
    return "composerResource";
}

QString MediaVocabulary::mediaResourceUrlBinding()
{
    return "url";
}

QString MediaVocabulary::titleBinding()
{
    return "title";
}

QString MediaVocabulary::tagBinding()
{
    return "tag";
}

QString MediaVocabulary::descriptionBinding()
{
    return "description";
}

QString MediaVocabulary::durationBinding()
{
    return "duration";
}

QString MediaVocabulary::lastPlayedBinding()
{
    return "lastPlayed";
}

QString MediaVocabulary::playCountBinding()
{
    return "playCount";
}

QString MediaVocabulary::artworkBinding()
{
    return "artwork";
}

QString MediaVocabulary::createdBinding()
{
    return "created";
}

QString MediaVocabulary::genreBinding()
{
    return "genre";
}

QString MediaVocabulary::releaseDateBinding()
{
    return "releaseDate";
}

QString MediaVocabulary::ratingBinding()
{
    return "rating";
}

QString MediaVocabulary::relatedToBinding()
{
    return "relatedTo";
}

QString MediaVocabulary::musicArtistBinding()
{
    return "artist";
}

QString MediaVocabulary::musicArtistNameBinding()
{
    return "artist";
}

QString MediaVocabulary::musicArtistDescriptionBinding()
{
    return "artistDescription";
}

QString MediaVocabulary::musicComposerNameBinding()
{
    return "composer";
}

QString MediaVocabulary::musicComposerDescriptionBinding()
{
    return "composerDescription";
}

QString MediaVocabulary::musicArtistArtworkBinding()
{
    return "artistArtwork";
}

QString MediaVocabulary::musicAlbumBinding()
{
    return "album";
}

QString MediaVocabulary::musicAlbumTitleBinding()
{
    return "albumTitle";
}

QString MediaVocabulary::musicAlbumYearBinding()
{
    return "albumYearDate";
}

QString MediaVocabulary::musicTrackNumberBinding()
{
    return "trackNumber";
}

QString MediaVocabulary::musicGenreBinding()
{
    return "genre";
}

QString MediaVocabulary::videoGenreBinding()
{
    return "genre";
}

QString MediaVocabulary::videoSeriesTitleBinding()
{
    return "seriesTitle";
}

QString MediaVocabulary::videoSeriesDescriptionBinding()
{
    return "videoSeriesDescription";
}

QString MediaVocabulary::videoSeriesArtworkBinding()
{
    return "videoSeriesArtwork";
}

QString MediaVocabulary::videoSynopsisBinding()
{
    return "synopsis";
}

QString MediaVocabulary::videoSeasonBinding()
{
    return "season";
}

QString MediaVocabulary::videoEpisodeNumberBinding()
{
    return "episodeNumber";
}

QString MediaVocabulary::videoAudienceRatingBinding()
{
    return "audienceRating";
}

QString MediaVocabulary::videoWriterBinding()
{
    return "writer";
}

QString MediaVocabulary::videoWriterDescriptionBinding()
{
    return "writerDescription";
}

QString MediaVocabulary::videoWriterArtworkBinding()
{
    return "writerArtwork";
}

QString MediaVocabulary::videoDirectorBinding()
{
    return "director";
}

QString MediaVocabulary::videoDirectorDescriptionBinding()
{
    return "directorDescription";
}

QString MediaVocabulary::videoDirectorArtworkBinding()
{
    return "directorArtwork";
}

QString MediaVocabulary::videoProducerBinding()
{
    return "producer";
}

QString MediaVocabulary::videoProducerDescriptionBinding()
{
    return "producerDescription";
}

QString MediaVocabulary::videoProducerArtworkBinding()
{
    return "producerArtwork";
}

QString MediaVocabulary::videoActorBinding()
{
    return "actor";
}

QString MediaVocabulary::videoActorDescriptionBinding()
{
    return "actorDescription";
}

QString MediaVocabulary::videoActorArtworkBinding()
{
    return "actorArtwork";
}

QString MediaVocabulary::videoCinematographerBinding()
{
    return "cinematographer";
}

QString MediaVocabulary::videoAssistantDirectorBinding()
{
    return "assistantDirector";
}

QString MediaVocabulary::resourceBindingForCategory(const QString & categoryType)
{
    if (categoryType == "Artist") {
        return artistResourceBinding();
    } else if (categoryType == "Album") {
        return albumResourceBinding();
    } else if (categoryType == "TV Series") {
        return videoSeriesResourceBinding();
    } else if (categoryType == "Actor") {
        return actorResourceBinding();
    } else if (categoryType == "Director") {
        return directorResourceBinding();
    } else if (categoryType == "Writer") {
        return writerResourceBinding();
    } else if (categoryType == "Producer") {
        return producerResourceBinding();
    } else {
        return QString();
    }

}

QStringList MediaVocabulary::storageProcedure(QUrl mediaProperty)
{
    if (mediaProperty == artwork()) {
        return QStringList() << QString("[Resource]::[Property]::[ResourceValue]") <<
        QString("[ResourceValue]::[Type]::%1")
               .arg("http://http://www.semanticdesktop.org/ontologies/nfo#Image") <<
               QString("[ResourceValue]::%1::[Value]")
               .arg("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url");
    } else if (mediaProperty == musicArtist()) {
        if (m_musicVocabulary == nmm) {
            return QStringList() << QString("[Resource]::[Property]::[ResourceValue]") <<
                                    QString("[ResourceValue]::[Type]::%1").arg(typeMusicArtist().toString()) <<
                                    QString("[ResourceValue]::%1::[Value]").arg(musicArtistName().toString());
        } else {
            return  QStringList() << QString("[Resource]::[Property]::[Value]");
        }
    } else if (mediaProperty == musicAlbum()) {
        if (m_musicVocabulary == nmm) {
            return QStringList() << QString("[Resource]::[Property]::[ResourceValue]") <<
                                    QString("[ResourceValue]::[Type]::%1").arg(typeMusicAlbum().toString()) <<
                                    QString("[ResourceValue]::%1::[Value]").arg(musicAlbumName().toString());
        } else {
            return QStringList() << QString("[Resource]::[Property]::[Value]");
        }
    } else if (mediaProperty == videoSeries()) {
        return QStringList() << QString("[Resource]::[Property]::[ResourceValue]") <<
                                QString("[ResourceValue]::[Type]::%1").arg(typeTVSeries().toString()) <<
                                QString("[ResourceValue]::%1::[Value]").arg(videoSeriesTitle().toString());
    } else if (mediaProperty == videoWriter() ||
        mediaProperty == videoDirector() ||
        mediaProperty == videoAssistantDirector() ||
        mediaProperty == videoProducer() ||
        mediaProperty == videoActor() ||
        mediaProperty == videoCinematographer() ) {
        return QStringList() << QString("[Resource]::[Property]::[ResourceValue]") <<
        QString("[ResourceValue]::[Type]::%1").arg(typeNCOContact().toString()) <<
        QString("[ResourceValue]::%1::[Value]").arg(ncoFullname().toString());
    } else {
        return QStringList() << QString("[Resource]::[Property]::[Value]");
    }
}

QString MediaVocabulary::fileUrl(const QString &resourceBinding)
{
    return MediaQuery::addOptional(QString("?%1 nie:url ?%2 . ")
                   .arg(resourceBinding)
                   .arg(mediaResourceUrlBinding()));
}
