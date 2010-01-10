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
#include "mediaquery.h"

class MediaVocabulary {
    
    public:
        MediaVocabulary();
        ~MediaVocabulary();
        
        enum MediaVocabularyNamespace { xesam = Qt::UserRole + 1,
        nie = Qt::UserRole + 2,
        nmm = Qt::UserRole + 3,
        nid3 = Qt::UserRole + 4};
        
        /*enum Constraint {Equal = 0, 
                         NotEqual = 1,
                         Contains = 2, 
                         GreaterThan = 3, 
                         LessThan = 4,
                         GreaterThanOrEqual = 5,
                         LessThanOrEqual = 6};
        
        enum Match {Required = 0, Optional = 1};*/
        
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
        QUrl typeMusicArtist();
        QUrl typeMusicAlbum();
        QUrl typeNCOContact();

        
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
        QUrl rating();
        QUrl ncoFullname();
        
        //These properties are applicable to Music
        QUrl musicArtist();
        QUrl musicArtistName();
        QUrl musicAlbum();
        QUrl musicAlbumName();
        QUrl musicAlbumYear();
        QUrl musicTrackNumber();
        QUrl musicGenre();
        
        //These properties are applicable to Movie and TV shows
        QUrl videoGenre();
        QUrl videoSeries();
        QUrl videoSeriesTitle();
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
        
        //SPARQL condition statements
        QString hasTypeAudio(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeAudioMusic(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeAudioStream(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeAnyAudio(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideo(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideoMovie(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideoTVShow(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeAnyVideo(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeImage(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeTVSeries(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeMusicArtist(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeMusicAlbum(MediaQuery::Match match = MediaQuery::Required);
        
        QString hasResource(const QString &uri);
        QString hasUrl(MediaQuery::Match match, 
                                        const QString &Url);
        QString hasTitle(MediaQuery::Match match = MediaQuery::Required, 
                         const QString &title = QString(), 
                         MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasDescription(MediaQuery::Match match = MediaQuery::Required, 
                               const QString &description = QString(), 
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasDuration(MediaQuery::Match match = MediaQuery::Required, 
                            int duration = -2, 
                            MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasLastPlayed(MediaQuery::Match match = MediaQuery::Required, 
                              const QDateTime &lastPlayed = QDateTime(), 
                              MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasPlayCount(MediaQuery::Match match = MediaQuery::Required, 
                             int playCount = -1, 
                             MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasArtwork(MediaQuery::Match match = MediaQuery::Required);
        QString hasCreated(MediaQuery::Match match = MediaQuery::Required, 
                           const QDate &created = QDate(), 
                           MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasGenre(MediaQuery::Match match = MediaQuery::Required, 
                      const QString &description = QString(), 
                      MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasReleaseDate(MediaQuery::Match match = MediaQuery::Required, 
                         const QDate &created = QDate(), 
                         MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasRating(MediaQuery::Match match = MediaQuery::Required, 
                             int rating = -1, 
                             MediaQuery::Constraint constraint = MediaQuery::Equal);
        
        QString hasMusicArtistName(MediaQuery::Match match = MediaQuery::Required, 
                                   const QString &artistName = QString(), 
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicAlbumTitle(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &albumTitle = QString(),
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicAlbumYear(MediaQuery::Match match = MediaQuery::Required,
                                  int year = 0, 
                                  MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicTrackNumber(MediaQuery::Match match = MediaQuery::Required,
                                    int trackNumber = 0, 
                                    MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoSeriesTitle(MediaQuery::Match match = MediaQuery::Required,
                                    const QString &seriesTitle = QString(),
                                    MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoSynopsis(MediaQuery::Match match = MediaQuery::Required,
                                 const QString &synopsis = QString(),
                                 MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoSeason(MediaQuery::Match match = MediaQuery::Required,
                               int season = 0, 
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoEpisodeNumber(MediaQuery::Match match = MediaQuery::Required,
                                      int episodeNumber = 0, 
                                      MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoAudienceRating(MediaQuery::Match match = MediaQuery::Required,
                                       const QString &audienceRating = QString(), 
                                       MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoWriter(MediaQuery::Match match = MediaQuery::Required,
                               const QString &writer = QString(), 
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoDirector(MediaQuery::Match match = MediaQuery::Required,
                                 const QString &director = QString(), 
                                 MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoAssistantDirector(MediaQuery::Match match = MediaQuery::Required,
                                          const QString &assistantDirector = QString(), 
                                          MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoProducer(MediaQuery::Match match = MediaQuery::Required,
                                 const QString &producer = QString(), 
                                 MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoActor(MediaQuery::Match match = MediaQuery::Required,
                              const QString &actor = QString(), 
                              MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoCinematographer(MediaQuery::Match match = MediaQuery::Required,
                                        const QString &cinematographer = QString(), 
                                        MediaQuery::Constraint constraint = MediaQuery::Equal);

        //SPARQL binding names
        static QString mediaResourceBinding();
        static QString mediaResourceUrlBinding();
        static QString titleBinding();
        static QString descriptionBinding();
        static QString durationBinding();
        static QString lastPlayedBinding();
        static QString playCountBinding();
        static QString artworkBinding();
        static QString createdBinding();
        static QString genreBinding();
        static QString releaseDateBinding();
        static QString ratingBinding();
        
        static QString musicArtistBinding();
        static QString musicArtistNameBinding();
        static QString musicAlbumBinding();
        static QString musicAlbumTitleBinding();
        static QString musicAlbumYearBinding();
        static QString musicTrackNumberBinding();
        static QString musicGenreBinding();
        
        static QString videoGenreBinding();
        static QString videoSeriesTitleBinding();
        static QString videoSynopsisBinding();
        static QString videoSeasonBinding();
        static QString videoEpisodeNumberBinding();
        static QString videoAudienceRatingBinding();
        static QString videoWriterBinding();
        static QString videoDirectorBinding();
        static QString videoAssistantDirectorBinding();
        static QString videoProducerBinding();
        static QString videoActorBinding();
        static QString videoCinematographerBinding();
        
        //RDF storage procedure lookup
        QStringList storageProcedure(QUrl mediaProperty);

    private:
        int m_vocabulary;
        int m_audioVocabulary;
        int m_videoVocabulary;
        int m_musicVocabulary;
        
        QUrl m_namespace;
        QUrl m_audioNameSpace;
        QUrl m_musicNameSpace;
        QUrl m_videoNameSpace;
        
        QString fileUrl(const QString &resourceBinding);
};

#endif // MEDIAVOCABULARY_H