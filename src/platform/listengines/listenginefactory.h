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

#ifndef LISTENGINEFACTORY_H
#define LISTENGINEFACTORY_H

#include "../mediaitemmodel.h"
#include "../downloader.h"
#include <KUrl>
#include <KDirLister>
#include <QObject>
#include <QList>
#include <QPair>

class ListEngine;

enum EngineType {
    EngineTypeUnknown = -1,
    EngineTypeMusic,
    EngineTypeFiles,
    EngineTypeVideo,
    EngineTypeCDAudio,
    EngineTypeDVDVideo,
    EngineTypeSavedLists,
    EngineTypeMediaLists,
    EngineTypeAudioStreams,
    EngineTypeSemantics,
    EngineTypeCache,
    EngineTypeAudioClips,
    EngineTypeTag,
    EngineTypeFeeds,
    EngineTypeAmpache
};

typedef QPair<EngineType, QString> EngineDescription;
typedef QPair<ListEngine *, EngineType> EnginePointerType;

/**
 * This class creates ListEngines as needed for the MediaItemModel.
 */

class ListEngineFactory : public QObject
{
    Q_OBJECT
    
    public:
      
        /**
         * Constructor
         */
        ListEngineFactory(MediaItemModel *parent);
        
        /**
         * Destructor
         */
        ~ListEngineFactory();

        /**
         * Creates an engine of the given type with "model" as the model
         */
        ListEngine *createEngine(const EngineType type, MediaItemModel* model);
        
        /** 
         * Returns an available ListEngine for the specified engine`.
         * ListEngineFactory factory will reuse ListEngines that are
         * idle and create a new ListEngine if no idle ListEngines
         * are available.
         *
         * @param type the engine type for which a ListEngine should be returned.
         */
        
        virtual ListEngine* availableListEngine(const EngineType type);
        
        /**
         * Generates a unique request signature the MediaItemModel
         * can use to uniquely identify a ListEngine load request.
         */
        QString generateRequestSignature();
        
        /**
         * Returns true if a ListEngine exists for the specified
         * engine.
         */
        bool engineExists(const EngineType type);
        
        /**
         * Returns the downloader.
         **/
        Downloader * downloader();

        /**
         * Returns the engine type to the specific string
         */
        EngineType engineTypeFromString(const QString &str);

        /**
         * Returns the engine string to the specific type
         */
        QString engineStringFromType(const EngineType type);

        /**
         * Stop all running list engines as soon as possible.
         */
        void stopAll(unsigned long waitToTerminate = 0, bool quitEventLoop = false);

        /**
         * Resume all running list engines if possible.
         */
        void resumeAll();

    public Q_SLOTS:
        /**
         * Loads specified LRI with an available list engine using the specified request signature
         */
        void load(MediaListProperties mediaListProperties, const QString &requestSignature, const QString &subRequestSignature);


    private:
        MediaItemModel * m_parent;
        int m_requestSignatureSeed;
        QList<EngineDescription> m_engines;
        QList<EnginePointerType> m_initializedEngines;
        Downloader * m_downloader;
};

#endif // LISTENGINEFACTORY_H
        
