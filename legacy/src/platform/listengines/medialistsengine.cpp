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

#include "medialistsengine.h"
#include "../devicemanager.h"
#include "../mediaitemmodel.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"

#include <QtGui/QApplication>
#include <QtCore/QFile>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/OpticalDisc>
//#include <Nepomuk2/ResourceManager>


MediaListsEngine::MediaListsEngine(ListEngineFactory * parent) : ListEngine(parent)
{
}

MediaListsEngine::~MediaListsEngine()
{
}

void MediaListsEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;
    bool m_nepomukInited = false; //Set to false here since we no longer inherit from NepomukListEngine. Remove and replace when we have a replacement for NepomukListEngine.

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
            mediaItem.url = semanticsLriForRecent("Audio");
            mediaItem.artwork = KIcon("chronometer");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Artists"), "semantics://recent?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Albums"), "semantics://recent?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Genres"), "semantics://recent?audio||limit=4||groupBy=genre");
            mediaItem.fields["isConfigurable"] = true;
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = semanticsLriForHighest("Audio");
            mediaItem.artwork = KIcon("rating");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Artists"), "semantics://highest?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Albums"), "semantics://highest?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Genres"), "semantics://highest?audio||limit=4||groupBy=genre");
            mediaItem.fields["isConfigurable"] = true;
            mediaList << mediaItem;
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = semanticsLriForFrequent("Audio");
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Artists"), "semantics://frequent?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Albums"), "semantics://frequent?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Genres"), "semantics://frequent?audio||limit=4||groupBy=genre");
            mediaItem.fields["isConfigurable"] = true;
            mediaList << mediaItem;
            mediaItem.title = i18n("Recently Added");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = semanticsLriForRecentlyAdded("Audio");
            mediaItem.artwork = KIcon("chronometer");
            mediaItem.fields["isConfigurable"] = true;
            mediaItem.clearContexts();
            mediaList << mediaItem;

            mediaItem.title = i18n("Artists");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://artists";
            mediaItem.artwork = KIcon("system-users");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=artist");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=artist");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Albums");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://albums";
            mediaItem.artwork = KIcon("media-optical");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=album");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=album");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "music://genres";
            mediaItem.artwork = KIcon("flag-blue");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Played"), "semantics://recent?audio||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?audio||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Frequently Played"), "semantics://frequent?audio||limit=4||groupBy=genre");
            mediaItem.fields["isConfigurable"] = false;
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
            mediaItem.fields["isConfigurable"] = false;
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
            mediaItem.fields["isConfigurable"] = false;
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
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Audio Feeds");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "feeds://audiofeeds";
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaItem.clearContexts();
            mediaItem.fields["isConfigurable"] = false;
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
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
        }

        mediaItem.title = i18n("Files and Folders");
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = "files://audio?browseFolder";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaItem.clearContexts();
        mediaItem.fields["isConfigurable"] = false;
        mediaList << mediaItem;
        
        // Show remote media servers
        mediaList.append(loadServerList("audio"));

        //Show Audio CDs if present
        QList<Solid::Device> cds = DeviceManager::instance()->deviceList(DeviceManager::AudioType);
        foreach (const Solid::Device& cd, cds) {
            if (m_stop) {
                return;
            }
            mediaItem.title = i18n("Audio CD");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = QString( "cdaudio://%1" ).arg(cd.udi());
            mediaItem.artwork = KIcon("media-optical-audio");
            mediaItem.fields["isConfigurable"] = false;
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
                            QString savedLri = nameUrl.at(2).trimmed();
                            if (savedLri.startsWith(QLatin1String("savedlists://"))) {
                                mediaItem.url = savedLri;
                                mediaItem.artwork = KIcon("view-list-text");
                            } else {
                                mediaItem.url = QString("savedlists://?%1").arg(savedLri);
                                mediaItem.artwork = KIcon("view-media-playlist");
                            }
                            mediaItem.isSavedList = true;
                            mediaItem.fields["isConfigurable"] = true;
                            if (m_nepomukInited) {
                                mediaList << mediaItem;
                            } else {
                                //Only show lists that aren't don't require nepomuk
                                if (mediaItem.url.startsWith(QLatin1String("savedlists://"))) {
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
            mediaItem.url = semanticsLriForRecent("Video");
            mediaItem.artwork = KIcon("chronometer");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Genres"), "semantics://recent?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Actors"), "semantics://recent?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Directors"), "semantics://recent?video||limit=4||groupBy=director");
            mediaItem.fields["isConfigurable"] = true;
            mediaList << mediaItem;
            mediaItem.title = i18n("Highest Rated");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = semanticsLriForHighest("Video");
            mediaItem.artwork = KIcon("rating");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Genres"), "semantics://highest?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Actors"), "semantics://highest?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Directors"), "semantics://highest?video||limit=4||groupBy=director");
            mediaItem.fields["isConfigurable"] = true;
            mediaList << mediaItem;
            mediaItem.title = i18n("Frequently Played");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = semanticsLriForFrequent("Video");
            mediaItem.artwork = KIcon("office-chart-bar");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Genres"), "semantics://frequent?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Actors"), "semantics://frequent?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Directors"), "semantics://frequent?video||limit=4||groupBy=director");
            mediaItem.fields["isConfigurable"] = true;
            mediaList << mediaItem;
            mediaItem.title = i18n("Recently Added");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = semanticsLriForRecentlyAdded("Video");
            mediaItem.artwork = KIcon("chronometer");
            mediaItem.fields["isConfigurable"] = true;
            mediaItem.clearContexts();
            mediaList << mediaItem;

            mediaItem.title = i18n("Movies");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://movies";
            mediaItem.artwork = KIcon("tool-animator");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||videoType=movie");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||videoType=movie");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||videoType=movie");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("TV Shows");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://tvshows";
            mediaItem.artwork = KIcon("video-television");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=seriesName");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=seriesName");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=seriesName");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Genres");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://genres";
            mediaItem.artwork = KIcon("flag-green");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=genre");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=genre");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Actors");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://actors";
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=actor");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=actor");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Directors");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://directors";
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=director");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=director");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=director");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Clips");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "video://clips";
            mediaItem.artwork = KIcon("video-x-generic");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||videoType=video clip");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||videoType=video clip");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||videoType=video clip");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Video Feeds");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "feeds://videofeeds";
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaItem.clearContexts();
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
            mediaItem.title = i18n("Tags");
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = "tag://videotags";
            mediaItem.artwork = KIcon("view-pim-notes");
            mediaItem.clearContexts();
            mediaItem.addContext(i18n("Recently Watched"), "semantics://recent?video||limit=4||groupBy=tag");
            mediaItem.addContext(i18n("Highest Rated"), "semantics://highest?video||limit=4||groupBy=tag");
            mediaItem.addContext(i18n("Frequently Watched"), "semantics://frequent?video||limit=4||groupBy=tag");
            mediaItem.fields["isConfigurable"] = false;
            mediaList << mediaItem;
        }

        mediaItem.title = i18n("Files and Folders");
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = "files://video?browseFolder";
        mediaItem.artwork = KIcon("document-open-folder");
        mediaItem.clearContexts();
        mediaItem.fields["isConfigurable"] = false;
        mediaList << mediaItem;
        
        QList<Solid::Device> dvds = DeviceManager::instance()->deviceList(DeviceManager::VideoType);
        foreach (const Solid::Device& dvd, dvds) {
            if (m_stop) {
                return;
            }
            const Solid::OpticalDisc* disc = dvd.as<const Solid::OpticalDisc>();
            if ( disc == NULL )
                continue;
            QString label = disc->label();
            mediaItem.title = label;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.url = QString( "dvdvideo://%1" ).arg(dvd.udi());
            mediaItem.artwork = KIcon("media-optical-dvd");
            mediaItem.fields["isConfigurable"] = false;
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
                    QStringList nameUrl = line.split("::a:");
                    if (nameUrl.count() >= 3) {
                        if (nameUrl.at(0) == "Video") {
                            mediaItem.title = nameUrl.at(1).trimmed();
                            mediaItem.fields["title"] = mediaItem.title;
                            QString savedLri = nameUrl.at(2).trimmed();
                            if (savedLri.startsWith(QLatin1String("savedlists://"))) {
                                mediaItem.url = savedLri;
                                mediaItem.artwork = KIcon("view-list-text");
                            } else {
                                mediaItem.url = QString("savedlists://?%1").arg(savedLri);
                                mediaItem.artwork = KIcon("view-media-playlist");
                            }
                            mediaItem.isSavedList = true;
                            mediaItem.fields["isConfigurable"] = true;
                            if (m_nepomukInited) {
                                mediaList << mediaItem;
                            } else {
                                //Only show lists that aren't don't require nepomuk
                                if (mediaItem.url.startsWith(QLatin1String("savedlists://"))) {
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
    m_requestSignature.clear();
    m_subRequestSignature.clear();
    m_loadWhenReady = false;
    //exec();    
}

QString MediaListsEngine::semanticsLriForRecent(const QString &type)
{
    QString lri;
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    if (type == "Audio") {
        int limit = generalGroup.readEntry("RecentAudioLimit", 20);
        lri = QString("semantics://recent?audio||limit=%1").arg(limit);
        if (generalGroup.hasKey("RecentAudioPlayed")) {
            QStringList entry = generalGroup.readEntry("RecentAudioPlayed", QStringList());
            QString comp = entry.at(0);
            QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
            lri.append(QString("||lastPlayed%1%2").arg(comp).arg(recentDateTime.toString("yyyyMMddHHmmss")));
        }
    } else {
        int limit = generalGroup.readEntry("RecentVideoLimit", 20);
        lri = QString("semantics://recent?video||limit=%1").arg(limit);
        if (generalGroup.hasKey("RecentVideoPlayed")) {
            QStringList entry = generalGroup.readEntry("RecentVideoPlayed", QStringList());
            QString comp = entry.at(0);
            QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
            lri.append(QString("||lastPlayed%1%2").arg(comp).arg(recentDateTime.toString("yyyyMMddHHmmss")));
        }
    }
    return lri;
}

QString MediaListsEngine::semanticsLriForHighest(const QString &type)
{
    QString lri;
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    if (type == "Audio") {
        int limit = generalGroup.readEntry("HighestAudioLimit", 20);
        lri = QString("semantics://highest?audio||limit=%1").arg(limit);
        if (generalGroup.hasKey("HighestAudioRated")) {
            QStringList entry = generalGroup.readEntry("HighestAudioRated", QStringList());
            QString comp = entry.at(0);
            int rating = entry.at(1).toInt();
            lri.append(QString("||rating%1%2").arg(comp).arg(rating));
        }
    } else {
        int limit = generalGroup.readEntry("HighestVideoLimit", 20);
        lri = QString("semantics://highest?video||limit=%1").arg(limit);
        if (generalGroup.hasKey("HighestVideoRated")) {
            QStringList entry = generalGroup.readEntry("HighestVideoRated", QStringList());
            QString comp = entry.at(0);
            int rating = entry.at(1).toInt();
            lri.append(QString("||rating%1%2").arg(comp).arg(rating));
        }
    }
    return lri;
}

QString MediaListsEngine::semanticsLriForFrequent(const QString &type)
{
    QString lri;
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    if (type == "Audio") {
        int limit = generalGroup.readEntry("FrequentAudioLimit", 20);
        lri = QString("semantics://frequent?audio||limit=%1").arg(limit);
        if (generalGroup.hasKey("FrequentAudioPlayed")) {
            QStringList entry = generalGroup.readEntry("FrequentAudioPlayed", QStringList());
            QString comp = entry.at(0);
            int playCount = entry.at(1).toInt();
            lri.append(QString("||playCount%1%2").arg(comp).arg(playCount));
        }
    } else {
        int limit = generalGroup.readEntry("FrequentVideoLimit", 20);
        lri = QString("semantics://frequent?video||limit=%1").arg(limit);
        if (generalGroup.hasKey("FrequentVideoPlayed")) {
            QStringList entry = generalGroup.readEntry("FrequentVideoPlayed", QStringList());
            QString comp = entry.at(0);
            int playCount = entry.at(1).toInt();
            lri.append(QString("||playCount%1%2").arg(comp).arg(playCount));
        }
    }
    return lri;
}

QString MediaListsEngine::semanticsLriForRecentlyAdded(const QString &type)
{
    QString lri;
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    if (type == "Audio") {
        int limit = generalGroup.readEntry("RecentlyAddedAudioLimit", 20);
        lri = QString("semantics://recentlyadded?audio||limit=%1").arg(limit);
    } else {
        int limit = generalGroup.readEntry("RecentlyAddedVideoLimit", 20);
        lri = QString("semantics://recentlyadded?video||limit=%1").arg(limit);
    }
    return lri;
}

QList<MediaItem> MediaListsEngine::loadServerList(QString type)
{
    //Load ampache server list
    QList<MediaItem> mediaList;
    QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/ampacheservers", false));

    if (!indexFile.exists()) {
        return mediaList;
    }
    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return mediaList;
    }

    QTextStream in(&indexFile);
    while (!in.atEnd()) {
        if (m_stop) {
            return mediaList;
        }
        QString line = in.readLine();
        QStringList nameUrl = line.split(":::");
        if (nameUrl.count() >= 6) {
            MediaItem mediaItem;
            if (nameUrl.at(0).toLower() == type.toLower()) {
                mediaItem.type = "Category";
                mediaItem.artwork = KIcon("repository");
                mediaItem.title = nameUrl.at(1).trimmed();
                mediaItem.fields["title"] = mediaItem.title;
                QString server = nameUrl.at(2).trimmed();
                QString userName = nameUrl.at(3).trimmed();
                QString key = nameUrl.at(4).trimmed();
                int pwdLength = nameUrl.at(5).trimmed().toInt();
                mediaItem.url = QString("ampache://%1?server=%2||username=%3||key=%4||request=root")
                                       .arg(type)
                                       .arg(server)
                                       .arg(userName)
                                       .arg(key);
                mediaItem.fields["server"] = server;
                mediaItem.fields["username"] = userName;
                mediaItem.fields["key"] = key;
                mediaItem.fields["pwdLength"] = pwdLength;
                mediaItem.fields["isConfigurable"] = true;
                mediaList << mediaItem;
            }
        }
    }
    return mediaList;
}

