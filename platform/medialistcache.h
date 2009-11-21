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

#ifndef MEDIALISTCACHE_H
#define MEDIALISTCACHE_H

#include "mediaitemmodel.h"
#include <QObject>

class MediaListCache : public QObject
{
    Q_OBJECT
    
    public:
        MediaListCache(QObject * parent);
        ~MediaListCache();
        
        void addMediaList(MediaListProperties mediaListProperties, QList<MediaItem> mediaList);
        void removeMediaList(QString lri);
        QList<MediaItem> mediaList(QString lri);
        MediaListProperties mediaListProperties(QString lri);
        bool isInCache(QString lri);
        
    private:
        QList< QList<MediaItem> > m_mediaListCache;
        QList<MediaListProperties> m_mediaListProperties;
        QList<QString> m_lris;
        QList<int> m_mediaListSizes;
        
};
#endif //MEDIALISTCACHE_H