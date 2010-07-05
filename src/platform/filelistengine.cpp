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

#include "mediaitemmodel.h"
#include "filelistengine.h"
#include "listenginefactory.h"
#include "mediaindexer.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KEncodingProber>
#include <KLocale>
#include <KIcon>
#include <KFileDialog>
#include <KMimeType>
#include <KDebug>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>

FileListEngine::FileListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
    m_indexFilesAction = false;
    m_indexFolderAction = false;
}

FileListEngine::~FileListEngine()
{
}

void FileListEngine::run()
{
    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    QStringList filterList = m_mediaListProperties.engineFilter().split("||");
    if (filterList.count() == 0) {
        return;
    }
            
    QList<MediaItem> mediaList;
    
    if (!m_indexFilesAction && !m_indexFolderAction) {
        if (filterList.at(0) == "browseFolder") {
            QString directoryPath = QDir::homePath();
            if (filterList.count() > 1) {
                directoryPath = filterList.at(1);
            }
            if (!directoryPath.isEmpty()) {
                QDir dir(directoryPath);
                QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
                for (int i = 0; i < dirList.count(); ++i) {
                    MediaItem mediaItem;
                    mediaItem.type = "Category";
                    mediaItem.title = dirList.at(i).baseName();
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.url = QString("files://%1?browseFolder||%2")
                                            .arg(m_mediaListProperties.engineArg())
                                            .arg(dirList.at(i).absoluteFilePath());
                    mediaItem.artwork = KIcon("document-open-folder");
                    mediaList.append(mediaItem);
                }
                QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Name);
                for (int i = 0; i < fileList.count(); ++i) {
                    KUrl fileUrl = KUrl(fileList.at(i).absoluteFilePath());
                    if (m_mediaListProperties.engineArg() == "audio") {
                        if (Utilities::isAudio(fileUrl.url())) {
                            MediaItem mediaItem = Utilities::mediaItemFromUrl(fileUrl);
                            mediaList.append(mediaItem);
                        }
                    } else if (m_mediaListProperties.engineArg() == "video") {
                        if (Utilities::isVideo(fileUrl.url())) {
                            MediaItem mediaItem = Utilities::mediaItemFromUrl(fileUrl);
                            mediaList.append(mediaItem);
                        }
                    }
                }
                if (directoryPath == QDir::homePath() && m_nepomukInited) {
                    MediaItem mediaItem;
                    mediaItem.type = "Category";
                    mediaItem.title = i18n("Indexer");
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.url = QString("files://%1?indexer")
                                    .arg(m_mediaListProperties.engineArg());
                    mediaItem.artwork = KIcon("system-run");
                    mediaList.append(mediaItem);
                }
            }
        } else if (filterList.at(0) == "sources") {
            QString directoryPath;
            if (filterList.count() > 1) {
                directoryPath = filterList.at(1);
            }
            if (!directoryPath.isEmpty()) {
                QString mimeFilter;
                QString type;
                if (m_mediaListProperties.engineArg() == "audio") {
                    mimeFilter = Utilities::audioMimeFilter();
                    type = "audio";
                } else if (m_mediaListProperties.engineArg() == "video") {
                    mimeFilter = Utilities::videoMimeFilter();
                    type = "video";
                }
                QFileInfoList fileList = crawlDir(QDir(directoryPath), mimeFilter.split(" "));
                for (int i = 0; i < fileList.count(); ++i) {
                    KUrl fileUrl = KUrl(fileList.at(i).absoluteFilePath());
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(fileUrl);
                    mediaList.append(mediaItem);
                }
            }
        } else if (filterList.at(0) == "indexer") {
            if (m_mediaListProperties.engineArg() == "audio") {
                MediaItem mediaItem;
                mediaItem.artwork = KIcon("document-open");
                mediaItem.url = "files://audio?indexFiles";
                mediaItem.title = i18n("Index audio file(s)");
                mediaItem.type = "Action";
                mediaList << mediaItem;
                mediaItem.artwork = KIcon("document-open-folder");
                mediaItem.url = "files://audio?indexFolder";
                mediaItem.title = i18n("Index folder containing audio file(s)");
                mediaItem.type = "Action";
                mediaList << mediaItem;
            } else if (m_mediaListProperties.engineArg() == "video") {
                MediaItem mediaItem;
                mediaItem.artwork = KIcon("document-open");
                mediaItem.url = "files://video?indexFiles";
                mediaItem.title = i18n("Index video file(s)");
                mediaItem.type = "Action";
                mediaList << mediaItem;
                mediaItem.artwork = KIcon("document-open-folder");
                mediaItem.url = "files://video?indexFolder";
                mediaItem.title = i18n("Index folder containing video file(s)");
                mediaItem.type = "Action";
                mediaList << mediaItem;           
            }
                
        } 
        m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
        emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
        m_requestSignature = QString();
        m_subRequestSignature = QString();
    }
    
    
    if (m_indexFilesAction || m_indexFolderAction) {
        MediaListProperties mediaListProperties;
        if (m_mediaListProperties.engineFilter() == "indexFiles") {
            if (m_mediaListProperties.engineArg() == "audio") {
                mediaList = readAudioUrlList(m_fileList);
                mediaListProperties.name = i18n("Audio Files");
                mediaListProperties.lri = QString("files://audio?getFiles||%1").arg(engineFilterFromUrlList(m_fileList));
            }
            if (m_mediaListProperties.engineArg() == "video") {
                mediaList = readVideoUrlList(m_fileList);
                mediaListProperties.name = i18n("Video Files");
                mediaListProperties.lri = QString("files://video?getFiles||%1").arg(engineFilterFromUrlList(m_fileList));
            }
            m_indexFilesAction = false;
        } else if (m_mediaListProperties.engineFilter() == "indexFolder") {
            if (m_mediaListProperties.engineArg() == "audio") {      
                if (!m_directoryPath.isEmpty()) {
                    QDir directory(m_directoryPath);
                    QFileInfoList fileInfoList = crawlDir(directory, Utilities::audioMimeFilter().split(" "));
                    KUrl::List fileList = QFileInfoListToKUrlList(fileInfoList);
                    mediaList = readAudioUrlList(fileList);
                }
                mediaListProperties.name = i18n("Audio Files");
                mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());           
                mediaListProperties.lri = QString("files://audio?getFolder||%1").arg(m_directoryPath);
            }
            if (m_mediaListProperties.engineArg() == "video") {
                if (!m_directoryPath.isEmpty()) {
                    QDir directory(m_directoryPath);
                    QFileInfoList fileInfoList = crawlDir(directory, Utilities::videoMimeFilter().split(" "));
                    KUrl::List fileList = QFileInfoListToKUrlList(fileInfoList);
                    mediaList = readVideoUrlList(fileList);
                }
                mediaListProperties.name = i18n("Video Files");
                mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
                mediaListProperties.lri = QString("files://video?getFolder||%1").arg(m_directoryPath);
            }
            m_indexFolderAction = false;
        }
        emit results(m_requestSignature, mediaList, mediaListProperties, true, m_subRequestSignature);
        if (m_nepomukInited && m_mediaListToIndex.count() > 0) {
            m_mediaIndexer = new MediaIndexer(this);
            connectIndexer();
            m_mediaIndexer->updateInfo(m_mediaListToIndex);
            m_mediaListToIndex.clear();
            m_requestSignature = QString();
            m_subRequestSignature = QString();
            exec();
        }
    } 
    //exec();
}

