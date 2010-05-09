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

#include "actionsmanager.h"
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"
#include "platform/playlist.h"
#include "infomanager.h"
#include "savedlistsmanager.h"
#include "bookmarksmanager.h"
#include "videosettings.h"

#include <KStandardDirs>
#include <KMessageBox>
#include <KHelpMenu>
#include <KDebug>
#include <QFile>

ActionsManager::ActionsManager(MainWindow * parent) : QObject(parent)
{
    /*Set up basics */
    m_parent = parent;
    ui = m_parent->ui;

    m_shortcutsConfig = KGlobal::config()->group("shortcuts");

    m_shortcutsCollection = new KActionCollection(this);
    m_shortcutsCollection->setConfigGlobal(true);
    m_shortcutsCollection->addAssociatedWidget(m_parent); //won't have to add each action to the parent
    m_othersCollection = new KActionCollection(this);
    m_contextMenuSource = MainWindow::Default;

    /*Set up actions*/
    //Add standard quit shortcut
    KAction *action = new KAction(KIcon("application-exit"), i18n("Quit"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(action, SIGNAL(triggered()), qApp, SLOT(quit()));
    m_shortcutsCollection->addAction("quit", action);

    //Play/Pause Action
    action = new KAction(KIcon("media-playback-start"), i18n("Play/Pause"), this);
    action->setShortcut(Qt::Key_Space);
    connect(action, SIGNAL(triggered()), this, SLOT(simplePlayPause()));
    m_shortcutsCollection->addAction("play_pause", action);
    //globals only work after adding them to the action manager
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_D));


    //Play Action
    action = new KAction(KIcon("media-playback-start"), i18n("Play"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(smartPlay()));
    m_othersCollection->addAction("play", action);

    //Pause Action
    action = new KAction(KIcon("media-playback-pause"), i18n("Pause"), this);
    connect(action, SIGNAL(triggered()), m_parent->playlist()->mediaObject(), SLOT(pause()));
    m_othersCollection->addAction("pause", action);


    //Play Next
    action = new KAction(KIcon("media-skip-forward"), i18n("Play next"), this);
    action->setShortcut(Qt::Key_Right);
    connect(action, SIGNAL(triggered()), m_parent->playlist(), SLOT(playNext()));
    m_shortcutsCollection->addAction("play_next", action);
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_F));

    //Play Previous
    action = new KAction(KIcon("media-skip-backward"), i18n("Play previous"), this);
    action->setShortcut(Qt::Key_Left);
    connect(action, SIGNAL(triggered()), m_parent->playlist(), SLOT(playPrevious()));
    m_shortcutsCollection->addAction("play_previous", action);
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_S));

    //Mute
    action = new KAction(KIcon("player-volume-muted"), i18n("Mute"), this);
    action->setShortcut(Qt::Key_M);
    connect(action, SIGNAL(triggered()), this, SLOT(muteAudio()));
    m_shortcutsCollection->addAction("mute", action);

    //Play All Action
    action = new KAction(KIcon("media-playback-start"), i18n("Play all"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(playAllSlot()));
    m_othersCollection->addAction("play_all", action);

    //Play Selected Action
    action = new KAction(KIcon("media-playback-start"), i18n("Play selected"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(playSelectedSlot()));
    m_othersCollection->addAction("play_selected", action);

    //Add Selected To Playlist Action
    action = new KAction(KIcon("mail-mark-notjunk"), i18n("Add to playlist"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(addSelectedToPlaylistSlot()));
    m_othersCollection->addAction("add_to_playlist", action);

    //Remove Selected From Playlist Action
    action = new KAction(KIcon(), i18n("Remove from playlist"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(removeSelectedFromPlaylistSlot()));
    m_othersCollection->addAction("remove_from_playlist", action);

    //Toggle Controls Shortcut
    action = new KAction(KIcon("layer-visible-off"), i18n("Hide controls"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_H);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleControls()));
    m_shortcutsCollection->addAction("toggle_controls", action);

    //Toggle Playlist Filter
    action = new KAction(KIcon("layer-visible-off"), i18n("Show playlist filter"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(action, SIGNAL(triggered()), this, SLOT(togglePlaylistFilter()));
    m_shortcutsCollection->addAction("toggle_playlist_filter", action);

    //Toggle Show Remaining Time Shortcut
    action = new KAction(KIcon("chronometer"), i18n("Show Remaining Time"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleShowRemainingTimeSlot()));
    m_shortcutsCollection->addAction("toggle_show_remaining_time", action);

    //Show VideoSettings
    action = new KAction(KIcon("video-display"), i18n("Show Video Settings"),this);
    action->setShortcut(Qt::CTRL + Qt::Key_V);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleVideoSettings()));
    m_shortcutsCollection->addAction("show_video_settings",action);

    //Full Screen
    action = new KAction(KIcon("view-fullscreen"), i18n("Fullscreen"), this);
    action->setShortcut(Qt::Key_F11);
    connect(action, SIGNAL(triggered()), this, SLOT(fullScreenToggle()));
    m_shortcutsCollection->addAction("toggle_fullscreen", action);

    //Cancel FullScreen/Cancel Hide Controls
    action = new KAction(this);
    action->setShortcut(Qt::Key_Escape);
    connect(action, SIGNAL(triggered()), this, SLOT(cancelFSHC()));
    m_parent->addAction(action);
    m_othersCollection->addAction("cancel", action); //shouldn't be editable

    //Remove Info for Selected MediaItems
    action = new KAction(KIcon("edit-delete-shred"), i18n("Remove selected info"), this);
    connect(action, SIGNAL(triggered()), m_parent->infoManager(), SLOT(removeSelectedItemsInfo()));
    m_othersCollection->addAction("remove_selected_info", action);

    //Refresh Media View
    action = new KAction(KIcon("view-refresh"), i18n("Refresh"), this);
    action->setShortcut(Qt::Key_F5);
    connect(action, SIGNAL(triggered()), m_parent->m_mediaItemModel, SLOT(reload()));
    m_shortcutsCollection->addAction("reload", action);

    //Remove selected from playlist
    action = new KAction(i18n("Remove from list"), this);
    connect(action, SIGNAL(triggered()), m_parent->savedListsManager(), SLOT(removeSelected()));
    m_othersCollection->addAction("remove_from_list", action);

    //Add selected to saved audio list
    m_addToAudioSavedList = new QMenu(i18n("Add to list"), m_parent);
    connect(m_addToAudioSavedList, SIGNAL(triggered(QAction *)), this, SLOT(addToSavedAudioList(QAction *)));

    //Add selected to saved video list
    m_addToVideoSavedList = new QMenu(i18n("Add to list "), m_parent);
    connect(m_addToVideoSavedList, SIGNAL(triggered(QAction *)), this, SLOT(addToSavedVideoList(QAction *)));

    //Add a new audio list
    action = new KAction(KIcon("list-add"), i18n("New list"), m_parent);
    connect(action, SIGNAL(triggered()), m_parent->savedListsManager(), SLOT(showAudioListSave()));
    m_othersCollection->addAction("new_audio_list", action);

    //Add a new video list
    action = new KAction(KIcon("list-add"), i18n("New list"), m_parent);
    connect(action, SIGNAL(triggered()), m_parent->savedListsManager(), SLOT(showVideoListSave()));
    m_othersCollection->addAction("new_video_list", action);

    //Show Items
    action = new KAction(KIcon("bangarang-category-browse"), i18n("Show items"), m_parent);
    connect(action, SIGNAL(triggered()), this, SLOT(loadSelectedSources()));
    m_othersCollection->addAction("show_items", action);

    //Show Now Playing Info
    action = new KAction(KIcon("help-about"), i18n("Show Information"), m_parent);
    connect(action, SIGNAL(triggered()), this, SLOT(showInfoForNowPlaying()));
    m_othersCollection->addAction("show_now_playing", action);

    //Show Info View
    action = new KAction(KIcon("help-about"), i18n("Show Information"), m_parent);
    connect(action, SIGNAL(triggered()), m_parent->infoManager(), SLOT(showInfoView()));
    m_othersCollection->addAction("show_info", action);

    //Add bookmark
    action = new KAction(KIcon("bookmark-new"), i18n("Add bookmark"), m_parent);
    connect(action, SIGNAL(triggered()), this, SLOT(addBookmarkSlot()));
    m_othersCollection->addAction("add_bookmark", action);

    //Bookmarks Menus
    m_bookmarksMenu = new QMenu(m_parent);
    connect(m_bookmarksMenu, SIGNAL(triggered(QAction *)), this, SLOT(activateBookmark(QAction *)));
    m_removeBookmarksMenu = new QMenu(i18n("Remove bookmarks"), m_parent);
    connect(m_removeBookmarksMenu, SIGNAL(triggered(QAction *)), this, SLOT(removeBookmark(QAction *)));

    //Show the Scripting Console
    action = new KAction(KIcon("applications-development"),i18n("Show Scripting-Console"),m_parent);
    connect(action,SIGNAL(triggered()),this,SLOT(showScriptConsoleSlot()));
    m_shortcutsCollection->addAction("show_scripting_console", action);

    //Edit Shortcuts
    action = new KAction(KIcon("configure-shortcuts"), i18n("Configure shortcuts..."), this);
    connect(action, SIGNAL(triggered()), this, SLOT(showShortcutsEditor()));
    connect(ui->cancelEditShortcuts, SIGNAL(clicked()), this, SLOT(cancelShortcuts()));
    connect(ui->saveShortcuts, SIGNAL(clicked()), this, SLOT(saveShortcuts()));
    m_shortcutsCollection->addAction("show_shortcuts_editor", action);

    //set up the shortcuts collection
    m_shortcutsCollection->readSettings(&m_shortcutsConfig);
    ui->shortcutsEditor->addCollection(m_shortcutsCollection);

    /*Set up other variables */
    m_nowPlayingContextMenu = new QMenu(m_parent);
    m_notifierMenu = new KMenu(m_parent);

    //controls always visible at startup
    m_controlsVisible = true;
    //filter ain't
    m_playlistFilterVisible = false;
    m_restoreFilter = "";
}

ActionsManager::~ActionsManager()
{
}

QAction * ActionsManager::action( QString name, bool shortcutsOnly )
{
  QAction *action = m_shortcutsCollection->action(name);
  if (!shortcutsOnly && action == NULL)
  {
    action = m_othersCollection->action(name);
  }
  return action;
}

QMenu * ActionsManager::mediaViewMenu(bool showAbout, MainWindow::ContextMenuSource menuSource)
{
    KHelpMenu * helpMenu = new KHelpMenu(m_parent, m_parent->aboutData(), false);
    helpMenu->menu();
    
    updateSavedListsMenus();
    
    m_contextMenuSource = menuSource;
    
    QMenu *menu = new QMenu(m_parent);
    QString type;
    bool selection = false;
    QList<MediaItem> selectedItems = selectedMediaItems();
    if (selectedItems.count() > 0) {
        type = selectedItems.at(0).type;
        selection = true;
    }
    bool isMedia = false;
    if ((type == "Audio") ||(type == "Video") || (type == "Image")) {
        isMedia = true;
    }
    bool isCategory = false;
    if (type == "Category") {
        isCategory = true;
    }
    if (isMedia || isCategory) {
        if (selection && isMedia) {
            menu->addAction(action("add_to_playlist"));
            menu->addAction(action("remove_from_playlist"));
            menu->addSeparator();
        }
        if (selection) menu->addAction(action("play_selected"));
        menu->addAction(action("play_all"));
        menu->addSeparator();
        if (selection && isCategory) {
            menu->addAction(action("show_items"));
            menu->addSeparator();
        }
        if (selection && isMedia) {
            if (type == "Audio") {
                if (m_addToAudioSavedList->actions().count() > 0) {
                    menu->addMenu(m_addToAudioSavedList);
                }
            } else if (type == "Video") {
                if (m_addToVideoSavedList->actions().count() > 0) {
                    menu->addMenu(m_addToVideoSavedList);
                }
            }
            if (m_parent->m_mediaItemModel->mediaListProperties().lri.startsWith("savedlists://")) {
                menu->addAction(action("remove_from_list"));
            }
            menu->addSeparator();
        }
        if (selection && isMedia) {
            menu->addAction(action("remove_selected_info"));
            menu->addSeparator();
        }
	
        menu->addAction(action("reload"));
        menu->addSeparator();
        
    } 
    if (showAbout) menu->addAction(helpMenu->action(KHelpMenu::menuAboutApp));
    return menu;
}

QMenu *ActionsManager::nowPlayingContextMenu()
{
    m_nowPlayingContextMenu->clear();
    if (m_parent->playlist()->mediaObject()->state() == Phonon::PlayingState ||
        m_parent->playlist()->mediaObject()->state() == Phonon::PausedState) {
        m_nowPlayingContextMenu->addAction(action("play_previous"));
        if (m_parent->playlist()->mediaObject()->state() == Phonon::PlayingState) {
            m_nowPlayingContextMenu->addAction(action("pause"));
        } else {
            m_nowPlayingContextMenu->addAction(action("play"));
        }
    } else {
        m_nowPlayingContextMenu->addAction(action("play"));
    }
    m_nowPlayingContextMenu->addAction(action("play_next"));
    m_nowPlayingContextMenu->addSeparator();
    if (m_parent->playlist()->mediaObject()->hasVideo()) {
        m_nowPlayingContextMenu->addAction(action("show_video_settings"));
    }
    m_nowPlayingContextMenu->addAction(action("toggle_controls"));
    return m_nowPlayingContextMenu;
}

KMenu *ActionsManager::notifierMenu()
{
    m_notifierMenu->clear();
    m_notifierMenu->addAction(action("play_previous"));
    m_notifierMenu->addAction(action("play_pause"));
    m_notifierMenu->addAction(action("play_next"));
    return m_notifierMenu;
}

QMenu *ActionsManager::addToSavedAudioListMenu()
{
    return m_addToAudioSavedList;
}

QMenu *ActionsManager::addToSavedVideoListMenu()
{
    return m_addToVideoSavedList;
}

QMenu *ActionsManager::bookmarksMenu()
{
    m_removeBookmarksMenu->clear();
    m_bookmarksMenu->clear();
    m_bookmarksMenu->addAction(action("toggle_show_remaining_time"));
    m_bookmarksMenu->addSeparator();
    Phonon::State state = m_parent->playlist()->mediaObject()->state();
    if (!(state == Phonon::PlayingState || state == Phonon::PausedState)) {
        action("add_bookmark")->setEnabled(false);
    } else {
        action("add_bookmark")->setEnabled(true);
    }
    m_bookmarksMenu->addAction(action("add_bookmark"));
    if (m_parent->playlist()->nowPlayingModel()->rowCount() > 0) {
        QString url = m_parent->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        QStringList bookmarks = m_parent->bookmarksManager()->bookmarks(url);
        for (int i = 0; i < bookmarks.count(); i++) {
            QString bookmarkName = m_parent->bookmarksManager()->bookmarkName(bookmarks.at(i));
            qint64 bookmarkTime = m_parent->bookmarksManager()->bookmarkTime(bookmarks.at(i));
            QTime bmTime(0, (bookmarkTime / 60000) % 60, (bookmarkTime / 1000) % 60);
            QString bookmarkTimeStr = bmTime.toString(QString("m:ss"));
            QString bookmarkTitle = QString("%1 (%2)").arg(bookmarkName).arg(bookmarkTimeStr);
            KAction * bookmarkAction = new KAction(KIcon("bookmarks-organize"), bookmarkTitle, m_bookmarksMenu);
            bookmarkAction->setData(QString("Activate:%1").arg(bookmarks.at(i)));
            m_bookmarksMenu->addAction(bookmarkAction);
            KAction * removeBookmarkAction = new KAction(KIcon("list-remove"), bookmarkTitle, m_removeBookmarksMenu);
            removeBookmarkAction->setData(QString("Remove:%1").arg(bookmarks.at(i)));
            m_removeBookmarksMenu->addAction(removeBookmarkAction);
        }
        if (bookmarks.count() > 0) {
            m_bookmarksMenu->addMenu(m_removeBookmarksMenu);
        }
    }
    return m_bookmarksMenu;
}

//------------------
//-- Action SLOTS --
//------------------
void ActionsManager::fullScreenToggle()
{
    if (m_parent->isFullScreen()) {
        m_parent->on_fullScreen_toggled(false);
    } else {
        m_parent->on_fullScreen_toggled(true);
    }
}

void ActionsManager::toggleControls()
{
    QAction *toggle = action("toggle_controls");
    if ((!m_parent->isFullScreen()) && (ui->stackedWidget->currentIndex() == 1)) {
        if (m_controlsVisible) {
            ui->widgetSet->setVisible(false);
            ui->nowPlayingToolbar->setVisible(false);
            toggle->setIcon(KIcon("layer-visible-on"));
	    toggle->setText(i18n("Show Controls"));
        } else {
            ui->widgetSet->setVisible(true);
            ui->nowPlayingToolbar->setVisible(true);
            toggle->setIcon(KIcon("layer-visible-off"));
	    toggle->setText(i18n("Hide Controls"));
        }
        m_controlsVisible = !m_controlsVisible;
    }
}

void ActionsManager::toggleVideoSettings()
{
    if(ui->contextStack->currentIndex() != 1 ) {
        m_contextStackWasVisible = ui->contextStack->isVisible();
        m_previousContextStackIndex = ui->contextStack->currentIndex();
        ui->contextStack->setCurrentIndex(1);
        ui->contextStack->setVisible(true);
        action("show_video_settings")->setText(i18n("Hide Video Settings"));
    } else {
        ui->contextStack->setVisible(m_contextStackWasVisible);
        ui->contextStack->setCurrentIndex(m_previousContextStackIndex);
        action("show_video_settings")->setText(i18n("Show Video Settings"));
    }
}

void ActionsManager::cancelFSHC()
{
    if (m_parent->isFullScreen()) {
        m_parent->on_fullScreen_toggled(false);
    } else if (ui->stackedWidget->currentIndex() == 1 && !m_controlsVisible) {
        toggleControls();
    } else if (ui->playlistFilterProxyLine->lineEdit()->hasFocus()) {
        togglePlaylistFilter();
    }
}

void ActionsManager::showShortcutsEditor()
{
    ui->contextStack->setCurrentIndex(2);
    ui->contextStack->setVisible(true);
}

void ActionsManager::saveShortcuts()
{
    ui->shortcutsEditor->writeConfiguration(&m_shortcutsConfig);
    ui->shortcutsEditor->commit();
    hideShortcutsEditor();
}

void ActionsManager::cancelShortcuts()
{
    //user canceld, it should be undone what had been edited
    ui->shortcutsEditor->undoChanges();
    hideShortcutsEditor();
}

void ActionsManager::hideShortcutsEditor()
{

    ui->contextStack->setCurrentIndex(0);
    ui->contextStack->setVisible(false);
}

void ActionsManager::simplePlayPause()
{
    if (m_parent->playlist()->mediaObject()->state() == Phonon::PlayingState) {
        m_parent->playlist()->mediaObject()->pause();
    } else {
        smartPlay();
    }
}

void ActionsManager::smartPlay()
{
    if (m_parent->playlist()->mediaObject()->state() == Phonon::PausedState) {
        m_parent->playlist()->mediaObject()->play();
    } else if (m_parent->playlist()->mediaObject()->state() != Phonon::PlayingState) {
        m_parent->playlist()->start();
    }
}

void ActionsManager::muteAudio()
{
    bool muted = m_parent->audioOutput()->isMuted();
    m_parent->audioOutput()->setMuted(!muted);
}

void ActionsManager::addSelectedToPlaylistSlot()
{
    QList<MediaItem> mediaList = selectedMediaItems();
    for (int i = 0; i < mediaList.count(); i++) {
        if (mediaList.at(i).type == "Audio" ||
            mediaList.at(i).type == "Video") {
            int playlistRow = m_parent->playlist()->playlistModel()->rowOfUrl(mediaList.at(i).url);
            if (playlistRow == -1) {
                m_parent->playlist()->addMediaItem(mediaList.at(i));
            }
        }
    }
}

void ActionsManager::removeSelectedFromPlaylistSlot()
{
    QList<MediaItem> mediaList = selectedMediaItems();
    for (int i = 0; i < mediaList.count(); i++) {
        if (mediaList.at(i).type == "Audio" ||
            mediaList.at(i).type == "Video") {
            int playlistRow = m_parent->playlist()->playlistModel()->rowOfUrl(mediaList.at(i).url);
            if (playlistRow != -1) {
                m_parent->playlist()->removeMediaItemAt(playlistRow);
            }
        }
    }
}

void ActionsManager::updateSavedListsMenus()
{
    m_addToAudioSavedList->clear();
    m_addToAudioSavedList->addAction(action("new_audio_list"));
    QStringList audioListNames = m_parent->savedListsManager()->savedListNames("Audio");
    for (int i = 0; i < audioListNames.count(); i++) {
        if (!((audioListNames.at(i) == m_parent->m_mediaItemModel->mediaListProperties().name)
            && (m_parent->m_mediaItemModel->mediaListProperties().lri.startsWith("savedlists://")))) { 
            KAction * addToSavedList = new KAction(KIcon("view-list-text"), audioListNames.at(i), m_addToAudioSavedList);
            addToSavedList->setData(audioListNames.at(i));
            m_addToAudioSavedList->addAction(addToSavedList);
        }
    }
    
    m_addToVideoSavedList->clear();
    m_addToVideoSavedList->addAction(action("new_video_list"));
    QStringList videoListNames = m_parent->savedListsManager()->savedListNames("Video");
    for (int i = 0; i < videoListNames.count(); i++) {
        if (!((videoListNames.at(i) == m_parent->m_mediaItemModel->mediaListProperties().name)
            && (m_parent->m_mediaItemModel->mediaListProperties().lri.startsWith("savedlists://")))) { 
            KAction * addToSavedList = new KAction(KIcon("view-list-text"), videoListNames.at(i), m_addToVideoSavedList);
            addToSavedList->setData(videoListNames.at(i));
            m_addToVideoSavedList->addAction(addToSavedList);
        }
    }
}

void ActionsManager::removeSelectedItemsInfoSlot()
{
    QList<MediaItem> mediaList = selectedMediaItems();
    m_parent->m_mediaItemModel->removeSourceInfo(mediaList);
}

void ActionsManager::playSelectedSlot()
{
    //Get selected mediaitems and play
    QList<MediaItem> mediaList = selectedMediaItems();
    m_parent->playlist()->playMediaList(mediaList);
    
    // Show Now Playing page
    ui->stackedWidget->setCurrentIndex(1);   
}

void ActionsManager::playAllSlot()
{
    //Play all media items in the media list view
    m_parent->playlist()->playMediaList(m_parent->m_mediaItemModel->mediaList());

    // Show Now Playing page
    ui->stackedWidget->setCurrentIndex(1);   
}

void ActionsManager::addToSavedAudioList(QAction *addAction)
{
    //Get list of selected items to add
    QList<MediaItem> mediaList = selectedMediaItems();
    
    //Add to saved list
    if (mediaList.count() > 0) {
        QString audioListName = addAction->data().toString();
        m_parent->savedListsManager()->saveMediaList(mediaList, audioListName, "Audio", true);
    }
}

void ActionsManager::addToSavedVideoList(QAction *addAction)
{
    //Get list of selected items to add
    QList<MediaItem> mediaList = selectedMediaItems();
    
    //Add to saved list
    if (mediaList.count() > 0) {
        QString videoListName = addAction->data().toString();
        m_parent->savedListsManager()->saveMediaList(mediaList, videoListName, "Video", true);
    }
}

void ActionsManager::loadSelectedSources()
{
    m_parent->addListToHistory();
    QList<MediaItem> mediaList;
    if (m_contextMenuSource == MainWindow::InfoBox) {
        mediaList = m_parent->infoManager()->selectedInfoBoxMediaItems();
    } else {
        QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
        }
    }
    m_parent->m_mediaItemModel->clearMediaListData();
    m_parent->m_mediaItemModel->loadSources(mediaList);
}

void ActionsManager::showInfoForNowPlaying()
{
    MediaItemModel * nowPlayingModel = m_parent->playlist()->nowPlayingModel();
    if(nowPlayingModel->rowCount() > 0) {
        MediaItem mediaItem = nowPlayingModel->mediaItemAt(0);
        m_parent->infoManager()->showInfoViewForMediaItem(mediaItem);
    }
}

void ActionsManager::showScriptConsoleSlot()
{
    m_parent->scriptConsole()->show();
}

const QList<MediaItem> ActionsManager::selectedMediaItems()
{
    QList<MediaItem> mediaList;
    if (m_contextMenuSource == MainWindow::InfoBox ||
        m_contextMenuSource == MainWindow::Default) {
        mediaList = m_parent->infoManager()->selectedInfoBoxMediaItems();
    }
    if (m_contextMenuSource == MainWindow::MediaList ||
        (m_contextMenuSource == MainWindow::Default && mediaList.count() == 0)) {
        for (int i = 0; i < ui->mediaView->selectionModel()->selectedIndexes().count(); ++i) {
            QModelIndex index = ui->mediaView->selectionModel()->selectedIndexes().at(i);
            if (index.column() == 0) {
                mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(index.row()));
            }
        }
    }
    return mediaList;
}

