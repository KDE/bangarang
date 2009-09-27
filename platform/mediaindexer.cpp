#include "mediaindexer.h"
#include "mediaitemmodel.h"
#include "utilities.h"

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
    Nepomuk::Resource res(url);
    if (Utilities::isMusic(url)) {
        if (!res.exists()) {
            res = Nepomuk::Resource(url, Soprano::Vocabulary::Xesam::Music());
        }
        if (!res.hasType(Soprano::Vocabulary::Xesam::Music())) {
            res.addType(Soprano::Vocabulary::Xesam::Music());
        }
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        QString title = TStringToQString(file.tag()->title()).trimmed();
        QString artist  = TStringToQString(file.tag()->artist()).trimmed();
        QString album   = TStringToQString(file.tag()->album()).trimmed();
        int track   = file.tag()->track();
        QString genre   = TStringToQString(file.tag()->genre()).trimmed();
        int duration = file.audioProperties()->length();
        res.setProperty(Soprano::Vocabulary::Xesam::title(), Nepomuk::Variant(title));
        res.setProperty(Soprano::Vocabulary::Xesam::artist(), Nepomuk::Variant(artist));
        res.setProperty(Soprano::Vocabulary::Xesam::album(), Nepomuk::Variant(album));
        res.setProperty(Soprano::Vocabulary::Xesam::trackNumber(), Nepomuk::Variant(track));
        res.setProperty(Soprano::Vocabulary::Xesam::genre(), Nepomuk::Variant(genre));
        res.setProperty(Soprano::Vocabulary::Xesam::mediaDuration(), Nepomuk::Variant(duration));
    }
}    

void MediaIndexerJob::indexMediaItem(MediaItem mediaItem)
{
    //Update RDF store
    Nepomuk::Resource res(mediaItem.url);
    if (mediaItem.type == "Audio") {
        if (mediaItem.fields["audioType"] == "Music") {
            if (!res.exists()) {
                res = Nepomuk::Resource(mediaItem.url, Soprano::Vocabulary::Xesam::Music());
            }
            if (!res.hasType(Soprano::Vocabulary::Xesam::Music())) {
                res.addType(Soprano::Vocabulary::Xesam::Music());
            }
            QString title = mediaItem.fields["title"].toString();
            QString artist  = mediaItem.fields["artist"].toString();
            QString album   = mediaItem.fields["album"].toString();
            int track   = mediaItem.fields["trackNumber"].toInt();
            QString genre   = mediaItem.fields["genre"].toString();
            int duration = mediaItem.fields["duration"].toInt();
            res.setProperty(Soprano::Vocabulary::Xesam::title(), Nepomuk::Variant(title));
            res.setProperty(Soprano::Vocabulary::Xesam::artist(), Nepomuk::Variant(artist));
            res.setProperty(Soprano::Vocabulary::Xesam::album(), Nepomuk::Variant(album));
            res.setProperty(Soprano::Vocabulary::Xesam::trackNumber(), Nepomuk::Variant(track));
            res.setProperty(Soprano::Vocabulary::Xesam::genre(), Nepomuk::Variant(genre));
            res.setProperty(Soprano::Vocabulary::Xesam::mediaDuration(), Nepomuk::Variant(duration));
        } else {
            if (!res.exists()) {
                res = Nepomuk::Resource(mediaItem.url, Soprano::Vocabulary::Xesam::Audio());
            }
            if (!res.hasType(Soprano::Vocabulary::Xesam::Audio())) {
                res.addType(Soprano::Vocabulary::Xesam::Audio());
            }
            QString title = mediaItem.fields["title"].toString();
            res.setProperty(Soprano::Vocabulary::Xesam::title(), Nepomuk::Variant(title));
        }
    } else if (mediaItem.type == "Video") {
        if (!res.exists()) {
            res = Nepomuk::Resource(mediaItem.url, Soprano::Vocabulary::Xesam::Video());
        }
        if (!res.hasType(Soprano::Vocabulary::Xesam::Video())) {
            res.addType(Soprano::Vocabulary::Xesam::Video());
        }
        QString title = mediaItem.fields["title"].toString();
        res.setProperty(Soprano::Vocabulary::Xesam::title(), Nepomuk::Variant(title));
    }
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
