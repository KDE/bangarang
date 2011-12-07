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

#include "nowplayingmanager.h"
#include "../common/bangarangapplication.h"
#include "../common/mainwindow.h"
#include "../common/bangarangnotifieritem.h"
#include "../common/actionsmanager.h"
#include "bangarangvideowidget.h"
#include "playlistview.h"
#include "bookmarksmanager.h"
#include "ui_mainwindow.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/playlist.h"
#include "../../platform/utilities/artwork.h"
#include "../../platform/utilities/general.h"

#include <KMessageBox>
#include <KMenu>
#include <KDebug>


NowPlayingManager::NowPlayingManager(MainWindow* parent) :
    QObject(parent)
{
    m_application = (BangarangApplication*)KApplication::kApplication();
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    m_pausePressed = false;
    m_stopPressed = false;
    m_loadingProgress = 0;

    //Connect to media object signals and slots
    connect(m_application->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(updateSeekTime(qint64)));
    connect(m_application->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged(Phonon::State, Phonon::State)));
    connect(m_application->mainWindow()->videoWidget(), SIGNAL(skipForward(int)), this, SLOT(skipForward(int)));
    connect(m_application->mainWindow()->videoWidget(), SIGNAL(skipBackward(int)), this, SLOT(skipBackward(int)));
    connectPhononWidgets();

    //Set up playlist
    connect(m_application->playlist(), SIGNAL(playlistFinished()), this, SLOT(playlistFinished()));
    connect(m_application->playlist(), SIGNAL(loading()), this, SLOT(playlistLoading()));
    connect(m_application->playlist(), SIGNAL(shuffleModeChanged(bool)), this, SLOT(shuffleModeChanged(bool)));
    connect(m_application->playlist(), SIGNAL(repeatModeChanged(bool)), this, SLOT(repeatModeChanged(bool)));

    //Setup Now Playing view
    connect(m_application->playlist()->nowPlayingModel(), SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));

    //Set up playlist view
    connect(ui->playlistView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playPlaylistItem(QModelIndex)));
    connect(ui->closePlaylistNotification, SIGNAL(clicked()), this, SLOT(closePlaylistNotification()));
    connect(ui->playlistNotificationNo, SIGNAL(clicked()), this, SLOT(selectPlaylistNotificationNo()));
    connect(ui->playlistNotificationYes, SIGNAL(clicked()), this, SLOT(selectPlaylistNotificationYes()));
    connect(ui->closePlaylistFilter, SIGNAL(clicked()), this, SLOT(closePlaylistFilter()));
    ui->playlistFilterProxyLine->setProxy(m_application->playlist()->filterProxyModel());
    m_playWhenPlaylistChanges = false;

    //Setup show queue, clear playlist shuffle and repeat
    connect(ui->showQueue, SIGNAL(clicked()), this, SLOT(toggleQueue()));
    connect(ui->shuffle, SIGNAL(clicked()), this, SLOT(toggleShuffle()));
    connect(ui->repeat, SIGNAL(clicked()), this, SLOT(toggleRepeat()));
    connect(ui->clearPlaylist, SIGNAL(clicked()), this, SLOT(clearPlaylist()));

    //Setup playback buttons
    connect(ui->mediaPlayPause, SIGNAL(pressed()), this, SLOT(mediaPlayPausePressed()));
    connect(ui->mediaPlayPause, SIGNAL(held()), this, SLOT(mediaPlayPauseHeld()));
    connect(ui->mediaPlayPause, SIGNAL(released()), this, SLOT(mediaPlayPauseReleased()));

    setShowRemainingTime(false);
    updateSeekTime(0);
    showApplicationBanner();
}

void NowPlayingManager::connectPhononWidgets()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->volumeSlider->setAudioOutput(m_application->audioOutput());
    ui->seekSlider->setMediaObject(m_application->mediaObject());
    connect(ui->volumeIcon, SIGNAL(toggled(bool)), m_application->audioOutput(), SLOT(setMuted(bool)));
    connect(m_application->audioOutput(), SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
    updateMuteStatus(m_application->audioOutput()->isMuted());
}

