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

#include "mediaindexer.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <KUrl>
#include <KDebug>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>

#include <QApplication>

MediaIndexerJob::MediaIndexerJob(QObject * parent) : QThread(parent)
{
}

MediaIndexerJob::~MediaIndexerJob()
{
}

void MediaIndexerJob::run()
{
    emit percentComplete(0);
    if (m_indexType == MediaIndexer::IndexMediaItem) {
        for (int i = 0; i < m_mediaList.count(); ++i) {
            indexMediaItem(m_mediaList.at(i));
            emit sourceInfoUpdated(m_mediaList.at(i));
            emit percentComplete(100*i/m_mediaList.count());
        }
    } else if (m_indexType == MediaIndexer::RemoveInfo) {
        for (int i = 0; i < m_mediaList.count(); ++i) {
            removeInfo(m_mediaList.at(i));     
            emit urlInfoRemoved(m_mediaList.at(i).url);
            emit percentComplete(100*i/m_mediaList.count());
        }
    } else if (m_indexType == MediaIndexer::WritePlaybackInfo) {
        MediaVocabulary mediaVocabulary = MediaVocabulary();
        kDebug() << "start writing playback info";
        Nepomuk::Resource res(m_url);
        if (res.exists()) {
            kDebug() << "setting last played";
            res.setProperty(mediaVocabulary.lastPlayed(), Nepomuk::Variant(m_playDateTime));
            if (m_incrementPlayCount) {
                int playCount = res.property(mediaVocabulary.playCount()).toInt();
                playCount = playCount + 1;
                kDebug() << "setting playcount";
                res.setProperty(mediaVocabulary.playCount(), Nepomuk::Variant(playCount));        
            }
        }
        kDebug() << "done writing playback info";
    }
    emit percentComplete(100);
    m_mediaList.clear();
    emit jobComplete();
}

void MediaIndexerJob::setMediaListToIndex(QList<MediaItem> mediaList)
{
    m_mediaList = mediaList;
    m_indexType = MediaIndexer::IndexMediaItem;
}

void MediaIndexerJob::setInfoToRemove(QList<MediaItem> mediaList)
{
    m_mediaList = mediaList;
    m_indexType = MediaIndexer::RemoveInfo;
}

void MediaIndexerJob::writePlaybackInfo(QString url, bool incrementPlayCount, QDateTime playDateTime)
{
    m_url = url;
    m_incrementPlayCount = incrementPlayCount;
    m_playDateTime = playDateTime;

    m_indexType = MediaIndexer::WritePlaybackInfo;
    start();
}

