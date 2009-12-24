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
#include "infomanager.h"
#include "savedlistsmanager.h"
#include "actionsmanager.h"
#include "mediaitemdelegate.h"
#include "nowplayingdelegate.h"
#include "videosettings.h"

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    qRegisterMetaType<MediaItem>("MediaItem");
    
    ui->setupUi(this);
    
    //Setup interface icons
    setupIcons();
    setupActions();

    // Hide certain widgets
    ui->previous->setVisible(false);
    ui->contextStack->setVisible(false);
    ui->playSelected->setVisible(false);
    ui->showInfo->setVisible(false);
    ui->infoSep1->setVisible(false);
    ui->infoSep2->setVisible(false);
    ui->saveInfo->setVisible(false);
    ui->sortList->setVisible(false);
    ui->configureAudioList->setVisible(false);
    ui->configureVideoList->setVisible(false);
    
    //Initialize Nepomuk
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
    
    //Set up device notifier
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString & )), this, SLOT(deviceAdded(const QString & )));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString & )), this, SLOT(deviceRemoved(const QString & )));
    
    //Set up media object
    m_media = new Phonon::MediaObject(this);
    m_videoWidget =  new Phonon::VideoWidget(ui->videoFrame);
    m_audioOutput = new Phonon::AudioOutput(this);
    Phonon::createPath(m_media, m_videoWidget);
    Phonon::createPath(m_media, m_audioOutput);
    m_media->setTickInterval(500);
    
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
    showRemainingTime = false;
    
    //Connect to media object signals and slots
    connect(m_media, SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_audioOutput, SLOT(setMuted(bool)));
    connect(m_audioOutput, SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
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
    m_mediaItemModel = new MediaItemModel(this);
    m_sharedMediaListCache = m_mediaItemModel->mediaListCache();
    m_itemDelegate = new MediaItemDelegate(this);
    ui->mediaView->setModel(m_mediaItemModel);
    ui->mediaView->setItemDelegate(m_itemDelegate);
    m_itemDelegate->setView(ui->mediaView);
    ui->mediaView->header()->setVisible(false);
    connect(m_itemDelegate, SIGNAL(categoryActivated(QModelIndex)), m_mediaItemModel, SLOT(categoryActivated(QModelIndex)));
    connect(m_itemDelegate, SIGNAL(actionActivated(QModelIndex)), m_mediaItemModel, SLOT(actionActivated(QModelIndex)));
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_mediaItemModel, SIGNAL(loading()), this, SLOT(hidePlayButtons()));
    connect(m_mediaItemModel, SIGNAL(propertiesChanged()), this, SLOT(updateListHeader()));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    ui->mediaView->setMainWindow(this);
    
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
    
    //Set up playlist view
    m_currentPlaylist = m_playlist->playlistModel();
    m_currentPlaylist->setMediaListCache(m_sharedMediaListCache);
    m_playlistItemDelegate = new MediaItemDelegate(this);
    ui->playlistView->setModel(m_currentPlaylist);
    ui->playlistView->setItemDelegate(m_playlistItemDelegate);
    m_playlistItemDelegate->setView(ui->playlistView);
    playWhenPlaylistChanges = false;
    connect(m_currentPlaylist, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    
    //Setup Now Playing view
    m_nowPlaying = m_playlist->nowPlayingModel();
    m_nowPlayingDelegate = new NowPlayingDelegate(this);
    ui->nowPlayingView->setModel(m_nowPlaying);
    ui->nowPlayingView->setItemDelegate(m_nowPlayingDelegate);
    connect(m_nowPlaying, SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));
    ui->nowPlayingView->header()->setVisible(false);
    ui->nowPlayingView->header()->hideSection(1);
    updateNowPlayingStyleSheet();
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateNowPlayingStyleSheet())); 
    
    //Setup Info View
    m_infoManager = new InfoManager(this);

    //Setup Saved Lists Manager
    m_savedListsManager = new SavedListsManager(this);
    
    //Setup Actions Manager
    m_actionsManager = new ActionsManager(this);
    
    //Setup Video Settings
    VideoSettings *videoSettings = new VideoSettings(m_videoWidget, this);
    videoSettings->setHideAction(m_actionsManager->showVideoSettings());
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
    ui->mediaPrevious->setDefaultAction(m_actionsManager->playPrevious());
    ui->mediaNext->setDefaultAction(m_actionsManager->playNext());
    ui->listSummary->setFont(KGlobalSettings::smallestReadableFont());
    ui->playlistDuration->setFont(KGlobalSettings::smallestReadableFont());
    ui->playbackMessage->clear();
    updateSeekTime(0);
    showApplicationBanner();
    updateCachedDevicesList();
    m_showQueue = false;
    m_repeat = false;
    m_shuffle = false;
    m_pausePressed = false;
    m_stopPressed = false;
    m_loadingProgress = 0;
    
    //Get command line args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() > 0) {
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
            KUrl cmdLineKUrl = args->url(0);
            if (!cmdLineKUrl.isLocalFile()) {
                QString tmpFile;
                if( KIO::NetAccess::download(cmdLineKUrl, tmpFile, this)) {
                    //KMessageBox::information(this,tmpFile);
                    cmdLineKUrl = KUrl(tmpFile);
                } else {
                    cmdLineKUrl = KUrl();
                }
            }
            MediaItem mediaItem = Utilities::mediaItemFromUrl(cmdLineKUrl);
            QList<MediaItem> mediaList;
            mediaList << mediaItem;
            m_playlist->playMediaList(mediaList);
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

    // Set up cursor hiding for videos.
    m_videoWidget->setFocusPolicy(Qt::ClickFocus);
    KCursor::setAutoHideCursor(m_videoWidget, true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_mediaItemModel;
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
        ui->saveInfo->setVisible(false);
        ui->showInfo->setVisible(false);
        ui->infoSep1->setVisible(false);
        ui->infoSep2->setVisible(false);
    }
}

