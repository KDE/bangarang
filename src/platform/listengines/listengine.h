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

#include "../mediaitemmodel.h"
#include "listenginefactory.h"
#include <QtCore>
#include <QImage>

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
        virtual void updateSourceInfo(QList<MediaItem> mediaList, bool nepomukOnly = false)
        {
            Q_UNUSED(mediaList);
            Q_UNUSED(nepomukOnly);
        }

        /**
         * Stop execution at as soon as possible. Terminate after waitToTerminate millseconds.
         */
        void stop(unsigned long waitToTerminate = 0, bool quitEventLoop = false);

        /**
         * Resume execution if possible.  This only works for ListEngines designed to resume execution
         * after stop(0, false) was called.
         */
        void resume();
        
    public Q_SLOTS:
        virtual void downloadComplete(const KUrl &from, const KUrl &to)
        {
            Q_UNUSED(from);
            Q_UNUSED(to);
        }

        virtual void listingComplete(const KUrl & url)
        {
            Q_UNUSED(url);
        }

    Q_SIGNALS:
        void results(QString m_requestSignature, QList<MediaItem> mediaList, MediaListProperties m_mediaListProperties, bool done, QString m_subRequestSignature);
        void updateMediaItems(QList<MediaItem> mediaList);
        void updateMediaItem(MediaItem mediaItem);
        void updateArtwork(QImage artworkImage, MediaItem mediaItem);
        void updateMediaListPropertiesCategoryArtwork(QImage artworkImage, MediaItem mediaItem);
        void download(const KUrl &from, const KUrl &to);
        void listDir(const KUrl &url);
        void updateStatus(QHash<QString, QVariant> updatedStatus);
        void loadOtherEngine(const MediaListProperties &mediaListProperties, const QString &requestSignature, const QString &subRequestSignature);
        
    protected:
        ListEngineFactory * m_parent;
        MediaListProperties m_mediaListProperties;
        QString m_requestSignature;
        QString m_subRequestSignature;
        MediaItemModel * m_mediaItemModel;
        bool m_stop;
        void connectDownloader();
        void disconnectDownloader();

};
#endif // LISTENGINE_H