void ActionsManager::setContextMenuSource(MainWindow::ContextMenuSource menuSource)
{
    m_contextMenuSource = menuSource;
}

void ActionsManager::toggleShowRemainingTimeSlot()
{
    m_parent->setShowRemainingTime(!m_parent->showingRemainingTime());
    if (m_parent->showingRemainingTime()) {
        action("toggle_show_remaining_time")->setText(i18n("Show Elapsed Time"));
    } else {
        action("toggle_show_remaining_time")->setText(i18n("Show Remaining Time"));
    }
}

void ActionsManager::addBookmarkSlot()
{
    if (m_parent->playlist()->nowPlayingModel()->rowCount() > 0) {
        QString nowPlayingUrl = m_parent->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        qint64 time = m_parent->playlist()->mediaObject()->currentTime();
        QString name = QString("Bookmark-%1").arg(m_parent->bookmarksManager()->bookmarks(nowPlayingUrl).count() + 1);
        m_parent->bookmarksManager()->addBookmark(nowPlayingUrl, name, time);
        if (m_parent->bookmarksManager()->bookmarks(nowPlayingUrl).count() > 0) {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        }
    }
}

void ActionsManager::activateBookmark(QAction *bookmarkAction)
{
    QString bookmark = bookmarkAction->data().toString();
    if (!bookmark.isEmpty() && bookmark.startsWith("Activate:")) {
        bookmark.remove(0,9);
        qint64 time = m_parent->bookmarksManager()->bookmarkTime(bookmark);
        m_parent->playlist()->mediaObject()->seek(time);
    }
        
}

