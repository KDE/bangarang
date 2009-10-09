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
        ~ListEngine();
        virtual void setMediaListProperties(MediaListProperties mediaListProperties)
        {
            Q_UNUSED(mediaListProperties)
        }
        virtual MediaListProperties mediaListProperties()
        {
            //I don't know how else to silence compiler warnings
            MediaListProperties mediaListProperties;
            return mediaListProperties;
        }
        virtual void setFilterForSources(QString engineFilter)
        {
            Q_UNUSED(engineFilter);
        }
        virtual void setRequestSignature(QString requestSignature)
        {
            Q_UNUSED(requestSignature);
        }
        virtual void setSubRequestSignature(QString subRequestSignature)
        {
            Q_UNUSED(subRequestSignature);
        }
        virtual void activateAction(){}
        void setModel(MediaItemModel * mediaItemModel);
        MediaItemModel * model();
        
    private:
        MediaItemModel * m_mediaItemModel;
        
};
#endif // LISTENGINE_H