void MainWindow::on_nowPlaying_clicked()
{
    ui->stackedWidget->setCurrentIndex(1); // Show Now Playing page
}

void MainWindow::on_collectionButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0); // Show Collection page
}

void MainWindow::on_showPlaylist_clicked(bool checked)
{
    ui->contextStack->setVisible(checked);
    ui->contextStack->setCurrentIndex(0);  
    m_actionsManager->showVideoSettings()->setText(i18n("Show Video Settings"));
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
        ui->widgetSet->setVisible(true);
        ui->nowPlayingToolbar->setVisible(true);
    }
}

void MainWindow::on_seekTime_clicked()
{
    showRemainingTime = !showRemainingTime;
    if (showRemainingTime) {
        ui->seekTime->setToolTip(i18n("<b>Time remaining</b><br>Click to show elapsed time"));
    } else {
        ui->seekTime->setToolTip(i18n("<b>Time elapsed</b><br>Click to show remaining time"));
    }
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
    if (!m_showQueue) {
        m_playlist->playItemAt(index.row(), Playlist::PlaylistModel);
    } else {
        m_playlist->playItemAt(index.row(), Playlist::QueueModel);
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
        ui->saveInfo->setVisible(false);
        if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
            ui->playSelected->setVisible(true);
            ui->playAll->setVisible(false);
            ui->showInfo->setVisible(true);
            ui->infoSep1->setVisible(true);
            ui->infoSep2->setVisible(true);
            
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(true);
            ui->showInfo->setVisible(false);
            ui->infoSep1->setVisible(false);
            ui->infoSep2->setVisible(false);
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
    //Get all media items
    QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
    m_currentPlaylist->setMediaListProperties(m_mediaItemModel->mediaListProperties());
    
    m_playlist->playMediaList(mediaList);

    // Show Now Playing page
    ui->stackedWidget->setCurrentIndex(1);   
}

void MainWindow::on_playSelected_clicked()
{
    //Get selected media items
    QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
    m_mediaList.clear();
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        m_mediaList.append(mediaList.at(selectedRows.at(i).row()));
    }
    m_currentPlaylist->setMediaListProperties(m_mediaItemModel->mediaListProperties());
    
    m_playlist->playMediaList(m_mediaList);

    // Show Now Playing page
    ui->stackedWidget->setCurrentIndex(1);   
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
        }
        ui->audioLists->setFocus();
        ui->Filter->setClickMessage(i18n("Search for audio"));
    } else {
        if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0){
            selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            currentProperties.name = m_videoListsModel->mediaItemAt(selectedRow).title;
            currentProperties.lri = m_videoListsModel->mediaItemAt(selectedRow).url;
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
            ui->saveInfo->setVisible(false);
            ui->showInfo->setVisible(false);
            ui->infoSep1->setVisible(false);
            ui->infoSep2->setVisible(false);
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
    ui->clearPlaylist->setIcon(turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void MainWindow::on_shuffle_clicked()
{
    m_shuffle = !m_shuffle;
    if (m_shuffle) {
        m_playlist->setMode(Playlist::Shuffle);
        ui->shuffle->setToolTip(i18n("<b>Shuffle On</b><br>Click to turn off Shuffle"));
        ui->shuffle->setIcon(KIcon("bangarang-shuffle"));
    } else {
        m_playlist->setMode(Playlist::Normal);
        ui->shuffle->setToolTip(i18n("Turn on Shuffle"));
        ui->shuffle->setIcon(turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    }
}

void MainWindow::on_repeat_clicked()
{
    m_repeat = !m_repeat;
    m_playlist->setRepeat(m_repeat);
    if (m_repeat) {
        ui->repeat->setIcon(KIcon("bangarang-repeat"));
        ui->repeat->setToolTip(i18n("<b>Repeat On</b><br>Click to turn off repeat"));
    } else {
        ui->repeat->setIcon(turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
        ui->repeat->setToolTip(i18n("Turn on Repeat"));
    }    
}

void MainWindow::on_showQueue_clicked()
{
    m_showQueue = !m_showQueue;
    if (m_showQueue) {
        ui->playlistView->setModel(m_playlist->queueModel());
        ui->showQueue->setToolTip(i18n("<b>Showing Upcoming</b><br>Click to show playlist"));
        ui->playlistName->setText(i18n("<b>Playlist</b>(Upcoming)"));
        ui->showQueue->setIcon(KIcon("bangarang-preview"));
    } else {
        ui->playlistView->setModel(m_playlist->playlistModel());
        ui->showQueue->setToolTip(i18n("Show Upcoming"));
        playlistChanged();
        ui->showQueue->setIcon(turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
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
            m_menu->addAction(m_actionsManager->showNowPlayingInfo());
        }
    }
    m_menu->addAction(m_actionsManager->showVideoSettings());
    if (!isFullScreen()) {
        m_menu->addAction(m_actionsManager->showHideControls());
        m_menu->addSeparator();
    }
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
void MainWindow::updateSeekTime(qint64 time)
{
    //Update seek time
    int totalTimeMSecs = m_media->totalTime();
    QTime currentTime(0, (time / 60000) % 60, (time / 1000) % 60);
    QTime totalTime(0, (totalTimeMSecs / 60000) % 60, (totalTimeMSecs / 1000) % 60);
    QTime remainingTime;
    remainingTime = remainingTime.addSecs(currentTime.secsTo(totalTime));
    QString displayTime;
    if (!showRemainingTime) {
        displayTime = currentTime.toString(QString("m:ss"));
    } else {
        displayTime = remainingTime.toString(QString("m:ss"));
    }
    ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ui->seekTime->setText(displayTime);
    if (showRemainingTime) {
        ui->seekTime->setToolTip(i18n("<b>Time remaining</b><br>Click to show elapsed time"));
    } else {
        ui->seekTime->setToolTip(i18n("<b>Time elapsed</b><br>Click to show remaining time"));
    }
    
    
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
        ui->playbackMessage->setText(i18n("An error has been encountered during playback"));
        QTimer::singleShot(3000, ui->playbackMessage, SLOT(clear()));
    }
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
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
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
    
    ui->mediaView->header()->setStretchLastSection(false);
    ui->mediaView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->mediaView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    
    if ((m_mediaItemModel->rowCount() > 0) && (ui->mediaViewHolder->currentIndex() ==0)) {
        QString listItemType = m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
                ui->mediaView->header()->showSection(1);
                ui->playAll->setVisible(true);
            }
        } else if (listItemType == "Category") {
            ui->mediaView->header()->showSection(1);
            ui->playAll->setVisible(true);
        } else if ((listItemType == "Action") || (listItemType == "Message")) {
            ui->mediaView->header()->hideSection(1);
            ui->playAll->setVisible(false);
        }
        ui->saveInfo->setVisible(false);
    }
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

void MainWindow::sourceInfoUpdated(MediaItem mediaItem)
{
    ui->notificationText->setText(i18n("Updated info for ") + QString("<i>%1, %2</i>").arg(mediaItem.title).arg(mediaItem.subTitle));
    ui->notificationWidget->setVisible(true);
}

void MainWindow::sourceInfoRemoved(QString url)
{
    ui->notificationText->setText(i18n("Removed info for ") + QString("<i>%1</i>").arg(url));
    ui->notificationWidget->setVisible(true);
}

void MainWindow::mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
            ui->playSelected->setVisible(true);
        }
        ui->playAll->setVisible(false);
        QString listItemType = m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            if (!m_mediaItemModel->mediaItemAt(0).url.startsWith("DVDTRACK") && !m_mediaItemModel->mediaItemAt(0).url.startsWith("CDTRACK")) {
                ui->showInfo->setVisible(true);
                ui->infoSep1->setVisible(true);
                ui->infoSep2->setVisible(true);
            }
        }
    } else {
        if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(true);
        }
        ui->showInfo->setVisible(false);
        ui->infoSep1->setVisible(false);
        ui->infoSep2->setVisible(false);
    }
    ui->saveInfo->setVisible(false);
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
        if (m_mediaItemModel->mediaListProperties().lri != currentProperties.lri) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
            ui->saveInfo->setVisible(false);
            ui->showInfo->setVisible(false);
            ui->infoSep1->setVisible(false);
            ui->infoSep2->setVisible(false);
        }
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
        if (m_mediaItemModel->mediaListProperties().lri != currentProperties.lri) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
            ui->saveInfo->setVisible(false);
            ui->showInfo->setVisible(false);            
            ui->infoSep1->setVisible(false);
            ui->infoSep2->setVisible(false);
        }
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

    ui->playlistView->header()->setStretchLastSection(false);
    ui->playlistView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->playlistView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
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
        if (m_nowPlaying->mediaItemAt(0).type != "Application Banner") {        
            ui->nowPlaying->setIcon(m_nowPlaying->mediaItemAt(0).artwork);  
            QString title = m_nowPlaying->mediaItemAt(0).title;
            QString subTitle = m_nowPlaying->mediaItemAt(0).subTitle;
            QString description = m_nowPlaying->mediaItemAt(0).fields["description"].toString();
            QString toolTipText = i18n("View Now Playing") + QString("<br><b>%1</b>").arg(title);
            if (!subTitle.isEmpty()) {
                toolTipText += QString("<br><i>%2</i>").arg(subTitle);
            }
            if (!description.isEmpty()) {
                toolTipText += QString("<br>%3").arg(description);
            }
            ui->nowPlaying->setToolTip(toolTipText);
            setWindowTitle(QString(m_nowPlaying->mediaItemAt(0).title + " - Bangarang"));
        }
    
        ui->nowPlayingView->header()->setStretchLastSection(false);
        ui->nowPlayingView->header()->setResizeMode(0, QHeaderView::Stretch);
        ui->nowPlayingView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
        ui->nowPlayingView->header()->hideSection(1);
        
        if (m_nowPlaying->mediaItemAt(0).type == "Video") {
            ui->viewerStack->setCurrentIndex(1);
        } else if (m_nowPlaying->mediaItemAt(0).type == "Audio") {
            ui->viewerStack->setCurrentIndex(0);
        }
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


