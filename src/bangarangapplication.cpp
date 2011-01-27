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
#include "bangarangnotifieritem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "infomanager.h"
#include "savedlistsmanager.h"
#include "bookmarksmanager.h"
#include "actionsmanager.h"
#include "audiosettings.h"
#include "videosettings.h"
#include "dbusobjects.h"
#include "platform/mediaitemmodel.h"
#include "platform/medialistcache.h"
#include "platform/playlist.h"
#include "platform/utilities/utilities.h"

#include <KCmdLineArgs>
#include <KConfig>
#include <KUrl>
#include <KMessageBox>
#include <KDebug>
#include <KStandardDirs>
#include <Nepomuk/ResourceManager>
#include <Solid/Device>

#include <QDBusConnection>

void BangarangApplication::setup()
{
    //Register custom types that are used in signals/slots
    qRegisterMetaType<MediaItem>("MediaItem");
    qRegisterMetaType<MediaListProperties>("MediaListProperties");
    qRegisterMetaType<QList<MediaItem> >("QList<MediaItem>");
    qRegisterMetaType<QList<QImage> >("QList<QImage>");
    
    //setup locale
    m_locale = new KLocale( aboutData()->appName() );
    
    //Set up media object
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setTickInterval(500);
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this); // default to music category;
    connect(m_audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));


    
    //Set up playlist
    m_playlist = new Playlist(this, m_mediaObject);
    connect(m_playlist->nowPlayingModel(), SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));
    
    //Set up Media Lists view browsing model
    m_browsingModel = new MediaItemModel(this);
    
    //Set up shared media list cache
    m_sharedMediaListCache = m_browsingModel->mediaListCache();
    m_playlist->playlistModel()->setMediaListCache(m_sharedMediaListCache);
    
    // Set up system tray icon
    m_statusNotifierItem = new BangarangNotifierItem(this);
    
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
    m_audioSettings->setMediaController(m_playlist->mediaController());
    
    //Setup Video Settings
    m_videoSettings = new VideoSettings(m_mainWindow, m_mainWindow->videoWidget());
    m_videoSettings->setMediaController(m_playlist->mediaController());
    
    //Complete Setup
    m_mainWindow->completeSetup();

    //Create media paths
    m_audioPath = Phonon::createPath(m_mediaObject, m_audioOutput);
    m_videoPath = Phonon::createPath(m_mediaObject, m_mainWindow->videoWidget());
    m_volume = m_audioOutput->volume();
    m_audioSettings->setAudioPath(&m_audioPath);

    m_statusNotifierItem->setAssociatedWidget(m_mainWindow);
    m_statusNotifierItem->contextMenu()->addAction(m_actionsManager->action("mute"));
    m_statusNotifierItem->contextMenu()->addSeparator();
    m_statusNotifierItem->contextMenu()->addAction(m_actionsManager->action("play_previous"));
    m_statusNotifierItem->contextMenu()->addAction(m_actionsManager->action("play_pause"));
    m_statusNotifierItem->contextMenu()->addAction(m_actionsManager->action("play_next"));
    m_statusNotifierItem->contextMenu()->addSeparator();
    m_statusNotifierItem->contextMenu()->addAction(m_actionsManager->action("quit"));

    connect(m_statusNotifierItem, SIGNAL(changeVolumeRequested(int)), this, SLOT(volumeChanged(int)));

    //Initialize Nepomuk
    if (!Utilities::nepomukInited()) {
        KMessageBox::information(m_mainWindow, i18n("Bangarang is unable to access the Nepomuk Semantic Desktop repository. Media library, rating and play count functions will be unavailable."), i18n("Bangarang"), i18n("Don't show this message again"));
    }
    
    //Load Config
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    QSize windowSize = QSize(generalGroup.readEntry("WindowWidth", 740),
                             generalGroup.readEntry("WindowHeight", 520));
    m_mainWindow->setGeometry(QRect(m_mainWindow->rect().topLeft(), windowSize));
    QList<int> nowPlayingSplitterSizes = generalGroup.readEntry("NowPlayingSplitterSizes", QList<int>() << 375 << 362 );
    m_mainWindow->ui->nowPlayingSplitter->setSizes(nowPlayingSplitterSizes);
    QList<int> mediaListSplitterSizes = generalGroup.readEntry("MediaListSplitterSizes", QList<int>() << 170 << 565);
    m_mainWindow->ui->mediaListsSplitter->setSizes(mediaListSplitterSizes);
    QList<int> mediaViewSplitterSizes = generalGroup.readEntry("MediaViewSplitterSizes", QList<int>() << 338 << 220);
    m_mainWindow->ui->mediaViewSplitter->setSizes(mediaViewSplitterSizes);
    m_mainWindow->ui->mediaLists->setCurrentIndex(generalGroup.readEntry("mediaListsType", 0));
    bool infoViewVisible = (generalGroup.readEntry("InfoViewVisible", true));
    if (m_infoManager->infoViewVisible() != infoViewVisible) {
        m_infoManager->toggleInfoView();
    }
    m_playlist->setShuffleMode(generalGroup.readEntry("Shuffle", false));
    m_playlist->setRepeatMode(generalGroup.readEntry("Repeat", false));
    m_audioOutput->setVolume(static_cast<qreal>(generalGroup.readEntry("Volume", static_cast<int>(m_volume * 100))) / 100);
    m_audioSettings->restoreAudioSettings(&generalGroup);
    m_videoSettings->restoreVideoSettings(&generalGroup);

    //connect signal from notfier, so that pause/resume will work
    connect(m_statusNotifierItem, SIGNAL(changeStateRequested(Phonon::State)), this,
      SLOT(handleNotifierStateRequest(Phonon::State)));

    //Setup D-Bus Objects
    m_mprisRootObject = new MprisRootObject(this);
    QDBusConnection::sessionBus().registerObject("/", m_mprisRootObject, QDBusConnection::ExportAllContents);
    QDBusConnection::sessionBus().registerObject("/Player", new MprisPlayerObject(this), QDBusConnection::ExportAllContents);
    QDBusConnection::sessionBus().registerObject("/TrackList", new MprisTrackListObject(this), QDBusConnection::ExportAllContents);
    QDBusConnection::sessionBus().registerService("org.mpris.bangarang");
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
    generalGroup.writeEntry("Volume", static_cast<int>(m_volume*100));
    generalGroup.writeEntry("WindowWidth", m_mainWindow->width());
    generalGroup.writeEntry("WindowHeight", m_mainWindow->height());
    generalGroup.writeEntry("NowPlayingSplitterSizes", m_mainWindow->ui->nowPlayingSplitter->sizes());
    generalGroup.writeEntry("MediaListSplitterSizes", m_mainWindow->ui->mediaListsSplitter->sizes());
    generalGroup.writeEntry("MediaViewSplitterSizes", m_mainWindow->ui->mediaViewSplitter->sizes());
    generalGroup.writeEntry("mediaListsType", m_mainWindow->ui->mediaLists->currentIndex());
    m_audioSettings->saveAudioSettings(&generalGroup);
    m_videoSettings->saveVideoSettings(&generalGroup);
    config.sync();
    m_savedListsManager->savePlaylist();
    
     //destroying inverse to construction
    delete m_videoSettings;
    delete m_audioSettings;
    delete m_bookmarksManager;
    delete m_actionsManager;
    delete m_savedListsManager;
    delete m_infoManager;
    delete m_mainWindow;
    delete m_browsingModel;
    delete m_statusNotifierItem;
    delete m_playlist;
    delete m_audioOutput;
    delete m_mediaObject;
    delete m_locale;
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
    m_audioSettings->setMediaController(m_playlist->mediaController());
    m_videoSettings->setMediaController(m_playlist->mediaController());
    m_videoPath.reconnect(m_mediaObject, m_mainWindow->videoWidget());
    m_audioPath.reconnect(m_mediaObject, m_audioOutput);
    m_audioOutput->setVolume(m_volume);

    delete oldMediaObject;
    return m_mediaObject;
}

