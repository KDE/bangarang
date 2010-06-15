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
#include <KConfig>
#include <KConfigGroup>
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
            contextTitles.clear();
            contextTitles << i18n("Recently Played Artists") << i18n("Highest Rated Artists") << i18n("Frequently Played Artists");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=4||groupBy=artist";
            contextLRIs << "semantics://highest?audio||limit=4||groupBy=artist";
            contextLRIs << "semantics://frequent?audio||limit=4||groupBy=artist";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Albums");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://albums";
            mediaItem.artwork = KIcon("media-optical");
            mediaItem.fields["categoryType"] = QString("Albums");
            contextTitles.clear();
            contextTitles << i18n("Recently Played Albums") << i18n("Highest Rated Albums") << i18n("Frequently Played Albums");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=4||groupBy=album";
            contextLRIs << "semantics://highest?audio||limit=4||groupBy=album";
            contextLRIs << "semantics://frequent?audio||limit=4||groupBy=album";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://genres";
            mediaItem.fields["categoryType"] = QString("Audio Genres");
            mediaItem.artwork = KIcon("flag-blue");
            contextTitles.clear();
            contextTitles << i18n("Recently Played Genres") << i18n("Highest Rated Genres") << i18n("Frequently Played Genres");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=4||groupBy=genre";
            contextLRIs << "semantics://highest?audio||limit=4||groupBy=genre";
            contextLRIs << "semantics://frequent?audio||limit=4||groupBy=genre";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Songs");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://songs";
            mediaItem.fields["categoryType"] = QString("Songs");
            mediaItem.artwork = KIcon("audio-mpeg");
            contextTitles.clear();
            contextTitles << i18n("Recently Played Songs") << i18n("Highest Rated Songs") << i18n("Frequently Played Songs");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=4||audioType=music";
            contextLRIs << "semantics://highest?audio||limit=4||audioType=music";
            contextLRIs << "semantics://frequent?audio||limit=4||audioType=music";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Audio Feeds");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "feeds://audiofeeds";
            mediaItem.fields["categoryType"] = QString("Audio Feeds");
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaList << mediaItem;
            mediaItem.title = i18n("Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "audioclips://";
            mediaItem.fields["categoryType"] = QString("Audio Clips");
            mediaItem.artwork = KIcon("audio-x-wav");
            contextTitles.clear();
            contextTitles << i18n("Recently Played Clips") << i18n("Highest Rated Clips") << i18n("Frequently Played Clips");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=4||audioType=audio clip";
            contextLRIs << "semantics://highest?audio||limit=4||audioType=audio clip";
            contextLRIs << "semantics://frequent?audio||limit=4||audioType=audio clip";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Audio Streams");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "audiostreams://";
            mediaItem.artwork = KIcon("text-html");
            mediaItem.fields["categoryType"] = QString("Audio Streams");
            contextTitles.clear();
            contextTitles << i18n("Recently Played Streams") << i18n("Highest Rated Streams") << i18n("Frequently Played Streams");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?audio||limit=4||audioType=audio stream";
            contextLRIs << "semantics://highest?audio||limit=4||audioType=audio stream";
            contextLRIs << "semantics://frequent?audio||limit=4||audioType=audio stream";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://audiotags";
            mediaItem.fields["categoryType"] = QString("Audio Tags");
            mediaItem.artwork = KIcon("view-pim-notes");
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
            KConfig config;
            KConfigGroup generalGroup( &config, "General" );
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            int limit = generalGroup.readEntry("FrequentAudioLimit", 20);
            mediaItem.url = QString("semantics://frequent?audio||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaList << mediaItem;
            mediaItem.title = i18n("Recently Played");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("RecentAudioLimit", 20);
            mediaItem.url = QString("semantics://recent?audio||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("chronometer");
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("HighestAudioLimit", 20);
            mediaItem.url = QString("semantics://highest?audio||limit=%1").arg(limit);
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
            contextTitles.clear();
            contextTitles << i18n("Recently Watched Movies") << i18n("Highest Rated Movies") << i18n("Frequently Watched Movies");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=4||videoType=movie";
            contextLRIs << "semantics://highest?video||limit=4||videoType=movie";
            contextLRIs << "semantics://frequent?video||limit=4||videoType=movie";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("TV Shows");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://tvshows";
            mediaItem.artwork = KIcon("video-television");
            mediaItem.fields["categoryType"] = QString("TV Series");
            contextTitles.clear();
            contextTitles << i18n("Recently Watched TV Shows") << i18n("Highest Rated TV Shows") << i18n("Frequently Watched TV Shows");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=4||groupBy=seriesName";
            contextLRIs << "semantics://highest?video||limit=4||groupBy=seriesName";
            contextLRIs << "semantics://frequent?video||limit=4||groupBy=seriesName";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://genres";
            mediaItem.fields["categoryType"] = QString("Video Genres");
            mediaItem.artwork = KIcon("flag-green");
            contextTitles.clear();
            contextTitles << i18n("Recently Watched Genres") << i18n("Highest Rated Genres") << i18n("Frequently Watched Genres");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=4||groupBy=genre";
            contextLRIs << "semantics://highest?video||limit=4||groupBy=genre";
            contextLRIs << "semantics://frequent?video||limit=4||groupBy=genre";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Actors");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://actors";
            mediaItem.fields["categoryType"] = QString("Actors");
            mediaItem.artwork = KIcon("view-media-artist");
            contextTitles.clear();
            contextTitles << i18n("Recently Watched Actors") << i18n("Highest Rated Actors") << i18n("Frequently Watched Actors");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=4||groupBy=actor";
            contextLRIs << "semantics://highest?video||limit=4||groupBy=actor";
            contextLRIs << "semantics://frequent?video||limit=4||groupBy=actor";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Directors");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://directors";
            mediaItem.fields["categoryType"] = QString("Directors");
            mediaItem.artwork = KIcon("view-media-artist");
            contextTitles.clear();
            contextTitles << i18n("Recently Watched Directors") << i18n("Highest Rated Directors") << i18n("Frequently Watched Directors");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=4||groupBy=director";
            contextLRIs << "semantics://highest?video||limit=4||groupBy=director";
            contextLRIs << "semantics://frequent?video||limit=4||groupBy=director";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Feeds");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "feeds://videofeeds";
            mediaItem.fields["categoryType"] = QString("Video Feeds");
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://clips";
            mediaItem.fields["categoryType"] = QString("Video Clips");
            mediaItem.artwork = KIcon("video-x-generic");
            contextTitles.clear();
            contextTitles << i18n("Recently Watched Clips") << i18n("Highest Rated Clips") << i18n("Frequently Watched Clips");
            contextLRIs.clear();
            contextLRIs << "semantics://recent?video||limit=4||videoType=video clip";
            contextLRIs << "semantics://highest?video||limit=4||videoType=video clip";
            contextLRIs << "semantics://frequent?video||limit=4||videoType=video clip";
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://videotags";
            mediaItem.fields["categoryType"] = QString("Video Tags");
            mediaItem.artwork = KIcon("view-pim-notes");
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
            KConfig config;
            KConfigGroup generalGroup( &config, "General" );
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            int limit = generalGroup.readEntry("FrequentVideoLimit", 20);
            mediaItem.url = QString("semantics://frequent?video||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaList << mediaItem;
            mediaItem.title = i18n("Recently Played");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("RecentVideoLimit", 20);
            mediaItem.url = QString("semantics://recent?video||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("chronometer");
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("HighestVideoLimit", 20);
            mediaItem.url = QString("semantics://highest?video||limit=%1").arg(limit);
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
