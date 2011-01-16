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

#include "listengine.h"
#include "../downloader.h"

ListEngine::ListEngine(ListEngineFactory * parent) : QThread(parent)
{
    m_parent = parent;
    m_stop = false;
}

ListEngine::~ListEngine()
{
}

void ListEngine::setModel(MediaItemModel * mediaItemModel)
{
    m_mediaItemModel = mediaItemModel;
    connect(this, 
            SIGNAL(results(QString, QList<MediaItem>, MediaListProperties, bool, QString)),
            m_mediaItemModel,
            SLOT(addResults(QString, QList<MediaItem>, MediaListProperties, bool, QString)));
    connect(this, 
            SIGNAL(updateMediaItems(QList<MediaItem>)),
            m_mediaItemModel,
            SLOT(updateMediaItems(QList<MediaItem>)));
    connect(this, 
            SIGNAL(updateMediaItem(MediaItem)),
            m_mediaItemModel,
            SLOT(updateMediaItem(MediaItem)));
    connect(this, 
            SIGNAL(updateArtwork(QImage, MediaItem)),
            m_mediaItemModel,
            SLOT(updateArtwork(QImage, MediaItem)));
    connect(this, 
            SIGNAL(updateMediaListPropertiesCategoryArtwork(QImage, MediaItem)),
            m_mediaItemModel,
            SLOT(updateMediaListPropertiesCategoryArtwork(QImage, MediaItem)));
    connect(this,
            SIGNAL(updateStatus(QHash<QString,QVariant>)),
            m_mediaItemModel,
            SLOT(updateStatus(QHash<QString,QVariant>)));
    connect(this,
            SIGNAL(loadOtherEngine(MediaListProperties,QString,QString)),
            m_parent,
            SLOT(load(MediaListProperties,QString,QString)));
}

MediaItemModel * ListEngine::model()
{
    return m_mediaItemModel;
}

void ListEngine::setMediaListProperties(const MediaListProperties& mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

const MediaListProperties& ListEngine::mediaListProperties() const
{
    return m_mediaListProperties;
}

void ListEngine::setRequestSignature(const QString& requestSignature)
{
    m_requestSignature = requestSignature;
}

const QString& ListEngine::requestSignature() const {
	return m_requestSignature;
}

void ListEngine::setSubRequestSignature(const QString& subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

const QString& ListEngine::subRequestSignature() const {
	return m_subRequestSignature;
}

void ListEngine::connectDownloader()
{
    connect(this, SIGNAL(download(const KUrl, const KUrl)), m_parent->downloader(), SLOT(download(const KUrl, const KUrl)));
    connect(m_parent->downloader(), SIGNAL(downloadComplete(const KUrl, const KUrl)), this, SLOT(downloadComplete(const KUrl, const KUrl)));
    connect(this, SIGNAL(listDir(KUrl)), m_parent->downloader(), SLOT(listDir(KUrl)));
    connect(m_parent->downloader(), SIGNAL(listingComplete(KUrl)), this, SLOT(listingComplete(KUrl)));
}

void ListEngine::disconnectDownloader()
{
    disconnect(this, SIGNAL(download(const KUrl, const KUrl)), m_parent->downloader(), SLOT(download(const KUrl, const KUrl)));
    disconnect(m_parent->downloader(), SIGNAL(downloadComplete(const KUrl, const KUrl)), this, SLOT(downloadComplete(const KUrl, const KUrl)));
    disconnect(this, SIGNAL(listDir(KUrl)), m_parent->downloader(), SLOT(listDir(KUrl)));
    disconnect(m_parent->downloader(), SIGNAL(listingComplete(KUrl)), this, SLOT(listingComplete(KUrl)));
}

void ListEngine::stop(unsigned long waitToTerminate, bool quitEventLoop)
{
    m_stop = true;
    if (quitEventLoop) {
        quit();
    }
    if (waitToTerminate > 0 && isRunning()) {
        wait(waitToTerminate);
        terminate();
    }
}

void ListEngine::resume()
{
    m_stop = false;
}
