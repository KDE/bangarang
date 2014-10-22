/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://projects.kde.org/projects/playground/multimedia/bangarang>
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

#include "listengine.h"
#include <QtCore/QDir>
#include <KUrl>
//#include <Nepomuk2/Resource>
//#include <Nepomuk2/ResourceManager>
////#include <Soprano/Model>
#include <phonon/mediaobject.h>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

/**
* This ListEngine retrieves saved media lists.
* List Resource Identifiers handled are:
*   savedlists://[name]
*/
class SavedListsEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        SavedListsEngine(ListEngineFactory *parent);
        ~SavedListsEngine();
        void run();
        
};
#endif // SAVEDLISTSENGINE_H
