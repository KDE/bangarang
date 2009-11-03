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

ListEngineFactory::ListEngineFactory(MediaItemModel * parent) : QObject(parent)
{
    m_parent = parent;
    m_engines << "music://" << "files://" << "video://" << "cdaudio://" << "dvdvideo://" << "savedlists://" << "medialists://" << "audiostreams://";
}

ListEngineFactory::~ListEngineFactory()
{
    for (int i = 0; i < m_musicListEngines.count(); ++i) {
        if (!m_musicListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_musicListEngines.at(i)->terminate();
            m_musicListEngines.at(i)->wait();
        }
        delete m_musicListEngines.at(i);
    }
    for (int i = 0; i < m_fileListEngines.count(); ++i) {
        if (!m_fileListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_fileListEngines.at(i)->terminate();
            m_fileListEngines.at(i)->wait();
        }
        delete m_fileListEngines.at(i);
    }
    for (int i = 0; i < m_videoListEngines.count(); ++i) {
        if (!m_videoListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_videoListEngines.at(i)->terminate();
            m_videoListEngines.at(i)->wait();
        }
        delete m_videoListEngines.at(i);
    }
    for (int i = 0; i < m_cdListEngines.count(); ++i) {
        if (!m_cdListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_cdListEngines.at(i)->terminate();
            m_cdListEngines.at(i)->wait();
        }
        delete m_cdListEngines.at(i);
    }
    for (int i = 0; i < m_dvdListEngines.count(); ++i) {
        if (!m_dvdListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_dvdListEngines.at(i)->terminate();
            m_dvdListEngines.at(i)->wait();
        }
        delete m_dvdListEngines.at(i);
    }
    for (int i = 0; i < m_savedListsEngines.count(); ++i) {
        if (!m_savedListsEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_savedListsEngines.at(i)->terminate();
            m_savedListsEngines.at(i)->wait();
        }
        delete m_savedListsEngines.at(i);
    }
    for (int i = 0; i < m_mediaListsEngines.count(); ++i) {
        if (!m_mediaListsEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_mediaListsEngines.at(i)->terminate();
            m_mediaListsEngines.at(i)->wait();
        }
        delete m_mediaListsEngines.at(i);
    }
    for (int i = 0; i < m_audioStreamListEngines.count(); ++i) {
        if (!m_audioStreamListEngines.at(i)->isFinished()) {
            //This could be dangerous but list engines can't be left running after main program termination
            m_audioStreamListEngines.at(i)->terminate();
            m_audioStreamListEngines.at(i)->wait();
        }
        delete m_audioStreamListEngines.at(i);
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
                break;
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
                break;
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
            m_cdListEngines << cdListEngine;
        }
        cdListEngine->setModel(m_parent);
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
            m_dvdListEngines << dvdListEngine;
        }
        dvdListEngine->setModel(m_parent);
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
            m_savedListsEngines << savedListsEngine;
        }
        savedListsEngine->setModel(m_parent);
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
            m_mediaListsEngines << mediaListsEngine;
        }
        mediaListsEngine->setModel(m_parent);
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
            m_audioStreamListEngines << audioStreamListEngine;
        }
        audioStreamListEngine->setModel(m_parent);
        return audioStreamListEngine;        
    }
    return new ListEngine(this);
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