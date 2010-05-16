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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/utilities.h"
#include "platform/mediaitemmodel.h"
#include "platform/medialistcache.h"
#include "platform/playlist.h"
#include "platform/bangarangvideowidget.h"
#include "infomanager.h"
#include "savedlistsmanager.h"
#include "bookmarksmanager.h"
#include "actionsmanager.h"
#include "mediaitemdelegate.h"
#include "nowplayingdelegate.h"
#include "videosettings.h"
#include "audiosettings.h"
#include "scriptconsole.h"

#include <KAction>
#include <KCmdLineArgs>
#include <KCursor>
#include <KUrl>
#include <KIcon>
#include <KIconEffect>
#include <KMessageBox>
#include <KSqueezedTextLabel>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KDebug>
#include <KHelpMenu>
#include <KMenu>
//#ifdef HAVE_KSTATUSNOTIFIERITEM
#include <KStatusNotifierItem>
//#endif
#include <kio/netaccess.h>
#include <kio/copyjob.h>
#include <kio/job.h>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/OpticalDisc>
#include <Solid/DeviceNotifier>
#include <Nepomuk/ResourceManager>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QtGlobal>
#include <QPalette>
#include <QPushButton>
#include <QAbstractItemModel>
#include <QHeaderView>
#include <QStringListModel>
#include <QFile>
#include <QScrollBar>
#include <QTimer>
#include <kross/core/action.h> 
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    qRegisterMetaType<MediaItem>("MediaItem");
    qRegisterMetaType<MediaListProperties>("MediaListProperties");
    qRegisterMetaType<QList<MediaItem> >("QList<MediaItem>");
    
    ui->setupUi(this);
    setGeometry(0,0,760,520);

    // Set up system tray icon
    m_sysTray = new KStatusNotifierItem(i18n("Bangarang"), this);
    m_sysTray->setIconByName("bangarang-notifier");

    //Setup interface icons
    setupIcons();

    // Hide certain widgets
    ui->previous->setVisible(false);
    ui->contextStack->setVisible(false);
    ui->playSelected->setVisible(false);
    ui->configureAudioList->setVisible(false);
    ui->configureVideoList->setVisible(false);
    ui->semanticsHolder->setVisible(false);
    
    //Initialize Nepomuk
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
        ui->Filter->setVisible(false);
    }
    
    //Set up device notifier
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString & )), this, SLOT(deviceAdded(const QString & )));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString & )), this, SLOT(deviceRemoved(const QString & )));
    
    //Set up media object
    m_media = new Phonon::MediaObject(this);
    m_videoWidget =  new BangarangVideoWidget(ui->videoFrame);
    m_audioOutputMusicCategory = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    m_audioOutputVideoCategory = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    m_audioOutput = m_audioOutputMusicCategory; // default to music category;
    m_videoPath = Phonon::createPath(m_media, m_videoWidget);
    m_audioPath = Phonon::createPath(m_media, m_audioOutput);
    m_media->setTickInterval(500);
    m_volume = m_audioOutput->volume();
    connect(m_videoWidget,SIGNAL(skipForward(int)),this, SLOT(skipForward(int)));
    connect(m_videoWidget,SIGNAL(skipBackward(int)),this, SLOT(skipBackward(int)));
    connect(m_videoWidget,SIGNAL(fullscreenChanged(bool)),this,SLOT(on_fullScreen_toggled(bool)));
    
    //Add video widget to video frame on viewer stack
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_videoWidget);
    layout->setContentsMargins(0,0,0,0);
    ui->videoFrame->setLayout(layout);
    ui->videoFrame->setFrameShape(QFrame::NoFrame);

    //Set up volume and seek slider
    ui->volumeSlider->setAudioOutput(m_audioOutput);
    ui->volumeSlider->setMuteVisible( false );
    ui->seekSlider->setMediaObject(m_media);
    ui->seekSlider->setIconVisible(false);
    setShowRemainingTime(false);
    ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    
    //Connect to media object signals and slots
    connect(m_media, SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_audioOutputMusicCategory, SLOT(setMuted(bool)));
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_audioOutputVideoCategory, SLOT(setMuted(bool)));
    connect(m_audioOutputMusicCategory, SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
    connect(m_audioOutputMusicCategory, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
    connect(m_audioOutputVideoCategory, SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
    connect(m_audioOutputVideoCategory, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
    connect(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
    
    //Set up Audio lists view 
    MediaListProperties audioListsProperties;
    audioListsProperties.lri = "medialists://audio";
    m_audioListsModel = new MediaItemModel(this);
    m_audioListsModel->setMediaListProperties(audioListsProperties);
    ui->audioLists->setModel(m_audioListsModel);
    connect(ui->audioLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(audioListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_audioListsModel, SIGNAL(mediaListChanged()), this, SLOT(audioListsChanged()));
    m_audioListsModel->load();
    
    //Set up Video lists view 
    MediaListProperties videoListsProperties;
    videoListsProperties.lri = "medialists://video";
    m_videoListsModel = new MediaItemModel(this);
    m_videoListsModel->setMediaListProperties(videoListsProperties);
    ui->videoLists->setModel(m_videoListsModel);
    connect(ui->videoLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(videoListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_videoListsModel, SIGNAL(mediaListChanged()), this, SLOT(videoListsChanged()));
    m_videoListsModel->load();
    
    //Set up media list view
    ui->mediaView->setMainWindow(this);
    m_mediaItemModel = (MediaItemModel *)ui->mediaView->model();
    m_sharedMediaListCache = m_mediaItemModel->mediaListCache();
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_mediaItemModel, SIGNAL(mediaListPropertiesChanged()), this, SLOT(mediaListPropertiesChanged()));
    connect(m_mediaItemModel, SIGNAL(loading()), this, SLOT(hidePlayButtons()));
    connect(m_mediaItemModel, SIGNAL(propertiesChanged()), this, SLOT(updateListHeader()));
    connect((MediaItemDelegate *)ui->mediaView->itemDelegate(), SIGNAL(categoryActivated(QModelIndex)), this, SLOT(mediaListCategoryActivated(QModelIndex)));
    connect((MediaItemDelegate *)ui->mediaView->itemDelegate(), SIGNAL(actionActivated(QModelIndex)), this, SLOT(mediaListActionActivated(QModelIndex)));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    
    //Set up MediaItemModel notifications
    ui->notificationWidget->setVisible(false);
    connect(m_mediaItemModel, SIGNAL(sourceInfoUpdateRemovalStarted()), this, SLOT(showNotification()));
    connect(m_mediaItemModel, SIGNAL(sourceInfoUpdateProgress(int)), ui->notificationProgress, SLOT(setValue(int)));
    connect(m_mediaItemModel, SIGNAL(sourceInfoRemovalProgress(int)), ui->notificationProgress, SLOT(setValue(int)));
    connect(m_mediaItemModel, SIGNAL(sourceInfoUpdateRemovalComplete()), this, SLOT(delayedNotificationHide()));
    connect(m_mediaItemModel, SIGNAL(sourceInfoUpdated(MediaItem)), this, SLOT(sourceInfoUpdated(MediaItem)));
    connect(m_mediaItemModel, SIGNAL(sourceInfoRemoved(QString)), this, SLOT(sourceInfoRemoved(QString)));
    
    //Set up playlist
    m_playlist = new Playlist(this, m_media);
    connect(m_playlist, SIGNAL(playlistFinished()), this, SLOT(playlistFinished()));
    connect(m_playlist, SIGNAL(loading()), this, SLOT(showLoading()));
    connect(m_playlist, SIGNAL(shuffleModeChanged(bool)), this, SLOT(shuffleModeChanged(bool)));
    connect(m_playlist, SIGNAL(repeatModeChanged(bool)), this, SLOT(repeatModeChanged(bool)));
    
    //Set up playlist view
    m_currentPlaylist = m_playlist->playlistModel();
    m_currentPlaylist->setMediaListCache(m_sharedMediaListCache);
    m_playlistItemDelegate = new MediaItemDelegate(this);
    m_playlist->filterProxyModel()->setSourceModel(m_currentPlaylist);
    ui->playlistView->setModel(m_playlist->filterProxyModel());
    ui->playlistFilterProxyLine->lineEdit()->setClickMessage(i18n("Search in playlist..."));
    ui->playlistFilterProxyLine->setProxy(m_playlist->filterProxyModel());
    ui->playlistFilter->setVisible(false);
    m_playlistItemDelegate->setUseProxy(true);
    ui->playlistView->setItemDelegate(m_playlistItemDelegate);
    m_playlistItemDelegate->setView(ui->playlistView);
    playWhenPlaylistChanges = false;
    connect(m_currentPlaylist, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    connect(m_currentPlaylist, SIGNAL(mediaListPropertiesChanged()), this, SLOT(playlistChanged()));

    //Setup Now Playing view
    m_nowPlaying = m_playlist->nowPlayingModel();
    m_nowPlayingDelegate = new NowPlayingDelegate(this);
    ui->nowPlayingView->setModel(m_nowPlaying);
    ui->nowPlayingView->setItemDelegate(m_nowPlayingDelegate);
    connect(m_nowPlaying, SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));
    ui->nowPlayingView->header()->setVisible(false);
    ui->nowPlayingView->header()->hideSection(1);
    updateCustomColors();
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateCustomColors())); 
    
    //Setup Info View
    m_infoManager = new InfoManager(this);

    //Setup Saved Lists Manager
    m_savedListsManager = new SavedListsManager(this);
    
    //Setup Bookmarks Manager
    m_bookmarksManager = new BookmarksManager(this);
    
    //Setup Actions Manager
    m_actionsManager = new ActionsManager(this);
    
    //Setup Audio settings
    m_audioSettings = new AudioSettings(this);
    m_audioSettings->setAudioPath(&m_audioPath);
    
    //Setup Video Settings
    VideoSettings *videoSettings = new VideoSettings(m_videoWidget, this);
    videoSettings->setHideAction(m_actionsManager->action("show_video_settings"));
    ui->videoSettingsPage->layout()->addWidget(videoSettings);
    
    //Set up defaults
    ui->nowPlayingSplitter->setCollapsible(0,true);
    ui->nowPlayingSplitter->setCollapsible(1,false);
    ui->stackedWidget->setCurrentIndex(1);
    ui->mediaViewHolder->setCurrentIndex(0);
    ui->audioListsStack->setCurrentIndex(0);
    ui->videoListsStack->setCurrentIndex(0);
    ui->contextStack->setCurrentIndex(0);
    ui->mediaPlayPause->setHoldDelay(1000);
    ui->mediaPrevious->setDefaultAction(m_actionsManager->action("play_previous"));
    ui->mediaNext->setDefaultAction(m_actionsManager->action("play_next"));
    ui->listSummary->setFont(KGlobalSettings::smallestReadableFont());
    ui->playlistDuration->setFont(KGlobalSettings::smallestReadableFont());
    ui->playbackMessage->clear();
    ui->collectionButton->setFocus();
    updateSeekTime(0);
    showApplicationBanner();
    updateCachedDevicesList();
    m_showQueue = false;
    m_pausePressed = false;
    m_stopPressed = false;
    m_loadingProgress = 0;
    
    QList<MediaItem> mediaList;
    //Get command line args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    kDebug() << KCmdLineArgs::parsedArgs();
    if (args->count() > 0) {
        for(int i = 0; i < args->count(); i++) {
            if (args->isSet("play-dvd")) {
                //Play DVD
                kDebug() << "playing DVD";
                MediaItem mediaItem;
                mediaItem.title = i18n("DVD Video");
                mediaItem.url = "dvdvideo://";
                mediaItem.type = "Category";
                QList<MediaItem> mediaList;
                mediaList << mediaItem;
                m_playlist->playMediaList(mediaList);
            } else if (args->isSet("play-cd")) {
                //Play CD
                kDebug() << "playing CD";
                MediaItem mediaItem;
                mediaItem.title = i18n("Audio CD");
                mediaItem.url = "cdaudio://";
                mediaItem.type = "Category";
                QList<MediaItem> mediaList;
                mediaList << mediaItem;
                m_playlist->playMediaList(mediaList);
            } else {
                //Play Url
                KUrl cmdLineKUrl = args->url(i);
                MediaItem mediaItem = Utilities::mediaItemFromUrl(cmdLineKUrl);
                mediaList << mediaItem;
                m_playlist->playMediaList(mediaList);
            }
        }
    } else {
        if (m_nepomukInited) {
            //Preload queries that are likely to be long so that, if necessary, they can be cached early
            //TODO:prehaps this could be configurable.
            /*MediaListProperties mediaListProperties;
            mediaListProperties.lri = "music://songs";
            m_mediaItemModel->setMediaListProperties(mediaListProperties);
            m_mediaItemModel->load();
            mediaListProperties.lri = "video://movies";
            m_mediaItemModel->setMediaListProperties(mediaListProperties);
            m_mediaItemModel->load();*/
        } else {
            KMessageBox::information(this, i18n("Bangarang is unable to access the Nepomuk Semantic Desktop repository. Media library, rating and play count functions will be unavailable."), i18n("Bangarang"), i18n("Don't show this message again"));
        }
    }
    
    //Set default media list
    ui->mediaLists->setCurrentIndex(0);
    
    //Install event filter for hiding widgets in Now Playing view
    ui->nowPlayingView->setMouseTracking(true);
    m_videoWidget->setMouseTracking(true);
    ui->nowPlayingView->viewport()->installEventFilter(this);
    m_videoWidget->installEventFilter(this);

    // Set up cursor hiding and context menu for videos.
    m_videoWidget->setFocusPolicy(Qt::ClickFocus);
    KCursor::setAutoHideCursor(m_videoWidget, true);
    m_videoWidget->setContextMenu(m_actionsManager->nowPlayingContextMenu());

    // Set up system tray actions
    m_sysTray->setStandardActionsEnabled(false);
    m_sysTray->setContextMenu(m_actionsManager->notifierMenu());
    
    //Load config
    KConfig config;
    KConfigGroup generalGroup( &config, "General" );
    m_playlist->setShuffleMode(generalGroup.readEntry("Shuffle", false));
    m_playlist->setRepeatMode(generalGroup.readEntry("Repeat", false));
    m_audioSettings->restoreAudioSettings(&generalGroup);
    m_savedListsManager->loadPlaylist();

    m_scriptConsole = new ScriptConsole();
    m_scriptConsole->addObject(m_videoWidget,"videoWidget");
    m_scriptConsole->addObject(m_media,"media");
    m_scriptConsole->addObject(m_actionsManager,"actionsManager");
    m_scriptConsole->addObject(m_playlist,"playlist");
    m_scriptConsole->addObject(m_sysTray,"sysTray");
    m_scriptConsole->addObject(m_savedListsManager,"savedListsManager");
    m_scriptConsole->addObject(m_playlistItemDelegate,"playlistItemDelegate");
    m_scriptConsole->addObject(m_nowPlayingDelegate,"nowPlayingDelegate");
    m_scriptConsole->addObject(this,"mainwindow");
}

MainWindow::~MainWindow()
{
    //Bookmark last position if program is closed while watching video.
    if (m_media->state() == Phonon::PlayingState || m_media->state() == Phonon::PausedState) {
        if (m_playlist->nowPlayingModel()->rowCount() > 0 && m_media->currentTime() > 10000) {
            MediaItem nowPlayingItem = m_playlist->nowPlayingModel()->mediaItemAt(0);
            if (nowPlayingItem.type == "Video") {
                QString nowPlayingUrl = nowPlayingItem.url;
                qint64 time = m_media->currentTime();
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
    m_audioSettings->saveAudioSettings(&generalGroup);
    config.sync();
    m_savedListsManager->savePlaylist();

    delete ui;
}


/*---------------------
 -- UI widget slots  --
 ----------------------*/
void MainWindow::on_Filter_returnPressed()
{
    ui->mediaView->setFocus();
    ui->audioLists->selectionModel()->clearSelection();
    if (!ui->Filter->text().isEmpty()) {
        if (ui->mediaLists->currentIndex() == 0) {
            MediaListProperties searchProperties;
            searchProperties.name = "Audio Search";
            searchProperties.lri = QString("music://search?%1").arg(ui->Filter->text());
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(searchProperties);
            m_mediaItemModel->load();
        } else if (ui->mediaLists->currentIndex() == 1) {
            MediaListProperties searchProperties;
            searchProperties.name = "Video Search";
            searchProperties.lri = QString("video://search?%1").arg(ui->Filter->text());
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(searchProperties);
            m_mediaItemModel->load();
        }
        m_mediaListHistory.clear();
        m_mediaListPropertiesHistory.clear();
        ui->previous->setVisible(false);
        ui->mediaViewHolder->setCurrentIndex(0);
    }
}

void MainWindow::on_nowPlaying_clicked()
{
    ui->stackedWidget->setCurrentIndex(1); // Show Now Playing page
}

void MainWindow::on_collectionButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0); // Show Collection page
    ui->collectionButton->setFocus();
}

void MainWindow::on_showPlaylist_clicked(bool checked)
{
    ui->contextStack->setVisible(checked);
    if (ui->playlistFilterProxyLine->lineEdit()->text().isEmpty() &&
        m_actionsManager->m_playlistFilterVisible
       ) {
        m_actionsManager->action("toggle_playlist_filter")->trigger();
    }
    ui->contextStack->setCurrentIndex(0);  
    m_actionsManager->action("show_video_settings")->setText(i18n("Show Video Settings"));
}

void MainWindow::on_fullScreen_toggled(bool fullScreen)
{
    if (fullScreen) {
        showFullScreen();
        ui->fullScreen->setIcon(KIcon("view-restore"));
        ui->fullScreen->setToolTip(i18n("<b>Fullscreen</b><br>Click to exit fullscreen"));
        ui->fullScreen->setChecked(true);
        ui->widgetSet->setVisible(false);
        ui->nowPlayingToolbar->setVisible(false);
    } else {
        showNormal();
        ui->fullScreen->setIcon(KIcon("view-fullscreen"));
        ui->fullScreen->setToolTip(i18n("Show fullscreen"));
        ui->fullScreen->setChecked(false);
        if (m_actionsManager->m_controlsVisible)
        {
          ui->widgetSet->setVisible(true);
          ui->nowPlayingToolbar->setVisible(true);
        }
    }
}

void MainWindow::on_seekTime_clicked()
{
    QPoint menuLocation = ui->seekTime->mapToGlobal(QPoint(0,ui->showMenu->height()));
    m_actionsManager->bookmarksMenu()->popup(menuLocation);
}

void MainWindow::on_mediaPlayPause_pressed()
{
    if ((m_media->state() == Phonon::PlayingState)) {
        m_media->pause();
        m_pausePressed = true;
        ui->mediaPlayPause->setToolTip(i18n("<b>Paused</b><br>Hold to stop"));
    }
}

void MainWindow::on_mediaPlayPause_held()
{
    if ((m_media->state() != Phonon::LoadingState) && (m_media->state() != Phonon::StoppedState)) {
        if (m_pausePressed) {
            m_pausePressed = false;
        }
        m_stopPressed = true;
        ui->mediaPlayPause->setIcon(KIcon("media-playback-stop"));
        m_playlist->stop();
    }
}

void MainWindow::on_mediaPlayPause_released()
{
    if (m_stopPressed) {
        m_stopPressed = false;
    } else {
        if ((!m_pausePressed) && (m_media->state() == Phonon::PausedState)) {
            m_media->play();
        } else if ((m_media->state() == Phonon::StoppedState) || (m_media->state() == Phonon::LoadingState)) {
            if (m_currentPlaylist->rowCount() > 0) {
                m_playlist->start();
            }
        }
    }
    m_pausePressed = false;
    if ((m_media->state() == Phonon::PausedState) || (m_media->state() == Phonon::StoppedState)) {
        ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
        ui->mediaPlayPause->setToolTip("");
    }
}

void MainWindow::on_playlistView_doubleClicked(const QModelIndex & index)
{
    int row = m_playlist->filterProxyModel()->mapToSource(index).row();
    if (!m_showQueue) {
        m_playlist->playItemAt(row, Playlist::PlaylistModel);
    } else {
        m_playlist->playItemAt(row, Playlist::QueueModel);
    }
    ui->playlistView->selectionModel()->clear();
}

void MainWindow::on_previous_clicked()
{
    if (ui->mediaViewHolder->currentIndex() == 0) {//Load media list from history
        m_mediaItemModel->clearMediaListData();
        m_mediaItemModel->setMediaListProperties(m_mediaListPropertiesHistory.last());
        m_mediaItemModel->loadMediaList(m_mediaListHistory.last(), true);
        ui->mediaView->verticalScrollBar()->setValue(m_mediaListScrollHistory.last());
        
        //Clean up history and update previous button
        m_mediaListHistory.removeLast();
        m_mediaListPropertiesHistory.removeLast();
        if (m_mediaListPropertiesHistory.count() > 0) {
            ui->previous->setVisible(true);
            ui->previous->setText(m_mediaListPropertiesHistory.last().name);
        } else {
            ui->previous->setVisible(false);
        }
        ui->playAll->setVisible(true);
        ui->playSelected->setVisible(false);
    } else {
        ui->mediaViewHolder->setCurrentIndex(0);
        if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
            ui->playSelected->setVisible(true);
            ui->playAll->setVisible(false);
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(true);
        }
        if (m_mediaListPropertiesHistory.count() > 0) {
            ui->previous->setVisible(true);
            ui->previous->setText(m_mediaListPropertiesHistory.last().name);
        } else {
            ui->previous->setVisible(false);
        }
    }
}

void MainWindow::on_playAll_clicked()
{
    m_actionsManager->action("play_all")->trigger();
}

void MainWindow::on_playSelected_clicked()
{
    m_actionsManager->setContextMenuSource(MainWindow::Default);
    m_actionsManager->action("play_selected")->trigger();
}

void MainWindow::on_mediaLists_currentChanged(int i)
{
    int selectedRow = -1;
    MediaListProperties currentProperties;
    if (i == 0) {
        if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0){
            selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
            currentProperties.name = m_audioListsModel->mediaItemAt(selectedRow).title;
            currentProperties.lri = m_audioListsModel->mediaItemAt(selectedRow).url;
            currentProperties.category = m_audioListsModel->mediaItemAt(selectedRow);
        }
        ui->audioLists->setFocus();
        ui->Filter->setClickMessage(i18n("Search for audio"));
    } else {
        if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0){
            selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            currentProperties.name = m_videoListsModel->mediaItemAt(selectedRow).title;
            currentProperties.lri = m_videoListsModel->mediaItemAt(selectedRow).url;
            currentProperties.category = m_videoListsModel->mediaItemAt(selectedRow);
        }
        ui->videoLists->setFocus();
        ui->Filter->setClickMessage(i18n("Search for video"));
    }
    if (selectedRow != -1) {
        if ((m_mediaItemModel->mediaListProperties().engine() != currentProperties.engine()) || (m_mediaItemModel->mediaListProperties().engineArg() != currentProperties.engineArg())) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
        }
    }
}

