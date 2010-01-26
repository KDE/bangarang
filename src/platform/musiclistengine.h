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

#ifndef MUSICLISTENGINE_H
#define MUSICLISTENGINE_H

#include "nepomuklistengine.h"
#include <QtCore>
#include <QString>
#include <Soprano/QueryResultIterator>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

/**
* This ListEngine retrieves Music MediaItems from the nepomuk data store.
* List Resource Identifiers handled are:
*   music://artists?[artist]||[album]||[genre]
*   music://albums?[artist]||[album]||[genre]
*   music://songs?[artist]||[album]||[genre]
*   music://search?[search term]
*/
class MusicListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        MusicListEngine(ListEngineFactory *parent);
        ~MusicListEngine();
        void run();
        void setFilterForSources(const QString& engineFilter);
};

#endif // MUSICLISTENGINE_H

