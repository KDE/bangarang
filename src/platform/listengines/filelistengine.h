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
#include <KFilePlacesModel>
#include <KDirModel>
#include <KDirSortFilterProxyModel>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

/**
* This ListEngine retrieves media files.
* List Resource Identifiers handled are:
*  files://audio?browseFolder||[folder]
*  files://video?browseFolder||[folder]
*/
class FileListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        FileListEngine(ListEngineFactory *parent);
        ~FileListEngine();
        void run();
        void setFilterForSources(const QString& engineFilter);
        void updateSourceInfo(QList<MediaItem> mediaList, bool nepomukOnly = false);
        
    private:
        KUrl::List m_fileList;
        QString m_directoryPath;
        KFilePlacesModel *m_filePlacesModel;
        KDirModel *m_dirModel;
        KDirSortFilterProxyModel *m_dirSortProxyModel;
        bool m_updateNepomukOnly;
        QList<MediaItem> getFiles(QList<MediaItem> mediaList, bool basicInfo = false, bool emitStatus = false);
        QFileInfoList crawlDir(const QDir &dir, QString engineArg);

    private Q_SLOTS:
        void listingComplete(const KUrl &url);
};
#endif // FILELISTENGINE_H