void NowPlayingManager::disconnectPhononWidgets()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    disconnect(ui->volumeIcon, SIGNAL(toggled(bool)), m_application->audioOutput(), SLOT(setMuted(bool)));
    disconnect(m_application->audioOutput(), SIGNAL(mutedChanged(bool)), this, SLOT(updateMuteStatus(bool)));
}

void NowPlayingManager::setShowRemainingTime(bool showRemainingTime)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    m_showRemainingTime = showRemainingTime;
    if (m_showRemainingTime) {
        ui->seekTime->setToolTip(i18n("<b>Time remaining</b><br>Click to show elapsed time and bookmarks"));
    } else {
        ui->seekTime->setToolTip(i18n("<b>Time elapsed</b><br>Click to show remaining time and bookmarks"));
    }
}

bool NowPlayingManager::showingRemainingTime()
{
    return m_showRemainingTime;
}

bool NowPlayingManager::newPlaylistNotification(QString text, QObject *receiver, const char *slot)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    bool question = (receiver != NULL);
    if (ui->playlistNotification->isVisible() && !ui->closePlaylistNotification->isVisible()) {
        return false; //There is a pending notification that needs to be answered!
    }
    if (question && !slot) {
        return false; //receiver set but no slot. That's wrong!
    }
    ui->playlistNotificationLabel->setText(text);

    ui->closePlaylistNotification->setVisible(!question);
    ui->playlistNotificationNo->setVisible(question);
    ui->playlistNotificationYes->setVisible(question);

    if (question) {
        connect(this, SIGNAL(playlistNotificationResult(bool)), receiver, slot);
    }
    return true;

}

void NowPlayingManager::mediaPlayPausePressed()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if ((m_application->mediaObject()->state() == Phonon::PlayingState)) {
        m_application->mediaObject()->pause();
        m_pausePressed = true;
        ui->mediaPlayPause->setToolTip(i18n("<b>Paused</b><br>Hold to stop"));
    }
}

void NowPlayingManager::mediaPlayPauseHeld()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if ((m_application->mediaObject()->state() != Phonon::LoadingState) && (m_application->mediaObject()->state() != Phonon::StoppedState)) {
        if (m_pausePressed) {
            m_pausePressed = false;
        }
        m_stopPressed = true;
        ui->mediaPlayPause->setIcon(KIcon("media-playback-stop"));
        m_application->playlist()->stop();
    }
}

void NowPlayingManager::mediaPlayPauseReleased()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (m_stopPressed) {
        m_stopPressed = false;
    } else {
        if ((!m_pausePressed) && (m_application->mediaObject()->state() == Phonon::PausedState)) {
            m_application->mediaObject()->play();
        } else if ((m_application->mediaObject()->state() == Phonon::StoppedState) || (m_application->mediaObject()->state() == Phonon::LoadingState)) {
            if (ui->playlistView->model()->rowCount() > 0 && ui->playlistView->selectionModel()->selectedIndexes().count() == 0) {
                m_application->playlist()->start();
            } else if (ui->playlistView->model()->rowCount() > 0 && ui->playlistView->selectionModel()->selectedIndexes().count() > 0) {
                QModelIndex index = ui->playlistView->selectionModel()->selectedIndexes().at(0);
                playPlaylistItem(index);
            }
        }
    }
    m_pausePressed = false;
    if ((m_application->mediaObject()->state() == Phonon::PausedState) || (m_application->mediaObject()->state() == Phonon::StoppedState)) {
        ui->mediaPlayPause->setIcon(KIcon("media-playback-start"));
        ui->mediaPlayPause->setToolTip("");
    }
}

void NowPlayingManager::playPlaylistItem(const QModelIndex &index)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    int row = m_application->playlist()->filterProxyModel()->mapToSource(index).row();
    m_application->playlist()->playItemAt(row, ui->playlistView->currentModelType());
    ui->playlistView->selectionModel()->clear();
}

