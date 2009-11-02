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
#include <kuiserverjobtracker.h>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>

#include <QApplication>

MediaIndexerJob::MediaIndexerJob(QObject * parent) : KJob(parent)
{
    setCapabilities(KJob::NoCapabilities);
    running = false;
}

MediaIndexerJob::~MediaIndexerJob()
{
    if (running) {
        setPercent(100);
        emitResult();
    }
}

void MediaIndexerJob::start()
{
    running = true;
    index();
}

void MediaIndexerJob::index()
{
    if (m_indexType == MediaIndexer::IndexUrl) {
        QList<QString> urlsToIndex = m_urlsToIndex;
        m_urlsToIndex.clear();
        QString descriptionTitle = QString("Bangarang: Indexing %1 items").arg(urlsToIndex.count());
        for (int i = 0; i < urlsToIndex.count(); ++i) {
            emit description(this, descriptionTitle, qMakePair(QString("Current Item"), QString("%1").arg(urlsToIndex.at(i))));
            
            indexUrl(urlsToIndex.at(i));     
            setPercent(100*i/urlsToIndex.count());
        }
        emit description(this, QString("Bangarang: %1 items indexed").arg(urlsToIndex.count()));
        emitResult();
        running = false;
    } else if (m_indexType == MediaIndexer::IndexMediaItem) {
        QList<MediaItem> mediaList = m_mediaListToIndex;
        m_mediaListToIndex.clear();
        QString descriptionTitle = QString("Bangarang: Indexing %1 items").arg(mediaList.count());
        for (int i = 0; i < mediaList.count(); ++i) {
            emit description(this, descriptionTitle, qMakePair(QString("Current Item"), QString("%1").arg(mediaList.at(i).title)));
            
            indexMediaItem(mediaList.at(i));     
            setPercent(100*i/mediaList.count());
        }
        emit description(this, QString("Bangarang: %1 items indexed").arg(mediaList.count()));
        emitResult();
        running = false;
    }
    emit jobComplete();
}

void MediaIndexerJob::setUrlsToIndex(QList<QString> urls)
{
    if (!running) {
        m_urlsToIndex << urls;
        m_indexType = MediaIndexer::IndexUrl;
    }
}

void MediaIndexerJob::setMediaListToIndex(QList<MediaItem> mediaList)
{
    if (!running) {
        m_mediaListToIndex << mediaList;
        m_indexType = MediaIndexer::IndexMediaItem;
    }
}

