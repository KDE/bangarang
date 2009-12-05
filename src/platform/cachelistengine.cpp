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

#include "mediaitemmodel.h"
#include "cachelistengine.h"
#include "listenginefactory.h"
#include "medialistcache.h"

CacheListEngine::CacheListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
}

CacheListEngine::~CacheListEngine()
{
}

void CacheListEngine::run()
{
    QList<MediaItem> mediaList;
    MediaListProperties mediaListProperties;
    
    QString lri = m_mediaListProperties.engineFilter();

    if (!lri.isEmpty()) {
        if (model()->mediaListCache()->isInCache(lri)) {
            mediaList = model()->mediaListCache()->mediaList(lri);
            mediaListProperties = model()->mediaListCache()->mediaListProperties(lri);
        }
    }
    model()->addResults(m_requestSignature, mediaList, mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}
