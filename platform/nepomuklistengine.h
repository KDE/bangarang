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

#ifndef NEPOMUKLISTENGINE_H
#define NEPOMUKLISTENGINE_H

#include "listengine.h"
#include "mediaindexer.h"
#include "mediaitemmodel.h"
#include "listenginefactory.h"
#include <QtCore>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>

/* This is an abstract base class for all list engines which
 * use nepomuk. It simply tries to initialize nepomuk during construction
 * and stores the result in local variables.
 */
class NepomukListEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        NepomukListEngine(ListEngineFactory *parent);
        virtual ~NepomukListEngine();

        virtual void run();
        
        virtual void removeSourceInfo(QList<MediaItem> mediaList);
        virtual void updateSourceInfo(QList<MediaItem> mediaList);
        
    protected:
        MediaIndexer* m_mediaIndexer;
        Soprano::Model * m_mainModel;
        bool m_nepomukInited;
        bool m_removeSourceInfo;
        bool m_updateSourceInfo;
        QList<MediaItem> m_mediaItemsInfoToRemove;
        QList<MediaItem> m_mediaItemsInfoToUpdate;
        
    private Q_SLOTS:
        void disconnectIndexer();
               
};
#endif // NEPOMUKLISTENGINE_H
