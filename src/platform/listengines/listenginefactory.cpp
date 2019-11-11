/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://commits.kde.org/bangarang>
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
#include "../downloader.h"
#include "listengine.h"

#include "filelistengine.h"
#include "savedlistsengine.h"
#include "medialistsengine.h"
#include "cachelistengine.h"

#include <QDebug>

ListEngineFactory::ListEngineFactory(MediaItemModel * parent) : QObject(parent)
{
    m_parent = parent;
    m_engines
              << EngineDescription(EngineTypeFiles, "files://")
              << EngineDescription(EngineTypeSavedLists, "savedlists://")
              << EngineDescription(EngineTypeMediaLists, "medialists://")
              << EngineDescription(EngineTypeCache, "cache://");

    m_downloader = new Downloader(this);
}

ListEngineFactory::~ListEngineFactory()
{
    stopAll(200, true);
}

ListEngine* ListEngineFactory::createEngine(const EngineType type, MediaItemModel* model)
{
    ListEngine *eng;
    switch (type) {

        case EngineTypeFiles:
            eng = new FileListEngine(this);
            break;
            
            case EngineTypeSavedLists:
            eng = new SavedListsEngine(this);
               break;

        case EngineTypeMediaLists:
            eng = new MediaListsEngine(this);
            break;

    case EngineTypeCache:
        eng = new CacheListEngine(this);
        break;

        default:
            eng = new ListEngine(this);
            break;
    }
    eng->setModel(model);
    return eng;
}

QString ListEngineFactory::engineStringFromType(const EngineType type)
{
    for (int i = 0; i < m_engines.count(); i++) {
        const EngineDescription &cur = m_engines.at(i);
        if ( cur.first == type )
            return cur.second;
    }
    return QString();
}

EngineType ListEngineFactory::engineTypeFromString(const QString& str)
{
    for (int i = 0; i < m_engines.count(); i++) {
        const EngineDescription &cur = m_engines.at(i);
        if ( cur.second == str )
            return cur.first;
    }
    return EngineTypeUnknown;
}


ListEngine * ListEngineFactory::availableListEngine(const EngineType type)
{
    for (int i = 0; i < m_initializedEngines.count(); ++i)
    {
        const EnginePointerType &cur = m_initializedEngines.at(i);
        if (cur.second == type && !cur.first->isRunning()) {
            return cur.first;
        }
    }
    ListEngine *eng = createEngine(type, m_parent);
    m_initializedEngines << EnginePointerType(eng, type);
    return eng;
}

QString ListEngineFactory::generateRequestSignature()
{
    m_requestSignatureSeed = m_requestSignatureSeed + 1;
    return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + m_requestSignatureSeed;
}

bool ListEngineFactory::engineExists(const EngineType type)
{
    return ( type != EngineTypeUnknown );
}

Downloader * ListEngineFactory::downloader()
{
    return m_downloader;
}

void ListEngineFactory::stopAll(unsigned long waitToTerminate, bool quitEventLoop)
{
    for (int i = 0; i < m_initializedEngines.count(); ++i) {
        ListEngine *eng = m_initializedEngines.at(i).first;
        if (eng->isRunning()) {
            eng->stop(waitToTerminate, quitEventLoop);
        }
    }
}

void ListEngineFactory::resumeAll()
{
    for (int i = 0; i < m_initializedEngines.count(); ++i) {
        ListEngine *eng = m_initializedEngines.at(i).first;
        if (eng->isRunning()) {
            eng->resume();
        }
    }
}

void ListEngineFactory::load(MediaListProperties mediaListProperties, const QString &requestSignature, const QString &subRequestSignature)
{
    if (mediaListProperties.engine().isEmpty()) {
        return;
    }
    EngineType type = engineTypeFromString(mediaListProperties.engine());
    if (engineExists(type)) {
        stopAll();
        ListEngine * eng = availableListEngine(type);
        eng->setRequestSignature(requestSignature);
        eng->setSubRequestSignature(subRequestSignature);
        eng->setMediaListProperties(mediaListProperties);
        eng->start();
    }
}
