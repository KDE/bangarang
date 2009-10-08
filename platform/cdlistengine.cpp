#include "mediaitemmodel.h"
#include "cdlistengine.h"
#include "listenginefactory.h"
#include "mediaindexer.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KIcon>
#include <KFileDialog>
#include <KMimeType>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Phonon/MediaController>

CDListEngine::CDListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;
    
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        //resource manager inited successfully
        m_mediaIndexer = new MediaIndexer(this);
    } else {
        //no resource manager
    };
    
    m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    
    //m_mediaListProperties.dataEngine = "files://";
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();

    m_cdObject = new KCompactDisc::KCompactDisc();
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setCurrentSource(Phonon::Cd);
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    m_loadWhenReady = false;
}

CDListEngine::~CDListEngine()
{
    delete m_mediaObject;
}

void CDListEngine::run()
{
    Phonon::MediaController *mediaController = new Phonon::MediaController(m_mediaObject);
    QList<MediaItem> mediaList;
    m_loadWhenReady = true;
    if (m_mediaObject->state() == Phonon::StoppedState) {
        MediaItem mediaItem;
        int trackCount = mediaController->availableTitles();
        QString album = "Audio CD";
        QString title;
        QString artist;
        //int duration;
        for (int i = 1; i <= trackCount; i++) {
            title = QString("Track %1").arg(i);
            mediaItem.url = QString("CDTRACK%1").arg(i);
            mediaItem.artwork = KIcon("media-optical-audio");
            mediaItem.title = title;
            mediaItem.subTitle = QString("Audio CD - %1 Tracks").arg(trackCount);
            mediaItem.type = "Audio";
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["audioType"] = "CD Track";
            mediaItem.fields["trackNumber"] = i;
            mediaList << mediaItem;
        }

        /*mediaItem.url = "-";
        mediaItem.title = QString("Number of tracks: %1").arg(trackCount);
        mediaItem.type = "Audio";
        mediaList << mediaItem;*/
        
        model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
        m_requestSignature = QString();
        m_subRequestSignature = QString();
        m_loadWhenReady = false;
    }
    //exec();    
    delete mediaController;
}

void CDListEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties CDListEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void CDListEngine::setFilterForSources(QString engineFilter)
{
    Q_UNUSED(engineFilter);
}

void CDListEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void CDListEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void CDListEngine::activateAction()
{
        
}

void CDListEngine::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::LoadingState) && m_loadWhenReady) {
        start();
    }
}
