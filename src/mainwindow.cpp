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
#include "platform/utilities/utilities.h"
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
#include "medialistsettings.h"

#include <KAction>
#include <KAcceleratorManager>
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
#include <QPropertyAnimation>
#include <kross/core/action.h> 
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    
    ui->setupUi(this);

    //Set up menu hiding timer
    m_menuTimer = new QTimer(this);
    m_menuTimer->setInterval(3000);
    m_menuTimer->setSingleShot(true);
    connect(m_menuTimer, SIGNAL(timeout()), ui->floatingMenuHolder, SLOT(hide()));

    //Setup interface icons
    setupIcons();

    // Hide certain widgets
    ui->previous->setVisible(false);
    ui->contextStackHolder->setVisible(false);
    ui->playSelected->setVisible(false);
    ui->configureAudioList->setVisible(false);
    ui->configureVideoList->setVisible(false);
    ui->semanticsHolder->setVisible(false);
    ui->loadingIndicator->setVisible(false);
    ui->extSubtitle->setVisible(false);
    ui->playbackMessage->setVisible(false);

    //Initialize Nepomuk
    m_nepomukInited = Utilities::nepomukInited();
    if (!m_nepomukInited) {
        ui->Filter->setVisible(false);
    }
    
    //Set up device notifier
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString & )), this, SLOT(deviceAdded(const QString & )));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString & )), this, SLOT(deviceRemoved(const QString & )));

    //Set up media object
    m_videoWidget =  new BangarangVideoWidget(ui->videoFrame);
    connect(m_videoWidget,SIGNAL(skipForward(int)),this, SLOT(skipForward(int)));
    connect(m_videoWidget,SIGNAL(skipBackward(int)),this, SLOT(skipBackward(int)));
    connect(m_videoWidget,SIGNAL(fullscreenChanged(bool)),this,SLOT(on_fullScreen_toggled(bool)));
    
    //Add video widget to video frame
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_videoWidget);
    layout->setContentsMargins(0,0,0,0);
    ui->videoFrame->setLayout(layout);
    ui->videoFrame->setFrameShape(QFrame::NoFrame);

    //Set up volume and seek slider
    ui->volumeSlider->setMuteVisible( false );
    ui->seekSlider->setIconVisible(false);
    setShowRemainingTime(false);
    ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    KAcceleratorManager::setNoAccel(ui->seekTime);
    
    //Connect to media object signals and slots
    connect(m_application->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
    connect(m_application->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
    connectPhononWidgets();
    
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
    ui->mediaListFilterProxyLine->lineEdit()->setClickMessage(QString());
    ui->mediaListFilterProxyLine->setProxy(ui->mediaView->filterProxyModel());
    ui->mediaListFilter->setVisible(false);
    ui->mediaView->setSourceModel(m_application->browsingModel());
    connect(m_application->browsingModel(), SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_application->browsingModel(), SIGNAL(mediaListPropertiesChanged()), this, SLOT(mediaListPropertiesChanged()));
    connect(m_application->browsingModel(), SIGNAL(loading()), this, SLOT(mediaListLoading()));
    connect(m_application->browsingModel(), SIGNAL(loadingStateChanged(bool)), this, SLOT(mediaListLoadingStateChanged(bool)));
    connect(m_application->browsingModel(), SIGNAL(propertiesChanged()), this, SLOT(updateListHeader()));
    
    connect((MediaItemDelegate *)ui->mediaView->itemDelegate(), SIGNAL(categoryActivated(QModelIndex)), this, SLOT(mediaListCategoryActivated(QModelIndex)));
    connect((MediaItemDelegate *)ui->mediaView->itemDelegate(), SIGNAL(actionActivated(QModelIndex)), this, SLOT(mediaListActionActivated(QModelIndex)));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    
    //Set up Browsing Model status notifications
    ui->notificationWidget->setVisible(false);
    connect(m_application->browsingModel(), SIGNAL(statusUpdated()), this, SLOT(browsingModelStatusUpdated()));
    
    //Set up playlist
    connect(m_application->playlist(), SIGNAL(playlistFinished()), this, SLOT(playlistFinished()));
    connect(m_application->playlist(), SIGNAL(loading()), this, SLOT(playlistLoading()));
    connect(m_application->playlist(), SIGNAL(shuffleModeChanged(bool)), this, SLOT(shuffleModeChanged(bool)));
    connect(m_application->playlist(), SIGNAL(repeatModeChanged(bool)), this, SLOT(repeatModeChanged(bool)));
    
    
    //Set up playlist view
    ui->playlistView->setMainWindow(this);
    ui->playlistFilterProxyLine->lineEdit()->setClickMessage(QString());
    ui->playlistFilterProxyLine->setProxy(m_application->playlist()->filterProxyModel());
    ui->playlistFilter->setVisible(false);
    ui->playlistNotification->setVisible(false);
    ui->playlistNotificationNo->setText(i18n("No"));
    ui->playlistNotificationYes->setText(i18n("Yes"));
    playWhenPlaylistChanges = false;

    //Setup Now Playing view
    ui->nowPlayingView->setMainWindow( this );
    updateCustomColors();
    connect( (MediaItemModel *) ui->nowPlayingView->model(), SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateCustomColors()));
    m_videoSize = Normal;
    ui->videoFrame->setVisible(false);
    ui->nowPlayingView->move(0,0);
    ui->nowPlayingView->resize(ui->nowPlayingHolder->size());
    ui->videoFrame->move(0,0);
    ui->videoFrame->resize(ui->nowPlayingHolder->size());
    KAcceleratorManager::setNoAccel(ui->showPlaylist);
    KAcceleratorManager::setNoAccel(ui->showPlaylist_2);

    //Setup Media List Settings
    m_mediaListSettings =  new MediaListSettings(this);
    
    //Set up defaults
    ui->nowPlayingSplitter->setCollapsible(0,true);
    ui->nowPlayingSplitter->setCollapsible(1,false);
    ui->stackedWidget->setCurrentIndex((int) MainNowPlaying);
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
    ui->playlistView->setupActions();
}