void MainWindow::on_clearPlaylist_clicked()
{
    ui->clearPlaylist->setIcon(KIcon("bangarang-clearplaylist"));
    KGuiItem clearPlaylist;
    clearPlaylist.setText(i18n("Clear Playlist"));
    if (KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to clear the current playlist?"), QString(), clearPlaylist) == KMessageBox::Continue) {
        m_playlist->clearPlaylist();
        showApplicationBanner();
        setWindowTitle(i18n("Bangarang"));
        ui->nowPlaying->setIcon(KIcon("tool-animator"));
        ui->nowPlaying->setText(i18n("Now Playing"));
    }
    ui->clearPlaylist->setIcon(Utilities::turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void MainWindow::on_shuffle_clicked()
{
    bool shuffleMode = m_playlist->shuffleMode();
    m_playlist->setShuffleMode(!shuffleMode);
}

void MainWindow::on_repeat_clicked()
{
    bool repeatMode = m_playlist->repeatMode();
    m_playlist->setRepeatMode(!repeatMode);
}

void MainWindow::on_showQueue_clicked()
{
    m_showQueue = !m_showQueue;
    if (m_showQueue) {
        m_playlist->filterProxyModel()->setSourceModel(m_playlist->queueModel());
        ui->playlistView->setModel(m_playlist->filterProxyModel());
        ui->playlistView->setDragDropMode(QAbstractItemView::InternalMove);
        ui->showQueue->setToolTip(i18n("<b>Showing Upcoming</b><br>Click to show playlist"));
        ui->playlistName->setText(i18n("<b>Playlist</b>(Upcoming)"));
        ui->showQueue->setIcon(KIcon("bangarang-preview"));
    } else {
        m_playlist->filterProxyModel()->setSourceModel(m_playlist->playlistModel());
        ui->playlistView->setModel(m_playlist->filterProxyModel());
        ui->playlistView->setDragDropMode(QAbstractItemView::DragDrop);
        ui->showQueue->setToolTip(i18n("Show Upcoming"));
        playlistChanged();
        ui->showQueue->setIcon(Utilities::turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    }
}

void MainWindow::on_showMenu_clicked()
{
    m_helpMenu = new KHelpMenu(this, m_aboutData, false);
    m_helpMenu->menu();
    m_menu = new KMenu(this);
    //m_menu->addAction(m_actionsManager->editShortcuts());
    if (m_playlist->nowPlayingModel()->rowCount() > 0) {
        MediaItem mediaItem = m_playlist->nowPlayingModel()->mediaItemAt(0);
        if ((mediaItem.type == "Audio") || (mediaItem.type == "Video")) {
            m_menu->addAction(m_actionsManager->action("show_now_playing_info"));
        }
    }
    m_menu->addAction(m_actionsManager->action("show_video_settings"));
    m_menu->addAction(m_actionsManager->action("show_audio_settings"));
    if (!isFullScreen()) {
        m_menu->addAction(m_actionsManager->action("toggle_controls"));
        m_menu->addSeparator();
    }
    m_menu->addAction(m_actionsManager->action("show_scripting_console"));
    m_menu->addAction(m_actionsManager->action("show_shortcuts_editor"));
    m_menu->addAction(m_helpMenu->action(KHelpMenu::menuAboutApp));
    QPoint menuLocation = ui->showMenu->mapToGlobal(QPoint(0,ui->showMenu->height()));
    m_menu->popup(menuLocation);
}

void MainWindow::on_showMediaViewMenu_clicked()
{
    QMenu * menu = m_actionsManager->mediaViewMenu(true);
    QPoint menuLocation = ui->showMediaViewMenu->mapToGlobal(QPoint(0,ui->showMediaViewMenu->height()));
    menu->exec(menuLocation);
}

/*----------------------------------------
  -- SLOTS for SIGNALS from Media Object --
  ----------------------------------------*/
void MainWindow::volumeChanged(qreal newVolume)
{
    //Phonon::AudioOutput::volume() only return the volume at app start.
    //Therefore I need to track volume changes independently.
    m_volume = newVolume;
}

void MainWindow::updateSeekTime(qint64 time)
{
    //Update seek time
    int totalTimeMSecs = m_media->totalTime();
    QTime currentTime(0, (time / 60000) % 60, (time / 1000) % 60);
    QTime totalTime(0, (totalTimeMSecs / 60000) % 60, (totalTimeMSecs / 1000) % 60);
    QTime remainingTime;
    remainingTime = remainingTime.addSecs(currentTime.secsTo(totalTime));
    QString displayTime;
    if (!m_showRemainingTime) {
        displayTime = currentTime.toString(QString("m:ss"));
    } else {
        displayTime = remainingTime.toString(QString("m:ss"));
    }
    ui->seekTime->setText(displayTime);
    
    //Update Now Playing Button text
    if (m_nowPlaying->rowCount() > 0) {
        if (m_nowPlaying->mediaItemAt(0).type != "Application Banner") { 
            QString title = m_nowPlaying->mediaItemAt(0).title;
            ui->nowPlaying->setText(i18n("Now Playing") + QString("(")+ displayTime + QString(")\n") + title);
        } else {
            ui->nowPlaying->setText(i18n("Now Playing"));
        }
    }
}

void MainWindow::mediaStateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    if (newstate == Phonon::PlayingState) {
        ui->mediaPlayPause->setIcon(KIcon("media-playback-pause"));
        if (m_media->hasVideo()) {
            ui->viewerStack->setCurrentIndex(1);
        } else {
            ui->viewerStack->setCurrentIndex(0);
        }
        ui->mediaPlayPause->setToolTip(i18n("<b>Playing</b><br>Click to pause<br>Click and hold to stop"));
    } else {
        if ((!m_pausePressed) && (!m_stopPressed)) {
            ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
        }
    }
    if (newstate == Phonon::LoadingState || newstate == Phonon::BufferingState) {
        showLoading();
    }
    
    if (newstate == Phonon::ErrorState) {
        if (m_media->errorString().isEmpty()) {
            ui->playbackMessage->setText(i18n("An error has been encountered during playback"));
        } else {
            ui->playbackMessage->setText(m_media->errorString());
        }
        QTimer::singleShot(3000, ui->playbackMessage, SLOT(clear()));
        
        //Use a new media object instead and discard
        //the old media object (whose state appears to be broken after errors) 
        Phonon::MediaObject *oldMediaObject = m_media;
        m_media = new Phonon::MediaObject(this);
        m_videoPath.reconnect(m_media, m_videoWidget);
        m_audioPath.reconnect(m_media, m_audioOutput);
        m_media->setTickInterval(500);
        ui->seekSlider->setMediaObject(m_media);
        connect(m_media, SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
        connect(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
        m_playlist->setMediaObject(m_media);
        delete oldMediaObject;
        oldMediaObject = 0;
        
        if (m_playlist->rowOfNowPlaying() < (m_playlist->playlistModel()->rowCount() - 1)) {
            m_playlist->playNext();
        } else {
            m_playlist->stop();
        }
        m_audioOutput->setVolume(m_volume);
        ui->volumeSlider->setAudioOutput(m_audioOutput);
        ui->volumeIcon->setChecked(false);
    }
    m_videoWidget->setContextMenu(m_actionsManager->nowPlayingContextMenu());
    Q_UNUSED(oldstate);
}

void MainWindow::showLoading()
{
    if ((m_media->state() == Phonon::LoadingState || 
         m_media->state() == Phonon::BufferingState || 
         m_playlist->state() == Playlist::Loading) && 
         (m_playlist->state() != Playlist::Finished)) {
        m_loadingProgress += 1;
        if ((m_loadingProgress > 7) || (m_loadingProgress < 0)) {
            m_loadingProgress = 0;
        }
        QString iconName= QString("bangarang-loading-%1").arg(m_loadingProgress);
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui->seekTime->setIcon(KIcon(iconName));
        if (m_playlist->state() == Playlist::Loading) {
            ui->seekTime->setToolTip(i18n("Loading playlist..."));
        } else if (m_media->state() == Phonon::BufferingState) {
            ui->seekTime->setToolTip(i18n("Buffering..."));
        } else {
            ui->seekTime->setToolTip(i18n("Loading..."));
        }
        QTimer::singleShot(100, this, SLOT(showLoading()));
    } else {
        ui->seekTime->setIcon(KIcon("bookmarks-organize"));
        if (m_playlist->nowPlayingModel()->rowCount() > 0) {
            if (m_bookmarksManager->hasBookmarks(m_playlist->nowPlayingModel()->mediaItemAt(0))) {
                ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            } else {
                ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
            }
        } else {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }
    }
}

void MainWindow::updateMuteStatus(bool muted)
{
    if (muted) {
        ui->volumeIcon->setIcon(KIcon("dialog-cancel"));
        ui->volumeIcon->setToolTip(i18n("<b>Muted</b><br>Click to restore volume"));
    } else {
        ui->volumeIcon->setIcon(KIcon("speaker"));
        ui->volumeIcon->setToolTip(i18n("Mute volume"));
    }
}


/*---------------------------
  -- SLOTS for media lists --
  ---------------------------*/
void MainWindow::mediaListChanged()
{
    ui->listTitle->setText(m_mediaItemModel->mediaListProperties().name);
    ui->listSummary->setText(m_mediaItemModel->mediaListProperties().summary);
    
    if ((m_mediaItemModel->rowCount() > 0) && (ui->mediaViewHolder->currentIndex() ==0)) {
        QString listItemType = m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
                ui->playAll->setVisible(true);
            }
        } else if (listItemType == "Category") {
            ui->playAll->setVisible(true);
        } else if ((listItemType == "Action") || (listItemType == "Message")) {
            ui->playAll->setVisible(false);
        }
    }
}

void MainWindow::mediaListPropertiesChanged()
{
    ui->listTitle->setText(m_mediaItemModel->mediaListProperties().name);
    ui->listSummary->setText(m_mediaItemModel->mediaListProperties().summary);
}

void MainWindow::mediaListCategoryActivated(QModelIndex index)
{
    addListToHistory();
    m_mediaItemModel->categoryActivated(index);
}

void MainWindow::mediaListActionActivated(QModelIndex index)
{
    addListToHistory();
    m_mediaItemModel->actionActivated(index);
}


void MainWindow::showNotification()
{
    ui->notificationText->setText(i18n("Updating..."));
    ui->notificationWidget->setVisible(true);
}

void MainWindow::delayedNotificationHide()
{
    ui->notificationText->setText(i18n("Complete"));
    QTimer::singleShot(3000, ui->notificationWidget, SLOT(hide()));
}

void MainWindow::sourceInfoUpdated(const MediaItem &mediaItem)
{
    QFontMetrics fm =  ui->notificationText->fontMetrics();
	//TODO: I tried fixing this word puzzle, I couldn't test it myself, but it should work. Please check if it really is :-) thanks
    QString notificationText = i18n("Updated info for <i>%1, %2</i>", mediaItem.title, mediaItem.subTitle);
    notificationText = fm.elidedText(notificationText, Qt::ElideMiddle, ui->notificationText->width());
    
    ui->notificationText->setText(notificationText);
    ui->notificationWidget->setVisible(true);
}

void MainWindow::sourceInfoRemoved(QString url)
{
    QFontMetrics fm =  ui->notificationText->fontMetrics();
	//TODO: I tried fixing this word puzzle, I couldn't test it myself, but it should work. Please check if it really is :-) thanks
    QString notificationText = i18n("Removed info for <i>%1</i>", url);
    notificationText = fm.elidedText(notificationText, Qt::ElideMiddle, ui->notificationText->width());
    ui->notificationText->setText(notificationText);
    ui->notificationWidget->setVisible(true);
}

void MainWindow::mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
            ui->playSelected->setVisible(true);
        }
        ui->playAll->setVisible(false);
    } else {
        if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(true);
        }
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void MainWindow::audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if ((ui->mediaLists->currentIndex() == 0) && (selected.indexes().count() > 0)) {
        MediaListProperties currentProperties;
        int selectedRow = selected.indexes().at(0).row();
        currentProperties.name = m_audioListsModel->mediaItemAt(selectedRow).title;
        currentProperties.lri = m_audioListsModel->mediaItemAt(selectedRow).url;
        currentProperties.category = m_audioListsModel->mediaItemAt(selectedRow);
        if (m_mediaItemModel->mediaListProperties().lri != currentProperties.lri) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
        }
        m_infoManager->setContext(m_audioListsModel->mediaItemAt(selectedRow));
    }
    Q_UNUSED(deselected);
}    

