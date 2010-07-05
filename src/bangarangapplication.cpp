/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#include "bangarangapplication.h"
#include "mainwindow.h"
#include "infomanager.h"
#include "savedlistsmanager.h"
#include "bookmarksmanager.h"
#include "actionsmanager.h"
#include "audiosettings.h"
#include "platform/mediaitemmodel.h"
#include "platform/medialistcache.h"
#include "platform/playlist.h"
#include "platform/utilities.h"

#include <KCmdLineArgs>
#include <KConfig>
#include <KUrl>
#include <KMessageBox>
#include <KDebug>
#include <Nepomuk/ResourceManager>

void BangarangApplication::setup()
{
    //Register custom types that are used in signals/slots
    qRegisterMetaType<MediaItem>("MediaItem");
    qRegisterMetaType<MediaListProperties>("MediaListProperties");
    qRegisterMetaType<QList<MediaItem> >("QList<MediaItem>");
    
    //Set up media object
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setTickInterval(500);
    
    //Set up playlist
    m_playlist = new Playlist(this, m_mediaObject);
    
    //Set up Media Lists view browsing model
    m_browsingModel = new MediaItemModel(this);
    
    //Set up shared media list cache
    m_sharedMediaListCache = m_browsingModel->mediaListCache();
    m_playlist->playlistModel()->setMediaListCache(m_sharedMediaListCache);
    
    // Set up system tray icon
    m_statusNotifierItem = new KStatusNotifierItem(i18n("Bangarang"), this);
    m_statusNotifierItem->setIconByName("bangarang-notifier");
    m_statusNotifierItem->setStandardActionsEnabled(false);
    
    //Set up main window
    m_mainWindow = new MainWindow();
        
    //Setup Info Manager
    m_infoManager = new InfoManager(m_mainWindow);

    //Setup Saved Lists Manager
    m_savedListsManager = new SavedListsManager(m_mainWindow);
    
    //Setup Actions Manager
    m_actionsManager = new ActionsManager(m_mainWindow);

    //Setup Bookmarks Manager
    m_bookmarksManager = new BookmarksManager(m_mainWindow);
    
    //Setup Audio Settings
    m_audioSettings = new AudioSettings(m_mainWindow);
    
    //Complete Setup
    m_mainWindow->completeSetup();
    m_statusNotifierItem->setContextMenu(m_actionsManager->notifierMenu());
    
    //Initialize Nepomuk
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
    if (!m_nepomukInited) {
        KMessageBox::information(m_mainWindow, i18n("Bangarang is unable to access the Nepomuk Semantic Desktop repository. Media library, rating and play count functions will be unavailable."), i18n("Bangarang"), i18n("Don't show this message again"));
    }
    
    //Process command line arguments
    QList<MediaItem> mediaList;
    bool itemLoaded = false;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() > 0) {
        for(int i = 0; i < args->count(); i++) {
            if (args->isSet("play-dvd")) {
                //Play DVD
                kDebug() << "playing DVD";
                MediaItem mediaItem;
                mediaItem.title = i18n("DVD Video");
                mediaItem.url = "dvdvideo://";
                mediaItem.type = "Category";
                mediaList << mediaItem;
                itemLoaded = true;
                break;
            } else if (args->isSet("play-cd")) {
                //Play CD
                kDebug() << "playing CD";
                MediaItem mediaItem;
                mediaItem.title = i18n("Audio CD");
                mediaItem.url = "cdaudio://";
                mediaItem.type = "Category";
                mediaList << mediaItem;
                itemLoaded = true;
                break;
            } else {
                //Play Url
                KUrl cmdLineKUrl = args->url(i);
                MediaItem mediaItem = Utilities::mediaItemFromUrl(cmdLineKUrl);
                mediaList << mediaItem;
                itemLoaded = true;
            }
        }
        if (mediaList.count() > 0) {
            m_playlist->playMediaList(mediaList);
        }
    }
    
    //Load Config
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    m_playlist->setShuffleMode(generalGroup.readEntry("Shuffle", false));
    m_playlist->setRepeatMode(generalGroup.readEntry("Repeat", false));
    bool infoViewVisible = (generalGroup.readEntry("InfoViewVisible", false));
    if (m_infoManager->infoViewVisible() != infoViewVisible) {
        m_infoManager->toggleInfoView();
    }
    m_audioSettings->restoreAudioSettings(&generalGroup);
    if (!itemLoaded) {
        m_savedListsManager->loadPlaylist();
    }
    
}

BangarangApplication::~BangarangApplication()
{
    //Bookmark last position if program is closed while watching video.
    if (m_mediaObject->state() == Phonon::PlayingState || m_mediaObject->state() == Phonon::PausedState) {
        if (m_playlist->nowPlayingModel()->rowCount() > 0 && m_mediaObject->currentTime() > 10000) {
            MediaItem nowPlayingItem = m_playlist->nowPlayingModel()->mediaItemAt(0);
            if (nowPlayingItem.type == "Video") {
                QString nowPlayingUrl = nowPlayingItem.url;
                qint64 time = m_mediaObject->currentTime();
                QString existingBookmark = m_bookmarksManager->bookmarkLookup(nowPlayingUrl, i18n("Resume"));
                m_bookmarksManager->removeBookmark(nowPlayingUrl, existingBookmark);
                m_bookmarksManager->addBookmark(nowPlayingUrl, i18n("Resume"), time);
            }
        }
    }
    
    //Save application config
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    generalGroup.writeEntry("Shuffle", m_playlist->shuffleMode());
    generalGroup.writeEntry("Repeat", m_playlist->repeatMode());
    generalGroup.writeEntry("InfoViewVisible", m_infoManager->infoViewVisible());
    m_audioSettings->saveAudioSettings(&generalGroup);
    config.sync();
    m_savedListsManager->savePlaylist();
}

MainWindow * BangarangApplication::mainWindow()
{
    return m_mainWindow;
}

Playlist * BangarangApplication::playlist()
{
    return m_playlist;
}

Phonon::MediaObject * BangarangApplication::mediaObject()
{
    return m_mediaObject;
}

Phonon::MediaObject * BangarangApplication::newMediaObject()
{
    Phonon::MediaObject * oldMediaObject = m_mediaObject;
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setTickInterval(500);
    m_playlist->setMediaObject(m_mediaObject);
    delete oldMediaObject;
    return m_mediaObject;
}

MediaItemModel * BangarangApplication::browsingModel()
{
    return m_browsingModel;
}

MediaListCache * BangarangApplication::sharedMediaListCache()
{
    return m_sharedMediaListCache;
}

InfoManager * BangarangApplication::infoManager()
{
    return m_infoManager;
}

SavedListsManager * BangarangApplication::savedListsManager()
{
    return m_savedListsManager;
}

ActionsManager * BangarangApplication::actionsManager()
{
    return m_actionsManager;
}

BookmarksManager * BangarangApplication::bookmarksManager()
{
    return m_bookmarksManager;
}

KStatusNotifierItem * BangarangApplication::statusNotifierItem()
{
    return m_statusNotifierItem;
}

AudioSettings * BangarangApplication::audioSettings()
{
    return m_audioSettings;
}

const KAboutData * BangarangApplication::aboutData()
{
    return KCmdLineArgs::aboutData();
}

bool BangarangApplication::nepomukInited()
{
    return m_nepomukInited;
}
