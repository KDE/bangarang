#ifndef DVDLISTENGINE_H
#define DVDLISTENGINE_H

#include "listengine.h"
#include <QtCore>
#include <QDir>
#include <KUrl>
#include <libkcompactdisc/kcompactdisc.h>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>
#include <Phonon>

class MediaItem;
class MediaListProperties;
class ListEngineFactory;
class MediaIndexer;

class DVDListEngine : public ListEngine
{
    Q_OBJECT
    
    public:
        DVDListEngine(ListEngineFactory *parent);
        ~DVDListEngine();
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
        MediaIndexer * m_mediaIndexer;
        QFileInfoList crawlDir(QDir dir, QStringList mimeFilter);
        KUrl::List QFileInfoListToKUrlList(QFileInfoList fileInfoList);
        KCompactDisc::KCompactDisc *m_cdObject;
        Phonon::MediaObject *m_mediaObject;
        bool m_loadWhenReady;
    
    private slots:
        void stateChanged(Phonon::State newState, Phonon::State oldState);
        
    Q_SIGNALS:
        void results(QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done);
        
};
#endif // DVDLISTENGINE_H
