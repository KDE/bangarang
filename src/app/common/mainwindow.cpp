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
#include "actionsmanager.h"
#include "ui_mainwindow.h"
#include "../nowplaying/bangarangvideowidget.h"
#include "../nowplaying/nowplayingmanager.h"
#include "../medialists/medialistsmanager.h"
#include "../medialists/infomanager.h"
#include "../medialists/medialistsettings.h"
#include "../../platform/utilities/utilities.h"

#include <KAction>
#include <KAcceleratorManager>
#include <KCursor>
#include <KIcon>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KDebug>
#include <KHelpMenu>
#include <KMenu>

#include <Nepomuk/ResourceManager>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QtGlobal>
#include <QPalette>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    m_application = (BangarangApplication *)KApplication::kApplication();

    ui->setupUi(this);
    m_audioListsStack = new AudioListsStack(0);
    m_videoListsStack = new VideoListsStack(0);
    KAcceleratorManager::setNoAccel(ui->audioListSelect);
    KAcceleratorManager::setNoAccel(ui->videoListSelect);
    
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
    m_audioListsStack->ui->configureAudioList->setVisible(false);
    m_videoListsStack->ui->configureVideoList->setVisible(false);
    ui->semanticsHolder->setVisible(false);
    ui->loadingIndicator->setVisible(false);
    ui->extSubtitle->setVisible(false);
    ui->playbackMessage->setVisible(false);
    ui->notificationWidget->setVisible(false);
    ui->videoListsStackHolder->hide();
    ui->videoListsSelectHolder->show();
    ui->videoListLabel->hide();
    ui->audioListsStackHolder->layout()->addWidget(m_audioListsStack);
    ui->audioListsStackHolder->show();
    ui->audioListsSelectHolder->hide();
    ui->audioListLabel->show();

    //Initialize Nepomuk
    m_nepomukInited = Utilities::nepomukInited();
    if (!m_nepomukInited) {
        ui->Filter->setVisible(false);
    }

    //Set up video widget
    m_videoWidget =  new BangarangVideoWidget(ui->videoFrame);
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
    ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
    KAcceleratorManager::setNoAccel(ui->seekTime);
    
    //Set up playlist view
    ui->playlistView->setMainWindow(this);
    ui->playlistFilterProxyLine->lineEdit()->setClickMessage(QString());
    ui->playlistFilter->setVisible(false);
    ui->playlistNotification->setVisible(false);
    ui->playlistNotificationNo->setText(i18n("No"));
    ui->playlistNotificationYes->setText(i18n("Yes"));
    
    //Setup Now Playing view
    ui->nowPlayingView->setMainWindow( this );
    updateCustomColors();
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
    m_audioListsStack->ui->audioListsStack->setCurrentIndex(0);
    m_videoListsStack->ui->videoListsStack->setCurrentIndex(0);
    ui->contextStack->setCurrentIndex(0);
    ui->mediaPlayPause->setHoldDelay(1000);
    ui->listSummary->setFont(KGlobalSettings::smallestReadableFont());
    ui->playlistDuration->setFont(KGlobalSettings::smallestReadableFont());
    ui->playbackMessage->clear();
    ui->collectionButton->setFocus();

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
    resetTabOrder();
}

/*---------------
  -- Accessors --
  ---------------*/
AudioListsStack *MainWindow::audioListsStack() {
    return m_audioListsStack;
}

VideoListsStack *MainWindow::videoListsStack() {
    return m_videoListsStack;
}

MediaListSettings *MainWindow::mediaListSettings()
{
    return m_mediaListSettings;
}

BangarangVideoWidget * MainWindow::videoWidget()
{
    return m_videoWidget;
}