/*------------------------
  --- Action functions ---
  ------------------------*/

void MainWindow::playAll()
{
    //Get all media items
    QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
    m_currentPlaylist->setMediaListProperties(m_mediaItemModel->mediaListProperties());
    
    m_playlist->playMediaList(mediaList);
    
    // Show Now Playing page
    ui->stackedWidget->setCurrentIndex(1);   
}

void MainWindow::playSelected()
{
    //Get selected media items
    QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
    m_mediaList.clear();
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        m_mediaList.append(mediaList.at(selectedRows.at(i).row()));
    }
    m_currentPlaylist->setMediaListProperties(m_mediaItemModel->mediaListProperties());
    
    m_playlist->playMediaList(m_mediaList);
    
    // Show Now Playing page
    ui->stackedWidget->setCurrentIndex(1);   
}

void MainWindow::addSelectedToPlaylist()
{
    for (int i = 0; i < ui->mediaView->selectionModel()->selectedIndexes().count(); ++i) {
        QModelIndex index = ui->mediaView->selectionModel()->selectedIndexes().at(i);
        
        if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
            int playlistRow = m_currentPlaylist->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
            if (playlistRow == -1) {
                MediaItemModel * model = (MediaItemModel *)index.model();
                m_playlist->addMediaItem(model->mediaItemAt(index.row()));
            }
        }
    }
}

