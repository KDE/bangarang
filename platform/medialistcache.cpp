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

#include "medialistcache.h"

#include <KDebug>

MediaListCache::MediaListCache(QObject * parent) : QObject(parent)
{
}

MediaListCache::~MediaListCache()
{
}

void MediaListCache::addMediaList(MediaListProperties mediaListProperties, QList<MediaItem> mediaList)
{
    QString lri = mediaListProperties.lri;
    if (m_lris.indexOf(lri) == -1) {
        m_mediaListCache.append(mediaList);
        m_mediaListProperties.append(mediaListProperties);
        m_mediaListSizes.append(mediaList.count());
        m_lris.append(lri);
    } else {
        //Only one instance of any lri mediaList can exist in the cache
        removeMediaList(lri);
        m_mediaListCache.append(mediaList);
        m_mediaListProperties.append(mediaListProperties);
        m_mediaListSizes.append(mediaList.count());
        m_lris.append(lri);
    }
}

void MediaListCache::removeMediaList(QString lri)
{
    if (m_lris.indexOf(lri) != -1) {
        int index = m_lris.indexOf(lri);
        m_mediaListCache.removeAt(index);
        m_mediaListProperties.removeAt(index);
        m_mediaListSizes.removeAt(index);
        m_lris.removeAt(index);
    }
}

QList<MediaItem> MediaListCache::mediaList(QString lri)
{
    if (m_lris.indexOf(lri) != -1) {
        int index = m_lris.indexOf(lri);
        return m_mediaListCache.at(index);
    } else {
        QList<MediaItem> emptyList;
        return emptyList;
    }
}

MediaListProperties MediaListCache::mediaListProperties(QString lri)
{
    if (m_lris.indexOf(lri) != -1) {
        int index = m_lris.indexOf(lri);
        return m_mediaListProperties.at(index);
    } else {
        MediaListProperties mediaListProperties;
        return mediaListProperties;
    }
}

bool MediaListCache::isInCache(QString lri)
{
    if (m_lris.indexOf(lri) != -1) {
        return true;
    } else {
        return false;
    }
}
