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

#ifndef SAVEDLISTSENGINE_H
#define SAVEDLISTSENGINE_H

#include "nepomuklistengine.h"
#include <QtCore>
#include <QDir>
#include <KUrl>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>
#include <Phonon/MediaObject>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

/**
* This ListEngine retrieves saved media lists.
* List Resource Identifiers handled are:
*   savedlists://[name]
*/
class SavedListsEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        SavedListsEngine(ListEngineFactory *parent);
        ~SavedListsEngine();
        void run();
        
    private:
        Soprano::Model * m_mainModel;
};
#endif // SAVEDLISTSENGINE_H
