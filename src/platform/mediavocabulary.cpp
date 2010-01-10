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
    if (m_videoVocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::Video();
    } else if (m_videoVocabulary == MediaVocabulary::nie) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Video");
    } else if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Video");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeVideoMovie()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#Movie");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeVideoTVShow()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#TVShow");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::typeTVSeries()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#TVSeries");
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
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#MusicAlbum");
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
    //TODO:Waiting for Nepmouk ontology for useCount and lastUsed
    //if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::lastUsed();
    //}
    
    return returnUrl;
}

QUrl MediaVocabulary::playCount()
{
    QUrl returnUrl = QUrl();
    //TODO:Waiting for Nepmouk ontology for useCount and lastUsed
    //if (m_vocabulary == MediaVocabulary::xesam) {
        returnUrl = Soprano::Vocabulary::Xesam::useCount();
    //}
    
    return returnUrl;
}

QUrl MediaVocabulary::artwork()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/nmm#artwork");
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
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#releaseDate");
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
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
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
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#releaseDate");
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

QUrl MediaVocabulary::videoSeries()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#series");
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
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#synopsis");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoSeason()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#season");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoEpisodeNumber()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#episodeNumber");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoAudienceRating()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#audienceRating");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoWriter()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#writer");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoDirector()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#director");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoAssistantDirector()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#assistantDirector");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoProducer()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#producer");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoActor()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#actor");
    }
    
    return returnUrl;
}

QUrl MediaVocabulary::videoCinematographer()
{
    QUrl returnUrl = QUrl();
    if (m_videoVocabulary == MediaVocabulary::nmm) {
        returnUrl = QUrl("http://www.semanticdesktop.org/ontologies/nmm#cinematographer");
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
    QString statement = MediaQuery::hasType(resourceBinding, typeVideo());
    statement += fileUrl(resourceBinding);
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
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
    QString statement = MediaQuery::hasType(resourceBinding, typeVideoTVShow());
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, title, constraint);
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, description, constraint);
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
    if (playCount != -1) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, playCount, constraint);
    }
    if (match == MediaQuery::Optional) {
        statement = MediaQuery::addOptional(statement);
    }
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, genre, constraint);
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
    if (rating != -1) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, rating, constraint);
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
        QString artistResourceBinding = "artistResource";
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtist(), artistResourceBinding);
        statement += MediaQuery::hasProperty(artistResourceBinding, MediaVocabulary::musicArtistName(), propertyBinding);
    } else {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicArtistName(), propertyBinding);
    }
        
    if (!artistName.isEmpty()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, artistName, constraint);
    }
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
        QString albumResourceBinding = "albumResource";
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicAlbum(), albumResourceBinding);
        statement += MediaQuery::hasProperty(albumResourceBinding, MediaVocabulary::musicAlbumName(), propertyBinding);
    } else {
        statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::musicAlbumName(), propertyBinding);
    }
    
    if (!albumTitle.isEmpty()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, albumTitle, constraint);
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
    QString seriesResourceBinding = "seriesResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoSeries(), seriesResourceBinding);
    statement += MediaQuery::hasProperty(seriesResourceBinding, MediaVocabulary::videoSeriesTitle(), propertyBinding);
    if (!seriesTitle.isNull()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, seriesTitle, constraint);
    }
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, synopsis, constraint);
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, audienceRating, constraint);
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
    QString contactResourceBinding = "writerResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoWriter(), contactResourceBinding);
    statement += MediaQuery::hasProperty(contactResourceBinding, MediaVocabulary::ncoFullname(), propertyBinding);
    if (!writer.isEmpty()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, writer, constraint);
    }
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
    QString contactResourceBinding = "directorResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoDirector(), contactResourceBinding);
    statement += MediaQuery::hasProperty(contactResourceBinding, MediaVocabulary::ncoFullname(), propertyBinding);
    if (!director.isEmpty()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, director, constraint);
    }
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, assistantDirector, constraint);
    }
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
    QString contactResourceBinding = "producerResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoProducer(), contactResourceBinding);
    statement += MediaQuery::hasProperty(contactResourceBinding, MediaVocabulary::ncoFullname(), propertyBinding);
    if (!producer.isEmpty()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, producer, constraint);
    }
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
    QString contactResourceBinding = "actorResource";
    QString statement = MediaQuery::hasProperty(resourceBinding, MediaVocabulary::videoActor(), contactResourceBinding);
    statement += MediaQuery::hasProperty(contactResourceBinding, MediaVocabulary::ncoFullname(), propertyBinding);
    if (!actor.isEmpty()) {
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, actor, constraint);
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
        statement += QString("FILTER ") + MediaQuery::filterConstraint(propertyBinding, cinematographer, constraint);
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

QString MediaVocabulary::mediaResourceUrlBinding()
{
    return "url";
}

QString MediaVocabulary::titleBinding()
{
    return "title";
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

QString MediaVocabulary::musicArtistBinding()
{
    return "artist";
}

QString MediaVocabulary::musicArtistNameBinding()
{
    return "artist";
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

QString MediaVocabulary::videoDirectorBinding()
{
    return "director";
}

QString MediaVocabulary::videoAssistantDirectorBinding()
{
    return "assistantDirector";
}

QString MediaVocabulary::videoProducerBinding()
{
    return "producer";
}

QString MediaVocabulary::videoActorBinding()
{
    return "actor";
}

QString MediaVocabulary::videoCinematographerBinding()
{
    return "cinematographer";
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
