/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://commits.kde.org/bangarang>
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

#include "filelistengine.h"
#include "../mediaitemmodel.h"
#include "listenginefactory.h"

//#include "../mediaindexer.h"
#include "../utilities/utilities.h"

#include <QApplication>
#include <KEncodingProber>
#include <KLocalizedString>
#include <QIcon>
#include <QFileDialog>
#include <KFilePlacesModel>
#include <KDirLister>
#include <QMimeType>
#include <QDebug>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

FileListEngine::FileListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_filePlacesModel = new KFilePlacesModel(parent);
    m_dirModel = new KDirModel(parent);
    m_dirModel->setDirLister(parent->downloader()->dirLister());
    m_dirSortProxyModel = new KDirSortFilterProxyModel(parent);
    m_dirSortProxyModel->setSourceModel(m_dirModel);
    m_dirSortProxyModel->setSortFoldersFirst(true);
}

FileListEngine::~FileListEngine()
{
}

void FileListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;


    QStringList filterList = m_mediaListProperties.engineFilter().split("||");
    if (filterList.count() == 0) {
        return;
    }
            
    QList<OldMediaItem> mediaList;
    
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
                QString newBrowseUrl = m_filePlacesModel->url(m_filePlacesModel->index(i,0)).toDisplayString();
                if (!newBrowseUrl.isEmpty() && newBrowseUrl != "trash:/") {
                    OldMediaItem mediaItem;
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
            m_requestSignature.clear();
            m_subRequestSignature.clear();
        } else {
            //Get folder listing
            connectDownloader();
            emit listDir(QUrl::fromLocalFile(browseUrl));
            exec();
        }
    } else if (filterList.at(0) == "sources") {
        //Recursively get all relevant files in specified folder
        if (m_mediaListProperties.engineFilterList().count() > 1) {
            OldMediaItem mediaItem;
            mediaItem.type = "Category";
            mediaItem.url = QString("files://%1?browseFolder||%2")
                            .arg(m_mediaListProperties.engineArg())
                            .arg(m_mediaListProperties.engineFilterList().at(1));
            QList<OldMediaItem> listToGetFiles;
            listToGetFiles.append(mediaItem);
            mediaList = getFiles(listToGetFiles, true); //Get basic file info first
            if (m_stop) {
                return;
            }

            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
            m_requestSignature.clear();
            m_subRequestSignature.clear();

            //Get more detailed info for each mediaitem and update;
            for (int i = 0; i < mediaList.count(); i++) {
                if (m_stop) {
                    return;
                }
                QApplication::processEvents();
                OldMediaItem mediaItem = Utilities::mediaItemFromUrl(QUrl::fromLocalFile(mediaList.at(i).url), true);
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

void FileListEngine::updateSourceInfo(QList<OldMediaItem> mediaList, bool nepomukOnly)
{
//    m_updateNepomukOnly = nepomukOnly;
//    NepomukListEngine::updateSourceInfo(mediaList);
}


void FileListEngine::listingComplete(const QUrl &url)
{
    QList<OldMediaItem> mediaList;
    m_dirSortProxyModel->sort(0);
    for (int i = 0; i < m_dirSortProxyModel->rowCount(); i++) {
        if (m_stop) {
            quit();
        }
        OldMediaItem mediaItem;
        KFileItem fileItem = m_dirModel->itemForIndex(m_dirSortProxyModel->mapToSource(m_dirSortProxyModel->index(i,0)));
        if (fileItem.isDir()) {
            mediaItem.type = "Category";
            mediaItem.fields["categoryType"] = "Basic+Artwork";
            mediaItem.title = fileItem.text();
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = QString("files://%1?browseFolder||%2")
                            .arg(m_mediaListProperties.engineArg())
                            .arg(fileItem.url().toDisplayString());
            mediaItem.artwork = QIcon::fromTheme(fileItem.iconName());
            mediaList.append(mediaItem);
        } else if (fileItem.isFile()) {
            if (Utilities::isM3u(fileItem.url().toDisplayString()) || Utilities::isPls(fileItem.url().toDisplayString())) {
                mediaItem = Utilities::mediaItemFromUrl(fileItem.url());
                mediaList.append(mediaItem);
            } else {
                if (m_mediaListProperties.engineArg() == "audio" || m_mediaListProperties.engineArg() == "media") {
                    if (Utilities::isAudioMimeType(fileItem.currentMimeType())) {
                        mediaItem.url = fileItem.url().toDisplayString();
                        mediaItem.fields["url"] = mediaItem.url;
                        mediaItem.type = "Audio";
                        if (Utilities::isMusicMimeType((fileItem.currentMimeType()))) {
                            mediaItem.fields["audioType"] = "Music";
                        } else {
                            mediaItem.fields["audioType"] = "Audio Clip";
                        }
                        mediaItem.title = fileItem.text();
                        mediaItem.fields["title"] = mediaItem.title;
                        mediaItem.artwork = QIcon::fromTheme(fileItem.iconName());
                        mediaList.append(mediaItem);
                    }
                }
                if (m_mediaListProperties.engineArg() == "video" || m_mediaListProperties.engineArg() == "media") {
                    if (Utilities::isVideoMimeType(fileItem.currentMimeType())) {
                        mediaItem.url = fileItem.url().toDisplayString();
                        mediaItem.fields["url"] = mediaItem.url;
                        mediaItem.type = "Video";
                        mediaItem.fields["videoType"] = "Video Clip";
                        mediaItem.title = fileItem.text();
                        mediaItem.fields["title"] = mediaItem.title;
                        mediaItem.artwork = QIcon::fromTheme("video-x-generic");
                        mediaList.append(mediaItem);
                    }
               }
            }
        }
    }
    m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature.clear();
    m_subRequestSignature.clear();
    disconnectDownloader();

    //Get more detailed info for each mediaitem and update;
    for (int i = 0; i < mediaList.count(); i++) {
        if (m_stop) {
            quit();
        }
        QApplication::processEvents();
        OldMediaItem mediaItem = Utilities::mediaItemFromUrl(QUrl::fromLocalFile(mediaList.at(i).url), true);
        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
        emit updateMediaItem(mediaItem);
    }

    quit();
    Q_UNUSED(url);
}

QList<OldMediaItem> FileListEngine::getFiles(QList<OldMediaItem> mediaList, bool basicInfo, bool emitStatus)
{
    //This routine looks for directories in mediaList, crawls and returns a media list with files only
    QList<OldMediaItem> crawledList;
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
                QUrl directoryUrl = QUrl::fromLocalFile(categoryProperties.engineFilterList().at(1));
                if (directoryUrl.isLocalFile()) {
                    QString mimeFilter;
                    QString type;
                    if (categoryProperties.engineArg() == "audio") {
                        mimeFilter = Utilities::audioMimeFilter();
                        type = "audio";
                    } else if (categoryProperties.engineArg() == "video") {
                        mimeFilter = Utilities::videoMimeFilter();
                        type = "video";
                    } else if (categoryProperties.engineArg() == "media") {
                        mimeFilter = Utilities::audioMimeFilter() + Utilities::videoMimeFilter();
                        type = "media";
                    }
                    QFileInfoList fileList = crawlDir(QDir(directoryUrl.path()), categoryProperties.engineArg());
                    for (int j = 0; j < fileList.count(); ++j) {
                        while (m_stop) {
                            //wait while m_stop is true
                            QApplication::processEvents();
                        }
                        QFileInfo fileInfo = fileList.at(j);
                        OldMediaItem mediaItem;
                        if (basicInfo) {
                            mediaItem.url = QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toDisplayString();
                            mediaItem.fields["url"] = mediaItem.url;
                            mediaItem.title = fileInfo.fileName();
                            mediaItem.fields["title"] = mediaItem.title;
                            mediaItem.fields["sourceLri"] = categoryProperties.lri;
                            if (categoryProperties.engineArg() == "audio") {
                                mediaItem.type = "Audio";
                                if (Utilities::isMusic(mediaItem.url)) {
                                    mediaItem.fields["audioType"] = "Music";
                                    mediaItem.artwork = QIcon::fromTheme("audio-mpeg");
                                } else {
                                    mediaItem.fields["audioType"] = "Audio Clip";
                                    mediaItem.artwork = QIcon::fromTheme("audio-x-wav");
                                }
                                crawledList.append(mediaItem);
                            } else if (categoryProperties.engineArg() == "video") {
                                mediaItem.type = "Video";
                                mediaItem.fields["videoType"] = "Video Clip";
                                mediaItem.artwork = QIcon::fromTheme("video-x-generic");
                                crawledList.append(mediaItem);
                            } else if (categoryProperties.engineArg() == "media") {
                                if (Utilities::isAudio(mediaItem.url)) {
                                    mediaItem.type = "Audio";
                                    mediaItem.fields["audioType"] = "Audio Clip";
                                    mediaItem.artwork = QIcon::fromTheme("audio-x-wav");
                                }
                                if (Utilities::isMusic(mediaItem.url)) {
                                    mediaItem.type = "Audio";
                                    mediaItem.fields["audioType"] = "Music";
                                    mediaItem.artwork = QIcon::fromTheme("audio-mpeg");
                                }
                                if (Utilities::isVideo(mediaItem.url)) {
                                    mediaItem.type = "Video";
                                    mediaItem.fields["videoType"] = "Video Clip";
                                    mediaItem.artwork = QIcon::fromTheme("video-x-generic");
                                }
                                crawledList.append(mediaItem);
                            }
                        } else {
                            QUrl fileUrl = QUrl::fromLocalFile(fileList.at(j).absoluteFilePath());
                            OldMediaItem mediaItem = Utilities::mediaItemFromUrl(fileUrl, true);
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
        } else if (engineArg == "media") {
            if (Utilities::isAudio(fileList.at(i).absoluteFilePath()) ||
                Utilities::isVideo(fileList.at(i).absoluteFilePath())) {
                addToList = true;
            }
        }
        if (addToList) {
            returnList << fileList.at(i);
        }
    }
    return returnList;
}
