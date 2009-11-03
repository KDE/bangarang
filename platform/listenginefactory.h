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
class DVDListEngine;
class SavedListsEngine;
class MediaListsEngine;
class AudioStreamListEngine;

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
        QList<DVDListEngine *> m_dvdListEngines;
        QList<SavedListsEngine *> m_savedListsEngines;
        QList<MediaListsEngine *> m_mediaListsEngines;
        QList<AudioStreamListEngine *> m_audioStreamListEngines;
};
#endif // LISTENGINEFACTORY_H
        