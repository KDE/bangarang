#ifndef LISTENGINEFACTORY_H
#define LISTENGINEFACTORY_H

#include "mediaitemmodel.h"
#include <QObject>
#include <QList>

class ListEngine;
class MusicListEngine;
class FileListEngine;
class VideoListEngine;
class CDListEngine;

class ListEngineFactory : public QObject
{
    Q_OBJECT
    
    public:
        ListEngineFactory(MediaItemModel *parent);
        ~ListEngineFactory();
        virtual ListEngine* availableListEngine(QString engine);
        QString generateRequestSignature();
        bool engineExists(QString engine);
        
    private:
        MediaItemModel * m_parent;
        int m_requestSignatureSeed;
        QStringList m_engines;
        QList<MusicListEngine *> m_musicListEngines;
        QList<FileListEngine *> m_fileListEngines;
        QList<VideoListEngine *> m_videoListEngines;
        QList<CDListEngine *> m_cdListEngines;
};
#endif // LISTENGINEFACTORY_H
        