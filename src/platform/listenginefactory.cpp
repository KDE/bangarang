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

#include "listenginefactory.h"
#include "listengine.h"
#include "musiclistengine.h"
#include "filelistengine.h"
#include "videolistengine.h"
#include "cdlistengine.h"
#include "dvdlistengine.h"
#include "savedlistsengine.h"
#include "medialistsengine.h"
#include "audiostreamlistengine.h"
#include "semanticslistengine.h"
#include "cachelistengine.h"
#include "audioclipslistengine.h"
#include <KDebug>

ListEngineFactory::ListEngineFactory(MediaItemModel * parent) : QObject(parent)
{
    m_parent = parent;
    m_engines << "music://" << "files://" << "video://" << "cdaudio://" << "dvdvideo://" << "savedlists://" << "medialists://" << "audiostreams://" << "semantics://" << "cache://" << "audioclips://";
}

ListEngineFactory::~ListEngineFactory()
{
    for (int i = 0; i < m_musicListEngines.count(); ++i) {
        if (!m_musicListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_musicListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_fileListEngines.count(); ++i) {
        if (!m_fileListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_fileListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_videoListEngines.count(); ++i) {
        if (!m_videoListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_videoListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_cdListEngines.count(); ++i) {
        if (!m_cdListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_cdListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_dvdListEngines.count(); ++i) {
        if (!m_dvdListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_dvdListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_savedListsEngines.count(); ++i) {
        if (!m_savedListsEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_savedListsEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_mediaListsEngines.count(); ++i) {
        if (!m_mediaListsEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_mediaListsEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_audioStreamListEngines.count(); ++i) {
        if (!m_audioStreamListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_audioStreamListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_semanticsListEngines.count(); ++i) {
        if (!m_semanticsListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_semanticsListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_cacheListEngines.count(); ++i) {
        if (!m_cacheListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_cacheListEngines.at(i)->terminate();
        }
    }
    for (int i = 0; i < m_audioClipsListEngines.count(); ++i) {
        if (!m_audioClipsListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_audioClipsListEngines.at(i)->terminate();
        }
    }
}

ListEngine * ListEngineFactory::availableListEngine(const QString &engine)
{
    if (engine.toLower() == "music://") {
        //Search for available list engine
        bool foundListEngine = false;
        MusicListEngine * musicListEngine;
        for (int i = 0; i < m_musicListEngines.count(); ++i) {
            if (!m_musicListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                musicListEngine = m_musicListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            musicListEngine = new MusicListEngine(this);
            musicListEngine->setModel(m_parent);
            m_musicListEngines << musicListEngine;
        }
        return musicListEngine;           
    } else if (engine.toLower() == "files://") {
        //Search for available list engine
        bool foundListEngine = false;
        FileListEngine * fileListEngine;
        for (int i = 0; i < m_fileListEngines.count(); ++i) {
            if (!m_fileListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                fileListEngine = m_fileListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            fileListEngine = new FileListEngine(this);
            fileListEngine->setModel(m_parent);
            m_fileListEngines << fileListEngine;
        }
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
            videoListEngine->setModel(m_parent);
            m_videoListEngines << videoListEngine;
        }
        return videoListEngine;        
    } else if (engine.toLower() == "cdaudio://") {
        //Search for available list engine
        bool foundListEngine = false;
        CDListEngine * cdListEngine;
        for (int i = 0; i < m_cdListEngines.count(); ++i) {
            if (!m_cdListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                cdListEngine = m_cdListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            cdListEngine = new CDListEngine(this);
            cdListEngine->setModel(m_parent);
            m_cdListEngines << cdListEngine;
        }
        return cdListEngine;        
    } else if (engine.toLower() == "dvdvideo://") {
        //Search for available list engine
        bool foundListEngine = false;
        DVDListEngine * dvdListEngine;
        for (int i = 0; i < m_dvdListEngines.count(); ++i) {
            if (!m_dvdListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                dvdListEngine = m_dvdListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            dvdListEngine = new DVDListEngine(this);
            dvdListEngine->setModel(m_parent);
            m_dvdListEngines << dvdListEngine;
        }
        return dvdListEngine;        
    } else if (engine.toLower() == "savedlists://") {
        //Search for available list engine
        bool foundListEngine = false;
        SavedListsEngine * savedListsEngine;
        for (int i = 0; i < m_savedListsEngines.count(); ++i) {
            if (!m_savedListsEngines.at(i)->isRunning()) {
                foundListEngine = true;
                savedListsEngine = m_savedListsEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            savedListsEngine = new SavedListsEngine(this);
            savedListsEngine->setModel(m_parent);
            m_savedListsEngines << savedListsEngine;
        }
        return savedListsEngine;        
    } else if (engine.toLower() == "medialists://") {
        //Search for available list engine
        bool foundListEngine = false;
        MediaListsEngine * mediaListsEngine;
        for (int i = 0; i < m_mediaListsEngines.count(); ++i) {
            if (!m_mediaListsEngines.at(i)->isRunning()) {
                foundListEngine = true;
                mediaListsEngine = m_mediaListsEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            mediaListsEngine = new MediaListsEngine(this);
            mediaListsEngine->setModel(m_parent);
            m_mediaListsEngines << mediaListsEngine;
        }
        return mediaListsEngine;        
    } else if (engine.toLower() == "audiostreams://") {
        //Search for available list engine
        bool foundListEngine = false;
        AudioStreamListEngine * audioStreamListEngine;
        for (int i = 0; i < m_audioStreamListEngines.count(); ++i) {
            if (!m_audioStreamListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                audioStreamListEngine = m_audioStreamListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            audioStreamListEngine = new AudioStreamListEngine(this);
            audioStreamListEngine->setModel(m_parent);
            m_audioStreamListEngines << audioStreamListEngine;
        }
        return audioStreamListEngine;        
    } else if (engine.toLower() == "semantics://") {
        //Search for available list engine
        bool foundListEngine = false;
        SemanticsListEngine * semanticsListEngine;
        for (int i = 0; i < m_semanticsListEngines.count(); ++i) {
            if (!m_semanticsListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                semanticsListEngine = m_semanticsListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            semanticsListEngine = new SemanticsListEngine(this);
            semanticsListEngine->setModel(m_parent);
            m_semanticsListEngines << semanticsListEngine;
        }
        return semanticsListEngine;        
    } else if (engine.toLower() == "cache://") {
        //Search for available list engine
        bool foundListEngine = false;
        CacheListEngine * cacheListEngine;
        for (int i = 0; i < m_cacheListEngines.count(); ++i) {
            if (!m_cacheListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                cacheListEngine = m_cacheListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            cacheListEngine = new CacheListEngine(this);
            cacheListEngine->setModel(m_parent);
            m_cacheListEngines << cacheListEngine;
        }
        return cacheListEngine;        
    } else if (engine.toLower() == "audioclips://") {
        //Search for available list engine
        bool foundListEngine = false;
        AudioClipsListEngine * audioClipsListEngine;
        for (int i = 0; i < m_audioClipsListEngines.count(); ++i) {
            if (!m_audioClipsListEngines.at(i)->isRunning()) {
                foundListEngine = true;
                audioClipsListEngine = m_audioClipsListEngines.at(i);
                break;
            }
        }
        if (!foundListEngine) {
            audioClipsListEngine = new AudioClipsListEngine(this);
            audioClipsListEngine->setModel(m_parent);
            m_audioClipsListEngines << audioClipsListEngine;
        }
        return audioClipsListEngine;        
    }
    return new ListEngine(this);
}

QString ListEngineFactory::generateRequestSignature()
{
    m_requestSignatureSeed = m_requestSignatureSeed + 1;
    return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + m_requestSignatureSeed;
}

bool ListEngineFactory::engineExists(const QString &engine)
{
    if (m_engines.indexOf(engine) != -1) {
        return true;
    } else {
        return false;
    }
}