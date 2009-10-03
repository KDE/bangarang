#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/utilities.h"
#include "platform/mediaitemmodel.h"
#include "platform/playlist.h"
#include "infomanager.h"
#include "mediaitemdelegate.h"
#include "nowplayingdelegate.h"


#include <KCmdLineArgs>
#include <KUrl>
#include <KIcon>
#include <KIconEffect>
#include <KMessageBox>
#include <KSqueezedTextLabel>
#include <KColorScheme>
#include <Phonon>

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);
    
    setPropertiesForLists();
    
    //Setup interface icons
    setupIcons();
    setupActions();

    // Hide certain widgets
    ui->previous->setVisible(false);
    ui->playlistHolder->setVisible(false);
    ui->playlistNameEdit->setVisible(false);
    ui->playSelected->setVisible(false);
    ui->addAudioListBox->setVisible(false);
    ui->showInfo->setVisible(false);
    ui->saveInfo->setVisible(false);
    ui->sortList->setVisible(false);
    
    
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
    connect(m_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
    
    //Set up media list view
    MediaListProperties mediaListProperties;
    mediaListProperties.lri = "music://artists";
    m_mediaItemModel = new MediaItemModel(this);
    m_mediaItemModel->setMediaListProperties(mediaListProperties);
    m_itemDelegate = new MediaItemDelegate(this);
    ui->mediaView->setModel(m_mediaItemModel);
    ui->mediaView->setItemDelegate(m_itemDelegate);
    m_itemDelegate->setView(ui->mediaView);
    ui->mediaView->header()->setVisible(false);
    connect(m_itemDelegate, SIGNAL(categoryActivated(QModelIndex)), m_mediaItemModel, SLOT(categoryActivated(QModelIndex)));
    connect(m_itemDelegate, SIGNAL(actionActivated(QModelIndex)), m_mediaItemModel, SLOT(actionActivated(QModelIndex)));
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_mediaItemModel, SIGNAL(loading()), this, SLOT(hidePlayButtons()));
    connect(m_mediaItemModel, SIGNAL(propertiesChanged()), this, SLOT(updateListTitle()));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    ui->mediaView->setMainWindow(this);
    
    //Set up playlist
    m_playlist = new Playlist(this, m_media);
    
    //Set up playlist view
    m_currentPlaylist = m_playlist->playlistModel();
    MediaListProperties currentPlaylistItemProperties;
    currentPlaylistItemProperties.name = QString("Current Playlist");
    m_currentPlaylist->setMediaListProperties(currentPlaylistItemProperties);
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
    ui->nowPlayingView->header()->hideSection(2);
    
    //Setup Info View
    m_infoManager = new InfoManager(this);
    
    //Set up defaults
    ui->audioLists->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(1);
    ui->mediaViewHolder->setCurrentIndex(0);
    updateSeekTime(0);
    showApplicationBanner();
    m_showQueue = false;
    m_repeat = false;
    m_shuffle = false;
    
    //Get command line args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() > 0) {
        if (args->isSet("play-dvd")) {
            //Play DVD
            
        } else if (args->isSet("play-cd")) {
            //Play CD
        
        } else {
            //Play Url
            KUrl cmdLineKUrl = args->url(0);
            MediaItem mediaItem = Utilities::mediaItemFromUrl(cmdLineKUrl);
            QList<MediaItem> mediaList;
            mediaList << mediaItem;
            m_playlist->playMediaList(mediaList);
        }
    }
    
    //Install event filter for hiding widgets in Now Playing view
    ui->nowPlayingView->installEventFilter(this);
    m_videoWidget->installEventFilter(this);
}

MainWindow::~MainWindow()
{
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
    ui->playlistHolder->setVisible(checked);
}

void MainWindow::on_fullScreen_toggled(bool fullScreen)
{
    if (fullScreen) {
        showFullScreen();
        ui->fullScreen->setIcon(KIcon("view-restore"));
        ui->fullScreen->setToolTip("<b>Fullscreen</b><br>Click to exit fullscreen");
        ui->widgetSet->setVisible(false);
        ui->nowPlayingToolbar->setVisible(false);
    } else {
        showNormal();
        ui->fullScreen->setIcon(KIcon("view-fullscreen"));
        ui->fullScreen->setToolTip("Show fullscreen");
        ui->widgetSet->setVisible(true);
        ui->nowPlayingToolbar->setVisible(true);
    }
}

