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

#include "nepomuklistengine.h"
#include <KDebug>

NepomukListEngine::NepomukListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
        m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
        m_mediaIndexer = new MediaIndexer(this);
    } else {
        m_nepomukInited = false; //no resource manager
    }
    m_removeSourceInfo = false;

}

NepomukListEngine::~NepomukListEngine()
{
	if (m_nepomukInited) {
        delete m_mediaIndexer;
    }
}

void NepomukListEngine::run()
{
    if (m_removeSourceInfo) {
        connect(m_mediaIndexer, SIGNAL(urlInfoRemoved(QString)), model(), SLOT(removeMediaItem(QString)));
        connect(m_mediaIndexer, SIGNAL(indexingComplete()), this, SLOT(disconnectIndexer()));
        m_mediaIndexer->removeInfo(m_mediaItemsInfoToRemove);
        m_removeSourceInfo = false;
        m_mediaItemsInfoToRemove.clear();
    }
    if (m_updateSourceInfo) {
        connect(m_mediaIndexer, SIGNAL(sourceInfoUpdated(MediaItem)), model(), SLOT(updateMediaItem(MediaItem)));
        connect(m_mediaIndexer, SIGNAL(indexingComplete()), this, SLOT(disconnectIndexer()));
        m_mediaIndexer->indexMediaItems(m_mediaItemsInfoToUpdate);
        m_updateSourceInfo = false;
        m_mediaItemsInfoToUpdate.clear();
    }
}

void NepomukListEngine::removeSourceInfo(QList<MediaItem> mediaList)
{
    if (m_nepomukInited) {
        m_mediaItemsInfoToRemove = mediaList;
        m_removeSourceInfo = true;
        NepomukListEngine::run();
    }
}

void NepomukListEngine::updateSourceInfo(QList<MediaItem> mediaList)
{
    if (m_nepomukInited) {
        m_mediaItemsInfoToUpdate = mediaList;
        m_updateSourceInfo = true;
        NepomukListEngine::run();
    }
}

void NepomukListEngine::disconnectIndexer()
{
    disconnect(m_mediaIndexer, SIGNAL(urlInfoRemoved(QString)), model(), SLOT(removeMediaItem(QString)));
    disconnect(m_mediaIndexer, SIGNAL(sourceInfoUpdated(MediaItem)), model(), SLOT(updateMediaItem(MediaItem)));
}