void NowPlayingManager::showApplicationBanner()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    MediaItem applicationBanner;
    applicationBanner.artwork = KIcon("bangarang");
    applicationBanner.title = i18n("Bangarang");
    applicationBanner.subTitle = i18n("Entertainment... Now");
    applicationBanner.type = "Application Banner";
    applicationBanner.url = "-";
    m_application->playlist()->nowPlayingModel()->loadMediaItem(applicationBanner, true);
    m_application->mainWindow()->setVideoSize(MainWindow::Normal);
    ui->videoFrame->setVisible(false);
}

void NowPlayingManager::toggleShuffle()
{
    bool shuffleMode = m_application->playlist()->shuffleMode();
    m_application->playlist()->setShuffleMode(!shuffleMode);
}

void NowPlayingManager::toggleRepeat()
{
    bool repeatMode = m_application->playlist()->repeatMode();
    m_application->playlist()->setRepeatMode(!repeatMode);
}

void NowPlayingManager::toggleQueue()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    Playlist::Model type = ui->playlistView->toggleModel();
    if (type == Playlist::QueueModel) {
        ui->showQueue->setToolTip(i18n("<b>Showing Upcoming</b><br>Click to show playlist"));
        ui->showQueue->setIcon(KIcon("bangarang-preview"));
    } else {
        ui->showQueue->setToolTip(i18n("Show Upcoming"));
        ui->showQueue->setIcon(Utilities::turnIconOff(KIcon("bangarang-preview"), QSize(22, 22)));
    }
 }

void NowPlayingManager::updateSeekTime(qint64 time)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;

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
            QFontMetrics fm(ui->nowPlaying->font());
            title = fm.elidedText(title, Qt::ElideRight, ui->nowPlaying->width() - 30 - 3*4);
            QString nowPlayingText = i18n("Now Playing") + QString(" (")+ displayTime + QString(")\n") + title;
            ui->nowPlaying->setText(nowPlayingText);
        } else {
            ui->nowPlaying->setText(i18n("Now Playing"));
        }
    }
}

void NowPlayingManager::mediaStateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
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
            if (m_application->mainWindow()->videoSize() == MainWindow::Mini) {
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
            showErrorMessage(i18n("An error has been encountered during playback"));
        } else {
            showErrorMessage(m_application->mediaObject()->errorString());
        }

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
    }

    if (newstate == Phonon::PausedState)
      m_application->statusNotifierItem()->setState(newstate);
    else if (newstate == Phonon::StoppedState)
      m_application->statusNotifierItem()->setState(newstate);

    Q_UNUSED(oldstate);
}

void NowPlayingManager::showLoading()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
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

void NowPlayingManager::updateMuteStatus(bool muted)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
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

void NowPlayingManager::playlistFinished()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    showApplicationBanner();
    m_application->mainWindow()->setWindowTitle(i18n("Bangarang"));
    ui->nowPlaying->setIcon(KIcon("tool-animator"));
    ui->nowPlaying->setText(i18n("Now Playing"));
    ui->nowPlaying->setToolTip(i18n("View Now Playing"));
    ui->seekTime->setText("0:00");
}

void NowPlayingManager::repeatModeChanged(bool repeat)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (repeat) {
        ui->repeat->setIcon(KIcon("bangarang-repeat"));
        ui->repeat->setToolTip(i18n("<b>Repeat On</b><br>Click to turn off repeat"));
    } else {
        ui->repeat->setIcon(Utilities::turnIconOff(KIcon("bangarang-repeat"), QSize(22, 22)));
        ui->repeat->setToolTip(i18n("Turn on Repeat"));
    }
}

void NowPlayingManager::shuffleModeChanged(bool shuffle)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (shuffle) {
        ui->shuffle->setToolTip(i18n("<b>Shuffle On</b><br>Click to turn off Shuffle"));
        ui->shuffle->setIcon(KIcon("bangarang-shuffle"));
    } else {
        ui->shuffle->setToolTip(i18n("Turn on Shuffle"));
        ui->shuffle->setIcon(Utilities::turnIconOff(KIcon("bangarang-shuffle"), QSize(22, 22)));
    }
}