void MainWindow::on_seekTime_clicked()
{
    showRemainingTime = !showRemainingTime;
    if (showRemainingTime) {
        ui->seekTime->setToolTip("<b>Time remaining</b><br>Click to show elapsed time");
    } else {
        ui->seekTime->setToolTip("<b>Time elapsed</b><br>Click to show remaining time");
    }
}

void MainWindow::on_mediaPlayPause_clicked()
{
    if (m_media->state() == Phonon::PausedState) {
		m_media->play();
    } else if (m_media->state() == Phonon::PlayingState){
        m_media->pause();
    } else {
        if (m_currentPlaylist->rowCount() > 0) {
            m_playlist->start();
        }
    }
}

void MainWindow::on_mediaNext_clicked()
{
    m_playlist->playNext();        
}

void MainWindow::on_mediaPrevious_clicked()
{
    m_playlist->playPrevious();
}

void MainWindow::on_playlistView_doubleClicked(const QModelIndex & index)
{
    m_playlist->playItemAt(index.row());
}

void MainWindow::on_volumeIcon_toggled(bool muted)
{
    if (muted) {
        ui->volumeIcon->setIcon(KIcon("dialog-cancel"));
        ui->volumeIcon->setToolTip("<b>Muted</b><br>Click to restore volume");
    } else {
        ui->volumeIcon->setIcon(KIcon("preferences-desktop-text-to-speech"));
        ui->volumeIcon->setToolTip("Mute volume");
    }
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
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(true);
            ui->showInfo->setVisible(false);
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
    QListWidgetItem * current;
    if (i == 0) {
        current = ui->audioLists->currentItem();
        ui->audioLists->setFocus();
        ui->Filter->setClickMessage("Search for audio");
    //} else if (i == 1) {
    } else {
        current = ui->videoLists->currentItem();
        ui->videoLists->setFocus();
        ui->Filter->setClickMessage("Search for video");
    }
    MediaListProperties currentProperties = listItemProperties(current);
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
    }
}

void MainWindow::on_audioLists_itemSelectionChanged()
{
    if (ui->audioLists->selectedItems().count() != 0) {
        QListWidgetItem * current = ui->audioLists->selectedItems().at(0);
        MediaListProperties currentProperties = listItemProperties(current);
        if (m_mediaItemModel->mediaListProperties().name != currentProperties.name) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
            ui->saveInfo->setVisible(false);
            ui->showInfo->setVisible(false);
        }
    }
}    

void MainWindow::on_videoLists_itemSelectionChanged()
{
    if (ui->videoLists->selectedItems().count() != 0) {
        QListWidgetItem * current = ui->videoLists->selectedItems().at(0);
        MediaListProperties currentProperties = listItemProperties(current);
        if (m_mediaItemModel->mediaListProperties().name != currentProperties.name) {
            m_mediaItemModel->clearMediaListData();
            m_mediaItemModel->setMediaListProperties(currentProperties);
            m_mediaItemModel->load();
            m_mediaListHistory.clear();
            m_mediaListPropertiesHistory.clear();
            ui->previous->setVisible(false);
            ui->mediaViewHolder->setCurrentIndex(0);
            ui->saveInfo->setVisible(false);
            ui->showInfo->setVisible(false);
        }
    }
}    

