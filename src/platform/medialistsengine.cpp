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
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>
#include <QFile>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/OpticalDisc>
#include <Nepomuk/ResourceManager>

MediaListsEngine::MediaListsEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
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
        mediaItem.title = i18n("Files and Folders");
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = "files://audio";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaList << mediaItem;
        if (m_nepomukInited) {
            QStringList contextTitles;
            contextTitles << i18n("Recently Played") << i18n("Highest Rated") << i18n("Frequently Played");
            QStringList contextLRIs;
            mediaItem.title = i18n("Artists");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://artists";
            mediaItem.artwork = KIcon("system-users");
            mediaItem.fields["categoryType"] = QString("Artists");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=5||groupBy=artist";
            contextLRIs << "semantics://highest?audio||limit=5||groupBy=artist";
            contextLRIs << "semantics://frequent?audio||limit=5||groupBy=artist";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Albums");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://albums";
            mediaItem.artwork = KIcon("media-optical");
            mediaItem.fields["categoryType"] = QString("Albums");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=5||groupBy=album";
            contextLRIs << "semantics://highest?audio||limit=5||groupBy=album";
            contextLRIs << "semantics://frequent?audio||limit=5||groupBy=album";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://genres";
            mediaItem.fields["categoryType"] = QString("Audio Genres");
            mediaItem.artwork = KIcon("flag-blue");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=5||groupBy=genre";
            contextLRIs << "semantics://highest?audio||limit=5||groupBy=genre";
            contextLRIs << "semantics://frequent?audio||limit=5||groupBy=genre";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Songs");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://songs";
            mediaItem.fields["categoryType"] = QString("Songs");
            mediaItem.artwork = KIcon("audio-mpeg");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=5||audioType=music";
            contextLRIs << "semantics://highest?audio||limit=5||audioType=music";
            contextLRIs << "semantics://frequent?audio||limit=5||audioType=music";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "audioclips://";
            mediaItem.fields["categoryType"] = QString("Audio Clips");
            mediaItem.artwork = KIcon("audio-x-wav");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=5||audioType=audio clip";
            contextLRIs << "semantics://highest?audio||limit=5||audioType=audio clip";
            contextLRIs << "semantics://frequent?audio||limit=5||audioType=audio clip";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Audio Streams");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "audiostreams://";
            mediaItem.artwork = KIcon("x-media-podcast");
            mediaItem.fields["categoryType"] = QString("Audio Streams");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=5||audioType=audio stream";
            contextLRIs << "semantics://highest?audio||limit=5||audioType=audio stream";
            contextLRIs << "semantics://frequent?audio||limit=5||audioType=audio stream";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://?audio";
            mediaItem.fields["categoryType"] = QString("Audio Tags");
            mediaItem.artwork = KIcon("nepomuk");
            mediaItem.fields["contextTitles"] = QStringList();
            mediaItem.fields["contextLRIs"] = QStringList();
            mediaList << mediaItem;
        }
        
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
            mediaItem.title = i18n("Audio CD");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "cdaudio://";
            mediaItem.artwork = KIcon("media-optical-audio");
            mediaList << mediaItem;
        }
        
        if (m_nepomukInited) {
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "semantics://frequent?audio||limit=20";
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaList << mediaItem;
            mediaItem.title = i18n("Recently Played");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "semantics://recent?audio||limit=20";
            mediaItem.artwork = KIcon("chronometer");
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "semantics://highest?audio||limit=20";
            mediaItem.artwork = KIcon("rating");
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
                            mediaItem.fields["title"] = mediaItem.title;
                            mediaItem.url = nameUrl.at(2).trimmed();
                            if (mediaItem.url.startsWith("savedlists://")) {
                                mediaItem.artwork = KIcon("view-list-text");
                            } else {
                                mediaItem.artwork = KIcon("view-media-playlist");
                            }
                            mediaItem.isSavedList = true;
                            if (m_nepomukInited) {
                                mediaList << mediaItem;
                            } else {
                                //Only show lists that aren't don't require nepomuk
                                if (mediaItem.url.startsWith("savedlists://")) {
                                    mediaList << mediaItem;
                                }
                            }
                        }
                    }
                }
            }
        }
        
    } else if (m_mediaListProperties.engineArg() == "video") {
        MediaItem mediaItem;
        mediaItem.type = "Category";
        mediaItem.isSavedList = false;
        mediaItem.title = i18n("Files and Folders");
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = "files://video";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaList << mediaItem;
        if (m_nepomukInited) {
            QStringList contextTitles;
            contextTitles << i18n("Recently Played") << i18n("Highest Rated") << i18n("Frequently Played");
            QStringList contextLRIs;
            mediaItem.title = i18n("Movies");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://movies";
            mediaItem.artwork = KIcon("tool-animator");
            mediaItem.fields["categoryType"] = QString("Movies");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=5||videoType=movie";
            contextLRIs << "semantics://highest?video||limit=5||videoType=movie";
            contextLRIs << "semantics://frequent?video||limit=5||videoType=movie";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("TV Shows");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://tvshows";
            mediaItem.artwork = KIcon("video-television");
            mediaItem.fields["categoryType"] = QString("TV Series");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=5||groupBy=seriesName";
            contextLRIs << "semantics://highest?video||limit=5||groupBy=seriesName";
            contextLRIs << "semantics://frequent?video||limit=5||groupBy=seriesName";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://genres";
            mediaItem.fields["categoryType"] = QString("Video Genres");
            mediaItem.artwork = KIcon("flag-green");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=5||groupBy=genre";
            contextLRIs << "semantics://highest?video||limit=5||groupBy=genre";
            contextLRIs << "semantics://frequent?video||limit=5||groupBy=genre";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://clips";
            mediaItem.fields["categoryType"] = QString("Video Clips");
            mediaItem.artwork = KIcon("video-x-generic");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=5||videoType=video clip";
            contextLRIs << "semantics://highest?video||limit=5||videoType=video clip";
            contextLRIs << "semantics://frequent?video||limit=5||videoType=video clip";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://?video";
            mediaItem.fields["categoryType"] = QString("Video Tags");
            mediaItem.artwork = KIcon("nepomuk");
            mediaItem.fields["contextTitles"] = QStringList();
            mediaItem.fields["contextLRIs"] = QStringList();
            mediaList << mediaItem;

        }
        
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
            mediaItem.title = i18n("DVD Video");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "dvdvideo://";
            mediaItem.artwork = KIcon("media-optical-dvd");
            mediaList << mediaItem;        
        }
        
        if (m_nepomukInited) {
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "semantics://frequent?video||limit=20";
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaList << mediaItem;
            mediaItem.title = i18n("Recently Played");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "semantics://recent?video||limit=20";
            mediaItem.artwork = KIcon("chronometer");
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "semantics://highest?video||limit=20";
            mediaItem.artwork = KIcon("rating");
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
                            mediaItem.fields["title"] = mediaItem.title;
                            mediaItem.url = nameUrl.at(2).trimmed();
                            if (mediaItem.url.startsWith("savedlists://")) {
                                mediaItem.artwork = KIcon("view-list-text");
                            } else {
                                mediaItem.artwork = KIcon("view-media-playlist");
                            }
                            mediaItem.isSavedList = true;
                            if (m_nepomukInited) {
                                mediaList << mediaItem;
                            } else {
                                //Only show lists that aren't don't require nepomuk
                                if (mediaItem.url.startsWith("savedlists://")) {
                                    mediaList << mediaItem;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
        
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    m_loadWhenReady = false;
    //exec();    
}
