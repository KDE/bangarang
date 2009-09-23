#ifndef LISTENGINEFACTORY_H
#define LISTENGINEFACTORY_H

#include "mediaitemmodel.h"
#include <QObject>
#include <QList>

class ListEngine;
class MusicListEngine;
class FileListEngine;

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
};
#endif // LISTENGINEFACTORY_H
        