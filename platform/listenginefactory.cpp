#include "listenginefactory.h"
#include "listengine.h"
#include "musiclistengine.h"
#include "filelistengine.h"
#include "videolistengine.h"

ListEngineFactory::ListEngineFactory(MediaItemModel * parent) : QObject(parent)
{
    m_parent = parent;
    m_engines << "music://" << "files://" << "video://";
}

ListEngineFactory::~ListEngineFactory()
{
    for (int i = 0; i < m_musicListEngines.count(); ++i) {
        delete m_musicListEngines.at(i);
    }
    for (int j = 0; j < m_fileListEngines.count(); ++j) {
        delete m_fileListEngines.at(j);
    }
    for (int j = 0; j < m_videoListEngines.count(); ++j) {
        delete m_videoListEngines.at(j);
    }
    
}

ListEngine * ListEngineFactory::availableListEngine(QString engine)
{
    if (engine.toLower() == "music://") {
        //Search for available list engine
        bool foundListEngine = false;
        MusicListEngine * musicListEngine;
        for (int i = 0; i < m_musicListEngines.count(); ++i) {
            if (!m_musicListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                musicListEngine = m_musicListEngines.at(i);
            }
        }
        if (!foundListEngine) {
            musicListEngine = new MusicListEngine(this);
            m_musicListEngines << musicListEngine;
        }
        musicListEngine->setModel(m_parent);
        return musicListEngine;           
    } else if (engine.toLower() == "files://") {
        //Search for available list engine
        bool foundListEngine = false;
        FileListEngine * fileListEngine;
        for (int i = 0; i < m_fileListEngines.count(); ++i) {
            if (!m_fileListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                fileListEngine = m_fileListEngines.at(i);
            }
        }
        if (!foundListEngine) {
            fileListEngine = new FileListEngine(this);
            m_fileListEngines << fileListEngine;
        }
        fileListEngine->setModel(m_parent);
        return fileListEngine;        
    } else if (engine.toLower() == "video://") {
        //Search for available list engine
        bool foundListEngine = false;
        VideoListEngine * videoListEngine;
        for (int i = 0; i < m_videoListEngines.count(); ++i) {
            if (!m_videoListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                videoListEngine = m_videoListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            videoListEngine = new VideoListEngine(this);
            m_videoListEngines << videoListEngine;
        }
        videoListEngine->setModel(m_parent);
        return videoListEngine;        
    }
}

QString ListEngineFactory::generateRequestSignature()
{
    m_requestSignatureSeed = m_requestSignatureSeed + 1;
    return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + m_requestSignatureSeed;
}

bool ListEngineFactory::engineExists(QString engine)
{
    if (m_engines.indexOf(engine) != -1) {
        return true;
    } else {
        return false;
    }
}