/*---------------------
 -- UI widget slots  --
 ----------------------*/
void MainWindow::on_nowPlayingHolder_resized()
{
    ui->nowPlayingView->move(0,0);
    ui->nowPlayingView->resize(ui->nowPlayingHolder->size());

    QFont extSubtitleFont = ui->extSubtitle->font();
    extSubtitleFont.setPixelSize(qMax(int(ui->nowPlayingHolder->height()*0.045), KGlobalSettings::smallestReadableFont().pixelSize()));
    ui->extSubtitle->setFont(extSubtitleFont);
    ui->extSubtitle->setMaximumWidth(0.8*ui->nowPlayingHolder->width());
    ui->extSubtitle->move((ui->nowPlayingHolder->width() - ui->extSubtitle->width())/2,
                          ui->nowPlayingHolder->geometry().bottom() - 20 - ui->extSubtitle->height());

    if (m_videoSize == Normal) {
        ui->videoFrame->setGeometry(QRect(QPoint(0, 0), ui->nowPlayingHolder->size()));
    } else {
        int width = qMax(200, ui->nowPlayingHolder->width()/3);
        int height = qMax(150, width*3/4);
        int left = ui->nowPlayingHolder->width() - width - 20;
        int top = ui->nowPlayingHolder->height() - height - 20;
        ui->videoFrame->setGeometry(left, top, width, height);
    }

    if (ui->contextStackHolder->isVisible()) {
        ui->floatingMenuHolder->setVisible(false);
        m_menuTimer->stop();
    } else {
        QFontMetrics fm(ui->showPlaylist_2->font());
        QRect textRect = fm.boundingRect(i18n("Playlist"));
        int width = textRect.width() + 36;
        ui->showPlaylist_2->setGeometry(QRect(ui->showPlaylist_2->rect().topLeft(),
                                              QSize(width,
                                                    qMax(24, textRect.height() + 8))));
        int left = ui->nowPlayingHolder->width() - (width + 24);
        ui->floatingMenuHolder->setGeometry(QRect(QPoint(left, 0),
                                                  QSize(width + 24,
                                                        qMax(24, textRect.height() + 8))));
        ui->floatingMenuHolder->setVisible(true);
        ui->floatingMenuHolder->raise();
        m_menuTimer->start();
    }
}

void MainWindow::on_Filter_returnPressed()
{
    ui->mediaView->setFocus();
    ui->audioLists->selectionModel()->clearSelection();
    if (!ui->Filter->text().isEmpty()) {
        addListToHistory();
        if (ui->mediaLists->currentIndex() == 0) {
            MediaListProperties searchProperties;
            searchProperties.name = i18n("Audio Search");
            searchProperties.lri = QString("music://search?%1").arg(ui->Filter->text());
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(searchProperties);
            m_application->browsingModel()->load();
        } else if (ui->mediaLists->currentIndex() == 1) {
            MediaListProperties searchProperties;
            searchProperties.name = i18n("Video Search");
            searchProperties.lri = QString("video://search?%1").arg(ui->Filter->text());
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(searchProperties);
            m_application->browsingModel()->load();
        }
        ui->mediaViewHolder->setCurrentIndex(0);
    }
}