void MediaIndexerJob::indexUrl(QString url)
{
    //Update RDF store
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    Nepomuk::Resource res(url);
    if (Utilities::isMusic(url)) {
        if (!res.exists()) {
            res = Nepomuk::Resource(url, mediaVocabulary.typeAudioMusic());
        }
        if (!res.hasType(mediaVocabulary.typeAudioMusic())) {
            res.addType(mediaVocabulary.typeAudioMusic());
        }
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        QString title = TStringToQString(file.tag()->title()).trimmed();
        QString artist  = TStringToQString(file.tag()->artist()).trimmed();
        QString album   = TStringToQString(file.tag()->album()).trimmed();
        int track   = file.tag()->track();
        QString genre   = TStringToQString(file.tag()->genre()).trimmed();
        int duration = file.audioProperties()->length();
        res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        res.setProperty(mediaVocabulary.musicArtist(), Nepomuk::Variant(artist));
        res.setProperty(mediaVocabulary.musicAlbumName(), Nepomuk::Variant(album));
        res.setProperty(mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
        res.setProperty(mediaVocabulary.musicGenre(), Nepomuk::Variant(genre));
        res.setProperty(mediaVocabulary.duration(), Nepomuk::Variant(duration));
    }
}    

void MediaIndexerJob::indexMediaItem(MediaItem mediaItem)
{
    //Update RDF store
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    Nepomuk::Resource res(mediaItem.url);
    if (mediaItem.type == "Audio") {
        // Update the media type
        QUrl audioType;
        if (mediaItem.fields["audioType"] == "Music") {
            audioType = mediaVocabulary.typeAudioMusic();
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            audioType = mediaVocabulary.typeAudioStream();
        } else if (mediaItem.fields["audioType"] == "Audio Clip") {
            audioType = mediaVocabulary.typeAudio();
        }
        if (!res.exists()) {
            res = Nepomuk::Resource(mediaItem.url, audioType);
        }
        if (!res.hasType(audioType)) {
            res.addType(audioType);
        }
        
        // Update the properties
        QString title = mediaItem.fields["title"].toString();
        res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        QString description = mediaItem.fields["description"].toString();
        res.setProperty(mediaVocabulary.description(), Nepomuk::Variant(description));
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            Nepomuk::Resource artworkRes(artworkUrl);
            if (!artworkRes.exists()) {
                artworkRes = Nepomuk::Resource(QUrl(artworkUrl), QUrl("http://http://www.semanticdesktop.org/ontologies/nfo#Image"));
            }
            res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(artworkRes));
        }
        if (mediaItem.fields["audioType"] == "Music") {
            QString artist  = mediaItem.fields["artist"].toString();
            QString album   = mediaItem.fields["album"].toString();
            int track   = mediaItem.fields["trackNumber"].toInt();
            QString genre   = mediaItem.fields["genre"].toString();
            int duration = mediaItem.fields["duration"].toInt();
            res.setProperty(mediaVocabulary.musicArtist(), Nepomuk::Variant(artist));
            res.setProperty(mediaVocabulary.musicAlbumName(), Nepomuk::Variant(album));
            if (track != 0) {
                res.setProperty(mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
            }
            res.setProperty(mediaVocabulary.musicGenre(), Nepomuk::Variant(genre));
            if (duration != 0) {
                res.setProperty(mediaVocabulary.duration(), Nepomuk::Variant(duration));
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
        res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        QString description = mediaItem.fields["description"].toString();
        res.setProperty(mediaVocabulary.description(), Nepomuk::Variant(description));
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            Nepomuk::Resource artworkRes(artworkUrl);
            if (!artworkRes.exists()) {
                artworkRes = Nepomuk::Resource(QUrl(artworkUrl), mediaVocabulary.typeImage());
            }
            res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(artworkRes));
        }
        if (mediaItem.fields["videoType"] == "Movie") {
            res.setProperty(mediaVocabulary.videoIsMovie(), Nepomuk::Variant(true));
            res.removeProperty(mediaVocabulary.videoIsTVShow());
            QString seriesName = mediaItem.fields["seriesName"].toString();
            res.setProperty(mediaVocabulary.videoSeriesName(), Nepomuk::Variant(seriesName));
        } else if (mediaItem.fields["videoType"] == "TV Show") {
            res.setProperty(mediaVocabulary.videoIsTVShow(), Nepomuk::Variant(true));
            res.removeProperty(mediaVocabulary.videoIsMovie());
            QString seriesName = mediaItem.fields["seriesName"].toString();
            res.setProperty(mediaVocabulary.videoSeriesName(), Nepomuk::Variant(seriesName));
            int season = mediaItem.fields["season"].toInt();
            if (season != 0) {
                res.setProperty(mediaVocabulary.videoSeriesSeason(), Nepomuk::Variant(season));
            } else {
                res.removeProperty(mediaVocabulary.videoSeriesSeason());
            }
            int episode = mediaItem.fields["episode"].toInt();
            if (episode != 0) {
                res.setProperty(mediaVocabulary.videoSeriesEpisode(), Nepomuk::Variant(episode));
            } else {
                res.removeProperty(mediaVocabulary.videoSeriesEpisode());
            }
        } else if (mediaItem.fields["videoType"] == "Video Clip") {
            //Remove properties identifying video as a movie or tv show
            res.removeProperty(mediaVocabulary.videoIsMovie());
            res.removeProperty(mediaVocabulary.videoIsTVShow());
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

MediaIndexer::MediaIndexer(QObject * parent) : QThread(parent)
{
}

MediaIndexer::~MediaIndexer()
{
}

void MediaIndexer::run()
{
    if (m_indexType == MediaIndexer::IndexUrl) {
        if (m_urls.count() > 0) {
            MediaIndexerJob * indexerJob = new MediaIndexerJob(this);
            indexerJob->setUrlsToIndex(m_urls);
            KUiServerJobTracker * jt = new KUiServerJobTracker(this);
            jt->registerJob(indexerJob);
            indexerJob->start();
        }
    } else if (m_indexType == MediaIndexer::IndexMediaItem) {
        if (m_mediaList.count() > 0) {
            MediaIndexerJob * indexerJob = new MediaIndexerJob(this);
            connect(indexerJob, SIGNAL(jobComplete()), this, SLOT(jobComplete()));
            indexerJob->setMediaListToIndex(m_mediaList);
            KUiServerJobTracker * jt = new KUiServerJobTracker(this);
            jt->registerJob(indexerJob);
            indexerJob->start();
        }
    }   
}

void MediaIndexer::indexUrls(QList<QString> urls)
{
    m_indexType = MediaIndexer::IndexUrl;
    m_urls = urls;
    start();
}

void MediaIndexer::indexMediaItems(QList<MediaItem> mediaList)
{
    m_indexType = MediaIndexer::IndexMediaItem;
    m_mediaList = mediaList;
    start();
}

void MediaIndexer::jobComplete()
{
    emit indexingComplete();
}
