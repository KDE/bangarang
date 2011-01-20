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

#include "../mediaitemmodel.h"
#include "filelistengine.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KEncodingProber>
#include <KLocale>
#include <KIcon>
#include <KFileDialog>
#include <KFilePlacesModel>
#include <KDirLister>
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
    m_filePlacesModel = new KFilePlacesModel(parent);
    m_dirModel = new KDirModel(parent);
    m_dirModel->setDirLister(parent->downloader()->dirLister());
    m_dirSortProxyModel = new KDirSortFilterProxyModel(parent);
    m_dirSortProxyModel->setSourceModel(m_dirModel);
    m_dirSortProxyModel->setSortFoldersFirst(true);
    m_updateNepomukOnly = false;
}

FileListEngine::~FileListEngine()
{
}

void FileListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    if (m_updateSourceInfo || m_removeSourceInfo) {
        //Make sure to crawl dirs if necessary
        if (m_updateSourceInfo) {
            QHash<QString, QVariant> status;
            status["description"] = i18n("Collecting file info...");
            status["progress"] = -1;
            emit updateStatus(status);
            m_mediaItemsInfoToUpdate = getFiles(m_mediaItemsInfoToUpdate, false, true);
        }

        NepomukListEngine::run();
        return;
    }


    QStringList filterList = m_mediaListProperties.engineFilter().split("||");
    if (filterList.count() == 0) {
        return;
    }
            
    QList<MediaItem> mediaList;
    
    if (filterList.at(0) == "browseFolder") {
        QString browseUrl;
        if (filterList.count() > 1) {
            browseUrl = filterList.at(1);
        }
        if (browseUrl.isEmpty()) {
            //Load Places if no folder is specified
            for (int i =0; i < m_filePlacesModel->rowCount(); i ++) {
                if (m_stop) {
                    return;
                }
                if (m_filePlacesModel->isHidden(m_filePlacesModel->index(i,0))) {
                    continue;
                }
                QString newBrowseUrl = m_filePlacesModel->url(m_filePlacesModel->index(i,0)).prettyUrl();
                if (!newBrowseUrl.isEmpty() && newBrowseUrl != "trash:/") {
                    MediaItem mediaItem;
                    mediaItem.type = "Category";
                    mediaItem.fields["categoryType"] = "Basic+Artwork";
                    mediaItem.title = m_filePlacesModel->text(m_filePlacesModel->index(i,0));
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.url = QString("files://%1?browseFolder||%2")
                                    .arg(m_mediaListProperties.engineArg())
                                    .arg(newBrowseUrl);
                    mediaItem.artwork = m_filePlacesModel->icon(m_filePlacesModel->index(i,0));
                    mediaList.append(mediaItem);
                }
            }
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
            m_requestSignature = QString();
            m_subRequestSignature = QString();
        } else {
            //Get folder listing
            connectDownloader();
            emit listDir(KUrl(browseUrl));
            exec();
        }
    } else if (filterList.at(0) == "sources") {
        //Recursively get all relevant files in specified folder
        if (m_mediaListProperties.engineFilterList().count() > 1) {
            MediaItem mediaItem;
            mediaItem.type = "Category";
            mediaItem.url = QString("files://%1?browseFolder||%2")
                            .arg(m_mediaListProperties.engineArg())
                            .arg(m_mediaListProperties.engineFilterList().at(1));
            QList<MediaItem> listToGetFiles;
            listToGetFiles.append(mediaItem);
            mediaList = getFiles(listToGetFiles, true); //Get basic file info first
            if (m_stop) {
                return;
            }

            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
            m_requestSignature = QString();
            m_subRequestSignature = QString();

            //Get more detailed info for each mediaitem and update;
            for (int i = 0; i < mediaList.count(); i++) {
                if (m_stop) {
                    return;
                }
                QApplication::processEvents();
                MediaItem mediaItem = Utilities::mediaItemFromUrl(KUrl(mediaList.at(i).url), true);
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                emit updateMediaItem(mediaItem);
            }
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

void FileListEngine::updateSourceInfo(QList<MediaItem> mediaList, bool nepomukOnly)
{
    m_updateNepomukOnly = nepomukOnly;
    NepomukListEngine::updateSourceInfo(mediaList);
}


void FileListEngine::listingComplete(const KUrl &url)
{
    QList<MediaItem> mediaList;
    m_dirSortProxyModel->sort(0);
    for (int i = 0; i < m_dirSortProxyModel->rowCount(); i++) {
        if (m_stop) {
            quit();
        }
        MediaItem mediaItem;
        KFileItem fileItem = m_dirModel->itemForIndex(m_dirSortProxyModel->mapToSource(m_dirSortProxyModel->index(i,0)));
        if (fileItem.isDir()) {
            mediaItem.type = "Category";
            mediaItem.fields["categoryType"] = "Basic+Artwork";
            mediaItem.title = fileItem.text();
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = QString("files://%1?browseFolder||%2")
                            .arg(m_mediaListProperties.engineArg())
                            .arg(fileItem.url().prettyUrl());
            mediaItem.artwork = KIcon(fileItem.iconName());
            mediaList.append(mediaItem);
        } else if (fileItem.isFile()) {
            if (Utilities::isM3u(fileItem.url().prettyUrl()) || Utilities::isPls(fileItem.url().prettyUrl())) {
                mediaItem = Utilities::mediaItemFromUrl(fileItem.url());
                mediaList.append(mediaItem);
            } else if (m_mediaListProperties.engineArg() == "audio") {
                if (Utilities::isAudioMimeType(fileItem.mimeTypePtr())) {
                    mediaItem.url = fileItem.url().prettyUrl();
                    mediaItem.fields["url"] = mediaItem.url;
                    mediaItem.type = "Audio";
                    if (Utilities::isMusicMimeType((fileItem.mimeTypePtr()))) {
                        mediaItem.fields["audioType"] = "Music";
                    } else {
                        mediaItem.fields["audioType"] = "Audio Clip";
                    }
                    mediaItem.title = fileItem.text();
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.artwork = KIcon(fileItem.iconName());
                    mediaList.append(mediaItem);
                }
            } else if (m_mediaListProperties.engineArg() == "video") {
                if (Utilities::isVideoMimeType(fileItem.mimeTypePtr())) {
                    mediaItem.url = fileItem.url().prettyUrl();
                    mediaItem.fields["url"] = mediaItem.url;
                    mediaItem.type = "Video";
                    mediaItem.fields["videoType"] = "Video Clip";
                    mediaItem.title = fileItem.text();
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.artwork = KIcon("video-x-generic");
                    mediaList.append(mediaItem);
                }
            }
        }
    }
    m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    disconnectDownloader();

    //Get more detailed info for each mediaitem and update;
    for (int i = 0; i < mediaList.count(); i++) {
        if (m_stop) {
            quit();
        }
        QApplication::processEvents();
        MediaItem mediaItem = Utilities::mediaItemFromUrl(KUrl(mediaList.at(i).url), true);
        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
        emit updateMediaItem(mediaItem);
    }

    quit();
    Q_UNUSED(url);
}

QList<MediaItem> FileListEngine::getFiles(QList<MediaItem> mediaList, bool basicInfo, bool emitStatus)
{
    //This routine looks for directories in mediaList, crawls and returns a media list with files only
    QList<MediaItem> crawledList;
    for (int i = 0; i < mediaList.count(); i++) {
        while (m_stop) {
            //wait while m_stop is true
            QApplication::processEvents();
        }
        if (mediaList.at(i).type == "Category" && !mediaList.at(i).url.isEmpty()) {
            MediaListProperties categoryProperties;
            categoryProperties.lri = mediaList.at(i).url;
            QStringList engineFilterList = categoryProperties.engineFilterList();
            QString firstFilter = engineFilterList.at(0);
            if ((categoryProperties.engine() == "files://") && (engineFilterList.at(0) == "browseFolder")) {
                KUrl directoryUrl(categoryProperties.engineFilterList().at(1));
                if (directoryUrl.isLocalFile()) {
                    QString mimeFilter;
                    QString type;
                    if (categoryProperties.engineArg() == "audio") {
                        mimeFilter = Utilities::audioMimeFilter();
                        type = "audio";
                    } else if (categoryProperties.engineArg() == "video") {
                        mimeFilter = Utilities::videoMimeFilter();
                        type = "video";
                    }
                    QFileInfoList fileList = crawlDir(QDir(directoryUrl.path()), categoryProperties.engineArg());
                    for (int j = 0; j < fileList.count(); ++j) {
                        while (m_stop) {
                            //wait while m_stop is true
                            QApplication::processEvents();
                        }
                        QFileInfo fileInfo = fileList.at(j);
                        MediaItem mediaItem;
                        if (basicInfo) {
                            mediaItem.url = KUrl(fileInfo.absoluteFilePath()).prettyUrl();
                            mediaItem.fields["url"] = mediaItem.url;
                            mediaItem.title = fileInfo.fileName();
                            mediaItem.fields["title"] = mediaItem.title;
                            mediaItem.fields["sourceLri"] = categoryProperties.lri;
                            if (categoryProperties.engineArg() == "audio") {
                                mediaItem.type = "Audio";
                                if (Utilities::isMusic(mediaItem.url)) {
                                    mediaItem.fields["audioType"] = "Music";
                                    mediaItem.artwork = KIcon("audio-mpeg");
                                } else {
                                    mediaItem.fields["audioType"] = "Audio Clip";
                                    mediaItem.artwork = KIcon("audio-x-wav");
                                }
                                crawledList.append(mediaItem);
                            } else if (categoryProperties.engineArg() == "video") {
                                mediaItem.type = "Video";
                                mediaItem.fields["videoType"] = "Video Clip";
                                mediaItem.artwork = KIcon("video-x-generic");
                                crawledList.append(mediaItem);
                            }
                        } else {
                            KUrl fileUrl = KUrl(fileList.at(j).absoluteFilePath());
                            MediaItem mediaItem = Utilities::mediaItemFromUrl(fileUrl, true);
                            crawledList.append(mediaItem);
                        }
                        if (emitStatus && (crawledList.count() % 20 == 0)) {
                            QHash<QString, QVariant> status;
                            status["description"] = i18n("Collecting file info (%1 files)...", crawledList.count());
                            status["progress"] = -1;
                            emit updateStatus(status);
                        }

                    }
                }
            }
        } else if ((mediaList.at(i).type == "Audio" || mediaList.at(i).type == "Video")
                    && !mediaList.at(i).url.isEmpty()) {
            crawledList.append(mediaList.at(i));
        }
    }

    return crawledList;

}

QFileInfoList FileListEngine::crawlDir(const QDir &dir, QString engineArg)
{
    //Crawl through directory specified and return relevant files
    QFileInfoList returnList;
    QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Name);
    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (int i = 0; i < dirList.count(); ++i) {
        fileList << crawlDir(QDir(dirList.at(i).absoluteFilePath()), engineArg);
    }

    for (int i = 0; i < fileList.count(); ++i) {
        bool addToList = false;
        if (engineArg == "audio") {
            if (Utilities::isAudio(fileList.at(i).absoluteFilePath())) {
                addToList = true;
            }
        } else if (engineArg == "video") {
            if (Utilities::isVideo(fileList.at(i).absoluteFilePath())) {
                addToList = true;
            }
        }
        if (addToList) {
            returnList << fileList.at(i);
        }
    }
    return returnList;
}