Phonon::AudioOutput * BangarangApplication::audioOutput()
{
    return m_audioOutput;
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

BangarangNotifierItem * BangarangApplication::statusNotifierItem()
{
    return m_statusNotifierItem;
}

AudioSettings * BangarangApplication::audioSettings()
{
    return m_audioSettings;
}

MprisRootObject* BangarangApplication::mprisRootObject()
{
    return m_mprisRootObject;
}

const KAboutData * BangarangApplication::aboutData()
{
    return KCmdLineArgs::aboutData();
}

void BangarangApplication::handleNotifierStateRequest(Phonon::State state)
{
  if (state == Phonon::PausedState)
    m_mediaObject->pause();
  else m_mediaObject->play();
}

qreal BangarangApplication::volume()
{
    return m_volume;
}

void BangarangApplication::volumeChanged(qreal newVolume)
{
    //Phonon::AudioOutput::volume() only return the volume at app start.
    //Therefore I need to track volume changes independently.
    m_volume = newVolume;
}

void BangarangApplication::volumeChanged(int delta)
{
    if ((m_volume == 0 && delta < 0) || (m_volume == 1 &&  delta > 0)) {
        return;
    }

    m_volume += (qreal)delta/25;
    if (m_volume < 0) {
        m_volume = 0;
    }
    if (m_volume > 1) {
        m_volume = 1;
    }

    m_audioOutput->setVolume(m_volume);
}

void BangarangApplication::nowPlayingChanged()
{
    MediaItemModel * nowPlayingModel = m_playlist->nowPlayingModel();
    if (nowPlayingModel->rowCount() == 0) {
        m_statusNotifierItem->setState(Phonon::StoppedState);
        return;
    }

    MediaItem nowPlayingItem = nowPlayingModel->mediaItemAt(0);
    QString type = nowPlayingItem.type;

    //Switch the audio output to the appropriate phonon category
    bool changed = false;
    m_volume = m_audioOutput->volume();
    if (type == "Audio" && m_audioOutput->category() != Phonon::MusicCategory) {
        m_audioPath.disconnect();
        delete m_audioOutput;
        m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        changed = true;
    } else if (type == "Video" && m_audioOutput->category() != Phonon::VideoCategory) {
        m_audioPath.disconnect();
        delete m_audioOutput;
        m_audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
        changed = true;
    }

    //Reconnect audio
    if (changed) {
        m_audioPath.reconnect(m_mediaObject, m_audioOutput);
        m_audioOutput->setVolume(m_volume);
        m_audioSettings->reconnectAudioPath(&m_audioPath);
        connect(m_audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
        m_mainWindow->connectPhononWidgets();
    }

}

void BangarangApplication::processCommandLineArgs()
{

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
                QStringList udis = Utilities::availableDiscUdis(Solid::OpticalDisc::Audio);
                foreach (QString udi, udis) {
                    Solid::Device device = Solid::Device( udi );
                    if ( !device.isValid() ) {
                        continue;
                    }
                    MediaItem mediaItem;
                    mediaItem.type = "Category";
                    mediaItem.title = i18n("Audio CD");
                    mediaItem.fields["title"] = mediaItem.title;
                    mediaItem.url = QString( "cdaudio://%1" ).arg(udi);
                    mediaList << mediaItem;
                    itemLoaded = true;
                    break;
                }
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
    if (!itemLoaded) {
        m_savedListsManager->loadPlaylist();
    }
}
