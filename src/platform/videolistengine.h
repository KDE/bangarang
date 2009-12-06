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
#include <QtCore>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

/**
 * This class retrieve video MediaItems from the nepomuk data store.
 */
class VideoListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        VideoListEngine(ListEngineFactory *parent);
        ~VideoListEngine();
        void run();
        void setFilterForSources(const QString& engineFilter);
        
    private:
        MediaItem createMediaItem(Soprano::QueryResultIterator& it);


    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
};

class VideoQuery {
    public:
        VideoQuery(bool distinct = true);
        
        void selectResource();
        void selectSeason(bool optional=false);
        void selectSeriesName(bool optional=false);
        void selectTitle(bool optional=false);
        void selectDuration(bool optional=false);
        void selectEpisode(bool optional=false);
        void selectDescription(bool optional=false);
        void selectCreated(bool optional=false);
        void selectGenre(bool optional=false);
        void selectIsTVShow(bool optional=false);
        void selectIsMovie(bool optional=false);
        void selectRating(bool optional=false);
        void selectArtwork(bool optional=false);
        
        void isTVShow(bool flag);
        void isMovie(bool flag);
        
        void hasSeason(int season);
        void hasNoSeason();
        
        void hasSeriesName(QString seriesName);
        void hasGenre(QString genre);
        void hasNoSeriesName();
        
        void searchString(QString str);
        
        void orderBy(QString var);
        
        
        Soprano::QueryResultIterator executeSelect(Soprano::Model* model);
        bool executeAsk(Soprano::Model* model);
        
    private:
        bool m_distinct;
        
        bool m_selectResource;
        bool m_selectSeason;
        bool m_selectSeriesName;
        bool m_selectTitle;
        bool m_selectDuration;
        bool m_selectEpisode;
        bool m_selectDescription;
        bool m_selectCreated;
        bool m_selectGenre;
        bool m_selectIsTVShow;
        bool m_selectIsMovie;
        bool m_selectRating;
        bool m_selectArtwork;
        
        QString m_seasonCondition;
        QString m_seriesNameCondition;
        QString m_titleCondition;
        QString m_durationCondition;
        QString m_episodeCondition;
        QString m_descriptionCondition;
        QString m_createdCondition;
        QString m_genreCondition;
        QString m_TVShowCondition;
        QString m_movieCondition;
        QString m_searchCondition;
        QString m_ratingCondition;
        QString m_artworkCondition;
        
        QString m_order;
        
        QString addOptional(bool optional, QString str);
        QString getPrefix();
};

#endif // VIDEOLISTENGINE_H