void FileListEngine::activateAction()
{
    QList<MediaItem> mediaList;
    MediaListProperties mediaListProperties;
    if (m_mediaListProperties.engineFilter() == "indexFiles") {
        if (m_mediaListProperties.engineArg() == "audio") {
            m_fileList = KFileDialog::getOpenUrls(KUrl(), Utilities::audioMimeFilter(), 0, i18n("Index audio file(s)"));
            m_indexFilesAction = true;
            start();
        }
        if (m_mediaListProperties.engineArg() == "video") {
            m_fileList = KFileDialog::getOpenUrls(KUrl(), Utilities::videoMimeFilter(), 0, i18n("Index video file(s)"));
            m_indexFilesAction = true;
            start();
        }
    } else if (m_mediaListProperties.engineFilter() == "indexFolder") {
        if (m_mediaListProperties.engineArg() == "audio") {
            m_directoryPath = KFileDialog::getExistingDirectory(KUrl(), 0, i18n("Index folder containing audio file(s)"));
            m_indexFolderAction = true;
            start();
        }
        if (m_mediaListProperties.engineArg() == "video") {
            m_directoryPath = KFileDialog::getExistingDirectory(KUrl(), 0, i18n("Index folder containing video file(s)"));           
            m_indexFolderAction = true;
            start();
        }
    }
}

void FileListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always crawl directory and return files
    QStringList filterList = engineFilter.split("||");
    QString filter;
    if (filterList.count() == 2) {
        filter = filterList.at(1);
    }
    m_mediaListProperties.lri = QString("files://%1?sources||%2")
                                       .arg(m_mediaListProperties.engineArg())
                                       .arg(filter);
}


QFileInfoList FileListEngine::crawlDir(const QDir &dir, const QStringList &mimeFilter)
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

KUrl::List FileListEngine::QFileInfoListToKUrlList(const QFileInfoList &fileInfoList)
{
    KUrl::List urlList;
    for (int i = 0; i < fileInfoList.count(); ++i) {
        urlList << KUrl(fileInfoList.at(i).absoluteFilePath());
    }
    return urlList;
}

QList<MediaItem> FileListEngine::readAudioUrlList(const KUrl::List &fileList)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    QList<MediaItem> mediaList;
    //QList<MediaItem> mediaListToIndex;
    for (int i = 0; i < fileList.count(); ++i) {
        MediaItem mediaItem;
        if (Utilities::isM3u(fileList.at(i).url()) || Utilities::isPls(fileList.at(i).url())) {
            mediaItem.artwork = KIcon("view-list-text");
            mediaItem.url = QString("savedlists://%1").arg(fileList.at(i).url());
            mediaItem.title = fileList.at(i).fileName();
            mediaItem.type = "Category";
            mediaList << mediaItem; 
            continue;
        } 
        mediaItem.artwork = KIcon("audio-mp4");
        mediaItem.url = fileList.at(i).prettyUrl();
        mediaItem.title = fileList.at(i).fileName();
        mediaItem.type = "Audio";
        mediaItem.fields["url"] = mediaItem.url;
        mediaItem.fields["title"] = fileList.at(i).fileName();
        if (Utilities::isMusic(mediaItem.url) && fileList.at(i).isLocalFile()) {
            TagLib::FileRef file(KUrl(mediaItem.url).path().toLocal8Bit().constData());
            if (file.isNull()) {
                continue;
            }
            QString title = TStringToQString(file.tag()->title()).trimmed();
            QString artist  = TStringToQString(file.tag()->artist()).trimmed();
            QString album   = TStringToQString(file.tag()->album()).trimmed();
            QString genre   = TStringToQString(file.tag()->genre()).trimmed();
            if (KUrl(mediaItem.url).path().endsWith(".mp3")) {
                // detect encoding for mpeg id3v2
                QString tmp = title + artist + album + genre;
                KEncodingProber prober(KEncodingProber::Universal);
                KEncodingProber::ProberState result = prober.feed(tmp.toAscii());
                if (result != KEncodingProber::NotMe) {
                    QByteArray encodingname = prober.encoding().toLower();
                    if ( prober.confidence() > 0.47
                        && ( ( encodingname == "gb18030" )
                        || ( encodingname == "big5" )
                        || ( encodingname == "euc-kr" )
                        || ( encodingname == "euc-jp" )
                        || ( encodingname == "koi8-r" ) ) ) {
                        title = QTextCodec::codecForName(encodingname)->toUnicode(title.toAscii());
                        artist = QTextCodec::codecForName(encodingname)->toUnicode(artist.toAscii());
                        album = QTextCodec::codecForName(encodingname)->toUnicode(album.toAscii());
                        genre = QTextCodec::codecForName(encodingname)->toUnicode(genre.toAscii());
                    } else if ((prober.confidence() < 0.3 || encodingname != "utf-8")
                        && QTextCodec::codecForLocale()->name().toLower() != "utf-8") {
                        title = QTextCodec::codecForLocale()->toUnicode(title.toAscii());
                        artist = QTextCodec::codecForLocale()->toUnicode(artist.toAscii());
                        album = QTextCodec::codecForLocale()->toUnicode(album.toAscii());
                        genre = QTextCodec::codecForLocale()->toUnicode(genre.toAscii());
                    }
                }
            }
            int track   = file.tag()->track();
            int duration = file.audioProperties()->length();
            int year = file.tag()->year();
            if (!title.isEmpty()) {
                mediaItem.title = title;
            }
            mediaItem.subTitle = artist + QString(" - ") + album;
            mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
            mediaItem.fields["duration"] = duration;
            mediaItem.fields["title"] = title;
            mediaItem.fields["artist"] = artist;
            mediaItem.fields["album"] = album;
            mediaItem.fields["genre"] = genre;
            mediaItem.fields["trackNumber"] = track;
            mediaItem.fields["year"] = year;
            mediaItem.fields["audioType"] = "Music";
            if (m_nepomukInited) {
                Nepomuk::Resource res(mediaItem.url);
                if (res.exists()) {
                    mediaItem.fields["rating"] = res.rating();
                    mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
                    mediaItem.fields["playCount"] = res.property(mediaVocabulary.playCount()).toInt();
                    mediaItem.fields["lastPlayed"] = res.property(mediaVocabulary.lastPlayed()).toDateTime();
                }
            }
            //Index all music files
            m_mediaListToIndex << mediaItem;
        } else {
            mediaItem.fields["audioType"] = "Audio Clip";
            if (m_nepomukInited) {
                bool foundInNepomuk = false;
                MediaItem foundMediaItem;
                Nepomuk::Resource res(fileList.at(i));
                if (res.exists() && (res.hasType(mediaVocabulary.typeAudio()) ||
                    res.hasType(mediaVocabulary.typeAudioMusic()) ||
                    res.hasType(mediaVocabulary.typeAudioStream())) ) {
                    foundMediaItem = Utilities::mediaItemFromNepomuk(res);
                    foundInNepomuk = true;
                }
                if (!foundInNepomuk || foundMediaItem.type.isEmpty()) {
                    //Index video items not found in nepomuk store
                    m_mediaListToIndex << mediaItem;
                } else {
                    mediaItem = foundMediaItem;
                }
            }
        }
        mediaList << mediaItem; 
    }
    return mediaList;
}