void MainWindow::on_clearPlaylist_clicked()
{
    ui->clearPlaylist->setIcon(KIcon("bangarang-clearplaylist"));
    KGuiItem clearPlaylist;
    clearPlaylist.setText(QString("Clear Playlist"));
    if (KMessageBox::warningContinueCancel(this, "Are you sure you want to clear the current playlist?", QString(), clearPlaylist) == KMessageBox::Continue) {
        m_playlist->clearPlaylist();
        showApplicationBanner();
        ui->nowPlaying->setIcon(KIcon("tool-animator"));
        ui->nowPlaying->setText("Now Playing");
    }
    ui->clearPlaylist->setIcon(turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void MainWindow::on_shuffle_clicked()
{
    m_shuffle = !m_shuffle;
    if (m_shuffle) {
        m_playlist->setMode(Playlist::Shuffle);
        ui->shuffle->setToolTip("<b>Shuffle On</b><br>Click to turn off Shuffle");
        ui->shuffle->setIcon(KIcon("bangarang-shuffle"));
    } else {
        m_playlist->setMode(Playlist::Normal);
        ui->shuffle->setToolTip("Turn on Shuffle");
        ui->shuffle->setIcon(turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    }
}

void MainWindow::on_repeat_clicked()
{
    m_repeat = !m_repeat;
    m_playlist->setRepeat(m_repeat);
    if (m_repeat) {
        ui->repeat->setIcon(KIcon("bangarang-repeat"));
        ui->repeat->setToolTip("<b>Repeat On</b><br>Click to turn off repeat");
    } else {
        ui->repeat->setIcon(turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
        ui->repeat->setToolTip("Turn on Repeat");
    }    
}

void MainWindow::on_showQueue_clicked()
{
    m_showQueue = !m_showQueue;
    if (m_showQueue) {
        ui->playlistView->setModel(m_playlist->queueModel());
        ui->showQueue->setToolTip("<b>Showing Upcoming</b><br>Click to show playlist");
        ui->playlistName->setText(ui->playlistName->text() + QString("(Upcoming)"));
        ui->showQueue->setIcon(KIcon("bangarang-preview"));
    } else {
        ui->playlistView->setModel(m_playlist->playlistModel());
        ui->showQueue->setToolTip("Show Upcoming");
        ui->playlistName->setText(ui->playlistName->text().remove("(Upcoming)"));
        ui->showQueue->setIcon(turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    }
}

void MainWindow::on_showInfo_clicked()
{
    m_infoManager->loadInfoView();    
}    

void MainWindow::on_saveInfo_clicked()
{
    m_infoManager->saveInfoView();    
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
    ui->seekTime->setText(displayTime);
    
    //Update Now Playing Button text
    QString title;
    if (m_nowPlaying->rowCount() > 0) {
        if (m_nowPlaying->mediaItemAt(0).type != "Application Banner") {        
            title = QString("\n") + m_nowPlaying->item(0,0)->data(Qt::DisplayRole).toString();
            QString subTitle = m_nowPlaying->item(0,0)->data(MediaItem::SubTitleRole).toString();
            ui->nowPlaying->setText(QString("Now Playing") + QString("(")+ displayTime + QString(")") + title);
        } else {
            ui->nowPlaying->setText(QString("Now Playing"));
        }
    }
}

void MainWindow::mediaStateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    if (newstate == Phonon::PlayingState) {
        ui->mediaPlayPause->setIcon(KIcon("media-playback-pause"));
    } else {
        ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
    }
    
    Q_UNUSED(oldstate);
}


/*---------------------------
  -- SLOTS for media lists --
  ---------------------------*/
void MainWindow::mediaListChanged()
{
    ui->listTitle->setText(m_mediaItemModel->mediaListProperties().name);
    
    ui->mediaView->header()->setStretchLastSection(false);
    ui->mediaView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->mediaView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->mediaView->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    
    if (m_mediaItemModel->rowCount() != 0) {
        QString listItemType = m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            ui->mediaView->header()->showSection(1);
            ui->mediaView->header()->hideSection(2);
            ui->playAll->setVisible(true);
        } else if (listItemType == "Category") {
            ui->mediaView->header()->hideSection(1);
            ui->mediaView->header()->showSection(2);
            ui->playAll->setVisible(true);
        } else if ((listItemType == "Action") || (listItemType == "Message")) {
            ui->mediaView->header()->hideSection(1);
            ui->mediaView->header()->hideSection(2);
            ui->playAll->setVisible(false);
        }
        ui->saveInfo->setVisible(false);
    }
}

void MainWindow::mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        ui->playSelected->setVisible(true);
        ui->playAll->setVisible(false);
        QString listItemType = m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            ui->showInfo->setVisible(true);
        }
    } else {
        ui->playSelected->setVisible(false);
        ui->playAll->setVisible(true);
        ui->showInfo->setVisible(false);
    }
    ui->saveInfo->setVisible(false);
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void MainWindow::playlistChanged()
{

    ui->playlistView->header()->setStretchLastSection(false);
    ui->playlistView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->playlistView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->playlistView->header()->setResizeMode(2, QHeaderView::ResizeToContents);
}

void MainWindow::nowPlayingChanged()
{
    if (m_nowPlaying->rowCount() > 0) {
        if (m_nowPlaying->mediaItemAt(0).type != "Application Banner") {        
            ui->nowPlaying->setIcon(m_nowPlaying->mediaItemAt(0).artwork);  
            setWindowTitle(QString(m_nowPlaying->mediaItemAt(0).title + " - Bangarang"));
        }
    } else {
        showApplicationBanner();
    }
    
    ui->nowPlayingView->header()->setStretchLastSection(false);
    ui->nowPlayingView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->nowPlayingView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    ui->nowPlayingView->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    ui->nowPlayingView->header()->hideSection(1);
    ui->nowPlayingView->header()->hideSection(2);
    
    if (m_nowPlaying->mediaItemAt(0).type == "Video") {
        ui->viewerStack->setCurrentIndex(1);
    } else if (m_nowPlaying->mediaItemAt(0).type == "Audio") {
        ui->viewerStack->setCurrentIndex(0);
    }
}

void MainWindow::hidePlayButtons()
{
    ui->playSelected->setVisible(false);
    ui->playAll->setVisible(false);
}

void MainWindow::updateListTitle()
{
    ui->listTitle->setText(m_mediaItemModel->mediaListProperties().name);
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

void MainWindow::setPropertiesForLists()
{
    MediaListProperties listItemProperties;
    //Audio
    listItemProperties.name = "Files and Folders";
    listItemProperties.lri = "files://audio";
    setListItemProperties(ui->audioLists->item(0), listItemProperties);
    listItemProperties.name = "Artists";
    listItemProperties.lri = "music://artists";
    setListItemProperties(ui->audioLists->item(1), listItemProperties);
    listItemProperties.name = "Albums";
    listItemProperties.lri = "music://albums";
    setListItemProperties(ui->audioLists->item(2), listItemProperties);
    listItemProperties.name = "Songs";
    listItemProperties.lri = "music://songs";
    setListItemProperties(ui->audioLists->item(3), listItemProperties);
    listItemProperties.name = "Genres";
    listItemProperties.lri = "music://genres";
    setListItemProperties(ui->audioLists->item(4), listItemProperties);
    listItemProperties.name = "Clips";
    listItemProperties.lri = "audioclips://";
    setListItemProperties(ui->audioLists->item(5), listItemProperties);
    //Video
    listItemProperties.name = "Files and Folders";
    listItemProperties.lri = "files://video";
    setListItemProperties(ui->videoLists->item(0), listItemProperties);    
    listItemProperties.name = "Movies";
    listItemProperties.lri = "video://movies";
    setListItemProperties(ui->videoLists->item(1), listItemProperties);    
    listItemProperties.name = "Series";
    listItemProperties.lri = "video://series";
    setListItemProperties(ui->videoLists->item(2), listItemProperties);    
    listItemProperties.name = "Video Clips";
    listItemProperties.lri = "video://clips";
    setListItemProperties(ui->videoLists->item(3), listItemProperties);    
}

MediaListProperties MainWindow::listItemProperties(QListWidgetItem * item)
{
    MediaListProperties listItemProperties;
    listItemProperties.name = item->data(ListChooser::NameRole).toString();
    listItemProperties.lri = item->data(ListChooser::LriRole).toString();
    listItemProperties.type = item->data(ListChooser::TypeRole).toString();
    
    return listItemProperties;
}

void MainWindow::setListItemProperties(QListWidgetItem * item, MediaListProperties listItemProperties)
{
    item->setData(ListChooser::NameRole, listItemProperties.name);
    item->setData(ListChooser::LriRole, listItemProperties.lri);
    item->setData(ListChooser::TypeRole, listItemProperties.type);
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
    ui->audioLists->item(0)->setIcon(KIcon("document-open-folder"));
    ui->audioLists->item(1)->setIcon(KIcon("system-users"));
    ui->audioLists->item(2)->setIcon(KIcon("media-optical"));
    ui->audioLists->item(3)->setIcon(KIcon("audio-mpeg"));
    ui->audioLists->item(4)->setIcon(KIcon("flag-blue"));
    ui->audioLists->item(5)->setIcon(KIcon("audio-x-wav"));
    ui->addAudioList->setIcon(KIcon("list-add"));
    ui->removeAudioList->setIcon(KIcon("list-remove"));
    ui->configureAudioList->setIcon(KIcon("configure"));
    
    //Video List Icons
    ui->videoLists->item(0)->setIcon(KIcon("document-open-folder"));
    ui->videoLists->item(1)->setIcon(KIcon("tool-animator"));
    ui->videoLists->item(2)->setIcon(KIcon("video-television"));
    ui->videoLists->item(3)->setIcon(KIcon("video-x-generic"));
    ui->addVideoList->setIcon(KIcon("list-add"));
    ui->removeVideoList->setIcon(KIcon("list-remove"));
    ui->configureVideoList->setIcon(KIcon("configure"));
    
    //Media View Icons
    ui->sortList->setIcon(KIcon("view-sort-ascending"));
    ui->playSelected->setIcon(KIcon("media-playback-start"));
    ui->playAll->setIcon(KIcon("media-playback-start"));
    ui->nowPlaying->setIcon(KIcon("tool-animator"));
    
    //Now Playing View bottom bar
    ui->collectionButton->setIcon(KIcon("view-media-playlist"));
    ui->fullScreen->setIcon(KIcon("view-fullscreen"));
    ui->volumeIcon->setIcon(KIcon("preferences-desktop-text-to-speech"));
    
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
    playAllAction = new QAction(KIcon("media-playback-start"), tr("Play all"), this);
    connect(playAllAction, SIGNAL(triggered()), this, SLOT(playAll()));
    playSelectedAction = new QAction(KIcon("media-playback-start"), tr("Play selected"), this);
    connect(playSelectedAction, SIGNAL(triggered()), this, SLOT(playSelected()));
}

void MainWindow::showApplicationBanner()
{
    MediaItem applicationBanner;
    applicationBanner.artwork = KIcon("bangarang");
    applicationBanner.title = "Bangarang";
    applicationBanner.subTitle = "Entertainment... Now";
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

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if ((event->type() == QEvent::Enter)) {
        //Hide the widgets in the Now Playing view
        if ((ui->stackedWidget->currentIndex() == 1) && (isFullScreen()) && (!ui->playlistHolder->isVisible())) {
            ui->widgetSet->setVisible(false);
            ui->nowPlayingToolbar->setVisible(false);
        }
    } else if ((event->type() == QEvent::Leave)) {
        //Show the widgets in the Now Playing view
        if ((ui->stackedWidget->currentIndex() == 1) && (isFullScreen())) {
            ui->widgetSet->setVisible(true);
            ui->nowPlayingToolbar->setVisible(true);
        }
    }
    
    // standard event processing
    return QObject::eventFilter(obj, event);
}

/*void MainWindow::saveInfo()
{
    if (m_infoItemsModel->rowCount() > 0) {
        for (int i = 0; i < m_infoItemsModel->rowCount) {
            MediaItem mediaItem = m_infoItemsModel->mediaItemAt(i);
            
            //Save rating
            Utilities::saveRating(mediaItem, rating);
            
            //Save new tags
            Utilities::saveTags(mediaItem, tags);

            if (m_infoItemsModel->mediaItemAt(0).type = "Audio") {
            
}*/