void MainWindow::on_configureAudioList_clicked()
{
    if ((ui->mediaLists->currentIndex() == 0) && (ui->audioLists->selectionModel()->selectedIndexes().count() > 0)) {
        int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_audioListsModel->mediaItemAt(selectedRow);
        if (selectedItem.fields["isConfigurable"].toBool()) {
            if (selectedItem.url.startsWith("semantics://recent") ||
                selectedItem.url.startsWith("semantics://frequent") ||
                selectedItem.url.startsWith("semantics://highest")) {
                m_mediaListSettings->showMediaListSettings();
            } else {
                m_application->savedListsManager()->showAudioSavedListSettings();
            }
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
    switchMainWidget(MainNowPlaying); // Show Now Playing page
}

void MainWindow::on_collectionButton_clicked()
{
    switchMainWidget(MainMediaList); // Show Collection page
    ui->collectionButton->setFocus();
}

void MainWindow::on_showPlaylist_clicked()
{
    if (ui->contextStackHolder->isVisible() && ui->contextStack->currentIndex() == 0) {
        ui->contextStackHolder->setVisible(false);
    } else {
        ui->contextStack->setCurrentIndex(0);
        ui->contextStackHolder->setVisible(true);
        QFrame *filter = currentFilterFrame();
        KFilterProxySearchLine *line = currentFilterProxyLine();
        if (filter->isVisible() && line->lineEdit()->text().isEmpty()) {
            m_application->actionsManager()->action("toggle_filter")->trigger();
        }
    }
    m_application->actionsManager()->action("show_video_settings")->setText(i18n("Show video settings"));
    m_application->actionsManager()->action("show_audio_settings")->setText(i18n("Show audio settings"));
}

void MainWindow::on_showPlaylist_2_clicked()
{
    on_showPlaylist_clicked();
}

void MainWindow::on_fullScreen_toggled(bool fullScreen)
{
    if (fullScreen) {
        showFullScreen();
        ui->fullScreen->setIcon(KIcon("view-restore"));
        ui->fullScreen->setToolTip(i18n("<b>Fullscreen</b><br>Click to exit fullscreen"));
        ui->fullScreen->setChecked(true);
        ui->widgetSet->setVisible(false);
    } else {
        showNormal();
        ui->fullScreen->setIcon(KIcon("view-fullscreen"));
        ui->fullScreen->setToolTip(i18n("Show fullscreen"));
        ui->fullScreen->setChecked(false);
        if (m_application->actionsManager()->m_controlsVisible) {
          ui->widgetSet->setVisible(true);
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
        QApplication::processEvents(); // Allows view to get updated before scrollbar is set
        ui->mediaView->verticalScrollBar()->setValue(m_mediaListScrollHistory.last());
        
        //Clean up history and update previous button
        m_mediaListHistory.removeLast();
        m_mediaListPropertiesHistory.removeLast();
        m_mediaListScrollHistory.removeLast();
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
        ui->audioListsStack->setCurrentIndex(0);
        if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0){
            selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
            loadMediaList(m_audioListsModel, selectedRow);
        }
        ui->audioLists->setFocus();
        ui->Filter->setClickMessage(i18n("Search for audio"));
        ui->Filter->clear();
    } else {
        ui->videoListsStack->setCurrentIndex(0);
        if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0){
            selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            loadMediaList(m_videoListsModel, selectedRow);
        }
        ui->videoLists->setFocus();
        ui->Filter->setClickMessage(i18n("Search for video"));
        ui->Filter->clear();
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
    m_menuTimer->stop();
    KMenu * menu = m_application->actionsManager()->nowPlayingMenu();

    QPoint menuLocation;
    if (ui->contextStackHolder->isVisible()) {
        menuLocation = ui->showMenu->mapToGlobal(QPoint(0,ui->showMenu->height()));
    } else {
        menuLocation = ui->showMenu_2->mapToGlobal(QPoint(0,ui->showMenu->height()));
    }
    menu->popup(menuLocation);
}

void MainWindow::on_showMenu_2_clicked()
{
    on_showMenu_clicked();
}

void MainWindow::on_showMediaViewMenu_clicked()
{
    QMenu * menu = m_application->actionsManager()->mediaViewMenu(true);
    QPoint menuLocation = ui->showMediaViewMenu->mapToGlobal(QPoint(0,ui->showMediaViewMenu->height()));
    menu->exec(menuLocation);
}

void MainWindow::on_closePlaylistFilter_clicked()
{
  m_application->actionsManager()->action("toggle_filter")->trigger();
}

void MainWindow::on_closeMediaListFilter_clicked()
{
  m_application->actionsManager()->action("toggle_filter")->trigger();
}

/*----------------------------------------
  -- SLOTS for SIGNALS from Media Object --
  ----------------------------------------*/
void MainWindow::updateSeekTime(qint64 time)
{
    //Update seek time
    int totalTimeMSecs = m_application->mediaObject()->totalTime();
    QTime currentTime(time/(60*60000), (time / 60000) % 60, (time / 1000) % 60);
    QTime totalTime(totalTimeMSecs/(60*60000), (totalTimeMSecs / 60000) % 60, (totalTimeMSecs / 1000) % 60);
    QString displayTime;
    if (!m_showRemainingTime) {
        displayTime = Utilities::durationString(time/1000);
    } else {
        displayTime = Utilities::durationString(currentTime.secsTo(totalTime));
    }
    ui->seekTime->setText(displayTime);
    
    //Update Now Playing Button text
    MediaItemModel * nowPlayingModel = m_application->playlist()->nowPlayingModel();
    if (nowPlayingModel->rowCount() > 0) {
        if (nowPlayingModel->mediaItemAt(0).type != "Application Banner") {
            QString title = nowPlayingModel->mediaItemAt(0).title;
            ui->nowPlaying->setText(i18n("Now Playing") + QString(" (")+ displayTime + QString(")\n") + title);
        } else {
            ui->nowPlaying->setText(i18n("Now Playing"));
        }
    }

    //Add currently playing item to browsing model if contents is "Recently Played"
    if (time > 12000 && time < 13000) {
        if (m_application->playlist()->nowPlayingModel()->rowCount() == 0) {
            return;
        }
        MediaItem nowPlayingItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
        MediaListProperties mediaListProperties = m_application->browsingModel()->mediaListProperties();
        if (!mediaListProperties.lri.startsWith(QString("semantics://recent?%1").arg(nowPlayingItem.type.toLower()))) {
            return;
        }
        QStringList filterList = mediaListProperties.engineFilterList();
        int filterIndex = -1;
        for (int i = 0; i < filterList.count(); i++) {
            if (filterList.at(i).startsWith("lastPlayed")) {
                filterIndex = i;
                break;
            }
        }
        if (filterIndex >= 0) {
            if (mediaListProperties.filterOperator(filterList.at(filterIndex)) != ">") {
                return;
            }
        }
        nowPlayingItem.artwork = Utilities::defaultArtworkForMediaItem(nowPlayingItem);
        nowPlayingItem.semanticComment = Utilities::wordsForTimeSince(nowPlayingItem.fields["lastPlayed"].toDateTime());
        m_application->browsingModel()->insertMediaItemAt(0, nowPlayingItem);
    }
}

void MainWindow::mediaStateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    if (newstate == Phonon::PlayingState) {
       m_application->statusNotifierItem()->setState(newstate);

        ui->mediaPlayPause->setIcon(KIcon("media-playback-pause"));
        bool nowPlayingItemIsVideo = false;
        if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
            MediaItem nowPlayingItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
            nowPlayingItemIsVideo = (nowPlayingItem.type == "Video");
        }
        if (nowPlayingItemIsVideo) {
            ui->videoFrame->setVisible(true);
            if (videoSize() == Mini) {
                ui->nowPlayingView->showInfo();
            } else {
                ui->videoFrame->setGeometry(QRect(QPoint(0, 0), ui->nowPlayingHolder->size()));
            }
        } else {
            ui->videoFrame->setVisible(false);
        }
        ui->mediaPlayPause->setToolTip(i18n("<b>Playing</b><br>Click to pause<br>Click and hold to stop"));
    } else {
        if ((!m_pausePressed) && (!m_stopPressed)) {
            ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
        }
    }
    showLoading();
    
    if (newstate == Phonon::ErrorState) {
        if (m_application->mediaObject()->errorString().isEmpty()) {
            ui->playbackMessage->setText(i18n("An error has been encountered during playback"));
        } else {
            ui->playbackMessage->setText(m_application->mediaObject()->errorString());
        }
        QFontMetrics fm(ui->playbackMessage->font());
        QSize textSize = fm.boundingRect(QRect(0, 0, ui->extSubtitle->maximumWidth(), fm.lineSpacing()),
                                         Qt::AlignCenter | Qt::TextWordWrap,
                                         ui->playbackMessage->text()).size();

        int top = ui->nowPlayingHolder->geometry().bottom() - 50 - textSize.height();
        int left = (ui->nowPlayingHolder->width() - textSize.width()) / 2;
        ui->playbackMessage->setGeometry(left - 8, top - 8, textSize.width() + 8, textSize.height() + 8);
        ui->playbackMessage->setVisible(true);
        ui->playbackMessage->raise();
        QTimer::singleShot(6000, ui->playbackMessage, SLOT(hide()));
        
        //Use a new media object instead and discard
        //the old media object (whose state appears to be broken after errors) 
        Phonon::MediaObject * mediaObject = m_application->newMediaObject();
        ui->seekSlider->setMediaObject(mediaObject);
        connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
        connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
        
        if (m_application->playlist()->rowOfNowPlaying() < (m_application->playlist()->playlistModel()->rowCount() - 1)) {
            m_application->playlist()->playNext();
        } else {
            m_application->playlist()->stop();
        }
        ui->volumeSlider->setAudioOutput(m_application->audioOutput());
        ui->volumeIcon->setChecked(false);
    }

    if (newstate == Phonon::PausedState)
      m_application->statusNotifierItem()->setState(newstate);
    else if (newstate == Phonon::StoppedState)
      m_application->statusNotifierItem()->setState(newstate);
    
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
            if (m_application->bookmarksManager()->hasBookmarks(m_application->playlist()->nowPlayingModel()->mediaItemAt(0)) ||
                m_application->playlist()->mediaController()->availableChapters() > 1
            ){
                ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            } else {
                ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
            }
        } else {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }
        setShowRemainingTime(m_showRemainingTime); //Make sure tooltip is updated
    }
}

