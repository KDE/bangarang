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

#ifndef MEDIALISTSENGINE_H
#define MEDIALISTSENGINE_H

#include "listengine.h"
#include <QtCore>
#include <QDir>
#include <KUrl>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

class MediaListsEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        MediaListsEngine(ListEngineFactory *parent);
        ~MediaListsEngine();
        void run();
        void setMediaListProperties(MediaListProperties mediaListProperties);
        MediaListProperties mediaListProperties();
        void setFilterForSources(QString engineFilter);
        void setRequestSignature(QString requestSignature);
        void setSubRequestSignature(QString subRequestSignature);
        void activateAction();
        
    private:
        ListEngineFactory * m_parent;
        MediaListProperties m_mediaListProperties;
        QString m_requestSignature;
        QString m_subRequestSignature;
        MediaIndexer * m_mediaIndexer;
        QFileInfoList crawlDir(QDir dir, QStringList mimeFilter);
        KUrl::List QFileInfoListToKUrlList(QFileInfoList fileInfoList);
        bool m_loadWhenReady;
    
    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};
#endif // MEDIALISTSENGINE_H