void MediaIndexerJob::indexMediaItem(MediaItem mediaItem)
{
    //Update RDF store
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    Nepomuk::Resource res(mediaItem.url);
    if (mediaItem.type == "Audio") {
        // Update the media type
        kDebug() << "Writing type...";
        QUrl audioType;
        if (mediaItem.fields["audioType"] == "Music") {
            audioType = mediaVocabulary.typeAudioMusic();
            if (!res.exists()) {
                res = Nepomuk::Resource(mediaItem.url, audioType);
            }
            removeType(res, mediaVocabulary.typeAudioStream());
            removeType(res, mediaVocabulary.typeAudio());
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            audioType = mediaVocabulary.typeAudioStream();
            if (!res.exists()) {
                res = Nepomuk::Resource(mediaItem.url, audioType);
            }
            removeType(res, mediaVocabulary.typeAudioMusic());
            removeType(res, mediaVocabulary.typeAudio());
        } else if (mediaItem.fields["audioType"] == "Audio Clip") {
            audioType = mediaVocabulary.typeAudio();
            if (!res.exists()) {
                res = Nepomuk::Resource(mediaItem.url, audioType);
            }
            removeType(res, mediaVocabulary.typeAudioMusic());
            removeType(res, mediaVocabulary.typeAudioStream());
        }
        if (!res.hasType(audioType)) {
            res.addType(audioType);
        }
        
        // Update the properties
        QString title = mediaItem.fields["title"].toString();
        if (!title.isEmpty()) {
            kDebug() << "Writing title...";
            res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        }
        QString description = mediaItem.fields["description"].toString();
        if (!description.isEmpty()) {
            kDebug() << "Writing description...";
            res.setProperty(mediaVocabulary.description(), Nepomuk::Variant(description));
        }
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            Nepomuk::Resource artworkRes(artworkUrl);
            kDebug() << "Writing artworkurl...";
            if (!artworkRes.exists()) {
                artworkRes = Nepomuk::Resource(QUrl(artworkUrl), QUrl("http://http://www.semanticdesktop.org/ontologies/nfo#Image"));
            }
            res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(artworkRes));
        }
        if (mediaItem.fields["audioType"] == "Music") {
            QString artist  = mediaItem.fields["artist"].toString();
            if (!artist.isEmpty()) {
                kDebug() << "Writing artist...";
                res.setProperty(mediaVocabulary.musicArtist(), Nepomuk::Variant(artist));
            }
            QString album   = mediaItem.fields["album"].toString();
            if (!album.isEmpty()) {
                kDebug() << "Writing album...";
                res.setProperty(mediaVocabulary.musicAlbumName(), Nepomuk::Variant(album));
            }
            QString genre  = mediaItem.fields["genre"].toString();
            if (!genre.isEmpty()) {
                kDebug() << "Writing genre...";
                res.setProperty(mediaVocabulary.genre(), Nepomuk::Variant(genre));
            }
            int track   = mediaItem.fields["trackNumber"].toInt();
            if (track != 0) {
                kDebug() << "Writing track...";
                res.setProperty(mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
            }
            int duration = mediaItem.fields["duration"].toInt();
            if (duration != 0) {
                kDebug() << "Writing duration...";
                res.setProperty(mediaVocabulary.duration(), Nepomuk::Variant(duration));
            }
            int year = mediaItem.fields["year"].toInt();
            if (year != 0) {
                QDate created = QDate(year, 1, 1);
                kDebug() << "Writing year..." << year;
                res.setProperty(mediaVocabulary.created(), Nepomuk::Variant(created));
            }
        } else if ((mediaItem.fields["audioType"] == "Audio Stream") ||
            (mediaItem.fields["audioType"] == "Audio Clip")) {
        }
    } else if (mediaItem.type == "Video") {
        //Update the media type
        if (!res.exists()) {
            res = Nepomuk::Resource(mediaItem.url, mediaVocabulary.typeVideo());
        }
        if (!res.hasType(mediaVocabulary.typeVideo())) {
            res.addType(mediaVocabulary.typeVideo());
        }
        
        //Update the properties
        QString title = mediaItem.fields["title"].toString();
        if (!title.isEmpty()) {
            kDebug() << "Writing title...";
            res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        }
        QString description = mediaItem.fields["description"].toString();
        if (!description.isEmpty()) {
            kDebug() << "Writing description...";
            res.setProperty(mediaVocabulary.description(), Nepomuk::Variant(description));
        }
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            Nepomuk::Resource artworkRes(artworkUrl);
            kDebug() << "Writing artworkurl...";
            if (!artworkRes.exists()) {
                artworkRes = Nepomuk::Resource(QUrl(artworkUrl), QUrl("http://http://www.semanticdesktop.org/ontologies/nfo#Image"));
            }
            res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(artworkRes));
        }
        if (mediaItem.fields["videoType"] == "Movie") {
            res.setProperty(mediaVocabulary.videoIsMovie(), Nepomuk::Variant(true));
            if (res.hasProperty(mediaVocabulary.videoIsTVShow())) {
                res.removeProperty(mediaVocabulary.videoIsTVShow());
            }
            QString seriesName = mediaItem.fields["seriesName"].toString();
            if (!seriesName.isEmpty()) {
                res.setProperty(mediaVocabulary.videoSeriesName(), Nepomuk::Variant(seriesName));
            }
            QString genre   = mediaItem.fields["genre"].toString();
            if (!genre.isEmpty()) {
                res.setProperty(mediaVocabulary.genre(), Nepomuk::Variant(genre));
            }
            int year = mediaItem.fields["year"].toInt();
            if (year != 0) {
                QDate created = QDate(year, 1, 1);
                res.setProperty(mediaVocabulary.created(), Nepomuk::Variant(created));
            }
        } else if (mediaItem.fields["videoType"] == "TV Show") {
            res.setProperty(mediaVocabulary.videoIsTVShow(), Nepomuk::Variant(true));
            if (res.hasProperty(mediaVocabulary.videoIsMovie())) {
                res.removeProperty(mediaVocabulary.videoIsMovie());
            }
            QString seriesName = mediaItem.fields["seriesName"].toString();
            if (!seriesName.isEmpty()) {
                res.setProperty(mediaVocabulary.videoSeriesName(), Nepomuk::Variant(seriesName));
            }
            int season = mediaItem.fields["season"].toInt();
            if (season != 0) {
                res.setProperty(mediaVocabulary.videoSeriesSeason(), Nepomuk::Variant(season));
            } else {
                if (res.hasProperty(mediaVocabulary.videoSeriesSeason())) {
                    res.removeProperty(mediaVocabulary.videoSeriesSeason());
                }
            }
            int episode = mediaItem.fields["episode"].toInt();
            if (episode != 0) {
                res.setProperty(mediaVocabulary.videoSeriesEpisode(), Nepomuk::Variant(episode));
            } else {
                if (res.hasProperty(mediaVocabulary.videoSeriesEpisode())) {
                    res.removeProperty(mediaVocabulary.videoSeriesEpisode());
                }
            }
            QString genre   = mediaItem.fields["genre"].toString();
            if (!genre.isEmpty()) {
                res.setProperty(mediaVocabulary.genre(), Nepomuk::Variant(genre));
            }
            int year = mediaItem.fields["year"].toInt();
            if (year != 0) {
                QDate created = QDate(year, 1, 1);
                res.setProperty(mediaVocabulary.created(), Nepomuk::Variant(created));
            }
        } else if (mediaItem.fields["videoType"] == "Video Clip") {
            //Remove properties identifying video as a movie or tv show
            if (res.hasProperty(mediaVocabulary.videoIsMovie())) {
                res.removeProperty(mediaVocabulary.videoIsMovie());
            }
            if (res.hasProperty(mediaVocabulary.videoIsTVShow())) {
                res.removeProperty(mediaVocabulary.videoIsTVShow());
            }
        }
    }
}