void MainWindow::updateMuteStatus(bool muted)
{
    if (muted) {
        ui->volumeIcon->setIcon(KIcon("dialog-cancel"));
        ui->volumeIcon->setToolTip(i18n("<b>Muted</b><br>Click to restore volume"));
        ui->volumeIcon->setChecked(true);
    } else {
        ui->volumeIcon->setIcon(KIcon("speaker"));
        ui->volumeIcon->setToolTip(i18n("Mute volume"));
        ui->volumeIcon->setChecked(false);
        m_application->audioOutput()->setVolume(m_application->volume());
    }
}

void MainWindow::connectPhononWidgets()
{
    ui->volumeSlider->setAudioOutput(m_application->audioOutput());
    ui->seekSlider->setMediaObject(m_application->mediaObject());
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_application->audioOutput(), SLOT(setMuted(bool)));
    connect(m_application->audioOutput(), SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
    updateMuteStatus(false);
}


/*---------------------------
  -- SLOTS for media lists --
  ---------------------------*/
void MainWindow::mediaListChanged()
{
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
    
    if ((m_application->browsingModel()->rowCount() > 0) && (ui->mediaViewHolder->currentIndex() ==0)) {
        MediaItem firstListItem = m_application->browsingModel()->mediaItemAt(0);
        QString listItemType = firstListItem.type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Category")) {
            if (!firstListItem.fields["isTemplate"].toBool()) {
                ui->playAll->setVisible(true);
            }
        /*} else if (listItemType == "Category") {
            ui->playAll->setVisible(true);
        } else if ((listItemType == "Action") || (listItemType == "Message")) {*/
        } else {
            ui->playAll->setVisible(false);
        }
        ui->playSelected->setVisible(false);
    }
}

