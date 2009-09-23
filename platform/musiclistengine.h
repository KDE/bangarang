#ifndef MUSICLISTENGINE_H
#define MUSICLISTENGINE_H

#include "mediaitemmodel.h"
#include "listengine.h"
#include "listenginefactory.h"
#include <QtCore>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>

class MusicListEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        MusicListEngine(ListEngineFactory *parent);
        ~MusicListEngine();
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
#endif // MUSICLISTENGINE_H