void NowPlayingManager::nowPlayingChanged()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    MediaItemModel * nowPlayingModel = m_application->playlist()->nowPlayingModel();
    if (nowPlayingModel->rowCount() == 0) {
        m_application->statusNotifierItem()->setState(Phonon::StoppedState);
        return;
    }

    MediaItem nowPlayingItem = nowPlayingModel->mediaItemAt(0);
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
        m_application->mainWindow()->setWindowTitle(QString(nowPlayingItem.title + " - Bangarang"));
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

void NowPlayingManager::playlistLoading()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (ui->playlistFilter->isVisible()) {
        ui->playlistFilterProxyLine->lineEdit()->clear();
    }
    showLoading();
}

void NowPlayingManager::closePlaylistNotification()
{
    selectPlaylistNotificationNo();
}

void NowPlayingManager::selectPlaylistNotificationNo()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    emit playlistNotificationResult(true);
    ui->playlistNotification->setVisible(false);
}

void NowPlayingManager::selectPlaylistNotificationYes()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    emit playlistNotificationResult(false);
    ui->playlistNotification->setVisible(false);
}

void NowPlayingManager::skipForward(int i)
{
    if (m_application->mediaObject()->isSeekable()) {
        m_application->mediaObject()->seek(m_application->mediaObject()->currentTime() + qint64(i)*100);
    }
}

void NowPlayingManager::skipBackward(int i)
{
    if (m_application->mediaObject()->isSeekable()) {
        m_application->mediaObject()->seek(m_application->mediaObject()->currentTime() + qint64(i)*100);
    }
}

void NowPlayingManager::clearPlaylist()
{
    if (m_application->mainWindow()->currentMainWidget() != MainWindow::MainNowPlaying) {
        return;
    }

    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->clearPlaylist->setIcon(KIcon("bangarang-clearplaylist"));
    KGuiItem clearPlaylist;
    clearPlaylist.setText(i18n("Clear Playlist"));
    if (KMessageBox::warningContinueCancel(m_application->mainWindow(), i18n("Are you sure you want to clear the current playlist?"), QString(), clearPlaylist) == KMessageBox::Continue) {
        m_application->playlist()->clearPlaylist();
        showApplicationBanner();
        m_application->mainWindow()->setWindowTitle(i18n("Bangarang"));
        ui->nowPlaying->setIcon(KIcon("tool-animator"));
        ui->nowPlaying->setText(i18n("Now Playing"));
    }
    ui->clearPlaylist->setIcon(Utilities::turnIconOff(KIcon("bangarang-clearplaylist"), QSize(22, 22)));
}

void NowPlayingManager::closePlaylistFilter()
{
    m_application->actionsManager()->action("toggle_filter")->trigger();
}

void NowPlayingManager::showMenu()
{
    m_application->mainWindow()->stopMenuTimer();
    KMenu * menu = m_application->actionsManager()->nowPlayingMenu();

    QPoint menuLocation;
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (ui->contextStackHolder->isVisible()) {
        menuLocation = ui->showMenu->mapToGlobal(QPoint(0,ui->showMenu->height()));
    } else {
        menuLocation = ui->showMenu_2->mapToGlobal(QPoint(0,ui->showMenu->height()));
    }
    menu->popup(menuLocation);
}

void NowPlayingManager::togglePlaylist()
{
    if (m_application->mainWindow()->currentMainWidget() != MainWindow::MainNowPlaying) {
        return;
    }

    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (ui->contextStackHolder->isVisible() && ui->contextStack->currentIndex() == 0) {
        ui->contextStackHolder->setVisible(false);
    } else {
        ui->contextStack->setCurrentIndex(0);
        ui->contextStackHolder->setVisible(true);
        QFrame *filter = m_application->mainWindow()->currentFilterFrame();
        KFilterProxySearchLine *line = m_application->mainWindow()->currentFilterProxyLine();
        if (filter->isVisible() && line->lineEdit()->text().isEmpty()) {
            m_application->actionsManager()->action("toggle_filter")->trigger();
        }
    }
    m_application->actionsManager()->action("show_video_settings")->setText(i18n("Show video settings"));
    m_application->actionsManager()->action("show_audio_settings")->setText(i18n("Show audio settings"));
}

void NowPlayingManager::showErrorMessage(QString error)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->playbackMessage->setText(error);
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
}
