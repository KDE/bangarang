#include "thread.h"
#include "artwork.h"

#include <KDebug>

Utilities::Thread::Thread(QObject *parent) :
    QThread(parent)
{
}

Utilities::Thread::~Thread()
{
}

void Utilities::Thread::run()
{
    if (m_action == "getArtworks") {
        if (m_mediaItem.type == "Audio" || m_mediaItem.type == "Video" ||
            (m_mediaItem.type == "Category" && (m_mediaItem.subType() == "AudioFeed" ||
                                                m_mediaItem.subType() == "VideoFeed"))) {
            QImage artwork = Utilities::getArtworkImageFromMediaItem(m_mediaItem, m_ignoreCache);
            QList<QImage> artworks;
            artworks.append(artwork);
            emit gotArtworks(artworks, m_mediaItem);
        } else if (m_mediaItem.type == "Category") {
            QList<QImage> artworks;
            QString itemTitle = m_mediaItem.fields["title"].toString();
            if (m_mediaItem.subType() == "AudioGenre") {
                artworks = Utilities::getGenreArtworks(itemTitle, "audio", m_ignoreCache);
            } else if (m_mediaItem.subType() == "VideoGenre") {
                artworks = Utilities::getGenreArtworks(itemTitle, "video", m_ignoreCache);
            } else if (m_mediaItem.subType() == "Artist") {
                artworks = Utilities::getArtistArtworks(itemTitle, m_ignoreCache);
            } else if (m_mediaItem.subType() == "Album") {
                artworks.append(Utilities::getAlbumArtwork(itemTitle, m_ignoreCache));
            } else if (m_mediaItem.subType() == "AudioTag") {
                artworks = Utilities::getTagArtworks(itemTitle, "audio", m_ignoreCache);
            } else if (m_mediaItem.subType() == "VideoTag") {
                artworks = Utilities::getTagArtworks(itemTitle, "video", m_ignoreCache);
            } else if (m_mediaItem.subType() == "TV Series") {
                artworks = Utilities::getTVSeriesArtworks(itemTitle, m_ignoreCache);
            } else if (m_mediaItem.subType() == "TV Season") {
                QString seriesName = m_mediaItem.fields["seriesName"].toString();
                int season = m_mediaItem.fields["season"].toInt();
                artworks = Utilities::getTVSeasonArtworks(seriesName, season, m_ignoreCache);
            } else if (m_mediaItem.subType() == "Actor") {
                artworks = Utilities::getActorArtworks(itemTitle, m_ignoreCache);
            } else if (m_mediaItem.subType() == "Director") {
                artworks = Utilities::getDirectorArtworks(itemTitle, m_ignoreCache);
            }
            emit gotArtworks(artworks, m_mediaItem);
        }
    } else if (m_action == "getArtwork") {
        QImage artwork = Utilities::getArtworkImageFromMediaItem(m_mediaItem);
        emit gotArtwork(artwork, m_mediaItem);
    }
}

void Utilities::Thread::getArtworksFromMediaItem(const MediaItem &mediaItem, bool ignoreCache)
{
    if (!isRunning()) {
        m_action = "getArtworks";
        m_ignoreCache = ignoreCache;
        m_mediaItem = mediaItem;
        start();
    }
}

void Utilities::Thread::getArtworkFromMediaItem(const MediaItem &mediaItem, bool ignoreCache)
{
    if (!isRunning()) {
        m_action = "getArtwork";
        m_ignoreCache = ignoreCache;
        m_mediaItem = mediaItem;
        start();
    }
}
