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
#include "savedlistsengine.h"
#include "listenginefactory.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

#include <KIcon>
#include <KMimeType>
#include <KStandardDirs>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>

SavedListsEngine::SavedListsEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

SavedListsEngine::~SavedListsEngine()
{
}

void SavedListsEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    
    QList<MediaItem> mediaList;
    
    if (m_mediaListProperties.engineArg().isEmpty() && !m_mediaListProperties.engineFilter().isEmpty()) {
        MediaListProperties mediaListProperties = m_mediaListProperties;
        mediaListProperties.lri = m_mediaListProperties.engineFilter();
        kDebug() << mediaListProperties.lri;
        emit loadOtherEngine(mediaListProperties, m_requestSignature, m_subRequestSignature);
        m_requestSignature = QString();
        m_subRequestSignature = QString();
        return;
    }

    if (!m_mediaListProperties.engineArg().isEmpty()) {
        QString workingDir;
        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1").arg(m_mediaListProperties.engineArg()), false));
        if (file.exists()) {
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
                return;
            }
        } else {
            KUrl url(m_mediaListProperties.engineArg());
            workingDir = url.directory(KUrl::AppendTrailingSlash);
            file.setFileName(url.path());
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
                return;
            }
        }
        
        //Make sure it's a valid M3U fileref
        QTextStream in(&file);
        bool valid = false;
        bool isM3U = false;
        bool isPLS = false;
        if (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed() == "#EXTM3U") {
                valid = true;
                isM3U = true;
            } else if (line.trimmed() == "[playlist]") {
                valid = true;
                isPLS = true;
            }
        }
        
        //Create a MediaItem for each entry
        if (valid) {
            while (!in.atEnd()) {
                if (m_stop) {
                    return;
                }
                QString line = in.readLine();
                QString title;
                QString url;
                int duration = 0;

                //Read playlist entry to get title and url
                if ((isM3U) && line.startsWith("#EXTINF:")) {
                    line = line.replace("#EXTINF:","");
                    QStringList durTitle = line.split(",");

                    if (durTitle.count() == 1) {
                        //No title
                        title = durTitle.at(0);
                    } else {
                        duration = durTitle.at(0).toInt();
                        title = durTitle.at(1);
                    }
                    url = in.readLine().trimmed();
                } else if ((isPLS) && line.startsWith("File")) {
                    url = line.mid(line.indexOf("=") + 1).trimmed();
                    if (!in.atEnd()) {
                        line = in.readLine();
                        title = line.mid(line.indexOf("=") + 1).trimmed();
                    }
                    if (!in.atEnd()) {
                        line = in.readLine();
                        duration = line.mid(line.indexOf("=") + 1).trimmed().toInt();
                    }
                }

                //Create a basic mediaItem for each entry
                MediaItem mediaItem;
                KUrl itemUrl(workingDir, url);
                if (!url.isEmpty()) {
                    if (!title.isEmpty()) {
                        mediaItem.title = title;
                    } else {
                        mediaItem.title = itemUrl.fileName();
                    }
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.url = itemUrl.prettyUrl();
                    mediaItem.fields["url"] = mediaItem.url;
                    if (Utilities::isVideo(mediaItem.url)) {
                        mediaItem.type = "Video";
                        mediaItem.artwork = KIcon("video-x-generic");
                        mediaItem.fields["videoType"] = "Video Clip";
                    } else {
                        mediaItem.type = "Audio";
                        mediaItem.artwork = KIcon("audio-x-generic");
                        mediaItem.fields["audioType"] = "Audio Clip";
                    }
                    if ((duration > 0) && (mediaItem.fields["duration"].toInt() <= 0)) {
                        mediaItem.duration = Utilities::durationString(duration);
                        mediaItem.fields["duration"] = duration;
                    } else if (duration == -1) {
                        mediaItem.duration = QString();
                        mediaItem.fields["audioType"] = "Audio Stream";
                    }
                    mediaList << mediaItem;
                }
            }
        }
        
    }
    
    m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);

    //Get more detailed mediaItem info
    for (int i = 0; i < mediaList.count(); i++) {
        if (m_stop) {
            return;
        }
        MediaItem detailedMediaItem = Utilities::mediaItemFromUrl(KUrl(mediaList.at(i).url));
        emit updateMediaItem(detailedMediaItem);
    }

    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        emit updateMediaItems(mediaItems);
    }

    m_requestSignature = QString();
    m_subRequestSignature = QString();
}
