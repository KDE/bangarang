#ifndef MEDIAINDEXER_H
#define MEDIAINDEXER_H

#include "mediaitemmodel.h"
#include <QtCore>
#include <KJob>

class MediaIndexerJob: public KJob
{
    public:
        MediaIndexerJob(QObject *parent);
        ~MediaIndexerJob();
        void setUrlsToIndex(QList<QString> urls);
        void setMediaListToIndex(QList<MediaItem> mediaList);
        void indexUrl(QString url);
        void indexMediaItem(MediaItem mediaItem);
        void start();
    
    private:
        void index();
        QList<QString> m_urlsToIndex;
        QList<MediaItem> m_mediaListToIndex;
        bool running;
        int m_indexType;
};

class MediaIndexer : public QThread
{
    Q_OBJECT
    
    public:
        enum IndexType { IndexUrl = Qt::UserRole + 1,
        IndexMediaItem = Qt::UserRole + 2};
        MediaIndexer(QObject *parent);
        ~MediaIndexer();
        void run();
        void indexUrls(QList<QString> urls);
        void indexMediaItems(QList<MediaItem> mediaList);
        
    private:
        QList<QString> m_urls;
        QList<MediaItem> m_mediaList;
        int m_indexType;
        
};
#endif // MEDIAINDEXER_H