void MainWindow::removeSelectedFromPlaylist()
{
    for (int i = 0; i < ui->mediaView->selectionModel()->selectedIndexes().count(); ++i) {
        QModelIndex index = ui->mediaView->selectionModel()->selectedIndexes().at(i);
        
        if ((index.data(MediaItem::TypeRole).toString() == "Audio") ||(index.data(MediaItem::TypeRole).toString() == "Video") || (index.data(MediaItem::TypeRole).toString() == "Image")) {
            int playlistRow = m_currentPlaylist->rowOfUrl(index.data(MediaItem::UrlRole).value<QString>());
            if (playlistRow != -1) {
                m_playlist->removeMediaItemAt(playlistRow);
            }
        }
    }
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
    ui->sortList->setIcon(KIcon("view-sort-ascending"));
    ui->playSelected->setIcon(KIcon("media-playback-start"));
    ui->playAll->setIcon(KIcon("media-playback-start"));
    ui->nowPlaying->setIcon(KIcon("tool-animator"));
    ui->saveInfo->setIcon(KIcon("document-save"));
    ui->showInfo->setIcon(KIcon("help-about"));
    ui->editInfo->setIcon(KIcon("document-edit"));
    
    //Now Playing View bottom bar
    ui->collectionButton->setIcon(KIcon("view-media-playlist"));
    ui->fullScreen->setIcon(KIcon("view-fullscreen"));
    ui->volumeIcon->setIcon(KIcon("speaker"));
    ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
    
    //Now Playing View top bar
    ui->showPlaylist->setIcon(KIcon("mail-mark-notjunk"));
    
    //Playlist View
    ui->repeat->setIcon(turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
    ui->shuffle->setIcon(turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    ui->showQueue->setIcon(turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    ui->clearPlaylist->setIcon(turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void MainWindow::setupActions()
{
    playAllAction = new QAction(KIcon("media-playback-start"), i18n("Play all"), this);
    connect(playAllAction, SIGNAL(triggered()), this, SLOT(playAll()));
    playSelectedAction = new QAction(KIcon("media-playback-start"), i18n("Play selected"), this);
    connect(playSelectedAction, SIGNAL(triggered()), this, SLOT(playSelected()));
    
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

KIcon MainWindow::turnIconOff(KIcon icon, QSize size)
{
    QImage image = KIcon(icon).pixmap(size).toImage();
    KIconEffect::toGray(image, 0.8);
    return KIcon(QPixmap::fromImage(image));
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

void MainWindow::mouseDoubleClickEvent (QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton){
    if(isFullScreen())
      on_fullScreen_toggled(false);
    else
      on_fullScreen_toggled(true);
  }
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

void MainWindow::updateNowPlayingStyleSheet()
{
    QColor highlightColor = QApplication::palette().color(QPalette::Highlight);
    int r = highlightColor.red();
    int g = highlightColor.green();
    int b = highlightColor.blue();
    QString styleSheet = QString("background-color: qlineargradient(spread:reflect, x1:0.494, y1:0, x2:0.505682, y2:1, stop:0 rgba(0, 0, 0, 0), stop:0.20 rgba(%1, %2, %3, 25), stop:0.5 rgba(%1, %2, %3, 55), stop:0.75 rgba(%1, %2, %3, 30), stop:1 rgba(0, 0, 0, 0)); color: rgb(255, 255, 255);").arg(r).arg(g).arg(b);
    ui->nowPlayingView->setStyleSheet(styleSheet);
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

Phonon::VideoWidget * MainWindow::videoWidget()
{
  return m_videoWidget;
}
