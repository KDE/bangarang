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

#ifndef FILELISTENGINE_H
#define FILELISTENGINE_H

#include "nepomuklistengine.h"
#include <QtCore>
#include <QDir>
#include <KUrl>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

/**
* This ListEngine retrieves media files.
* It also automatically updates Music file information in the nepomuk data store.
* List Resource Identifiers handled are:
*  files://audio
*  files://video
*  files://audio?getFiles||[filelist]
*  files://audio?getFolder||[folder]
*  files://video?getFiles||[filelist]
*  files://video?getFolder||[folder]
*/
class FileListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        FileListEngine(ListEngineFactory *parent);
        ~FileListEngine();
        void run();
        void activateAction();
        
    private:
        QFileInfoList crawlDir(const QDir &dir, const QStringList &mimeFilter);
        KUrl::List QFileInfoListToKUrlList(const QFileInfoList &fileInfoList);
        QList<MediaItem> readAudioUrlList(const KUrl::List &fileList);
        QList<MediaItem> readVideoUrlList(const KUrl::List &fileList);
        QString engineFilterFromUrlList(const KUrl::List &fileList);
        KUrl::List m_fileList;
        QString m_directoryPath;
        bool m_getFilesAction;
        bool m_getFolderAction;
        QList<MediaItem> m_mediaListToIndex;

    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};
#endif // FILELISTENGINE_H
