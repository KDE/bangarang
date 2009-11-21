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

#ifndef MEDIAINDEXER_H
#define MEDIAINDEXER_H

#include <QtCore>
#include <KJob>

class MediaItem;
namespace Nepomuk 
{
    class Resource;
}

class MediaIndexerJob: public KJob
{
    Q_OBJECT
    
    public:
        MediaIndexerJob(QObject *parent);
        ~MediaIndexerJob();
        void setUrlsToIndex(QList<QString> urls);
        void setMediaListToIndex(QList<MediaItem> mediaList);
        void indexUrl(QString url);
        void indexMediaItem(MediaItem mediaItem);
        void setInfoToRemove(QList<MediaItem> mediaList);
        void removeInfo(MediaItem mediaItem);
        void start();
    
    private:
        void index();
        QList<QString> m_urlsToIndex;
        QList<MediaItem> m_mediaListToIndex;
        bool running;
        int m_indexType;
        void removeType(Nepomuk::Resource res, QUrl mediaType);
        
    Q_SIGNALS:
        void jobComplete();
        void urlInfoRemoved(QString url);
        void sourceInfoUpdated(MediaItem mediaItem);
};

class MediaIndexer : public QThread
{
    Q_OBJECT
    
    public:
        enum IndexType { IndexUrl = Qt::UserRole + 1,
        IndexMediaItem = Qt::UserRole + 2,
        RemoveInfo = Qt::UserRole + 3};
        MediaIndexer(QObject *parent);
        ~MediaIndexer();
        void run();
        void indexUrls(QList<QString> urls);
        void indexMediaItems(QList<MediaItem> mediaList);
        void removeInfo(QList<MediaItem> mediaList);
        
    private:
        QList<QString> m_urls;
        QList<MediaItem> m_mediaList;
        int m_indexType;
        bool m_nepomukInited;
        
    private slots:
        void jobComplete();
        
    Q_SIGNALS:
        void indexingComplete();
        void urlInfoRemoved(QString url);
        void sourceInfoUpdated(MediaItem mediaItem);
        
};
#endif // MEDIAINDEXER_H
