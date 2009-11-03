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

#ifndef AUDIOSTREAMLISTENGINE_H
#define AUDIOSTREAMLISTENGINE_H

#include "listengine.h"
#include <QtCore>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;

class AudioStreamQuery {
    public:
        AudioStreamQuery(bool distinct = true);
        
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

class AudioStreamListEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        AudioStreamListEngine(ListEngineFactory *parent);
        ~AudioStreamListEngine();
        void run();
        void setMediaListProperties(MediaListProperties mediaListProperties);
        MediaListProperties mediaListProperties();
        void setFilterForSources(QString engineFilter);
        void setRequestSignature(QString requestSignature);
        void setSubRequestSignature(QString subRequestSignature);
        void activateAction();
        
    private:
        ListEngineFactory * m_parent;
        Soprano::Model * m_mainModel;
        MediaListProperties m_mediaListProperties;
        QString m_requestSignature;
        QString m_subRequestSignature;
        MediaItem createMediaItem(Soprano::QueryResultIterator& it);
        
    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};
#endif // AUDIOSTREAMLISTENGINE_H

