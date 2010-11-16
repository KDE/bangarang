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
#include "medialistsengine.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

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
    QThread::setTerminationEnabled(true);
    m_stop = false;

    QList<MediaItem> mediaList;
    if (m_mediaListProperties.engineArg() == "audio") {
        QStringList contextTitles;
        QStringList contextLRIs;
        MediaItem mediaItem;
        mediaItem.type = "Category";
        mediaItem.isSavedList = false;
        if (m_nepomukInited) {
            KConfig config;
            KConfigGroup generalGroup( &config, "General" );
            mediaItem.title = i18n("Recently Played");
            mediaItem.fields["title"] = mediaItem.title;
            int limit = generalGroup.readEntry("RecentAudioLimit", 20);
            mediaItem.url = QString("semantics://recent?audio||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("chronometer");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Artists"), "semantics://recent?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Albums"), "semantics://recent?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Genres"), "semantics://recent?audio||limit=4||groupBy=genre");
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("HighestAudioLimit", 20);
            mediaItem.url = QString("semantics://highest?audio||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("rating");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Artists"), "semantics://highest?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Albums"), "semantics://highest?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Genres"), "semantics://highest?audio||limit=4||groupBy=genre");
            mediaList << mediaItem;
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("FrequentAudioLimit", 20);
            mediaItem.url = QString("semantics://frequent?audio||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Artists"), "semantics://frequent?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Albums"), "semantics://frequent?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Genres"), "semantics://frequent?audio||limit=4||groupBy=genre");
            mediaList << mediaItem;
            
            mediaItem.title = i18n("Artists");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://artists";
            mediaItem.artwork = KIcon("system-users");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=artist");
            mediaList << mediaItem;
            mediaItem.title = i18n("Albums");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://albums";
            mediaItem.artwork = KIcon("media-optical");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=album");
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://genres";
            mediaItem.artwork = KIcon("flag-blue");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=genre");
            mediaList << mediaItem;
            mediaItem.title = i18n("Songs");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://songs";
            mediaItem.fields["categoryType"] = QString("Songs");
            mediaItem.artwork = KIcon("audio-mpeg");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||audioType=music");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||audioType=music");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||audioType=music");
            mediaList << mediaItem;
            mediaItem.title = i18n("Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "audioclips://";
            mediaItem.fields["categoryType"] = QString("Audio Clips");
            mediaItem.artwork = KIcon("audio-x-wav");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||audioType=audio clip");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||audioType=audio clip");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||audioType=audio clip");
            mediaList << mediaItem;
            mediaItem.title = i18n("Audio Streams");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "audiostreams://";
            mediaItem.artwork = KIcon("text-html");
            mediaItem.fields["categoryType"] = QString("Audio Streams");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||audioType=audio stream");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||audioType=audio stream");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||audioType=audio stream");
            mediaList << mediaItem;
            mediaItem.title = i18n("Audio Feeds");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "feeds://audiofeeds";
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaItem.clearContexts();
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://audiotags";
            mediaItem.fields["categoryType"] = QString("Audio Tags");
            mediaItem.artwork = KIcon("view-pim-notes");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=tag");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=tag");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=tag");
            mediaList << mediaItem;
        }
        
        mediaItem.title = i18n("Files and Folders");
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = "files://audio?browseFolder";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaItem.clearContexts();
        mediaList << mediaItem;
        
        //Show Audio CDs if present
        QStringList udis = Utilities::availableDiscUdis(Solid::OpticalDisc::Audio);
        foreach (QString udi, udis) {
            if (m_stop) {
                return;
            }
            Solid::Device device = Solid::Device( udi );
            if ( !device.isValid() )
                continue;
            mediaItem.title = i18n("Audio CD");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = QString( "cdaudio://%1" ).arg(udi);
            mediaItem.artwork = KIcon("media-optical-audio");
            mediaList << mediaItem;
        }

        //Load saved lists from index
        QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
        if (indexFile.exists()) {
            if (indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&indexFile);
                while (!in.atEnd()) {
                    if (m_stop) {
                        return;
                    }
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
        QStringList contextTitles;
        QStringList contextLRIs;
        MediaItem mediaItem;
        mediaItem.type = "Category";
        mediaItem.isSavedList = false;
        if (m_nepomukInited) {
            KConfig config;
            KConfigGroup generalGroup( &config, "General" );
            mediaItem.title = i18n("Recently Played");
            mediaItem.fields["title"] = mediaItem.title;
            int limit = generalGroup.readEntry("RecentVideoLimit", 20);
            mediaItem.url = QString("semantics://recent?video||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("chronometer");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Genres"), "semantics://recent?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Actors"), "semantics://recent?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Directors"), "semantics://recent?video||limit=4||groupBy=director");
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("HighestVideoLimit", 20);
            mediaItem.url = QString("semantics://highest?video||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("rating");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Genres"), "semantics://highest?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Actors"), "semantics://highest?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Directors"), "semantics://highest?video||limit=4||groupBy=director");
            mediaList << mediaItem;
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            limit = generalGroup.readEntry("FrequentVideoLimit", 20);
            mediaItem.url = QString("semantics://frequent?video||limit=%1").arg(limit);
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Genres"), "semantics://frequent?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Actors"), "semantics://frequent?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Directors"), "semantics://frequent?video||limit=4||groupBy=director");
            mediaList << mediaItem;
            
            mediaItem.title = i18n("Movies");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://movies";
            mediaItem.artwork = KIcon("tool-animator");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||videoType=movie");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||videoType=movie");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||videoType=movie");
            mediaList << mediaItem;
            mediaItem.title = i18n("TV Shows");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://tvshows";
            mediaItem.artwork = KIcon("video-television");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=seriesName");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=seriesName");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=seriesName");
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://genres";
            mediaItem.artwork = KIcon("flag-green");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=genre");
            mediaList << mediaItem;
            mediaItem.title = i18n("Actors");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://actors";
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=actor");
            mediaList << mediaItem;
            mediaItem.title = i18n("Directors");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://directors";
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=director");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=director");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=director");
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://clips";
            mediaItem.artwork = KIcon("video-x-generic");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||videoType=video clip");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||videoType=video clip");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||videoType=video clip");
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Feeds");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "feeds://videofeeds";
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaItem.clearContexts();
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://videotags";
            mediaItem.artwork = KIcon("view-pim-notes");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=tag");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=tag");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=tag");
            mediaList << mediaItem;
        }
        
        mediaItem.title = i18n("Files and Folders");
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = "files://video?browseFolder";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaItem.clearContexts();
        mediaList << mediaItem;
        
        QStringList udis = Utilities::availableDiscUdis(Solid::OpticalDisc::VideoDvd);
        foreach (QString udi, udis) {
            if (m_stop) {
                return;
            }
            Solid::Device device = Solid::Device( udi );
            if ( !device.isValid() )
                continue;
            const Solid::OpticalDisc* disc = Solid::Device(udi).as<const Solid::OpticalDisc>();
            if ( disc == NULL )
                continue;
            QString label = disc->label();
            mediaItem.title = label;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = QString( "dvdvideo://%1" ).arg(udi);
            mediaItem.artwork = KIcon("media-optical-dvd");
            mediaList << mediaItem;
        }
        
        //Load saved lists from index
        QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
        if (indexFile.exists()) {
            if (indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&indexFile);
                while (!in.atEnd()) {
                    if (m_stop) {
                        return;
                    }
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
