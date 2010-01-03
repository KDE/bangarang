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

#ifndef AUDIOCLIPSLISTENGINE_H
#define AUDIOCLIPSLISTENGINE_H

#include "nepomuklistengine.h"
#include <QtCore>
#include <Soprano/QueryResultIterator>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;


/**
 * This ListEngine retrieves Audio Clips from the nepmuk data store.
 * List Resource Identifiers handled are:
 *   audioclips://
 *   audioclips://search?[search term]
 */
class AudioClipsListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        AudioClipsListEngine(ListEngineFactory *parent);
        ~AudioClipsListEngine();
        void run();
        void setFilterForSources(const QString& engineFilter);

    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};

#endif // AUDIOCLIPSLISTENGINE_H

