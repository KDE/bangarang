/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Florian Weik (f.weik@web.de)
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

#ifndef TAGLISTENGINE_H
#define TAGLISTENGINE_H

#include "nepomuklistengine.h"
#include <QtCore>
#include <Soprano/QueryResultIterator>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

/**
 * This class retrieves a list of MediaItems based on 
 * their tags in the nepomuk data store. Return a list
 * of all tags if [tag] is empty
 *   tag://[tag]?audio
 *   tag://[tag]?video
 */
class TagListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        TagListEngine(ListEngineFactory *parent);
        ~TagListEngine();
        void run();
};

#endif // TAGLISTENGINE_H

