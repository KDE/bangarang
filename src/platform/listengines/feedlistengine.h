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

#ifndef FEEDLISTENGINE_H
#define FEEDLISTENGINE_H

#include "nepomuklistengine.h"
#include <QtCore>
#include <QDir>
#include <KUrl>
#include <kjob.h>
#include <kio/copyjob.h>
#include <QDomDocument>
#include <QDomNodeList>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

/**
* This ListEngine retrieves a media feeds.
*/
class FeedListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        FeedListEngine(ListEngineFactory *parent);
        ~FeedListEngine();
        void run();
        
    private:
        QString m_feedFilePath;
        QList<MediaItem> m_mediaList;
        QList<KUrl> m_artworkUrlList;
        bool m_fetchingThumbnails;

        QDomElement getPreferredTag(const QDomNodeList &itemNodes, const QStringList &tagPref);
        QList<QDomElement> getPreferredTags(const QDomNodeList &itemNodes, const QStringList &tagPref);
        
    private slots:
        void downloadComplete(const KUrl &from, const KUrl &to);
        
};
#endif // FEEDLISTENGINE_H
