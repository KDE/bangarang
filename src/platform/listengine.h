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

#ifndef LISTENGINE_H
#define LISTENGINE_H

#include "mediaitemmodel.h"
#include "listenginefactory.h"
#include <QtCore>

/**
* This is the base ListEngine class.
* It is a QThread to allow it to run asynchronously without blocking the gui
* on long queries.
*/
class ListEngine : public QThread
{
    Q_OBJECT
    
    public:
        /**
         * Constructor
         */
        ListEngine(ListEngineFactory *parent);
        
        /**
         * Destructor
         */
        virtual ~ListEngine();

        /**
         * Returns MediaListProperties currently used by the ListEngine
         */
        const MediaListProperties& mediaListProperties() const;
        
        /**
         * Returns the requestSignature currently used by the ListEngine
         */
        const QString& requestSignature() const;
        
        /**
         * Sets the MediaListProperties to be used by the ListEngine
         *
         * @param mediaListProperties the MediaListProperties to be used
         */
        void setMediaListProperties(const MediaListProperties& mediaListProperties);
        
        /**
         * Sets the request signature to be used by the ListEngine
         *
         * @param requestSignature the request signature to be used
         */
        void setRequestSignature(const QString& requestSignature);
        
        /**
         * Sets the sub-request signature to be used by the ListEngine.
         * This is used by the MediaItemModel when the ListEngine is used
         * to retrieve a list of MediaItems corresponding to a "Category" 
         * MediaItem found in while performing MediaItemModel::loadSources().
         */
        void setSubRequestSignature(const QString& subRequestSignature);
        
        /**
         * Returns the sub-request signature currently used by the ListEngine.
         *
         * @param subRequestSignature the sub-request signature to be used
         */
        const QString& subRequestSignature() const;

        /**
         * Sets model using this ListEngine
         *
         * @param mediaItemModel MediaItemModel using this ListEngine.
         */
        void setModel(MediaItemModel * mediaItemModel);
        
        /**
         * Returns the model used by this ListEngine
         */
        MediaItemModel * model();

        /**
         * Sets the filter to be used when loading only playable MediaItems.
         * This supports MediaItemModel::loadSources() method.
         *
         * @param engineFilter engine filter to be used for loading playable
         *                     MediaItems.
         * 
         */
        virtual void setFilterForSources(const QString& engineFilter)
        {
            Q_UNUSED(engineFilter);
        }
        
        /**
         * Method called by MediaItemModel to activate and "Action" MediaItem
         */
        virtual void activateAction(){}
        
        /**
         * Removes information for specified MediaItems from the source of
         * the mediaItems.
         * 
         * @param mediaList list of MediaItems whose information should be
         *                  removed from the source.
         */
        virtual void removeSourceInfo(QList<MediaItem> mediaList)
        {
            Q_UNUSED(mediaList);
        }
        
        /**
         * Update source with information from specified MediaItems.
         *
         * @param mediaList list of MediaItems whose information should be
         *                  upated in the source.
         */
        virtual void updateSourceInfo(QList<MediaItem> mediaList)
        {
            Q_UNUSED(mediaList);
        }
        
    protected:
        MediaListProperties m_mediaListProperties;
        QString m_requestSignature;
        QString m_subRequestSignature;

    private:
        MediaItemModel * m_mediaItemModel;
};
#endif // LISTENGINE_H