void MainWindow::audioListsChanged()
{
    QModelIndex index = m_audioListsModel->index(0, 0);
    if (index.isValid()) {
        ui->audioLists->selectionModel()->select( index, QItemSelectionModel::Select );
    }
}

void MainWindow::videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if ((ui->mediaLists->currentIndex() == 1) && (selected.indexes().count() > 0)) {
        MediaListProperties currentProperties;
        int selectedRow = selected.indexes().at(0).row();
        currentProperties.name = m_videoListsModel->mediaItemAt(selectedRow).title;
        currentProperties.lri = m_videoListsModel->mediaItemAt(selectedRow).url;
        currentProperties.category = m_videoListsModel->mediaItemAt(selectedRow);
        if (m_mediaItemModel->mediaListProperties().lri != currentProperties.lri) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
        }
        m_infoManager->setContext(m_videoListsModel->mediaItemAt(selectedRow));
    }
    Q_UNUSED(deselected);
}    

void MainWindow::videoListsChanged()
{
    QModelIndex index = m_videoListsModel->index(0, 0);
    if (index.isValid()) {
        ui->videoLists->selectionModel()->select( index, QItemSelectionModel::Select );
    }
}

void MainWindow::playlistChanged()
{

    if (ui->playlistView->model()->rowCount() > 0) {
        ui->playlistView->header()->setStretchLastSection(false);
        ui->playlistView->header()->setResizeMode(0, QHeaderView::Stretch);
        ui->playlistView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    }
    if (!m_showQueue) {
        ui->playlistName->setText(i18n("<b>Playlist</b>"));
        if (m_playlist->playlistModel()->rowCount() > 0) {
            QString duration = Utilities::mediaListDurationText(m_playlist->playlistModel()->mediaList());
            ui->playlistDuration->setText(i18np("1 item, %2", "%1 items, %2", m_playlist->playlistModel()->rowCount(), duration));
        } else {
            ui->playlistDuration->setText(QString());
        }
    }
}