void MediaIndexerJob::removeInfo(MediaItem mediaItem)
{
    kDebug() << "removing info for " << mediaItem.url;
    //Update RDF store
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    Nepomuk::Resource res(mediaItem.url);
    if (!res.exists()) {
        return;
    }
    if (mediaItem.type == "Audio") {
        // Update the media type
        kDebug() << "Removing type...";
        QUrl audioType;
        if (mediaItem.fields["audioType"] == "Music") {
            audioType = mediaVocabulary.typeAudioMusic();
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            audioType = mediaVocabulary.typeAudioStream();
        } else if (mediaItem.fields["audioType"] == "Audio Clip") {
            audioType = mediaVocabulary.typeAudio();
        }
        if (res.hasType(audioType)) {
            removeType(res, audioType);
        }
        
        // Update the properties
        if (res.hasProperty(mediaVocabulary.title())){
            kDebug() << "Removing title...";
            res.removeProperty(mediaVocabulary.title());
        }
        if (res.hasProperty(mediaVocabulary.description())) {
            kDebug() << "Removing description...";
            res.removeProperty(mediaVocabulary.description());
        }
        if (res.hasProperty(mediaVocabulary.artwork())) {
            kDebug() << "Removing artwork...";
            res.removeProperty(mediaVocabulary.artwork());
        }

        if (mediaItem.fields["audioType"] == "Music") {
            if (res.hasProperty(mediaVocabulary.musicArtist())) {
                kDebug() << "Removing artist...";
                res.removeProperty(mediaVocabulary.musicArtist());
            }
            if (res.hasProperty(mediaVocabulary.musicAlbumName())) {
                kDebug() << "Removing album...";
                res.removeProperty(mediaVocabulary.musicAlbumName());
            }
            if (res.hasProperty(mediaVocabulary.genre())) {
                kDebug() << "Removing genre...";
                res.removeProperty(mediaVocabulary.genre());
            }
            if (res.hasProperty(mediaVocabulary.musicTrackNumber())) {
                kDebug() << "Removing trackNumber...";
                res.removeProperty(mediaVocabulary.musicTrackNumber());
            }
            if (res.hasProperty(mediaVocabulary.duration())) {
                kDebug() << "Removing duration...";
                res.removeProperty(mediaVocabulary.duration());
            }
            if (res.hasProperty(mediaVocabulary.created())) {
                kDebug() << "Removing year...";
                res.removeProperty(mediaVocabulary.created());
            }
        } else if ((mediaItem.fields["audioType"] == "Audio Stream") ||
            (mediaItem.fields["audioType"] == "Audio Clip")) {
        }
    } else if (mediaItem.type == "Video") {
        //Update the media type
        if (res.hasType(mediaVocabulary.typeVideo())) {
            removeType(res, mediaVocabulary.typeVideo());
        }
        
        //Update the properties
        if (res.hasProperty(mediaVocabulary.title())){
            kDebug() << "Removing title...";
            res.removeProperty(mediaVocabulary.title());
        }
        if (res.hasProperty(mediaVocabulary.description())) {
            kDebug() << "Removing description...";
            res.removeProperty(mediaVocabulary.description());
        }
        if (res.hasProperty(mediaVocabulary.artwork())) {
            kDebug() << "Removing artwork...";
            res.removeProperty(mediaVocabulary.artwork());
        }
        if (mediaItem.fields["videoType"] == "Movie") {
            if (res.hasProperty(mediaVocabulary.videoIsMovie())) {
                res.removeProperty(mediaVocabulary.videoIsMovie());
            }
            if (res.hasProperty(mediaVocabulary.videoSeriesName())) {
                res.removeProperty(mediaVocabulary.videoSeriesName());
            }
            if (res.hasProperty(mediaVocabulary.genre())) {
                res.removeProperty(mediaVocabulary.genre());
            }
            if (res.hasProperty(mediaVocabulary.created())) {
                res.removeProperty(mediaVocabulary.created());
            }
        } else if (mediaItem.fields["videoType"] == "TV Show") {
            if (res.hasProperty(mediaVocabulary.videoIsTVShow())) {
                res.removeProperty(mediaVocabulary.videoIsTVShow());
            }
            if (res.hasProperty(mediaVocabulary.videoSeriesName())) {
                res.removeProperty(mediaVocabulary.videoSeriesName());
            }
            if (res.hasProperty(mediaVocabulary.videoSeriesSeason())) {
                res.removeProperty(mediaVocabulary.videoSeriesSeason());
            }
            if (res.hasProperty(mediaVocabulary.videoSeriesEpisode())) {
                res.removeProperty(mediaVocabulary.videoSeriesEpisode());
            }
            if (res.hasProperty(mediaVocabulary.genre())) {
                res.removeProperty(mediaVocabulary.genre());
            }
            if (res.hasProperty(mediaVocabulary.created())) {
                res.removeProperty(mediaVocabulary.created());
            }
        } else if (mediaItem.fields["videoType"] == "Video Clip") {
            if (res.hasProperty(mediaVocabulary.videoIsMovie())) {
                res.removeProperty(mediaVocabulary.videoIsMovie());
            }
            if (res.hasProperty(mediaVocabulary.videoIsTVShow())) {
                res.removeProperty(mediaVocabulary.videoIsTVShow());
            }
        }
    }
}