void MainWindow::mediaListPropertiesChanged()
{
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
}

void MainWindow::mediaListLoadingStateChanged(bool loading)
{
    if (loading) {
        if (!ui->loadingIndicator->isVisible()) {
            ui->loadingIndicator->show();
            showMediaListLoading();
        }
    } else {
        ui->loadingIndicator->hide();
    }
}

void MainWindow::showMediaListLoading()
{
    if (ui->loadingIndicator->isVisible()) {
        m_loadingProgress++;
        if ((m_loadingProgress > 7) || (m_loadingProgress < 0)) {
            m_loadingProgress = 0;
        }
        QString iconName = QString("bangarang-loading-%1").arg(m_loadingProgress);
        QPixmap pixmap = KIcon(iconName).pixmap(16, 16);
        ui->loadingIndicator->setPixmap(pixmap);
        QTimer::singleShot(100, this, SLOT(showMediaListLoading()));
    }
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


void MainWindow::delayedNotificationHide()
{
    QTimer::singleShot(3000, ui->notificationWidget, SLOT(hide()));
}

void MainWindow::browsingModelStatusUpdated()
{
    QHash<QString, QVariant> status = m_application->browsingModel()->status();
    QString description = status["description"].toString();
    int progress = status["progress"].toInt();
    if (!description.isEmpty()) {
        ui->notificationWidget->setVisible(true);
        QFontMetrics fm(ui->notificationText->font());
        QString notificationText = fm.elidedText(description, Qt::ElideRight, ui->notificationText->width());
        ui->notificationText->setText(notificationText);
    } else {
        ui->notificationText->setText(i18n("Complete"));
        delayedNotificationHide();
    }
    if (progress >= 0 && progress <= 100) {
        ui->notificationProgress->setValue(progress);
        ui->notificationProgress->setVisible(true);
    } else {
        ui->notificationProgress->setVisible(false);
    }
}

void MainWindow::mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (selected.indexes().count() > 0) {
        int firstRow = selected.indexes().at(0).row();
        MediaItem firstSelectedItem = m_application->browsingModel()->mediaItemAt(firstRow);
        QString itemType = firstSelectedItem.type;
        if ((itemType == "Audio") || (itemType == "Video") || (itemType == "Category")) {
            if (firstSelectedItem.fields.contains("isTemplate") &&
                firstSelectedItem.fields["isTemplate"].toBool()) {
                ui->playSelected->setVisible(false);
            } else {
                ui->playSelected->setVisible(true);
            }
            ui->playAll->setVisible(false);
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(false);
        }
    } else {
        MediaItem firstListItem = m_application->browsingModel()->mediaItemAt(0);
        QString itemType = firstListItem.type;
        if ((itemType == "Audio") || (itemType == "Video") || (itemType == "Category")) {
            if (firstListItem.fields.contains("isTemplate") &&
                firstListItem.fields["isTemplate"].toBool()) {
                ui->playAll->setVisible(false);
            } else {
                ui->playAll->setVisible(true);
            }
            ui->playSelected->setVisible(false);
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(false);
        }
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void MainWindow::audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if ((ui->mediaLists->currentIndex() == 0) && 
        (currentMainWidget() == MainMediaList) &&
        (selected.indexes().count() > 0)) {
        //Load selected media list
        MediaListProperties currentProperties;
        int selectedRow = selected.indexes().at(0).row();
        loadMediaList(m_audioListsModel, selectedRow);
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
    if ((ui->mediaLists->currentIndex() == 1) &&  
        (currentMainWidget() == MainMediaList) &&
        (selected.indexes().count() > 0)) {
        //Load selected media list
        MediaListProperties currentProperties;
        int selectedRow = selected.indexes().at(0).row();
        loadMediaList(m_videoListsModel, selectedRow);
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
    ui->closeMediaListFilter->setIcon(KIcon("dialog-close"));
    ui->infoIndexSelected->setIcon(KIcon("system-run"));
    ui->showInfoFetcherExpander->setIcon(KIcon("help-about"));
    ui->infoFetcherLink->setIcon(KIcon("emblem-symbolic-link"));


    //Now Playing View bottom bar
    ui->collectionButton->setIcon(KIcon("view-media-playlist"));
    ui->fullScreen->setIcon(KIcon("view-fullscreen"));
    ui->volumeIcon->setIcon(KIcon("speaker"));
    ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
    
    //Now Playing View top bar
    ui->showPlaylist->setIcon(KIcon("dialog-ok-apply"));
    ui->showPlaylist_2->setIcon(KIcon("dialog-ok-apply"));

    //Playlist View
    ui->repeat->setIcon(Utilities::turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
    ui->shuffle->setIcon(Utilities::turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    ui->showQueue->setIcon(Utilities::turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    ui->clearPlaylist->setIcon(Utilities::turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
    ui->closePlaylistFilter->setIcon(KIcon("dialog-close"));
    ui->closePlaylistNotification->setIcon(KIcon("dialog-close"));
    
    //Audio settings
    ui->restoreDefaultAudioSettings->setIcon(KIcon("edit-undo"));
    ui->restoreDefaultVideoSettings->setIcon(KIcon("edit-undo"));
}

void MainWindow::setupActions()
{
    ui->mediaPrevious->setDefaultAction(m_application->actionsManager()->action("play_previous"));
    ui->mediaNext->setDefaultAction(m_application->actionsManager()->action("play_next"));
//     m_videoWidget->setContextMenu(m_application->actionsManager()->nowPlayingContextMenu());
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
    setVideoSize(Normal);
    ui->videoFrame->setVisible(false);
}

void MainWindow::updateCachedDevicesList()
{
    m_devicesAdded.clear();
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString())) {
        const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc> ();
        if (disc == NULL)
            continue;
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
    if (isFullScreen() &&
        currentMainWidget() == MainNowPlaying &&
        event->type() == QEvent::MouseMove) {

        QMouseEvent * mouseEvent = (QMouseEvent *)event;
        QWidget* widget = (QWidget* )obj;

        if (widget->height() - mouseEvent->y() <= ui->widgetSet->height()) {
            //Show the widgets in the Now Playing view
            ui->widgetSet->setVisible(true);
        } else {
            //Hide the widgets in the Now Playing view
            ui->widgetSet->setVisible(false);
        }
    }

    //Show floating menu
    if (event->type() == QEvent::MouseMove) {
        if (!ui->contextStackHolder->isVisible()) {
            ui->floatingMenuHolder->setVisible(true);
            ui->floatingMenuHolder->raise();
            QPoint pos = ((QMouseEvent *)event)->globalPos();
            if (ui->floatingMenuHolder->rect().contains(ui->floatingMenuHolder->mapFromGlobal(pos))) {
                m_menuTimer->stop();
            } else {
                m_menuTimer->start();
            }
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
        if ( disc != NULL ) {
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
    viewPalette.setColor(QPalette::Window, palette().color(QPalette::Window));
    ui->mediaListFilter->setPalette(viewPalette);
    viewPalette.setColor(QPalette::Window, viewPalette.color(QPalette::AlternateBase));
    ui->infoFetcherExpander->setPalette(viewPalette);
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

void MainWindow::setVideoSize(VideoSize size)
{
    m_videoSize = size;
    if (m_videoSize == Normal) {
        ui->nowPlayingView->hideInfo();
        QPoint topLeft = ui->videoFrame->mapToParent(ui->videoFrame->rect().topLeft());
        QPropertyAnimation *animation = new QPropertyAnimation(ui->videoFrame, "geometry");
        animation->setDuration(500);
        animation->setStartValue(QRect(topLeft, ui->videoFrame->size()));
        animation->setEndValue(ui->nowPlayingHolder->rect());
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start();
    } else {
        int width = qMax(200, ui->nowPlayingHolder->width()/3);
        int height = qMax(150, width*3/4);
        int left = ui->nowPlayingHolder->width() - width - 20;
        int top = ui->nowPlayingHolder->height() - height - 20;
        QPropertyAnimation *animation = new QPropertyAnimation(ui->videoFrame, "geometry");
        connect(animation, SIGNAL(finished()), ui->nowPlayingView, SLOT(showInfo()));
        animation->setDuration(500);
        animation->setStartValue(ui->videoFrame->rect());
        animation->setEndValue(QRect(left, top, width, height));
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start();
    }
}

MainWindow::VideoSize MainWindow::videoSize()
{
    return m_videoSize;
}

void MainWindow::nowPlayingChanged()
{
    MediaItemModel * nowPlayingModel = m_application->playlist()->nowPlayingModel();
    if (nowPlayingModel->rowCount() == 0) {
        m_application->statusNotifierItem()->setState(Phonon::StoppedState);
        return;
    }
    
    MediaItem nowPlayingItem = nowPlayingModel->mediaItemAt(0);
    QString type = nowPlayingItem.type;
    //Tidy up view and switch to the correct viewing widget
    ui->nowPlayingView->tidyHeader();

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

    //Update Media List view
    int startRow = ui->mediaView->indexAt(ui->mediaView->rect().topLeft()).row();
    int endRow = ui->mediaView->indexAt(ui->mediaView->rect().bottomRight()).row();
    if (endRow == -1) {
        endRow = startRow + ui->mediaView->model()->rowCount();
    }
    for  (int i = startRow; i <= endRow; i++) {
        ui->mediaView->update(ui->mediaView->model()->index(i, 0));
    }
    
    //Update seekTime button
    if (m_application->bookmarksManager()->hasBookmarks(nowPlayingItem)) {
        ui->seekTime->setIcon(KIcon("bookmarks-organize"));
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    } else {
        ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    
    //Update status notifier
    //- Scale artwork to current desktop icon size otherwise notifier will show unknown icon
    int iconSize = KIconLoader::global()->currentSize(KIconLoader::Desktop);
    QPixmap artworkPix = nowPlayingItem.artwork.pixmap(iconSize, iconSize);
    m_application->statusNotifierItem()->setToolTip(QIcon(artworkPix), nowPlayingItem.title, nowPlayingItem.subTitle);
}

QFrame* MainWindow::currentFilterFrame()
{
  return (currentMainWidget() == MainMediaList) ? ui->mediaListFilter : ui->playlistFilter;
}

KFilterProxySearchLine* MainWindow::currentFilterProxyLine()
{
  return (currentMainWidget() == MainMediaList) ?
    ui->mediaListFilterProxyLine : ui->playlistFilterProxyLine;
}

void MainWindow::switchMainWidget(MainWindow::MainWidget which)
{
    ui->stackedWidget->setCurrentIndex((int) which);
    m_application->actionsManager()->updateToggleFilterText();

    //If no media list has been loaded yet, then load the selected one.
    if (which == MainMediaList) {
        if (m_application->browsingModel()->mediaListProperties().lri.isEmpty()) {
            if (ui->mediaLists->currentIndex() == 0) {
                QModelIndexList selected = ui->audioLists->selectionModel()->selectedIndexes();
                if (selected.count() > 0) {
                    int selectedRow = selected.at(0).row();
                    loadMediaList(m_audioListsModel, selectedRow);
                }
            } else {
                QModelIndexList selected = ui->videoLists->selectionModel()->selectedIndexes();
                if (selected.count() > 0) {
                    int selectedRow = selected.at(0).row();
                    loadMediaList(m_videoListsModel, selectedRow);
                }
            }
        }
    }
}

MainWindow::MainWidget MainWindow::currentMainWidget()
{
    return (MainWidget) ui->stackedWidget->currentIndex();
}

void MainWindow::mediaListLoading()
{
    if (ui->mediaListFilter->isVisible())
        ui->mediaListFilterProxyLine->lineEdit()->clear();
    hidePlayButtons();
}

void MainWindow::playlistLoading()
{
    if (ui->playlistFilter->isVisible())
        ui->playlistFilterProxyLine->lineEdit()->clear();
    showLoading();
}

void MainWindow::loadMediaList(MediaItemModel *listsModel, int row)
{
    MediaListProperties currentProperties;
    MediaItem selectedItem = listsModel->mediaItemAt(row);
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
    bool isAudioList = (listsModel->mediaListProperties().lri == "medialists://audio");
    bool isVideoList = (listsModel->mediaListProperties().lri == "medialists://video");
    bool selectedIsConfigurable = selectedItem.fields["isConfigurable"].toBool();
    if (isAudioList) {
        ui->configureAudioList->setVisible(selectedIsConfigurable);
    } else if (isVideoList) {
        ui->configureVideoList->setVisible(selectedIsConfigurable);
    }
}

void MainWindow::on_closePlaylistNotification_clicked()
{
    on_playlistNotificationNo_clicked();
}

void MainWindow::on_playlistNotificationNo_clicked()
{
    emit playlistNotificationResult(true);
    ui->playlistNotification->setVisible(false);
}

void MainWindow::on_playlistNotificationYes_clicked()
{
    emit playlistNotificationResult(false);
    ui->playlistNotification->setVisible(false);
}

bool MainWindow::newPlaylistNotification(QString text, QObject *receiver, const char* slot)
{
    bool question = (receiver != NULL);
    if (ui->playlistNotification->isVisible() && !ui->closePlaylistNotification->isVisible())
        return false; //There is a pending notification that needs to be answered!
    if (question && !slot)
        return false; //receiver set but no slot. That's wrong!
    ui->playlistNotificationLabel->setText(text);
    
    ui->closePlaylistNotification->setVisible(!question);
    ui->playlistNotificationNo->setVisible(question);
    ui->playlistNotificationYes->setVisible(question);
    
    if (question)
        connect(this, SIGNAL(playlistNotificationResult(bool)), receiver, slot);
    return true;
}
