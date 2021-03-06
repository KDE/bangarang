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

#ifndef SEMANTICSLISTENGINE_H
#define SEMANTICSLISTENGINE_H

#include "nepomuklistengine.h"
#include <Soprano/QueryResultIterator>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

/**
 * This class retrieves a list of MediaItems based on 
 * playback semantics in the nepomuk data store.
 * e.g. Highest rated, Frequently played, etc.
 * List Resource Identifiers handled are:
 *   semantics://frequent?audio
 *   semantics://frequent?video
 *   semantics://recent?audio
 *   semantics://recent?video
 *   semantics://highest?audio
 *   semantics://highest?video
 */
class SemanticsListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        SemanticsListEngine(ListEngineFactory *parent);
        ~SemanticsListEngine();
        void run();

    private:
        void addUniqueGenreGroup(QString field, MediaItem mediaItem, QList<MediaItem>* mediaList, int limit);
};

#endif // SEMANTICSLISTENGINE_H

