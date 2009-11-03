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
#include "medialistsengine.h"
#include "listenginefactory.h"
#include "mediaindexer.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <QApplication>
#include <KIcon>
#include <KMimeType>
#include <KStandardDirs>
#include <QFile>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/OpticalDisc>

MediaListsEngine::MediaListsEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;
    
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();

}

MediaListsEngine::~MediaListsEngine()
{
}

void MediaListsEngine::run()
{
    QList<MediaItem> mediaList;
    if (m_mediaListProperties.engineArg() == "audio") {
        MediaItem mediaItem;
        mediaItem.type = "Category";
        mediaItem.isSavedList = false;
        mediaItem.title = "Files and Folders";
        mediaItem.url = "files://audio";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaList << mediaItem;
        mediaItem.title = "Artists";
        mediaItem.url = "music://artists";
        mediaItem.artwork = KIcon("system-users");
        mediaList << mediaItem;
        mediaItem.title = "Albums";
        mediaItem.url = "music://albums";
        mediaItem.artwork = KIcon("media-optical");
        mediaList << mediaItem;
        mediaItem.title = "Songs";
        mediaItem.url = "music://songs";
        mediaItem.artwork = KIcon("audio-mpeg");
        mediaList << mediaItem;
        mediaItem.title = "Genres";
        mediaItem.url = "music://genres";
        mediaItem.artwork = KIcon("flag-blue");
        mediaList << mediaItem;
        mediaItem.title = "Clips";
        mediaItem.url = "audioclips://";
        mediaItem.artwork = KIcon("audio-x-wav");
        mediaList << mediaItem;
        mediaItem.title = "Audio Streams";
        mediaItem.url = "audiostreams://";
        mediaItem.artwork = KIcon("x-media-podcast");
        mediaList << mediaItem;
        
        //Show Audio CD if present
        bool audioCDFound = false;
        foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString()))
        {
            const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc> ();
            if (disc->availableContent() & Solid::OpticalDisc::Audio) {
                audioCDFound = true;
            }
        }
        if (audioCDFound) {
            mediaItem.title = "Audio CD";
            mediaItem.url = "cdaudio://";
            mediaItem.artwork = KIcon("media-optical-audio");
            mediaList << mediaItem;
        }
        
        //Load saved lists from index
        QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
        if (indexFile.exists()) {
            if (indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&indexFile);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList nameUrl = line.split(":::");
                    if (nameUrl.count() >= 3) {
                        if (nameUrl.at(0) == "Audio") {
                            mediaItem.title = nameUrl.at(1).trimmed();
                            mediaItem.url = nameUrl.at(2).trimmed();
                            mediaItem.artwork = KIcon("view-media-playlist");
                            mediaItem.isSavedList = true;
                            mediaList << mediaItem;
                        }
                    }
                }
            }
        }
        
    } else if (m_mediaListProperties.engineArg() == "video") {
        MediaItem mediaItem;
        mediaItem.type = "Category";
        mediaItem.isSavedList = false;
        mediaItem.title = "Files and Folders";
        mediaItem.url = "files://video";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaList << mediaItem;
        mediaItem.title = "Movies";
        mediaItem.url = "video://movies";
        mediaItem.artwork = KIcon("tool-animator");
        mediaList << mediaItem;
        mediaItem.title = "TV Shows";
        mediaItem.url = "video://tvshows";
        mediaItem.artwork = KIcon("video-television");
        mediaList << mediaItem;
        mediaItem.title = "Video Clips";
        mediaItem.url = "video://clips";
        mediaItem.artwork = KIcon("video-x-generic");
        mediaList << mediaItem;
        
        //Show DVD if present
        bool DVDFound = false;
        foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString()))
        {
            const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc> ();
            if (disc->availableContent() & Solid::OpticalDisc::VideoDvd) {
                DVDFound = true;
            }
        }
        if (DVDFound) {
            mediaItem.title = "DVD Video";
            mediaItem.url = "dvdvideo://";
            mediaItem.artwork = KIcon("media-optical-dvd");
            mediaList << mediaItem;        
        }
        
        //Load saved lists from index
        QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
        if (indexFile.exists()) {
            if (indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&indexFile);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList nameUrl = line.split(":::");
                    if (nameUrl.count() >= 3) {
                        if (nameUrl.at(0) == "Video") {
                            mediaItem.title = nameUrl.at(1).trimmed();
                            mediaItem.url = nameUrl.at(2).trimmed();
                            mediaItem.artwork = KIcon("view-media-playlist");
                            mediaItem.isSavedList = true;
                            mediaList << mediaItem;
                        }
                    }
                }
            }
        }
    }
        
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    m_loadWhenReady = false;
    //exec();    
}

void MediaListsEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties MediaListsEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void MediaListsEngine::setFilterForSources(QString engineFilter)
{
    Q_UNUSED(engineFilter);
}

void MediaListsEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void MediaListsEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void MediaListsEngine::activateAction()
{
        
}
