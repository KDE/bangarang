#ifndef VIDEOLISTENGINE_H
#define VIDEOLISTENGINE_H

#include "listengine.h"
#include <QtCore>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

class VideoListEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        VideoListEngine(ListEngineFactory *parent);
        ~VideoListEngine();
        void run();
        void setMediaListProperties(MediaListProperties mediaListProperties);
        MediaListProperties mediaListProperties();
        void setFilterForSources(QString engineFilter);
        void setRequestSignature(QString requestSignature);
        void setSubRequestSignature(QString subRequestSignature);
        void activateAction();
        
    private:
        ListEngineFactory * m_parent;
        Soprano::Model * m_mainModel;
        MediaListProperties m_mediaListProperties;
        QString m_requestSignature;
        QString m_subRequestSignature;
        
    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};
#endif // VIDEOLISTENGINE_H

