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

#ifndef MEDIALISTSENGINE_H
#define MEDIALISTSENGINE_H

#include "listengine.h"
#include <QtCore/QDir>
#include <KUrl>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

/**
* This ListEngine retrieves a convenient list of "Category" MediaItems.
* e.g. Albums, Highest Rated, Movies, Audio CD, etc.
* List Resource Identifiers handled are:
*   medialists://audio
*   medialists://video
*/
class MediaListsEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        MediaListsEngine(ListEngineFactory *parent);
        ~MediaListsEngine();
        void run();
        
    private:
        bool m_loadWhenReady;
        QString semanticsLriForRecent(const QString &type);
        QString semanticsLriForHighest(const QString &type);
        QString semanticsLriForFrequent(const QString &type);
        QString semanticsLriForRecentlyAdded(const QString &type);
        QList<MediaItem> loadServerList(QString type);
};
#endif // MEDIALISTSENGINE_H
