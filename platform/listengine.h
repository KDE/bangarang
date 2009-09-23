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

