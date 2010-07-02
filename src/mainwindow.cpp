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
#include "bangarangapplication.h"
#include "bangarangnotifieritem.h"
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
#include "medialistsettings.h"

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
#include <KNotification>
#include <KStatusNotifierItem>

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
    m_application = (BangarangApplication *)KApplication::kApplication();
    
    ui->setupUi(this);
    setGeometry(0,0,760,520);

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
    m_videoWidget =  new BangarangVideoWidget(ui->videoFrame);
    m_audioOutputMusicCategory = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    m_audioOutputVideoCategory = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    m_audioOutput = m_audioOutputMusicCategory; // default to music category;
    m_videoPath = Phonon::createPath(m_application->mediaObject(), m_videoWidget);
    m_audioPath = Phonon::createPath(m_application->mediaObject(), m_audioOutput);
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
    ui->seekSlider->setMediaObject(m_application->mediaObject());
    ui->seekSlider->setIconVisible(false);
    setShowRemainingTime(false);
    ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    
    //Connect to media object signals and slots
    connect(m_application->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_audioOutputMusicCategory, SLOT(setMuted(bool)));
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_audioOutputVideoCategory, SLOT(setMuted(bool)));
    connect(m_audioOutputMusicCategory, SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
    connect(m_audioOutputMusicCategory, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
    connect(m_audioOutputVideoCategory, SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
    connect(m_audioOutputVideoCategory, SIGNAL(volumeChanged(qreal)), this, SLOT(volumeChanged(qreal)));
    connect(m_application->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
    
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
    ui->mediaListFilter->setVisible(false);
    ui->mediaView->setModel(m_application->browsingModel());
    connect(m_application->browsingModel(), SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_application->browsingModel(), SIGNAL(mediaListPropertiesChanged()), this, SLOT(mediaListPropertiesChanged()));
    connect(m_application->browsingModel(), SIGNAL(loading()), this, SLOT(hidePlayButtons()));
    connect(m_application->browsingModel(), SIGNAL(propertiesChanged()), this, SLOT(updateListHeader()));
    
    connect((MediaItemDelegate *)ui->mediaView->itemDelegate(), SIGNAL(categoryActivated(QModelIndex)), this, SLOT(mediaListCategoryActivated(QModelIndex)));
    connect((MediaItemDelegate *)ui->mediaView->itemDelegate(), SIGNAL(actionActivated(QModelIndex)), this, SLOT(mediaListActionActivated(QModelIndex)));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    
    //Set up Browsing Model notifications
    ui->notificationWidget->setVisible(false);
    connect(m_application->browsingModel(), SIGNAL(sourceInfoUpdateRemovalStarted()), this, SLOT(showNotification()));
    connect(m_application->browsingModel(), SIGNAL(sourceInfoUpdateProgress(int)), ui->notificationProgress, SLOT(setValue(int)));
    connect(m_application->browsingModel(), SIGNAL(sourceInfoRemovalProgress(int)), ui->notificationProgress, SLOT(setValue(int)));
    connect(m_application->browsingModel(), SIGNAL(sourceInfoUpdateRemovalComplete()), this, SLOT(delayedNotificationHide()));
    connect(m_application->browsingModel(), SIGNAL(sourceInfoUpdated(MediaItem)), this, SLOT(sourceInfoUpdated(MediaItem)));
    connect(m_application->browsingModel(), SIGNAL(sourceInfoRemoved(QString)), this, SLOT(sourceInfoRemoved(QString)));
    
    //Set up playlist
    connect(m_application->playlist(), SIGNAL(playlistFinished()), this, SLOT(playlistFinished()));
    connect(m_application->playlist(), SIGNAL(loading()), this, SLOT(showLoading()));
    connect(m_application->playlist(), SIGNAL(shuffleModeChanged(bool)), this, SLOT(shuffleModeChanged(bool)));
    connect(m_application->playlist(), SIGNAL(repeatModeChanged(bool)), this, SLOT(repeatModeChanged(bool)));
    
    
    //Set up playlist view
    ui->playlistView->setMainWindow(this);
    ui->playlistFilterProxyLine->lineEdit()->setClickMessage(i18n("Search in playlist..."));
    ui->playlistFilterProxyLine->setProxy(m_application->playlist()->filterProxyModel());
    ui->playlistFilter->setVisible(false);
    playWhenPlaylistChanges = false;

    //Setup Now Playing view
    ui->nowPlayingView->setMainWindow( this );
    updateCustomColors();
    connect( (MediaItemModel *) ui->nowPlayingView->model(), SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateCustomColors())); 
    
    //Setup Video Settings
    m_videoSettings = new VideoSettings(m_videoWidget, this);
    ui->videoSettingsPage->layout()->addWidget(m_videoSettings);
    
    //Setup Media List Settings
    m_mediaListSettings =  new MediaListSettings(this);
    
    //Set up defaults
    ui->nowPlayingSplitter->setCollapsible(0,true);
    ui->nowPlayingSplitter->setCollapsible(1,false);
    ui->stackedWidget->setCurrentIndex(1);
    ui->mediaViewHolder->setCurrentIndex(0);
    ui->audioListsStack->setCurrentIndex(0);
    ui->videoListsStack->setCurrentIndex(0);
    ui->contextStack->setCurrentIndex(0);
    ui->mediaPlayPause->setHoldDelay(1000);
    ui->listSummary->setFont(KGlobalSettings::smallestReadableFont());
    ui->playlistDuration->setFont(KGlobalSettings::smallestReadableFont());
    ui->playbackMessage->clear();
    ui->collectionButton->setFocus();
    updateSeekTime(0);
    showApplicationBanner();
    updateCachedDevicesList();
    m_pausePressed = false;
    m_stopPressed = false;
    m_loadingProgress = 0;
    
    //Set default media list selection
    ui->mediaLists->setCurrentIndex(0);
    
    //Install event filter for hiding widgets in Now Playing view
    ui->nowPlayingView->setMouseTracking(true);
    m_videoWidget->setMouseTracking(true);
    ui->nowPlayingView->viewport()->installEventFilter(this);
    m_videoWidget->installEventFilter(this);

    // Set up cursor hiding and context menu for videos.
    m_videoWidget->setFocusPolicy(Qt::ClickFocus);
    KCursor::setAutoHideCursor(m_videoWidget, true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::completeSetup()
{
    setupActions();
    m_application->audioSettings()->setAudioPath(&m_audioPath);
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
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(searchProperties);
            m_application->browsingModel()->load();
        } else if (ui->mediaLists->currentIndex() == 1) {
            MediaListProperties searchProperties;
            searchProperties.name = "Video Search";
            searchProperties.lri = QString("video://search?%1").arg(ui->Filter->text());
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(searchProperties);
            m_application->browsingModel()->load();
        }
        m_mediaListHistory.clear();
        m_mediaListPropertiesHistory.clear();
        ui->previous->setVisible(false);
        ui->mediaViewHolder->setCurrentIndex(0);
    }
}

void MainWindow::on_configureAudioList_clicked()
{
    if ((ui->mediaLists->currentIndex() == 0) && (ui->audioLists->selectionModel()->selectedIndexes().count() > 0)) {
        int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_audioListsModel->mediaItemAt(selectedRow);
        if (selectedItem.url.startsWith("savedlists://")) {
            m_application->savedListsManager()->showAudioSavedListSettings();
        } else if (selectedItem.url.startsWith("semantics://recent") ||
            selectedItem.url.startsWith("semantics://frequent") ||
            selectedItem.url.startsWith("semantics://highest")) {
            m_mediaListSettings->showMediaListSettings();
        }
    }
}

void MainWindow::on_configureVideoList_clicked()
{
    if ((ui->mediaLists->currentIndex() == 1) && (ui->videoLists->selectionModel()->selectedIndexes().count() > 0)) {
        int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_videoListsModel->mediaItemAt(selectedRow);
        if (selectedItem.url.startsWith("savedlists://")) {
            m_application->savedListsManager()->showVideoSavedListSettings();
        } else if (selectedItem.url.startsWith("semantics://recent") ||
            selectedItem.url.startsWith("semantics://frequent") ||
            selectedItem.url.startsWith("semantics://highest")) {
            m_mediaListSettings->showMediaListSettings();
        }
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
        m_application->actionsManager()->m_playlistFilterVisible) {
        m_application->actionsManager()->action("toggle_playlist_filter")->trigger();
    }
    ui->contextStack->setCurrentIndex(0);  
    m_application->actionsManager()->action("show_video_settings")->setText(i18n("Show Video Settings"));
    m_application->actionsManager()->action("show_audio_settings")->setText(i18n("Show Audio Settings"));
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
        if (m_application->actionsManager()->m_controlsVisible) {
          ui->widgetSet->setVisible(true);
          ui->nowPlayingToolbar->setVisible(true);
        }
    }
}

void MainWindow::on_seekTime_clicked()
{
    QPoint menuLocation = ui->seekTime->mapToGlobal(QPoint(0,ui->showMenu->height()));
    m_application->actionsManager()->bookmarksMenu()->popup(menuLocation);
}

void MainWindow::on_mediaPlayPause_pressed()
{
    if ((m_application->mediaObject()->state() == Phonon::PlayingState)) {
        m_application->mediaObject()->pause();
        m_pausePressed = true;
        ui->mediaPlayPause->setToolTip(i18n("<b>Paused</b><br>Hold to stop"));
    }
}

void MainWindow::on_mediaPlayPause_held()
{
    if ((m_application->mediaObject()->state() != Phonon::LoadingState) && (m_application->mediaObject()->state() != Phonon::StoppedState)) {
        if (m_pausePressed) {
            m_pausePressed = false;
        }
        m_stopPressed = true;
        ui->mediaPlayPause->setIcon(KIcon("media-playback-stop"));
        m_application->playlist()->stop();
    }
}

void MainWindow::on_mediaPlayPause_released()
{
    if (m_stopPressed) {
        m_stopPressed = false;
    } else {
        if ((!m_pausePressed) && (m_application->mediaObject()->state() == Phonon::PausedState)) {
            m_application->mediaObject()->play();
        } else if ((m_application->mediaObject()->state() == Phonon::StoppedState) || (m_application->mediaObject()->state() == Phonon::LoadingState)) {
            if (ui->playlistView->model()->rowCount() > 0) {
                m_application->playlist()->start();
            }
        }
    }
    m_pausePressed = false;
    if ((m_application->mediaObject()->state() == Phonon::PausedState) || (m_application->mediaObject()->state() == Phonon::StoppedState)) {
        ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
        ui->mediaPlayPause->setToolTip("");
    }
}

void MainWindow::on_playlistView_doubleClicked(const QModelIndex & index)
{
    int row = m_application->playlist()->filterProxyModel()->mapToSource(index).row();
    m_application->playlist()->playItemAt(row, ui->playlistView->currentModelType());
    ui->playlistView->selectionModel()->clear();
}

void MainWindow::on_previous_clicked()
{
    if (ui->mediaViewHolder->currentIndex() == 0) {//Load media list from history
        m_application->browsingModel()->clearMediaListData();
        m_application->browsingModel()->setMediaListProperties(m_mediaListPropertiesHistory.last());
        m_application->browsingModel()->loadMediaList(m_mediaListHistory.last(), true);
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
    m_application->actionsManager()->action("play_all")->trigger();
}

void MainWindow::on_playSelected_clicked()
{
    m_application->actionsManager()->setContextMenuSource(MainWindow::Default);
    m_application->actionsManager()->action("play_selected")->trigger();
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
        if ((m_application->browsingModel()->mediaListProperties().engine() != currentProperties.engine()) || (m_application->browsingModel()->mediaListProperties().engineArg() != currentProperties.engineArg())) {
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(currentProperties);
            m_application->browsingModel()->load();
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
        m_application->playlist()->clearPlaylist();
        showApplicationBanner();
        setWindowTitle(i18n("Bangarang"));
        ui->nowPlaying->setIcon(KIcon("tool-animator"));
        ui->nowPlaying->setText(i18n("Now Playing"));
    }
    ui->clearPlaylist->setIcon(Utilities::turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void MainWindow::on_shuffle_clicked()
{
    bool shuffleMode = m_application->playlist()->shuffleMode();
    m_application->playlist()->setShuffleMode(!shuffleMode);
}

void MainWindow::on_repeat_clicked()
{
    bool repeatMode = m_application->playlist()->repeatMode();
    m_application->playlist()->setRepeatMode(!repeatMode);
}

void MainWindow::on_showQueue_clicked()
{
    Playlist::Model type = ui->playlistView->toggleModel();
    if (type == Playlist::QueueModel) {
        ui->showQueue->setToolTip(i18n("<b>Showing Upcoming</b><br>Click to show playlist"));
        ui->showQueue->setIcon(KIcon("bangarang-preview"));
    } else {
        ui->showQueue->setToolTip(i18n("Show Upcoming"));
        ui->showQueue->setIcon(Utilities::turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    }
}

void MainWindow::on_showMenu_clicked()
{
    m_helpMenu = new KHelpMenu(this, m_application->aboutData(), false);
    m_helpMenu->menu();
    m_menu = new KMenu(this);
    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        MediaItem mediaItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
        if ((mediaItem.type == "Audio") || (mediaItem.type == "Video")) {
            m_menu->addAction(m_application->actionsManager()->action("show_now_playing_info"));
        }
    }
    m_menu->addAction(m_application->actionsManager()->action("show_video_settings"));
    m_menu->addAction(m_application->actionsManager()->action("show_audio_settings"));
    if (!isFullScreen()) {
        m_menu->addAction(m_application->actionsManager()->action("toggle_controls"));
        m_menu->addSeparator();
    }
    m_menu->addAction(m_application->actionsManager()->action("show_shortcuts_editor"));
    m_menu->addAction(m_application->actionsManager()->action(KStandardAction::name(KStandardAction::ConfigureNotifications)));
    m_menu->addAction(m_helpMenu->action(KHelpMenu::menuAboutApp));
    QPoint menuLocation = ui->showMenu->mapToGlobal(QPoint(0,ui->showMenu->height()));
    m_menu->popup(menuLocation);
}

void MainWindow::on_showMediaViewMenu_clicked()
{
    QMenu * menu = m_application->actionsManager()->mediaViewMenu(true);
    QPoint menuLocation = ui->showMediaViewMenu->mapToGlobal(QPoint(0,ui->showMediaViewMenu->height()));
    menu->exec(menuLocation);
}

void MainWindow::on_closePlaylistFilter_clicked()
{
  m_application->actionsManager()->action("toggle_playlist_filter")->trigger();
}
/*----------------------------------------
  -- SLOTS for SIGNALS from Media Object --
  ----------------------------------------*/
void MainWindow::volumeChanged(qreal newVolume)
{
    //Phonon::AudioOutput::volume() only return the volume at app start.
    //Therefore I need to track volume changes independently.
    m_volume = newVolume;
    
    //NOTE: This will change the mute state to true in the app if volume reaches 0 level
    //(never adjusted volume to 0 and surprised about not hearing anything?)  
    updateMuteStatus(m_volume == 0);
}

void MainWindow::updateSeekTime(qint64 time)
{
    //Update seek time
    int totalTimeMSecs = m_application->mediaObject()->totalTime();
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
    
    m_application->statusNotifierItem()->updateAppIcon(time, m_application->mediaObject()->totalTime());
    
    //Update Now Playing Button text
    MediaItemModel * nowPlayingModel = m_application->playlist()->nowPlayingModel();
    if (nowPlayingModel->rowCount() > 0) {
        if (nowPlayingModel->mediaItemAt(0).type != "Application Banner") {
            QString title = nowPlayingModel->mediaItemAt(0).title;
            ui->nowPlaying->setText(i18n("Now Playing") + QString("(")+ displayTime + QString(")\n") + title);
        } else {
            ui->nowPlaying->setText(i18n("Now Playing"));
        }
    }
}

void MainWindow::mediaStateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    if (newstate == Phonon::PlayingState) {
       m_application->statusNotifierItem()->setState(newstate);
       
        ui->mediaPlayPause->setIcon(KIcon("media-playback-pause"));
        if (m_application->mediaObject()->hasVideo()) {
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
        if (m_application->mediaObject()->errorString().isEmpty()) {
            ui->playbackMessage->setText(i18n("An error has been encountered during playback"));
        } else {
            ui->playbackMessage->setText(m_application->mediaObject()->errorString());
        }
        QTimer::singleShot(3000, ui->playbackMessage, SLOT(clear()));
        
        //Use a new media object instead and discard
        //the old media object (whose state appears to be broken after errors) 
        Phonon::MediaObject * mediaObject = m_application->newMediaObject();
        m_videoPath.reconnect(mediaObject, m_videoWidget);
        m_audioPath.reconnect(mediaObject, m_audioOutput);
        ui->seekSlider->setMediaObject(mediaObject);
        connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
        connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
        
        if (m_application->playlist()->rowOfNowPlaying() < (m_application->playlist()->playlistModel()->rowCount() - 1)) {
            m_application->playlist()->playNext();
        } else {
            m_application->playlist()->stop();
        }
        m_audioOutput->setVolume(m_volume);
        ui->volumeSlider->setAudioOutput(m_audioOutput);
        ui->volumeIcon->setChecked(false);
    }
    if (newstate == Phonon::PausedState)
      m_application->statusNotifierItem()->setState(newstate);
    else if (newstate == Phonon::StoppedState)
      m_application->statusNotifierItem()->setState(newstate);
    
    m_videoWidget->setContextMenu(m_application->actionsManager()->nowPlayingContextMenu());
    Q_UNUSED(oldstate);
}

void MainWindow::showLoading()
{
    if ((m_application->mediaObject()->state() == Phonon::LoadingState || 
         m_application->mediaObject()->state() == Phonon::BufferingState || 
         m_application->playlist()->state() == Playlist::Loading) && 
         (m_application->playlist()->state() != Playlist::Finished)) {
        m_loadingProgress += 1;
        if ((m_loadingProgress > 7) || (m_loadingProgress < 0)) {
            m_loadingProgress = 0;
        }
        QString iconName= QString("bangarang-loading-%1").arg(m_loadingProgress);
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui->seekTime->setIcon(KIcon(iconName));
        if (m_application->playlist()->state() == Playlist::Loading) {
            ui->seekTime->setToolTip(i18n("Loading playlist..."));
        } else if (m_application->mediaObject()->state() == Phonon::BufferingState) {
            ui->seekTime->setToolTip(i18n("Buffering..."));
        } else {
            ui->seekTime->setToolTip(i18n("Loading..."));
        }
        QTimer::singleShot(100, this, SLOT(showLoading()));
    } else {
        ui->seekTime->setIcon(KIcon("bookmarks-organize"));
        if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
            if (m_application->bookmarksManager()->hasBookmarks(m_application->playlist()->nowPlayingModel()->mediaItemAt(0))) {
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
    m_application->statusNotifierItem()->setVolumeMuted(muted);
}


/*---------------------------
  -- SLOTS for media lists --
  ---------------------------*/
void MainWindow::mediaListChanged()
{
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
    
    if ((m_application->browsingModel()->rowCount() > 0) && (ui->mediaViewHolder->currentIndex() ==0)) {
        QString listItemType = m_application->browsingModel()->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            if (!m_application->browsingModel()->mediaItemAt(0).fields["isTemplate"].toBool()) {
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
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
}

void MainWindow::mediaListCategoryActivated(QModelIndex index)
{
    addListToHistory();
    m_application->browsingModel()->categoryActivated(index);
}

void MainWindow::mediaListActionActivated(QModelIndex index)
{
    addListToHistory();
    m_application->browsingModel()->actionActivated(index);
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
    QString notificationText = i18n("Updated info for <i>%1, %2</i>", mediaItem.title, mediaItem.subTitle);
    notificationText = fm.elidedText(notificationText, Qt::ElideMiddle, ui->notificationText->width());
    
    ui->notificationText->setText(notificationText);
    ui->notificationWidget->setVisible(true);
}

void MainWindow::sourceInfoRemoved(QString url)
{
    QFontMetrics fm =  ui->notificationText->fontMetrics();
    QString notificationText = i18n("Removed info for <i>%1</i>", url);
    notificationText = fm.elidedText(notificationText, Qt::ElideMiddle, ui->notificationText->width());
    ui->notificationText->setText(notificationText);
    ui->notificationWidget->setVisible(true);
}

void MainWindow::mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        int firstRow = selected.indexes().at(0).row();
        if (!m_application->browsingModel()->mediaItemAt(firstRow).fields["isTemplate"].toBool()) {
            ui->playSelected->setVisible(true);
        }
        ui->playAll->setVisible(false);
    } else {
        if (!m_application->browsingModel()->mediaItemAt(0).fields["isTemplate"].toBool()) {
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
        //Load selected media list
        MediaListProperties currentProperties;
        int selectedRow = selected.indexes().at(0).row();
        MediaItem selectedItem = m_audioListsModel->mediaItemAt(selectedRow);
        currentProperties.name = selectedItem.title;
        currentProperties.lri = selectedItem.url;
        currentProperties.category = selectedItem;
        if (m_application->browsingModel()->mediaListProperties().lri != currentProperties.lri) {
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(currentProperties);
            m_application->browsingModel()->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
        }
        
        //Update InfoManager Context
        m_application->infoManager()->setContext(selectedItem);
        
        //Determine if selected list is configurable
        if (selectedItem.url.startsWith("savedlists://") ||
            selectedItem.url.startsWith("semantics://recent?audio") ||
            selectedItem.url.startsWith("semantics://frequent?audio") ||
            selectedItem.url.startsWith("semantics://highest?audio")) {
            ui->configureAudioList->setVisible(true);
        } else {
            ui->configureAudioList->setVisible(false);
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
        //Load selected media list
        MediaListProperties currentProperties;
        int selectedRow = selected.indexes().at(0).row();
        MediaItem selectedItem = m_videoListsModel->mediaItemAt(selectedRow);
        currentProperties.name = selectedItem.title;
        currentProperties.lri = selectedItem.url;
        currentProperties.category = selectedItem;
        if (m_application->browsingModel()->mediaListProperties().lri != currentProperties.lri) {
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(currentProperties);
            m_application->browsingModel()->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
        }
        
        //Set InfoManager context
        m_application->infoManager()->setContext(selectedItem);

        //Determine if selected list is configurable
        if (selectedItem.url.startsWith("savedlists://") ||
            selectedItem.url.startsWith("semantics://recent?") ||
            selectedItem.url.startsWith("semantics://frequent?") ||
            selectedItem.url.startsWith("semantics://highest?")) {
            ui->configureVideoList->setVisible(true);
        } else {
            ui->configureVideoList->setVisible(false);
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
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
}


/*----------------------
  -- Helper functions --
  ----------------------*/
void MainWindow::addListToHistory()
{
    //Add medialList to history
    QList<MediaItem> mediaList = m_application->browsingModel()->mediaList();
    m_mediaListHistory.append(mediaList);
    m_mediaListPropertiesHistory << m_application->browsingModel()->mediaListProperties();
    m_mediaListScrollHistory << ui->mediaView->verticalScrollBar()->value();
    QString previousButtonText(m_application->browsingModel()->mediaListProperties().name);
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
    ui->semAConfigSave->setIcon(KIcon("document-save"));
    
    //Video List Icons
    ui->addVideoList->setIcon(KIcon("list-add"));
    ui->removeVideoList->setIcon(KIcon("list-remove"));
    ui->configureVideoList->setIcon(KIcon("configure"));
    ui->saveVideoList->setIcon(KIcon("document-save"));
    ui->vslsSave->setIcon(KIcon("document-save"));
    ui->semVConfigSave->setIcon(KIcon("document-save"));
    
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
    ui->closePlaylistFilter->setIcon(KIcon("dialog-close"));
    
    //Audio settings
    ui->restoreDefaultAudioSettings->setIcon(KIcon("edit-undo"));
}

void MainWindow::setupActions()
{
    m_videoSettings->setHideAction(m_application->actionsManager()->action("show_video_settings"));
    ui->mediaPrevious->setDefaultAction(m_application->actionsManager()->action("play_previous"));
    ui->mediaNext->setDefaultAction(m_application->actionsManager()->action("play_next"));
    m_videoWidget->setContextMenu(m_application->actionsManager()->nowPlayingContextMenu());
}

void MainWindow::showApplicationBanner()
{
    MediaItem applicationBanner;
    applicationBanner.artwork = KIcon("bangarang");
    applicationBanner.title = i18n("Bangarang");
    applicationBanner.subTitle = i18n("Entertainment... Now");
    applicationBanner.type = "Application Banner";
    applicationBanner.url = "-";
    m_application->playlist()->nowPlayingModel()->loadMediaItem(applicationBanner, true);
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

void MainWindow::skipForward(int i)
{
  if (m_application->mediaObject()->isSeekable())
    m_application->mediaObject()->seek(m_application->mediaObject()->currentTime() + qint64(i)*100);
  
}

void MainWindow::skipBackward(int i)
{
  if (m_application->mediaObject()->isSeekable())
    m_application->mediaObject()->seek(m_application->mediaObject()->currentTime() + qint64(i)*100);
}

Phonon::AudioOutput * MainWindow::audioOutput()
{
    return m_audioOutput;
}

Phonon::VideoWidget * MainWindow::videoWidget()
{
  return m_videoWidget;
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

void MainWindow::nowPlayingChanged()
{
    MediaItemModel * nowPlayingModel = m_application->playlist()->nowPlayingModel();
    if (nowPlayingModel->rowCount() == 0) {
        m_application->statusNotifierItem()->setToolTip("bangarang", i18n("Not Playing"), QString());
        m_application->statusNotifierItem()->setStatus(KStatusNotifierItem::Passive);
        return;
    }
    
    MediaItem nowPlayingItem = nowPlayingModel->mediaItemAt(0);
    QString type = nowPlayingItem.type;
    //Tidy up view and switch to the correct viewing widget
    ui->nowPlayingView->tidyHeader();
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
    bool changed = false;
    if (type == "Audio" && m_audioOutput->category() != Phonon::MusicCategory) {
        m_audioOutput = m_audioOutputMusicCategory;
        changed = true;
    } else if (type == "Video" && m_audioOutput->category() != Phonon::VideoCategory) {
        m_audioOutput = m_audioOutputVideoCategory;
        changed = true;
    }
    if (changed) {
        m_audioPath.reconnect(m_application->mediaObject(), m_audioOutput);
        m_audioOutput->setVolume(m_volume);
        ui->volumeSlider->setAudioOutput(m_audioOutput);
        ui->volumeIcon->setChecked(false);
        updateMuteStatus(false);   
    }
 
    //Update seekTime button
    if (m_application->bookmarksManager()->hasBookmarks(nowPlayingItem)) {
        ui->seekTime->setIcon(KIcon("bookmarks-organize"));
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    } else {
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    
    if (nowPlayingItem.title != i18n("Bangarang")) // dont show the bangarang media item
    {
      KNotification* notification = new KNotification("trackChange", this);
      notification->setTitle(i18n("Now playing"));
      notification->setPixmap(nowPlayingItem.artwork.pixmap(80, 80));
      notification->setText("<strong>" + nowPlayingItem.title + "</strong>\n" +
	nowPlayingItem.fields["album"].toString() + "\n" + nowPlayingItem.fields["artist"].toString());
      notification->sendEvent();
    }
    
    //Update status notifier
    //- Scale artwork to current desktop icon size otherwise notifier will show unknown icon
    int iconSize = KIconLoader::global()->currentSize(KIconLoader::Desktop);
    QPixmap artworkPix = nowPlayingItem.artwork.pixmap(iconSize, iconSize);
    m_application->statusNotifierItem()->setToolTip(QIcon(artworkPix), nowPlayingItem.title, nowPlayingItem.subTitle);
    m_application->statusNotifierItem()->setStatus(KStatusNotifierItem::Active);
}