void ActionsManager::removeBookmark(QAction *bookmarkAction)
{
    QString bookmark = bookmarkAction->data().toString();
    if (!bookmark.isEmpty() && bookmark.startsWith("Remove:")) {
        bookmark.remove(0,7);
        QString nowPlayingUrl = m_parent->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        m_parent->bookmarksManager()->removeBookmark(nowPlayingUrl, bookmark);
        if (m_parent->bookmarksManager()->bookmarks(nowPlayingUrl).count() == 0) {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }
    }
}

void ActionsManager::togglePlaylistFilter()
{
    if(!m_playlistFilterVisible) {
        ui->playlistFilter->setVisible(true);
        action("toggle_playlist_filter")->setText(i18n("Hide playlist filter"));
        if(!m_restoreFilter.isEmpty()) {
            ui->playlistFilterProxyLine->setText( m_restoreFilter );
            m_restoreFilter.clear();
        }
        ui->playlistFilterProxyLine->lineEdit()->setFocus(); //the user can star immediately to search
    } else {
        ui->playlistFilter->setVisible(false);
        action("toggle_playlist_filter")->setText(i18n("Show playlist filter"));
        if(!ui->playlistFilterProxyLine->lineEdit()->text().isEmpty())
        {
            m_restoreFilter = ui->playlistFilterProxyLine->lineEdit()->text();
            ui->playlistFilterProxyLine->setText( "" );
        }
    }
    m_playlistFilterVisible = !m_playlistFilterVisible;
}

