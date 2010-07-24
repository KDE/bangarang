/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#include "dbpediainfofetcher.h"
#include "dbpediaquery.h"
#include "mediaitemmodel.h"
#include <KLocale>
#include <KDebug>
#include <Soprano/LiteralValue>
#include <Soprano/Node>

DBPediaInfoFetcher::DBPediaInfoFetcher(QObject * parent) : InfoFetcher(parent)
{
    m_name = i18n("DBPedia");
    m_dbPediaQuery = new DBPediaQuery(this);
    connect (m_dbPediaQuery, SIGNAL(gotMovieInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotMovieInfo(bool , const QList<Soprano::BindingSet>, const QString)));
}

DBPediaInfoFetcher::~DBPediaInfoFetcher()
{
}

void DBPediaInfoFetcher::fetchInfo(QList<MediaItem> mediaList)
{
    m_mediaList.clear();
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        if (mediaItem.type == "Video") {
            QString subType = mediaItem.fields["videoType"].toString();
            if (subType == "Movie") {
                m_mediaList.append(mediaItem);
                kDebug() << "getting movie info:" << mediaList.count();
                m_dbPediaQuery->getMovieInfo(mediaItem.title);
            }
        }
    }
}

void DBPediaInfoFetcher::gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
{
    int index = -1;
    for (int i = 0; i< m_mediaList.count(); i++) {
        QString keyForCurrentItem = "Movie:" + m_mediaList.at(i).title;
        if (keyForCurrentItem == requestKey) {
            index = i;
            break;
        }
    }

    kDebug() << "Got movie info for " << requestKey << " index:" << index << " successful:" << successful;
    
    if (index != -1 && successful) {
        if (results.count() > 0) {
            MediaItem mediaItem = m_mediaList.at(index);
            
            Soprano::BindingSet binding = results.at(0);
            
            //Get Thumbnail
            KUrl thumbnailUrl = KUrl(binding.value("thumbnail").uri());
            if (thumbnailUrl.isValid()) {
                //getThumbnail(thumbnailUrl);
            }
            
            //Set Title
            mediaItem.title = binding.value("title").literal().toString().trimmed();
            mediaItem.fields["title"] = mediaItem.title;
            
            //Set Description
            mediaItem.fields["description"] = binding.value("description").literal().toString().trimmed();
            
            m_mediaList.replace(index, mediaItem);
            emit infoFetched(mediaItem);
        }
    }
}

