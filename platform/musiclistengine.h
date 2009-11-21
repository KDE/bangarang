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
#include <Soprano/QueryResultIterator>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

class MusicQuery {
    public:
        MusicQuery(bool distinct = true);
        
        void selectResource();
        void selectArtist(bool optional=false);
        void selectAlbum(bool optional=false);
        void selectTitle(bool optional=false);
        void selectDuration(bool optional=false);
        void selectCreated(bool optional=false);
        void selectTrackNumber(bool optional=false);
        void selectGenre(bool optional=false);
        void selectRating(bool optional=false);
        void selectDescription(bool optional=false);
        void selectArtwork(bool optional=false);
        
        void hasArtist(QString album);
        void hasNoArtist();
        void hasAlbum(QString album);
        void hasNoAlbum();
        void hasGenre(QString genre);
        void hasNoGenre();
        
        void searchString(QString str);
        
        void orderBy(QString var);
        
        
        Soprano::QueryResultIterator executeSelect(Soprano::Model* model);
        bool executeAsk(Soprano::Model* model);
        
    private:
        bool m_distinct;
        
        bool m_selectResource;
        bool m_selectArtist;
        bool m_selectAlbum;
        bool m_selectTitle;
        bool m_selectDuration;
        bool m_selectCreated;
        bool m_selectTrackNumber;
        bool m_selectGenre;
        bool m_selectRating;
        bool m_selectDescription;
        bool m_selectArtwork;
        
        QString m_artistCondition;
        QString m_albumCondition;
        QString m_titleCondition;
        QString m_durationCondition;
        QString m_createdCondition;
        QString m_trackNumberCondition;
        QString m_genreCondition;
        QString m_searchCondition;
        QString m_ratingCondition;
        QString m_descriptionCondition;
        QString m_artworkCondition;
        
        QString m_order;
        
        QString addOptional(bool optional, QString str);
        QString getPrefix();
};

class MusicListEngine : public NepomukListEngine
{
    Q_OBJECT
    
    public:
        MusicListEngine(ListEngineFactory *parent);
        ~MusicListEngine();
        void run();
        void setFilterForSources(const QString& engineFilter);
        void updateSourceInfo(QList<MediaItem> mediaList);
        
    private:
        MediaItem createMediaItem(Soprano::QueryResultIterator& it);
        
    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};
#endif // MUSICLISTENGINE_H