void MainWindow::nowPlayingChanged()
{
    if (m_nowPlaying->rowCount() > 0) {
        MediaItem nowPlayingItem = m_nowPlaying->mediaItemAt(0);
        //Tidy up view and switch to the correct viewing widget
        ui->nowPlayingView->header()->setStretchLastSection(false);
        ui->nowPlayingView->header()->setResizeMode(0, QHeaderView::Stretch);
        ui->nowPlayingView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
        ui->nowPlayingView->header()->hideSection(1);
        if (nowPlayingItem.type == "Video") {
            ui->viewerStack->setCurrentIndex(1);
        } else if (nowPlayingItem.type == "Audio") {
            ui->viewerStack->setCurrentIndex(0);
        }
                
        //Update Now Playing button in Media Lists view
        if (nowPlayingItem.type != "Application Banner") {        
            ui->nowPlaying->setIcon(nowPlayingItem.artwork);  
            QString title = nowPlayingItem.title;
            QString subTitle = nowPlayingItem.subTitle;
            QString description = nowPlayingItem.fields["description"].toString();
            QString toolTipText = i18n("View Now Playing") + QString("<br><b>%1</b>").arg(title);
            if (!subTitle.isEmpty()) {
                toolTipText += QString("<br><i>%2</i>").arg(subTitle);
            }
            if (!description.isEmpty()) {
                toolTipText += QString("<br>%3").arg(description);
            }
            ui->nowPlaying->setToolTip(toolTipText);
            setWindowTitle(QString(nowPlayingItem.title + " - Bangarang"));
        }
        
        //Switch the audio output to the appropriate phonon category
        if (nowPlayingItem.type == "Audio") {
            if (m_audioOutput->category() != Phonon::MusicCategory) {
                m_audioOutput = m_audioOutputMusicCategory;
                m_audioPath.reconnect(m_media, m_audioOutput);
                m_audioOutput->setVolume(m_volume);
                ui->volumeSlider->setAudioOutput(m_audioOutput);
                ui->volumeIcon->setChecked(false);
                updateMuteStatus(false);
            }
        } else if (nowPlayingItem.type == "Video") {
            if (m_audioOutput->category() != Phonon::VideoCategory) {
                m_audioOutput = m_audioOutputVideoCategory;
                m_audioPath.reconnect(m_media, m_audioOutput);
                m_audioOutput->setVolume(m_volume);
                ui->volumeSlider->setAudioOutput(m_audioOutput);
                ui->volumeIcon->setChecked(false);
                updateMuteStatus(false);
            }
        }
    
        //Update seekTime button
        if (m_bookmarksManager->hasBookmarks(nowPlayingItem)) {
            ui->seekTime->setIcon(KIcon("bookmarks-organize"));
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        } else {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }
        
        //Update status notifier
        //- Scale artwork to current desktop icon size otherwise notifier will show unknown icon
        int iconSize = KIconLoader::global()->currentSize(KIconLoader::Desktop);
        QPixmap artworkPix = nowPlayingItem.artwork.pixmap(iconSize, iconSize);
        m_sysTray->setToolTip(QIcon(artworkPix), nowPlayingItem.title, nowPlayingItem.subTitle);
        m_sysTray->setStatus(KStatusNotifierItem::Active);
    } else {
        m_sysTray->setToolTip("bangarang", i18n("Not Playing"), QString());
        m_sysTray->setStatus(KStatusNotifierItem::Passive);
    }
}

