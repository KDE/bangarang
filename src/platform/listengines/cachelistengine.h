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

#ifndef CACHELISTENGINE_H
#define CACHELISTENGINE_H

#include "listengine.h"

class OldMediaItem;
class MediaListProperties;
class ListEngineFactory;

/**
* This ListEngine retrieves cached media lists from the cache.
* List Resource Identifiers handled are:
*   cache://?[lri]
*/
class CacheListEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        CacheListEngine(ListEngineFactory *parent);
        ~CacheListEngine();
        void run();
};
#endif // CACHELISTENGINE_H
