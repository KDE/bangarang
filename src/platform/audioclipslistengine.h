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
        
    private:
        MediaItem createMediaItem(Soprano::QueryResultIterator& it);
        
    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};

/**
* This class constructs a SPARQL query to query the nepomuk store.
*/
class AudioClipsQuery {
    public:
        AudioClipsQuery(bool distinct = true);
        
        void selectResource();
        void selectTitle(bool optional=false);
        void selectGenre(bool optional=false);
        void selectRating(bool optional=false);
        void selectDescription(bool optional=false);
        void selectArtwork(bool optional=false);
        
        void searchString(QString str);
        
        void orderBy(QString var);
        
        Soprano::QueryResultIterator executeSelect(Soprano::Model* model);
        bool executeAsk(Soprano::Model* model);
        
    private:
        bool m_distinct;
        
        bool m_selectResource;
        bool m_selectTitle;
        bool m_selectGenre;
        bool m_selectRating;
        bool m_selectDescription;
        bool m_selectArtwork;
        
        QString m_titleCondition;
        QString m_genreCondition;
        QString m_searchCondition;
        QString m_ratingCondition;
        QString m_descriptionCondition;
        QString m_artworkCondition;
        
        QString m_order;
        
        QString addOptional(bool optional, QString str);
        QString getPrefix();
};

#endif // AUDIOCLIPSLISTENGINE_H

