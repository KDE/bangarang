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

class ListEngine : public QThread
{
    Q_OBJECT
    
    public:
        ListEngine(ListEngineFactory *parent);
        virtual ~ListEngine();

        void setMediaListProperties(const MediaListProperties& mediaListProperties);
        const MediaListProperties& mediaListProperties() const;
        void setRequestSignature(const QString& requestSignature);
        void setSubRequestSignature(const QString& subRequestSignature);
        const QString& requestSignature() const;
        const QString& subRequestSignature() const;

        void setModel(MediaItemModel * mediaItemModel);
        MediaItemModel * model();

        virtual void setFilterForSources(const QString& engineFilter)
        {
            Q_UNUSED(engineFilter);
        }
        virtual void activateAction(){}
        virtual void removeSourceInfo(QList<MediaItem> mediaList)
        {
            Q_UNUSED(mediaList);
        }
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