void MainWindow::playlistFinished()
{
    showApplicationBanner();
    setWindowTitle(i18n("Bangarang"));
    ui->nowPlaying->setIcon(KIcon("tool-animator"));
    ui->nowPlaying->setText(i18n("Now Playing"));
    ui->nowPlaying->setToolTip(i18n("View Now Playing"));
    ui->seekTime->setText("0:00");
}

void MainWindow::repeatModeChanged(bool repeat)
{
    if (repeat) {
        ui->repeat->setIcon(KIcon("bangarang-repeat"));
        ui->repeat->setToolTip(i18n("<b>Repeat On</b><br>Click to turn off repeat"));
    } else {
        ui->repeat->setIcon(Utilities::turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
        ui->repeat->setToolTip(i18n("Turn on Repeat"));
    }    
}

void MainWindow::shuffleModeChanged(bool shuffle)
{
    if (shuffle) {
        ui->shuffle->setToolTip(i18n("<b>Shuffle On</b><br>Click to turn off Shuffle"));
        ui->shuffle->setIcon(KIcon("bangarang-shuffle"));
    } else {
        ui->shuffle->setToolTip(i18n("Turn on Shuffle"));
        ui->shuffle->setIcon(Utilities::turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    }
}


void MainWindow::hidePlayButtons()
{
    ui->playSelected->setVisible(false);
    ui->playAll->setVisible(false);
}

void MainWindow::updateListHeader()
{
    ui->listTitle->setText(m_mediaItemModel->mediaListProperties().name);
    ui->listSummary->setText(m_mediaItemModel->mediaListProperties().summary);
}


/*----------------------
  -- Helper functions --
  ----------------------*/
void MainWindow::addListToHistory()
{
    //Add medialList to history
    QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
    m_mediaListHistory.append(mediaList);
    m_mediaListPropertiesHistory << m_mediaItemModel->mediaListProperties();
    m_mediaListScrollHistory << ui->mediaView->verticalScrollBar()->value();
    QString previousButtonText(m_mediaItemModel->mediaListProperties().name);
    ui->previous->setText(previousButtonText);
    ui->previous->setVisible(true);
}

KIcon MainWindow::addItemsIcon()
{
    QPixmap pixmap(16,16);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    QPen pen = p.pen();
    QColor color = KColorScheme(QPalette::Active).foreground().color();
    color.setAlpha(200);
    pen.setColor(color);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(3);
    p.setPen(pen);
    p.drawLine(4,8,12,8);
    p.drawLine(8,4,8,12);
    p.end();
    return KIcon(pixmap);
}

void MainWindow::setupIcons()
{
    //Main Window Icon
    setWindowIcon(KIcon("bangarang"));
    
    //Audio List Icons
    ui->addAudioList->setIcon(KIcon("list-add"));
    ui->removeAudioList->setIcon(KIcon("list-remove"));
    ui->configureAudioList->setIcon(KIcon("configure"));
    ui->saveAudioList->setIcon(KIcon("document-save"));
    ui->aslsSave->setIcon(KIcon("document-save"));
    
    
    //Video List Icons
    ui->addVideoList->setIcon(KIcon("list-add"));
    ui->removeVideoList->setIcon(KIcon("list-remove"));
    ui->configureVideoList->setIcon(KIcon("configure"));
    ui->saveVideoList->setIcon(KIcon("document-save"));
    ui->vslsSave->setIcon(KIcon("document-save"));
    
    //Media View Icons
    ui->seekTime->setIcon(KIcon("bookmarks-organize"));
    ui->playSelected->setIcon(KIcon("media-playback-start"));
    ui->playAll->setIcon(KIcon("media-playback-start"));
    ui->nowPlaying->setIcon(KIcon("tool-animator"));
    ui->showInfo->setIcon(KIcon("help-about"));
    
    //Now Playing View bottom bar
    ui->collectionButton->setIcon(KIcon("view-media-playlist"));
    ui->fullScreen->setIcon(KIcon("view-fullscreen"));
    ui->volumeIcon->setIcon(KIcon("speaker"));
    ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
    
    //Now Playing View top bar
    ui->showPlaylist->setIcon(KIcon("mail-mark-notjunk"));
    
    //Playlist View
    ui->repeat->setIcon(Utilities::turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
    ui->shuffle->setIcon(Utilities::turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    ui->showQueue->setIcon(Utilities::turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    ui->clearPlaylist->setIcon(Utilities::turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void MainWindow::showApplicationBanner()
{
    MediaItem applicationBanner;
    applicationBanner.artwork = KIcon("bangarang");
    applicationBanner.title = i18n("Bangarang");
    applicationBanner.subTitle = i18n("Entertainment... Now");
    applicationBanner.type = "Application Banner";
    applicationBanner.url = "-";
    m_nowPlaying->loadMediaItem(applicationBanner, true);
    ui->viewerStack->setCurrentIndex(0);
}

void MainWindow::updateCachedDevicesList()
{
    m_devicesAdded.clear();
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString())) {
        const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc> ();
        if (disc->availableContent() & Solid::OpticalDisc::Audio) {
            m_devicesAdded << QString("CD:%1").arg(device.udi());
        }
        if (disc->availableContent() & Solid::OpticalDisc::VideoDvd) {
            m_devicesAdded << QString("DVD:%1").arg(device.udi());
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (isFullScreen()
			&& ui->stackedWidget->currentIndex() == 1
			&& event->type() == QEvent::MouseMove) {

		QMouseEvent * mouseEvent = (QMouseEvent *)event;
		QWidget* widget = (QWidget* )obj;

		if (mouseEvent->y() <= ui->nowPlayingToolbar->height() ||
				widget->height() - mouseEvent->y() <= ui->widgetSet->height()) {

			//Show the widgets in the Now Playing view
			ui->widgetSet->setVisible(true);
			ui->nowPlayingToolbar->setVisible(true);
		} else {
			//Hide the widgets in the Now Playing view
			ui->widgetSet->setVisible(false);
			ui->nowPlayingToolbar->setVisible(false);
		}
	}

    // standard event processing
    return QObject::eventFilter(obj, event);
}

/*-------------------------
-- Device Notifier Slots --
---------------------------*/
void MainWindow::deviceAdded(const QString &udi)
{
    //Check type of device that was added
    //and reload media lists if necessary
    Solid::Device deviceAdded(udi);
    if (deviceAdded.isDeviceInterface(Solid::DeviceInterface::OpticalDisc)) {
        const Solid::OpticalDisc *disc = deviceAdded.as<const Solid::OpticalDisc> ();
        if (disc->availableContent() & Solid::OpticalDisc::Audio) {
            m_audioListsModel->clearMediaListData();
            m_audioListsModel->load();
            updateCachedDevicesList();
        } else if (disc->availableContent() & Solid::OpticalDisc::VideoDvd) {
            m_videoListsModel->clearMediaListData();
            m_videoListsModel->load();
            updateCachedDevicesList();
        }
    }
}

void MainWindow::deviceRemoved(const QString &udi)
{
    //Check type of device that was removed
    //and reload media lists if necessary
    if (m_devicesAdded.indexOf(QString("CD:%1").arg(udi)) != -1) {
        m_audioListsModel->clearMediaListData();
        m_audioListsModel->load();
        updateCachedDevicesList();
    }
    if (m_devicesAdded.indexOf(QString("DVD:%1").arg(udi)) != -1) {
        m_videoListsModel->clearMediaListData();
        m_videoListsModel->load();
        updateCachedDevicesList();
    }
}

void MainWindow::updateCustomColors()
{
    //Update custom colors in Now Playing View
    QColor highlightColor = QApplication::palette().color(QPalette::Highlight);
    int r = highlightColor.red();
    int g = highlightColor.green();
    int b = highlightColor.blue();
    QString styleSheet = QString("background-color: qlineargradient(spread:reflect, x1:0.494, y1:0, x2:0.505682, y2:1, stop:0 rgba(0, 0, 0, 0), stop:0.20 rgba(%1, %2, %3, 25), stop:0.5 rgba(%1, %2, %3, 55), stop:0.75 rgba(%1, %2, %3, 30), stop:1 rgba(0, 0, 0, 0)); color: rgb(255, 255, 255);").arg(r).arg(g).arg(b);
    ui->nowPlayingView->setStyleSheet(styleSheet);

    //Update custom colors in Media Lists View
    QPalette viewPalette = ui->mediaViewHolder->palette();
    viewPalette.setColor(QPalette::Window, viewPalette.color(QPalette::Base));
    ui->mediaListHolder->setPalette(viewPalette);
    ui->semanticsHolder->setPalette(viewPalette);
}

// TODO Please see if we can make it a setup point 
// Current accelration is MouseWheel Delta*100 equals the skipped milliseconds
void MainWindow::skipForward(int i)
{
  //kDebug() << "Scrolls" << i;
  
  if (m_media->isSeekable())
    m_media->seek(m_media->currentTime() + qint64(i)*100);
  
}

void MainWindow::skipBackward(int i)
{
  //kDebug() << "Scrolls" << i;
  if (m_media->isSeekable())
    m_media->seek(m_media->currentTime() + qint64(i)*100);
}

ActionsManager * MainWindow::actionsManager()
{
    return m_actionsManager;
}

SavedListsManager * MainWindow::savedListsManager()
{
    return m_savedListsManager;
}

void MainWindow::setAboutData(KAboutData *aboutData)
{
    m_aboutData = aboutData;
}

KAboutData * MainWindow::aboutData()
{
    return m_aboutData;
}

Playlist * MainWindow::playlist()
{
    return m_playlist;
}

Phonon::AudioOutput * MainWindow::audioOutput()
{
    return m_audioOutput;
}

InfoManager * MainWindow::infoManager()
{
    return m_infoManager;
}

BookmarksManager * MainWindow::bookmarksManager()
{
    return m_bookmarksManager;
}

Phonon::VideoWidget * MainWindow::videoWidget()
{
  return m_videoWidget;
}

ScriptConsole *MainWindow::scriptConsole()
{
  return m_scriptConsole;
}

bool MainWindow::showingRemainingTime()
{
    return m_showRemainingTime;
}

void MainWindow::setShowRemainingTime(bool showRemainingTime)
{
    m_showRemainingTime = showRemainingTime;
    if (m_showRemainingTime) {
        ui->seekTime->setToolTip(i18n("<b>Time remaining</b><br>Click to show elapsed time and bookmarks"));
    } else {
        ui->seekTime->setToolTip(i18n("<b>Time elapsed</b><br>Click to show remaining time and bookmarks"));
    }
}
