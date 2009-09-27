#include "mediaitemmodel.h"
#include "videolistengine.h"
#include "listenginefactory.h"

#include <Soprano/QueryResultIterator>
//#include <Soprano/Query>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KIcon>
#include <KUrl>
#include <taglib/fileref.h>
#include <QTime>
#include <nepomuk/variant.h>

VideoListEngine::VideoListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;    
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        //resource manager inited successfully
    } else {
        //no resource manager
    };
    
    m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    
}

VideoListEngine::~VideoListEngine()
{
}

void VideoListEngine::run()
{
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    if (engineArg.toLower() == "clips") {
        QString videoQuery;
        
        //Build clips query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT ?r ?title ?duration ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type xesam:Video . "
        "?r xesam:title ?title . ");
        QString whereOptionalConditions = QString("OPTIONAL { ?r xesam:mediaDuration ?duration } ");
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?title ");

        videoQuery = prefix + select + whereConditions + whereOptionalConditions + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( videoQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            MediaItem mediaItem;
            mediaItem.url = it.binding("r").uri().toString();
            mediaItem.title = it.binding("title").literal().toString();
            if (mediaItem.title.isEmpty()) {
                if (KUrl(mediaItem.url).isLocalFile()) {
                    mediaItem.title = KUrl(mediaItem.url).fileName();
                } else {
                    mediaItem.title = mediaItem.url;
                }
            }
            int duration = it.binding("duration").literal().toInt();
            if (duration != 0) {
                mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
            }
            mediaItem.type = "Video";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("video-x-generic");
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["duration"] = it.binding("duration").literal().toInt();
            mediaItem.fields["videoType"] = "VideoClip";
            mediaList.append(mediaItem);
            ++i;
        }
        
        m_mediaListProperties.name = QString("Video Clips");
        m_mediaListProperties.type = QString("Sources");
        
    } else if (engineArg.toLower() == "search") {
        QString videoQuery;
        
        //Build search query 
        QString prefix = QString("PREFIX xesam: <%1> "
        "PREFIX rdf: <%2> "
        "PREFIX xls: <%3> ")
        .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
        .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
        .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
        QString select = QString("SELECT DISTINCT ?r ?title ?duration ");
        QString whereConditions = QString("WHERE { "
        "?r rdf:type xesam:Video . ");
        QString whereOptionalConditions = QString("OPTIONAL {?r xesam:title ?title } "
        "OPTIONAL { ?r xesam:mediaDuration ?duration } ");
        QString searchCondition = QString("FILTER (regex(str(?title),\"%1\",\"i\")) ")
        .arg(engineFilter);
        QString whereTerminator = QString("} ");
        QString order = QString("ORDER BY ?title ");
        videoQuery = prefix + select + whereConditions + whereOptionalConditions + searchCondition + whereTerminator + order;
        
        //Execute Query
        Soprano::QueryResultIterator it = m_mainModel->executeQuery( videoQuery,                                   Soprano::Query::QueryLanguageSparql );
        
        //Build media list from results
        int i = 0;
        while( it.next() ) {
            MediaItem mediaItem;
            mediaItem.url = it.binding("r").uri().toString();
            mediaItem.title = it.binding("title").literal().toString();
            if (mediaItem.title.isEmpty()) {
                if (KUrl(mediaItem.url).isLocalFile()) {
                    mediaItem.title = KUrl(mediaItem.url).fileName();
                } else {
                    mediaItem.title = mediaItem.url;
                }
            }
            int duration = it.binding("duration").literal().toInt();
            if (duration != 0) {
                mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
            }
            mediaItem.type = "Video";
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("video-x-generic");
            mediaItem.fields["title"] = it.binding("title").literal().toString();
            mediaItem.fields["videoType"] = "VideoClip";
            mediaList.append(mediaItem);
            ++i;
        }
        
        if (mediaList.count() == 0) {
            MediaItem noResults;
            noResults.url = "video://";
            noResults.title = "No results";
            noResults.type = "Message";
            mediaList << noResults;
        }
        
        m_mediaListProperties.type = QString("Sources");
        
    }
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    //exec();
    
}

void VideoListEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties VideoListEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void VideoListEngine::setFilterForSources(QString engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("video://?%1").arg(engineFilter);
}

void VideoListEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void VideoListEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void VideoListEngine::activateAction()
{
    
}