MainWindow::VideoSize MainWindow::videoSize()
{
    return m_videoSize;
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

    if (m_videoSize == Normal && !this->isFullScreen()) {
        ui->videoFrame->setGeometry(QRect(QPoint(0, 0), ui->nowPlayingHolder->size()));
    } else if (m_videoSize == Normal && this->isFullScreen()) {
        ui->videoFrame->setGeometry(QRect(QPoint(0, 0), QSize(ui->nowPlayingHolder->width(), this->height())));
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
        int left = ui->nowPlayingHolder->width() - (width + qMax(24, textRect.height() + 8));
        ui->floatingMenuHolder->setGeometry(QRect(QPoint(left, 0),
                                                  QSize(width + qMax(24, textRect.height() + 8),
                                                        qMax(24, textRect.height() + 8))));
        ui->floatingMenuHolder->setVisible(true);
        ui->floatingMenuHolder->raise();
        m_menuTimer->start();
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
    m_application->actionsManager()->action("show_shortcuts_editor")->setText(i18n("Show shortcuts editor"));
}

void MainWindow::on_showPlaylist_2_clicked()
{
    on_showPlaylist_clicked();
}

void MainWindow::on_fullScreen_toggled(bool fullScreen)
{
    if (fullScreen) {
        showFullScreen();
        ui->widgetSet->setPalette(ui->contextStackHolder->palette());
        ui->widgetSet->setAutoFillBackground(true);
        ui->fullScreen->setIcon(KIcon("view-restore"));
        ui->fullScreen->setToolTip(i18n("<b>Fullscreen</b><br>Click to exit fullscreen"));
        ui->fullScreen->setChecked(true);
        ui->widgetSet->setVisible(false);
    } else {
        showNormal();
        ui->widgetSet->setPalette(this->palette());
        ui->widgetSet->setAutoFillBackground(false);
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


void MainWindow::on_showMenu_clicked()
{
    m_application->nowPlayingManager()->showMenu();
}

void MainWindow::on_showMenu_2_clicked()
{
    m_application->nowPlayingManager()->showMenu();
}

void MainWindow::on_showMediaViewMenu_clicked()
{
    m_application->mediaListsManager()->showMenu();
}


/*----------------------
  -- Helper functions --
  ----------------------*/
void MainWindow::setupIcons()
{
    //Main Window Icon
    setWindowIcon(KIcon("bangarang"));
    
    //Audio List Icons
    m_audioListsStack->ui->addAudioList->setIcon(KIcon("list-add"));
    m_audioListsStack->ui->removeAudioList->setIcon(KIcon("list-remove"));
    m_audioListsStack->ui->configureAudioList->setIcon(KIcon("configure"));
    m_audioListsStack->ui->saveAudioList->setIcon(KIcon("document-save"));
    m_audioListsStack->ui->aslsSave->setIcon(KIcon("document-save"));
    m_audioListsStack->ui->aslsExport->setIcon(KIcon("document-export"));
    m_audioListsStack->ui->semAConfigSave->setIcon(KIcon("document-save"));
    
    //Video List Icons
    m_videoListsStack->ui->addVideoList->setIcon(KIcon("list-add"));
    m_videoListsStack->ui->removeVideoList->setIcon(KIcon("list-remove"));
    m_videoListsStack->ui->configureVideoList->setIcon(KIcon("configure"));
    m_videoListsStack->ui->saveVideoList->setIcon(KIcon("document-save"));
    m_videoListsStack->ui->vslsSave->setIcon(KIcon("document-save"));
    m_videoListsStack->ui->vslsExport->setIcon(KIcon("document-export"));
    m_videoListsStack->ui->semVConfigSave->setIcon(KIcon("document-save"));
    
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
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (isFullScreen() &&
        !m_application->isTouchEnabled() &&
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
    emit switchedMainWidget(which);
}

MainWindow::MainWidget MainWindow::currentMainWidget()
{
    return (MainWidget) ui->stackedWidget->currentIndex();
}

void MainWindow::enableTouch() {
    kDebug() << "ENABLING TOUCH";
    int tTouchable = BangarangApplication::TOUCH_TOUCHABLE_METRIC;
    int tVisual = BangarangApplication::TOUCH_VISUAL_METRIC;
    ui->widgetSet->setMinimumHeight(48);
    ui->widgetSet->setMaximumHeight(48);
    ui->mediaListWidgetSet->setMinimumHeight(48);
    ui->mediaListWidgetSet->setMaximumHeight(48);
    ui->mediaListWidgetSet1->setMinimumHeight(48);
    ui->mediaListWidgetSet1->setMaximumHeight(48);
    ui->showMediaViewMenu->setMinimumSize(tTouchable, tTouchable);
    ui->audioListSelect->setMinimumSize(tTouchable, tTouchable);
    ui->videoListSelect->setMinimumSize(tTouchable, tTouchable);
    m_audioListsStack->enableTouch();
    m_videoListsStack->enableTouch();
    ui->Filter->setMinimumHeight(tTouchable);
    ui->closeMediaListFilter->setMinimumSize(tTouchable, tTouchable);
    ui->mediaListFilterProxyLine->setMinimumHeight(tTouchable);
    ui->mediaView->enableTouch();
    m_application->infoManager()->enableTouch();
    ui->playlistView->enableTouch();
    ui->closePlaylistFilter->setMinimumSize(tTouchable, tTouchable);
    ui->playlistFilterProxyLine->setMinimumHeight(tTouchable);
    ui->nowPlayingView->enableTouch();
    ui->fullScreen->setMinimumSize(tTouchable, tTouchable);
    ui->fullScreen->setIconSize(QSize(tVisual, tVisual));
    ui->seekTime->setMinimumHeight(tTouchable);
    ui->volumeIcon->setMinimumSize(tTouchable, tTouchable);
    ui->volumeIcon->setIconSize(QSize(tVisual, tVisual));
    ui->volumeSlider->setMaximumWidth(110);
    ui->volumeSlider->setMinimumWidth(110);
    ui->mediaPrevious->setMinimumSize(tTouchable + 8, tTouchable);
    ui->mediaNext->setMinimumSize(tTouchable + 8, tTouchable);
    ui->mediaPlayPause->setMinimumSize(50, 46);
    ui->mediaPlayPause->setIconSize(QSize(40, 40));
    ui->clearPlaylist->setMinimumSize(tTouchable, tTouchable);
    ui->shuffle->setMinimumSize(tTouchable, tTouchable);
    ui->repeat->setMinimumSize(tTouchable, tTouchable);
    ui->showQueue->setMinimumSize(tTouchable, tTouchable);
    ui->showPlaylist->setMinimumHeight(tTouchable);
    ui->showPlaylist_2->setMinimumHeight(tTouchable);
    ui->showMenu->setMinimumSize(tTouchable, tTouchable);
    ui->showMenu_2->setMinimumSize(tTouchable, tTouchable);
    ui->floatingMenuHolder->setMinimumSize(tTouchable, tTouchable);
    ui->widgetSet->setPalette(ui->contextStackHolder->palette());
    ui->widgetSet->setAutoFillBackground(true);
}

void MainWindow::resetTabOrder()
{
    //Set Tab Order for Media Lists view
    QWidget* visibleList = m_audioListsStack->ui->audioLists;
    if (ui->videoListsStackHolder->isVisible()) {
        visibleList = m_videoListsStack->ui->videoLists;
    }
    setTabOrder(visibleList, ui->mediaView);
}

void MainWindow::toggleMainWidget()
{
    if (currentMainWidget() == MainMediaList) {
        switchMainWidget(MainNowPlaying);
    } else {
        switchMainWidget(MainMediaList);
    }
}

void MainWindow::stopMenuTimer()
{
    m_menuTimer->stop();
}
