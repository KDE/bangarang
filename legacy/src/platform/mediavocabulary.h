/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://commits.kde.org/bangarang>
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

#include <QtCore/QDate>
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
        QUrl typeMediaStream();
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
	QUrl tag();
        QUrl relatedTo();
        
        //These properties are applicable to Music
        QUrl musicArtist();
        QUrl musicPerformer();
        QUrl musicComposer();
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
        QString hasTypeAudioFeed(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeAnyAudio(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideo(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideoMovie(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideoTVShow(MediaQuery::Match match = MediaQuery::Required);
        QString hasTypeVideoFeed(MediaQuery::Match match = MediaQuery::Required);
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
        QString hasTag(MediaQuery::Match match = MediaQuery::Required,
                       const QString &tag = QString(),
		       MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasDescription(MediaQuery::Match match = MediaQuery::Required, 
                               const QString &description = QString(), 
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasResourceDescription(MediaQuery::Match match = MediaQuery::Required,
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
        QString hasRelatedTo(const QString&resourceBinding, MediaQuery::Match match = MediaQuery::Required,
                       const QUrl &related = QUrl(),
                       MediaQuery::Constraint constraint = MediaQuery::Equal);

        QString hasMusicAnyArtistName(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &artistName = QString(),
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicAnyArtistDescription(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &artistDescription = QString(),
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicArtistName(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &artistName = QString(), 
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicArtistDescription(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &artistDescription = QString(),
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicArtistArtwork(MediaQuery::Match match = MediaQuery::Required);
        QString hasMusicComposerName(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &composerName = QString(),
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicComposerDescription(MediaQuery::Match match = MediaQuery::Required,
                                   const QString &composerDescription = QString(),
                                   MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasMusicComposerArtwork(MediaQuery::Match match = MediaQuery::Required);
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
        QString hasVideoSeriesDescription(MediaQuery::Match match = MediaQuery::Required,
                                    const QString &seriesDescription = QString(),
                                    MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoSeriesArtwork(MediaQuery::Match match = MediaQuery::Required);
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
        QString hasVideoWriterDescription(MediaQuery::Match match = MediaQuery::Required,
                               const QString &writerDescription = QString(),
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoWriterArtwork(MediaQuery::Match match = MediaQuery::Required);
        QString hasVideoDirector(MediaQuery::Match match = MediaQuery::Required,
                                 const QString &director = QString(), 
                                 MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoDirectorDescription(MediaQuery::Match match = MediaQuery::Required,
                               const QString &directorDescription = QString(),
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoDirectorArtwork(MediaQuery::Match match = MediaQuery::Required);
        QString hasVideoProducer(MediaQuery::Match match = MediaQuery::Required,
                                 const QString &producer = QString(), 
                                 MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoProducerDescription(MediaQuery::Match match = MediaQuery::Required,
                               const QString &producerDescription = QString(),
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoProducerArtwork(MediaQuery::Match match = MediaQuery::Required);
        QString hasVideoActor(MediaQuery::Match match = MediaQuery::Required,
                              const QString &actor = QString(), 
                              MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoActorDescription(MediaQuery::Match match = MediaQuery::Required,
                               const QString &actorDescription = QString(),
                               MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoActorArtwork(MediaQuery::Match match = MediaQuery::Required);
        QString hasVideoAssistantDirector(MediaQuery::Match match = MediaQuery::Required,
                                          const QString &assistantDirector = QString(),
                                          MediaQuery::Constraint constraint = MediaQuery::Equal);
        QString hasVideoCinematographer(MediaQuery::Match match = MediaQuery::Required,
                                        const QString &cinematographer = QString(), 
                                        MediaQuery::Constraint constraint = MediaQuery::Equal);

        //SPARQL binding names
        static QString mediaResourceBinding();
        static QString mediaResourceUrlBinding();
        static QString artistResourceBinding();
        static QString composerResourceBinding();
        static QString albumResourceBinding();
        static QString videoSeriesResourceBinding();
        static QString writerResourceBinding();
        static QString directorResourceBinding();
        static QString producerResourceBinding();
        static QString actorResourceBinding();
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
        static QString tagBinding();
        static QString relatedToBinding();

        static QString musicArtistBinding();
        static QString musicArtistNameBinding();
        static QString musicArtistDescriptionBinding();
        static QString musicComposerNameBinding();
        static QString musicComposerDescriptionBinding();
        static QString musicArtistArtworkBinding();
        static QString musicAlbumBinding();
        static QString musicAlbumTitleBinding();
        static QString musicAlbumYearBinding();
        static QString musicTrackNumberBinding();
        static QString musicGenreBinding();
        
        static QString videoGenreBinding();
        static QString videoSeriesTitleBinding();
        static QString videoSeriesDescriptionBinding();
        static QString videoSeriesArtworkBinding();
        static QString videoSynopsisBinding();
        static QString videoSeasonBinding();
        static QString videoEpisodeNumberBinding();
        static QString videoAudienceRatingBinding();
        static QString videoWriterBinding();
        static QString videoWriterDescriptionBinding();
        static QString videoWriterArtworkBinding();
        static QString videoDirectorBinding();
        static QString videoDirectorDescriptionBinding();
        static QString videoDirectorArtworkBinding();
        static QString videoProducerBinding();
        static QString videoProducerDescriptionBinding();
        static QString videoProducerArtworkBinding();
        static QString videoActorBinding();
        static QString videoActorDescriptionBinding();
        static QString videoActorArtworkBinding();
        static QString videoAssistantDirectorBinding();
        static QString videoCinematographerBinding();
        static QString resourceBindingForCategory(const QString & categoryType);
        
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
