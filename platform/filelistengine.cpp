#include "filelistengine.h"
#include <Soprano/QueryResultIterator>
//#include <Soprano/Query>
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
#include "utilities.h"
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>

FileListEngine::FileListEngine(ListEngineFactory * parent) : ListEngine(parent)
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
    
}

FileListEngine::~FileListEngine()
{
}

void FileListEngine::run()
{
    MediaItem mediaItem;
    QList<MediaItem> mediaList;
    if (m_mediaListProperties.engineFilter().isEmpty()) {        
        if (m_mediaListProperties.engineArg() == "audio") {
            mediaItem.artwork = KIcon("document-open");
            mediaItem.url = "files://audio?getFiles";
            mediaItem.title = "Open audio file(s)";
            mediaItem.type = "Action";
            mediaList << mediaItem;
            mediaItem.artwork = KIcon("document-open-folder");
            mediaItem.url = "files://audio?getFolder";
            mediaItem.title = "Open folder containing audio file(s)";
            mediaItem.type = "Action";
            mediaList << mediaItem;           
        } else if (m_mediaListProperties.engineArg() == "video") {
            mediaItem.artwork = KIcon("document-open");
            mediaItem.url = "files://video?getFiles";
            mediaItem.title = "Open video file(s)";
            mediaItem.type = "Action";
            mediaList << mediaItem;
            mediaItem.artwork = KIcon("document-open-folder");
            mediaItem.url = "files://video?getFolder";
            mediaItem.title = "Open folder containing video file(s)";
            mediaItem.type = "Action";
            mediaList << mediaItem;           
        } else if (m_mediaListProperties.engineArg() == "images") {
            mediaItem.artwork = KIcon("document-open");
            mediaItem.url = "files://images?getFiles";
            mediaItem.title = "Open image file(s)";
            mediaItem.type = "Action";
            mediaList << mediaItem;
            mediaItem.artwork = KIcon("document-open-folder");
            mediaItem.url = "files://images?getFolder";
            mediaItem.title = "Open folder containing image file(s)";
            mediaItem.type = "Action";
            mediaList << mediaItem;           
        }
    }
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    //exec();    
}

void FileListEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties FileListEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void FileListEngine::setFilterForSources(QString engineFilter)
{
    //Always return files
    //FIXME:Figure out how sources work
    //m_mediaListProperties.lri = QString("files://getFiles?%1").arg(engineFilter);
}

void FileListEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void FileListEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void FileListEngine::activateAction()
{
    QList<MediaItem> mediaList;
    QString audioMimeFilter = "audio/mpeg audio/mp4 audio/ogg audio/vorbis audio/aac audio/aiff audio/basic audio/flac audio/mp2 audio/mp3 audio/vnd.rn-realaudio audio/wav application/ogg audio/x-flac audio/x-musepack";
    QString videoMimeFilter = "video/mp4 video/mpeg video/ogg video/quicktime video/msvideo video/x-theora video/x-theora+ogg video/x-ogm video/x-ogm+ogg";
    
    if (m_mediaListProperties.engineFilter() == "getFiles") {
        if (m_mediaListProperties.engineArg() == "audio") {
            KUrl::List fileList = KFileDialog::getOpenUrls(KUrl(), QString(audioMimeFilter), static_cast<QWidget *>(model()->m_parent), "Open audio file(s)");
            QList<QString> urlsToIndex;
            for (int i = 0; i < fileList.count(); ++i) {
                MediaItem mediaItem;
                mediaItem.artwork = KIcon("audio-mp4");
                mediaItem.url = fileList.at(i).url();
                mediaItem.title = fileList.at(i).fileName();
                mediaItem.type = "Audio";
                if (Utilities::isMusic(mediaItem.url)) {
                    TagLib::FileRef file(KUrl(mediaItem.url).path().toUtf8());
                    QString title = TStringToQString(file.tag()->title()).trimmed();
                    QString artist  = TStringToQString(file.tag()->artist()).trimmed();
                    QString album = TStringToQString(file.tag()->album()).trimmed();
                    QString genre   = TStringToQString(file.tag()->genre()).trimmed();
                    int track   = file.tag()->track();
                    int duration = file.audioProperties()->length();
                    if (!title.isEmpty()) {
                        mediaItem.title = title;
                    }
                    mediaItem.subTitle = artist + QString(" - ") + album;
                    mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                    mediaItem.fields["title"] = title;
                    mediaItem.fields["artist"] = artist;
                    mediaItem.fields["album"] = album;
                    mediaItem.fields["genre"] = genre;
                    mediaItem.fields["trackNumber"] = track;
                    mediaItem.fields["audioType"] = "Music";
                }
                urlsToIndex << mediaItem.url;
                mediaList << mediaItem; 
            }
            m_mediaIndexer->indexUrls(urlsToIndex);
            m_mediaListProperties.name = "Audio Files";
        }
        if (m_mediaListProperties.engineArg() == "video") {
            KUrl::List fileList = KFileDialog::getOpenUrls(KUrl(), QString(videoMimeFilter), static_cast<QWidget *>(model()->m_parent), "Open video file(s)");
            for (int i = 0; i < fileList.count(); ++i) {
                MediaItem mediaItem;
                mediaItem.artwork = KIcon("video-x-generic");
                mediaItem.url = fileList.at(i).url();
                mediaItem.title = fileList.at(i).fileName();
                mediaItem.type = "Video";
                mediaItem.fields["title"] = fileList.at(i).fileName();
                mediaList << mediaItem;
            }
            m_mediaListProperties.name = "Video Files";
        }
    } else if (m_mediaListProperties.engineFilter() == "getFolder") {
        if (m_mediaListProperties.engineArg() == "audio") {
            QString directoryPath = KFileDialog::getExistingDirectory(KUrl(), static_cast<QWidget *>(model()->m_parent), "Open folder containing audio file(s)");       
            if (!directoryPath.isEmpty()) {
                QDir directory(directoryPath);
                QFileInfoList fileInfoList = crawlDir(directory, audioMimeFilter.split(" "));
                KUrl::List fileList = QFileInfoListToKUrlList(fileInfoList);
                QList<QString> urlsToIndex;
                for (int i = 0; i < fileList.count(); ++i) {
                    MediaItem mediaItem;
                    mediaItem.artwork = KIcon("audio-mp4");
                    mediaItem.url = fileList.at(i).url();
                    mediaItem.title = fileList.at(i).fileName();
                    mediaItem.type = "Audio";
                    if (Utilities::isMusic(mediaItem.url)) {
                        TagLib::FileRef file(KUrl(mediaItem.url).path().toUtf8());
                        QString title = TStringToQString(file.tag()->title()).trimmed();
                        QString artist  = TStringToQString(file.tag()->artist()).trimmed();
                        QString album   = TStringToQString(file.tag()->album()).trimmed();
                        QString genre   = TStringToQString(file.tag()->genre()).trimmed();
                        int track   = file.tag()->track();
                        int duration = file.audioProperties()->length();
                        if (!title.isEmpty()) {
                            mediaItem.title = title;
                        }
                        mediaItem.subTitle = artist + QString(" - ") + album;
                        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                        mediaItem.fields["title"] = title;
                        mediaItem.fields["artist"] = artist;
                        mediaItem.fields["album"] = album;
                        mediaItem.fields["genre"] = genre;
                        mediaItem.fields["trackNumber"] = track;
                        mediaItem.fields["audioType"] = "Music";
                    }
                    urlsToIndex << mediaItem.url;
                    mediaList << mediaItem; 
                }
                m_mediaIndexer->indexUrls(urlsToIndex);
            }
            m_mediaListProperties.name = "Audio Files";           
        }
        if (m_mediaListProperties.engineArg() == "video") {
            QString directoryPath = KFileDialog::getExistingDirectory(KUrl(), static_cast<QWidget *>(model()->m_parent), "Open folder containing video file(s)");           
            QDir directory(directoryPath);
            QFileInfoList fileInfoList = crawlDir(directory, videoMimeFilter.split(" "));
            KUrl::List fileList = QFileInfoListToKUrlList(fileInfoList);
            for (int i = 0; i < fileList.count(); ++i) {
                MediaItem mediaItem;
                mediaItem.artwork = KIcon("video-x-generic");
                mediaItem.url = fileList.at(i).url();
                mediaItem.title = fileList.at(i).fileName();
                mediaItem.type = "Video";
                mediaItem.fields["title"] = fileList.at(i).fileName();
                mediaList << mediaItem;
            }
            m_mediaListProperties.name = "Video Files";
    }
    
    }
    
    //if (mediaList.count() > 0) {
        model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    //}
        
}

QFileInfoList FileListEngine::crawlDir(QDir dir, QStringList mimeFilter)
{
    QFileInfoList returnList;
    QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Name);
    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (int i = 0; i < dirList.count(); ++i) {
        fileList << crawlDir(QDir(dirList.at(i).absoluteFilePath()), mimeFilter);
    }
    
    for (int i = 0; i < fileList.count(); ++i) {
        KMimeType::Ptr file = KMimeType::findByUrl(KUrl(fileList.at(i).absoluteFilePath()), 0, true);
        for (int j = 0; j < mimeFilter.count(); ++j) {
            if (file->is(mimeFilter.at(j))) {
                returnList << fileList.at(i);
                break;
            }
        }
    }
    return returnList;
}

KUrl::List FileListEngine::QFileInfoListToKUrlList(QFileInfoList fileInfoList)
{
    KUrl::List urlList;
    for (int i = 0; i < fileInfoList.count(); ++i) {
        urlList << KUrl(fileInfoList.at(i).absoluteFilePath());
    }
    return urlList;
}