void MediaIndexerJob::removeType(Nepomuk::Resource res, QUrl mediaType)
{
    QList<QUrl> types = res.types();
    for (int i = 0; i < types.count(); i++) {
        if (types.at(i).toString() == mediaType.toString()) {
            types.removeAt(i);
            break;
        }
    }
    res.setTypes(types);
}


MediaIndexer::MediaIndexer(QObject * parent) : QObject(parent)
{
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
}

MediaIndexer::~MediaIndexer()
{
    for (int i = 0; i < m_mediaIndexerJobs.count(); i++) {
        delete m_mediaIndexerJobs.at(i);
    }
}

void MediaIndexer::indexMediaItems(QList<MediaItem> mediaList)
{
    if (m_nepomukInited) {
        if (mediaList.count() > 0) {
            MediaIndexerJob * indexerJob = availableIndexerJob();
            connect(indexerJob, SIGNAL(percentComplete(int)), this, SIGNAL(percentComplete(int)));
            connect(indexerJob, SIGNAL(jobComplete()), this, SIGNAL(indexingComplete()));
            connect(indexerJob, SIGNAL(sourceInfoUpdated(MediaItem)), this, SIGNAL(sourceInfoUpdated(MediaItem)));
            connect(indexerJob, SIGNAL(started()), this, SIGNAL(started()));
            indexerJob->setMediaListToIndex(mediaList);
            indexerJob->start();
        }
    }
}

void MediaIndexer::removeInfo(QList<MediaItem> mediaList)
{
    if (m_nepomukInited) {
        if (mediaList.count() > 0) {
            MediaIndexerJob * indexerJob = availableIndexerJob();
            connect(indexerJob, SIGNAL(percentComplete(int)), this, SIGNAL(percentComplete(int)));
            connect(indexerJob, SIGNAL(jobComplete()), this, SIGNAL(indexingComplete()));
            connect(indexerJob, SIGNAL(urlInfoRemoved(QString)), this, SIGNAL(urlInfoRemoved(QString)));
            connect(indexerJob, SIGNAL(started()), this, SIGNAL(started()));
            indexerJob->setInfoToRemove(mediaList);
            indexerJob->start();
        }
    }
}

MediaIndexerJob * MediaIndexer::availableIndexerJob()
{
    MediaIndexerJob * mediaIndexerJob;
    bool found = false;
    for (int i = 0; i < m_mediaIndexerJobs.count(); i++) {
        if (!m_mediaIndexerJobs.at(i)->isRunning()) {
            mediaIndexerJob = m_mediaIndexerJobs.at(i);
            found = true;
            break;
        }
    }
    if (!found) {
        mediaIndexerJob = new MediaIndexerJob(0);
        m_mediaIndexerJobs.append(mediaIndexerJob);
    }
    
    return mediaIndexerJob;
}

void MediaIndexer::writePlaybackInfo(QString url, bool incrementPlayCount, QDateTime playDateTime)
{
    if (m_nepomukInited) {
        MediaIndexerJob * indexerJob = availableIndexerJob();
        indexerJob->writePlaybackInfo(url, incrementPlayCount, playDateTime);
    }
}
