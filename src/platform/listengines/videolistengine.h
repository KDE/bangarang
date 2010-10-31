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

#ifndef VIDEOLISTENGINE_H
#define VIDEOLISTENGINE_H

#include "nepomuklistengine.h"
#include "../mediavocabulary.h"
#include <QtCore>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

/**
 * This class retrieve video MediaItems from the nepomuk data store.
 * List Resource Identifiers handled are:
 *   video://clips
 *   video://tvshows?[genre]
 *   video://seasons?[genre]||[series name]
 *   video://episodes?[genre]||[series name]||[season]
 *   video://movies?[genre]||[series name]
 *   video://search?[search term]
 *   video://sources?[genre]||[series name]||[season]
 */
class VideoListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        VideoListEngine(ListEngineFactory *parent);
        ~VideoListEngine();
        void run();
        void setFilterForSources(const QString& engineFilter);
};

#endif // VIDEOLISTENGINE_H