QList<MediaItem> FileListEngine::readVideoUrlList(const KUrl::List &fileList)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
    mediaVocabulary.setVideoVocabulary(MediaVocabulary::nmm);
    QList<MediaItem> mediaList;
    //QList<MediaItem> mediaListToIndex;
    for (int i = 0; i < fileList.count(); ++i) {
        MediaItem mediaItem;
        mediaItem.artwork = KIcon("video-x-generic");
        mediaItem.url = fileList.at(i).prettyUrl();
        mediaItem.title = fileList.at(i).fileName();
        mediaItem.type = "Video";
        mediaItem.fields["url"] = mediaItem.url;
        mediaItem.fields["title"] = fileList.at(i).fileName();
        mediaItem.fields["videoType"] = "Video Clip";
        mediaItem.artwork = KIcon("video-x-generic");
        if (m_nepomukInited) {
            bool foundInNepomuk = false;
            MediaItem foundMediaItem;
            Nepomuk::Resource res(fileList.at(i));
            if (res.exists() && (res.hasType(mediaVocabulary.typeVideo()) ||
                res.hasType(mediaVocabulary.typeVideoMovie()) ||
                res.hasType(mediaVocabulary.typeVideoTVShow())) ) {
                foundMediaItem = Utilities::mediaItemFromNepomuk(res);
                foundInNepomuk = true;
            }
            if (!foundInNepomuk) {
                //Index video items not found in nepomuk store
                m_mediaListToIndex << mediaItem;
            } else {
                mediaItem = foundMediaItem;
            }
        }
        mediaList << mediaItem;
    }
    return mediaList;
}

QString FileListEngine::engineFilterFromUrlList(const KUrl::List &fileList)
{
    QString engineFilter;
    for (int i = 0; i < fileList.count(); i++) {
        QString url = fileList.at(i).url();
        engineFilter.append(QString("%1||").arg(url));
    }
    return engineFilter;
}



