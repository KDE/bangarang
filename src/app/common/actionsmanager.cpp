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
#include "bangarangapplication.h"
#include "bangarangnotifieritem.h"
#include "../../platform/utilities/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/playlist.h"
#include "../../platform/ontologyupdater.h"
#include "../../platform/infofetchers/infofetcher.h"
#include "../medialists/medialistsmanager.h"
#include "../medialists/infomanager.h"
#include "../medialists/savedlistsmanager.h"
#include "../nowplaying/bookmarksmanager.h"
#include "../nowplaying/videosettings.h"
#include "../nowplaying/audiosettings.h"
#include "../nowplaying/nowplayingdelegate.h"
#include "../nowplaying/nowplayingmanager.h"

#include <KStandardDirs>
#include <KMessageBox>
#include <KHelpMenu>
#include <KMessageBox>
#include <KDebug>
#include <KNotifyConfigWidget>
#include <QFile>


ActionsManager::ActionsManager(MainWindow * parent) : QObject(parent)
{
    /*Set up basics */
    m_application = (BangarangApplication *)KApplication::kApplication();
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
    connect(action, SIGNAL(triggered()), m_application->playlist()->mediaObject(), SLOT(pause()));
    m_othersCollection->addAction("pause", action);

    //Play Next
    action = new KAction(KIcon("media-skip-forward"), i18n("Play next"), this);
    connect(action, SIGNAL(triggered()), m_application->playlist(), SLOT(playNext()));
    m_shortcutsCollection->addAction("play_next", action);
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_F));

    //Play Previous
    action = new KAction(KIcon("media-skip-backward"), i18n("Play previous"), this);
    connect(action, SIGNAL(triggered()), m_application->playlist(), SLOT(playPrevious()));
    m_shortcutsCollection->addAction("play_previous", action);
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_S));

    //Mute
    action = new KAction(KIcon("dialog-cancel"), i18n("Mute"), this);
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
    action = new KAction(KIcon("dialog-ok-apply"), i18n("Add to playlist"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(addSelectedToPlaylistSlot()));
    m_othersCollection->addAction("add_to_playlist", action);

    //Add After Now Playing Action
    action = new KAction(KIcon("dialog-ok-apply"), i18n("Add after Now Playing"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(addAfterNowPlaying()));
    m_othersCollection->addAction("add_after_now_playing", action);

    //Remove Selected From Playlist Action
    action = new KAction(KIcon("list-remove"), i18n("Remove from playlist"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(removeSelectedFromPlaylistSlot()));
    m_othersCollection->addAction("remove_from_playlist", action);
    
    //Remove the selection of the playlist from the playlist
    action = new KAction(KIcon("list-remove"), i18n("Remove from playlist"), this);
    action->setShortcut(Qt::Key_Delete);
    connect(action, SIGNAL(triggered()), this, SLOT(removePlaylistSelectionFromPlaylistSlot()));
    m_othersCollection->addAction("remove_playlistselection_from_playlist", action);

    //Toggle Controls Shortcut
    action = new KAction(KIcon("layer-visible-off"), i18n("Hide controls"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_H);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleControls()));
    m_shortcutsCollection->addAction("toggle_controls", action);

    //Toggle Playlist/Media Lists Filter
    action = new KAction(KIcon("view-filter"), i18n("Show filter"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleFilter()));
    m_shortcutsCollection->addAction("toggle_filter", action);

    //Toggle Show Remaining Time Shortcut
    action = new KAction(KIcon("chronometer"), i18n("Show remaining time"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleShowRemainingTimeSlot()));
    m_shortcutsCollection->addAction("toggle_show_remaining_time", action);

    //Show VideoSettings
    action = new KAction(KIcon("video-display"), i18n("Show video settings"),this);
    action->setShortcut(Qt::CTRL + Qt::Key_V);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleVideoSettings()));
    m_shortcutsCollection->addAction("show_video_settings",action);

    //Show Audio Settings
    action = new KAction(KIcon("speaker"), i18n("Show audio settings"),this);
    action->setShortcut(Qt::CTRL + Qt::Key_U);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleAudioSettings()));
    m_shortcutsCollection->addAction("show_audio_settings",action);

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

    //Add Info for Selected MediaItems
    action = new KAction(KIcon("document-save"), i18n("Save selected info"), this);
    connect(action, SIGNAL(triggered()), m_application->infoManager(), SLOT(addSelectedItemsInfo()));
    m_othersCollection->addAction("add_selected_info", action);

    //Remove Info for Selected MediaItems
    action = new KAction(KIcon("trash-empty"), i18n("Remove selected info"), this);
    connect(action, SIGNAL(triggered()), m_application->infoManager(), SLOT(removeSelectedItemsInfo()));
    m_othersCollection->addAction("remove_selected_info", action);

    //Refresh Media View
    action = new KAction(KIcon("view-refresh"), i18n("Refresh"), this);
    action->setShortcut(Qt::Key_F5);
    connect(action, SIGNAL(triggered()), this, SLOT(mediaViewRefresh()));
    m_shortcutsCollection->addAction("reload", action);

    //Select all items
    action = new KAction(KIcon("edit-select-all"), i18n("Select All"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_A);
    connect(action, SIGNAL(triggered()), this, SLOT(selectAll()));
    m_parent->addAction(action);
    m_othersCollection->addAction("select_all", action);

    //Remove selected from playlist
    action = new KAction(i18n("Remove from list"), this);
    connect(action, SIGNAL(triggered()), m_application->savedListsManager(), SLOT(removeSelected()));
    m_othersCollection->addAction("remove_from_list", action);

    //Add temporary audio streams in the playlist to the MediaList "Audio Streams"
    action = new KAction(KIcon("list-add"), i18n("Add to \"Audio Streams\""), this);
    connect(action, SIGNAL(triggered()), this, SLOT(addTemporaryAudioStreams()));
    m_othersCollection->addAction("add_temp_audio_streams", action);

    //Add selected to saved audio list
    m_addToAudioSavedList = new QMenu(i18n("Add to list"), m_parent);
    connect(m_addToAudioSavedList, SIGNAL(triggered(QAction *)), this, SLOT(addToSavedAudioList(QAction *)));

    //Add selected to saved video list
    m_addToVideoSavedList = new QMenu(i18n("Add to list "), m_parent);
    connect(m_addToVideoSavedList, SIGNAL(triggered(QAction *)), this, SLOT(addToSavedVideoList(QAction *)));

    //Add a new audio list
    action = new KAction(KIcon("list-add"), i18n("New list"), m_parent);
    connect(action, SIGNAL(triggered()), m_application->savedListsManager(), SLOT(showAudioListSave()));
    m_othersCollection->addAction("new_audio_list", action);

    //Add a new video list
    action = new KAction(KIcon("list-add"), i18n("New list"), m_parent);
    connect(action, SIGNAL(triggered()), m_application->savedListsManager(), SLOT(showVideoListSave()));
    m_othersCollection->addAction("new_video_list", action);

    //Show Items
    action = new KAction(KIcon("bangarang-category-browse"), i18n("Show items"), m_parent);
    connect(action, SIGNAL(triggered()), this, SLOT(loadSelectedSources()));
    m_othersCollection->addAction("show_items", action);

    //Show Now Playing Info
    action = new KAction(KIcon("help-about"), i18n("Show information"), m_parent);
    connect(action, SIGNAL(triggered()), this, SLOT(showInfoForNowPlaying()));
    m_othersCollection->addAction("show_now_playing_info", action);

    //Show Info View
    action = new KAction(KIcon("help-about"), i18n("Show Info View"), m_parent);
    action->setShortcut(Qt::CTRL + Qt::Key_I);
    connect(action, SIGNAL(triggered()), m_application->infoManager(), SLOT(toggleInfoView()));
    m_parent->addAction(action);
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

    //Edit Shortcuts
    action = new KAction(KIcon("configure-shortcuts"), i18n("Show shortcuts editor"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(toggleShortcutsEditor()));
    connect(ui->cancelEditShortcuts, SIGNAL(clicked()), this, SLOT(cancelShortcuts()));
    connect(ui->saveShortcuts, SIGNAL(clicked()), this, SLOT(saveShortcuts()));
    m_shortcutsCollection->addAction("show_shortcuts_editor", action);
    
    //Update Ontologies
    action = new KAction(KIcon("system-run"), i18n("Update ontologies..."), this);
    connect(action, SIGNAL(triggered()), this, SLOT(updateOntologies()));
    m_shortcutsCollection->addAction("update_ontologies", action);

    //Hide in system tray
    action = new KAction(KIcon("view-close"), i18n("Hide in system tray"), this);
    connect(action, SIGNAL(triggered()), m_application->statusNotifierItem(), SLOT(activate()));
    m_shortcutsCollection->addAction("hide_in_system_tray", action);
    
    //set up the shortcuts collection
    m_shortcutsCollection->readSettings(&m_shortcutsConfig);
    ui->shortcutsEditor->addCollection(m_shortcutsCollection);

    /*Set up other variables */
    m_nowPlayingContextMenu = new QMenu(m_parent);
    m_infoMenu = new QMenu(i18n("Manage info"), m_parent);
    
    //controls always visible at startup
    m_controlsVisible = true;
    //filter ain't
    m_playlistRestoreFilter = "";
    m_mediaListRestoreFilter = "";
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
    KHelpMenu * helpMenu = new KHelpMenu(m_parent, m_application->aboutData(), false);
    helpMenu->menu();
    
    updateSavedListsMenus();
    
    m_contextMenuSource = menuSource;
    
    QMenu *menu = new QMenu(m_parent);
    QString type;
    bool selection = false;
    bool isMedia = false;
    bool isFeed = false;
    bool isCategory = false;
    bool isInPlaylist = false;
    QList<MediaItem> selectedItems = selectedMediaItems();
    if (selectedItems.count() > 0) {
        type = selectedItems.at(0).type;
        selection = true;
        isMedia = Utilities::isMedia(type);
        isCategory = Utilities::isCategory(type);
        isFeed = Utilities::isFeed(selectedItems.at(0).fields["categoryType"].toString());
        for (int i = 0; i < selectedItems.count(); i++) {
            if (m_application->playlist()->isInPlaylist(selectedItems.at(i))) {
                isInPlaylist = true;
                break;
            }
        }
    }
    
    //Playlist/Playback actions
    if (isMedia || isCategory) {
        if (selection && !isInPlaylist) {
            menu->addAction(action("add_to_playlist"));
            if (isMedia && (m_application->playlist()->mediaObject()->state() == Phonon::PlayingState ||
                            m_application->playlist()->mediaObject()->state() == Phonon::PausedState)) {
                action("add_after_now_playing")->setIcon(KIcon("dialog-ok-apply"));
                action("add_after_now_playing")->setText(i18n("Add after Now Playing"));
                menu->addAction(action("add_after_now_playing"));
            }
        }
        if (selection && isMedia && isInPlaylist) {
            menu->addAction(action("remove_from_playlist"));
        }

        if (selection) {
            menu->addAction(action("play_selected"));
        }
        menu->addAction(action("play_all"));
        menu->addSeparator();
    } 
    
    //Browsing actions
    if (menuSource == MainWindow::Default || MainWindow::MediaList) {
        if (m_application->infoManager()->infoViewVisible()) {
            action("show_info")->setText(i18n("Hide Info View"));
        } else {
            action("show_info")->setText(i18n("Show Info View"));
        }
        if (menuSource == MainWindow::Default) {
            menu->addAction(action("show_info"));
            if (ui->mediaView->sourceModel()->containsPlayable()) {
                menu->addAction(action("toggle_filter"));
            }
        }
    }
    if (selection && isCategory) {
        menu->addAction(action("show_items"));
    }
    menu->addAction(action("reload"));
    menu->addSeparator();  
    
    //Saved List actions
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
        if (m_application->browsingModel()->mediaListProperties().lri.startsWith("savedlists://")) {
            menu->addAction(action("remove_from_list"));
        }
        menu->addSeparator();
    }
    menu->addMenu(infoMenu());
    if (selection && (isMedia || isFeed)) {
        menu->addSeparator();
    }

    //About menu
    if (showAbout) {
        menu->addAction(helpMenu->action(KHelpMenu::menuAboutApp));
    }
    return menu;
}

QMenu *ActionsManager::playlistViewMenu()
{
    m_contextMenuSource = MainWindow::Playlist;
    QMenu *menu = new QMenu(m_parent);
    menu->addAction(action("remove_from_playlist"));
    const QList<MediaItem> selectedItems = selectedMediaItems();
    bool showPlayAfterAction = true;
    if (selectedItems.count() == 1) {
        const MediaItem &mediaItem = selectedItems.at(0);
        if (mediaItem.url == m_application->playlist()->nowPlayingModel()->mediaItemAt(0).url) {
            showPlayAfterAction = false;
        }
    }
    if (showPlayAfterAction) {
        action("add_after_now_playing")->setIcon(KIcon("media-playback-start"));
        action("add_after_now_playing")->setText(i18n("Play after Now Playing"));
        menu->addAction(action("add_after_now_playing"));
    }
    //check for temporary audio streams
    foreach(const MediaItem &itm, selectedItems) {
        if (Utilities::isTemporaryAudioStream(itm)) {
            menu->addAction(action("add_temp_audio_streams"));
            break;
        }
    }
    return menu;
}

QMenu *ActionsManager::nowPlayingContextMenu()
{
    m_nowPlayingContextMenu->clear();
    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        NowPlayingDelegate *delegate = (NowPlayingDelegate *)ui->nowPlayingView->itemDelegate();
        if (!delegate->showingInfo() ||
            (ui->videoFrame->isVisible() && m_application->mainWindow()->videoSize() == MainWindow::Normal)) {
            action("show_now_playing_info")->setText(i18n("Show information"));
            action("show_now_playing_info")->setIcon(KIcon("help-about"));
        } else if (delegate->showingInfo()) {
            action("show_now_playing_info")->setText(i18n("Hide information"));
            action("show_now_playing_info")->setIcon(KIcon("help-about"));
        }
        m_nowPlayingContextMenu->addAction(action("show_now_playing_info"));
        m_nowPlayingContextMenu->addSeparator();
    }
    if (m_application->playlist()->mediaObject()->state() == Phonon::PlayingState ||
        m_application->playlist()->mediaObject()->state() == Phonon::PausedState) {
        m_nowPlayingContextMenu->addAction(action("play_previous"));
        if (m_application->playlist()->mediaObject()->state() == Phonon::PlayingState) {
            m_nowPlayingContextMenu->addAction(action("pause"));
        } else {
            m_nowPlayingContextMenu->addAction(action("play"));
        }
    } else {
        m_nowPlayingContextMenu->addAction(action("play"));
    }
    m_nowPlayingContextMenu->addAction(action("play_next"));
    m_nowPlayingContextMenu->addSeparator();
    if (m_application->playlist()->mediaObject()->hasVideo()) {
        m_nowPlayingContextMenu->addAction(action("show_video_settings"));
    }
    m_nowPlayingContextMenu->addAction(action("toggle_controls"));
    return m_nowPlayingContextMenu;
}

KMenu *ActionsManager::nowPlayingMenu()
{
    KHelpMenu * helpMenu = new KHelpMenu(m_parent, m_application->aboutData(), false);
    helpMenu->menu();
    
    m_nowPlayingMenu = new KMenu(m_parent);
    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        MediaItem nowPlayingItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
        if (Utilities::isMedia(nowPlayingItem.type)) {
            NowPlayingDelegate *delegate = (NowPlayingDelegate *)ui->nowPlayingView->itemDelegate();
            if (!delegate->showingInfo() ||
                (ui->videoFrame->isVisible() && m_application->mainWindow()->videoSize() == MainWindow::Normal)) {
                action("show_now_playing_info")->setText(i18n("Show information"));
                action("show_now_playing_info")->setIcon(KIcon("help-about"));
            } else if (delegate->showingInfo()) {
                action("show_now_playing_info")->setText(i18n("Hide information"));
                action("show_now_playing_info")->setIcon(KIcon("help-about"));
            }
            m_nowPlayingMenu->addAction(action("show_now_playing_info"));
            m_nowPlayingMenu->addSeparator();
        }
    }
    if (ui->contextStackHolder->isVisible() && ui->contextStack->currentIndex() == 0) {
        m_nowPlayingMenu->addAction(action("toggle_filter"));
        m_nowPlayingMenu->addSeparator();
    }
    m_nowPlayingMenu->addAction(action("hide_in_system_tray"));
    if (!m_parent->isFullScreen()) {
        m_nowPlayingMenu->addAction(action("toggle_controls"));
    }
    m_nowPlayingMenu->addAction(action("show_video_settings"));
    m_nowPlayingMenu->addAction(action("show_audio_settings"));
    if (!m_application->isTouchEnabled()) {
        m_nowPlayingMenu->addAction(action("show_shortcuts_editor"));
    }
    m_nowPlayingMenu->addSeparator();
    m_nowPlayingMenu->addAction(helpMenu->action(KHelpMenu::menuAboutApp));
    return m_nowPlayingMenu;
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
    Phonon::State state = m_application->playlist()->mediaObject()->state();
    action("add_bookmark")->setEnabled((state == Phonon::PlayingState || state == Phonon::PausedState));
    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        QString url = m_application->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        Phonon::MediaController *mctrl = m_application->playlist()->mediaController();
        int noChaps = mctrl->availableChapters();
        if (noChaps > 1) {
            for (int i = 0; i < noChaps; i++) {
                QString title = i18n("Chapter %1", i);
                QAction * ac = m_bookmarksMenu->addAction(KIcon("media-optical-dvd"), title);
                ac->setData(QString("Chapter:%1").arg(i));
            }
            m_bookmarksMenu->addSeparator();
        }
        //real bookmarks
        m_bookmarksMenu->addAction(action("add_bookmark"));
        QStringList bookmarks = m_application->bookmarksManager()->bookmarks(url);
        for (int i = 0; i < bookmarks.count(); i++) {
            QString bookmarkName = m_application->bookmarksManager()->bookmarkName(bookmarks.at(i));
            qint64 bookmarkTime = m_application->bookmarksManager()->bookmarkTime(bookmarks.at(i));
            QTime bmTime(0, (bookmarkTime / 60000) % 60, (bookmarkTime / 1000) % 60);
            QString bookmarkTitle = QString("%1 (%2)").arg(bookmarkName).arg(bmTime.toString(QString("m:ss")));
            QAction * action = m_bookmarksMenu->addAction(KIcon("bookmarks-organize"), bookmarkTitle);
            action->setData(QString("Activate:%1").arg(bookmarks.at(i)));
            action = m_removeBookmarksMenu->addAction(KIcon("list-remove"), bookmarkTitle);
            action->setData(QString("Remove:%1").arg(bookmarks.at(i)));
        }
        if (bookmarks.count() > 0) {
            m_bookmarksMenu->addMenu(m_removeBookmarksMenu);
        }
    }
    return m_bookmarksMenu;
}

QMenu * ActionsManager::infoMenu()
{
    m_infoMenu->clear();
    QList<MediaItem> selectedItems = selectedMediaItems();
    if (selectedItems.count() > 0) {
        //m_infoMenu->addAction(action("add_selected_info"));
        m_infoMenu->addAction(action("remove_selected_info"));
    }
    m_infoMenu->addAction(action("update_ontologies"));
    return m_infoMenu;   
}

//------------------
//-- Action SLOTS --
//------------------
void ActionsManager::mediaViewRefresh()
{
    //Clear image cache
    if (m_application->browsingModel()->rowCount() > 0 ){
        MediaItem mediaItem = m_application->browsingModel()->mediaItemAt(0);
        Utilities::clearSubTypesFromImageCache(mediaItem.subType());
    }
    m_application->browsingModel()->reload();
}

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
    if ((!m_parent->isFullScreen() || m_application->isTouchEnabled()) && (m_parent->currentMainWidget() == MainWindow::MainNowPlaying)) {
        if (m_controlsVisible) {
            ui->widgetSet->setVisible(false);
            toggle->setIcon(KIcon("layer-visible-on"));
            toggle->setText(i18n("Show Controls"));
        } else {
            ui->widgetSet->setVisible(true);
            toggle->setIcon(KIcon("layer-visible-off"));
            toggle->setText(i18n("Hide Controls"));
        }
        m_controlsVisible = !m_controlsVisible;
    }
}

void ActionsManager::toggleVideoSettings()
{
    if(ui->contextStack->currentIndex() != 2 || !ui->contextStackHolder->isVisible()) {
        m_contextStackWasVisible = ui->contextStackHolder->isVisible();
        m_previousContextStackIndex = ui->contextStack->currentIndex();
        ui->contextStack->setCurrentIndex(2);
        ui->contextStackHolder->setVisible(true);
        m_application->videoSettings()->updateSubtitleCombo();
        action("show_video_settings")->setText(i18n("Hide video settings"));
    } else {
        if (m_contextStackWasVisible && m_previousContextStackIndex == 0) { //if the playlist was showing, show it
            ui->contextStack->setCurrentIndex(m_previousContextStackIndex);
            ui->contextStackHolder->setVisible(true);
        } else {
            ui->contextStackHolder->setVisible(false);
        }
        action("show_video_settings")->setText(i18n("Show video settings"));
    }
    //All other actions for the contextStack setting should now say "Show"
    action("show_audio_settings")->setText(i18n("Show audio settings"));
    action("show_shortcuts_editor")->setText(i18n("Show shortcuts editor"));
}

void ActionsManager::toggleAudioSettings()
{
    if(ui->contextStack->currentIndex() != 1 || !ui->contextStackHolder->isVisible()) {
        m_contextStackWasVisible = ui->contextStackHolder->isVisible();
        m_previousContextStackIndex = ui->contextStack->currentIndex();
        ui->contextStack->setCurrentIndex(1);
        ui->contextStackHolder->setVisible(true);
        m_application->audioSettings()->updateAudioChannelCombo();
        action("show_audio_settings")->setText(i18n("Hide audio settings"));
    } else {
        if (m_contextStackWasVisible && m_previousContextStackIndex == 0) { //if the playlist was showing, show it
            ui->contextStack->setCurrentIndex(m_previousContextStackIndex);
            ui->contextStackHolder->setVisible(true);
        } else {
            ui->contextStackHolder->setVisible(false);
        }
        action("show_audio_settings")->setText(i18n("Show audio settings"));
    }
    //All other actions for the contextStack setting should now say "Show"
    action("show_video_settings")->setText(i18n("Show video settings"));
    action("show_shortcuts_editor")->setText(i18n("Show shortcuts editor"));
}

void ActionsManager::cancelFSHC()
{
    if (m_parent->currentFilterProxyLine()->lineEdit()->hasFocus()) {
        toggleFilter();
        return;
    }

    MainWindow::MainWidget cmw = m_parent->currentMainWidget();
    if (cmw == MainWindow::MainNowPlaying) {
        if (ui->playlistView->hasFocus() && ui->playlistView->selectionModel()->hasSelection()) {
            ui->playlistView->clearSelection();
            return;
        } else if (!m_controlsVisible) {
            toggleControls();
            return;
        }


    } else if (cmw == MainWindow::MainMediaList) {
        if (ui->mediaView->hasFocus() && ui->mediaView->selectionModel()->hasSelection()) {
            ui->mediaView->clearSelection();
            return;
        }
    }
    
    
    if (m_parent->isFullScreen()) {
        fullScreenToggle();
    }
}

void ActionsManager::toggleShortcutsEditor()
{
    if(ui->contextStack->currentIndex() != 3 || !ui->contextStackHolder->isVisible()) {
        m_contextStackWasVisible = ui->contextStackHolder->isVisible();
        m_previousContextStackIndex = ui->contextStack->currentIndex();
        ui->contextStack->setCurrentIndex(3);
        ui->contextStackHolder->setVisible(true);
        action("show_shortcuts_editor")->setText(i18n("Hide shortcuts editor"));
    } else {
        if (m_contextStackWasVisible && m_previousContextStackIndex == 0) { //if the playlist was showing, show it
            ui->contextStack->setCurrentIndex(m_previousContextStackIndex);
            ui->contextStackHolder->setVisible(true);
        } else {
            ui->contextStackHolder->setVisible(false);
        }
        action("show_shortcuts_editor")->setText(i18n("Show shortcuts editor"));
    }
    //All other actions for the contextStack setting should now say "Show"
    action("show_video_settings")->setText(i18n("Show video settings"));
    action("show_audio_settings")->setText(i18n("Show audio settings"));
}

void ActionsManager::saveShortcuts()
{
    ui->shortcutsEditor->writeConfiguration(&m_shortcutsConfig);
    ui->shortcutsEditor->commit();
    toggleShortcutsEditor();
}

void ActionsManager::cancelShortcuts()
{
    //user canceld, it should be undone what had been edited
    ui->shortcutsEditor->undoChanges();
    toggleShortcutsEditor();
}

void ActionsManager::simplePlayPause()
{
    if (m_application->playlist()->mediaObject()->state() == Phonon::PlayingState) {
        m_application->playlist()->mediaObject()->pause();
    } else {
        smartPlay();
    }
}

void ActionsManager::smartPlay()
{
    if (m_application->playlist()->mediaObject()->state() == Phonon::PausedState) {
        m_application->playlist()->mediaObject()->play();
    } else if (m_application->playlist()->mediaObject()->state() != Phonon::PlayingState) {
        m_application->playlist()->start();
    }
}

void ActionsManager::muteAudio()
{
    bool muted = m_application->audioOutput()->isMuted();
    m_application->audioOutput()->setMuted(!muted);
    if (m_application->audioOutput()->isMuted()) {
        action("mute")->setText(i18n("Restore Volume"));
        action("mute")->setIcon(KIcon("speaker"));
    } else {
        action("mute")->setText(i18n("Mute"));
        action("mute")->setIcon(KIcon("dialog-cancel"));
    }
}

void ActionsManager::addSelectedToPlaylistSlot()
{
    QList<MediaItem> mediaList = selectedMediaItems();
    MediaItemModel *plmod = m_application->playlist()->playlistModel();
    foreach (MediaItem item, mediaList) {
        if (Utilities::isMedia(item.type)) {
            if (plmod->rowOfUrl(item.url) >= 0)
                continue;
        } else if ( !Utilities::isCategory(item.type) )
            continue;
        m_application->playlist()->addMediaItem(item);
    }
}

void ActionsManager::addAfterNowPlaying()
{
    //Get selected mediaitems and play
    QList<MediaItem> mediaList = selectedMediaItems();

    //Insert into Playlist
    if (m_application->playlist()->queueModel()->rowCount() <= 1) {
        m_application->playlist()->addMediaList(mediaList);
    } else {
        m_application->playlist()->insertMediaListAt(1, Playlist::QueueModel, mediaList);
    }
}

void ActionsManager::removeSelectedFromPlaylistSlot()
{
    QList<MediaItem> mediaList = selectedMediaItems();
    m_application->playlist()->removeMediaListItems(mediaList);
}

void ActionsManager::removePlaylistSelectionFromPlaylistSlot()
{
  m_contextMenuSource = MainWindow::Playlist;
  removeSelectedFromPlaylistSlot();
}

void ActionsManager::updateSavedListsMenus()
{
    m_addToAudioSavedList->clear();
    m_addToAudioSavedList->addAction(action("new_audio_list"));
    QStringList audioListNames = m_application->savedListsManager()->savedListNames("Audio");
    for (int i = 0; i < audioListNames.count(); i++) {
        if (!((audioListNames.at(i) == m_application->browsingModel()->mediaListProperties().name)
            && (m_application->browsingModel()->mediaListProperties().lri.startsWith("savedlists://")))) { 
            KAction * addToSavedList = new KAction(KIcon("view-list-text"), audioListNames.at(i), m_addToAudioSavedList);
            addToSavedList->setData(audioListNames.at(i));
            m_addToAudioSavedList->addAction(addToSavedList);
        }
    }
    
    m_addToVideoSavedList->clear();
    m_addToVideoSavedList->addAction(action("new_video_list"));
    QStringList videoListNames = m_application->savedListsManager()->savedListNames("Video");
    for (int i = 0; i < videoListNames.count(); i++) {
        if (!((videoListNames.at(i) == m_application->browsingModel()->mediaListProperties().name)
            && (m_application->browsingModel()->mediaListProperties().lri.startsWith("savedlists://")))) { 
            KAction * addToSavedList = new KAction(KIcon("view-list-text"), videoListNames.at(i), m_addToVideoSavedList);
            addToSavedList->setData(videoListNames.at(i));
            m_addToVideoSavedList->addAction(addToSavedList);
        }
    }
}

void ActionsManager::removeSelectedItemsInfoSlot()
{
    QList<MediaItem> mediaList = selectedMediaItems();
    m_application->browsingModel()->removeSourceInfo(mediaList);
}

void ActionsManager::playSelectedSlot()
{
    //Get selected mediaitems and play
    QList<MediaItem> mediaList = selectedMediaItems();
    m_application->playlist()->playMediaList(mediaList);
    
    // Show Now Playing page
    m_parent->switchMainWidget(MainWindow::MainNowPlaying);   
}

void ActionsManager::playAllSlot()
{
    //Play all media items in the media list view
    QList<MediaItem> mediaList;
    for (int i = 0; i < ui->mediaView->model()->rowCount(); ++i) {
        QModelIndex proxyIndex = ui->mediaView->model()->index(i, 0);
        QModelIndex index = ui->mediaView->filterProxyModel()->mapToSource(proxyIndex);
        mediaList.append(m_application->browsingModel()->mediaItemAt(index.row()));
    }

    m_application->playlist()->playMediaList(mediaList);

    // Show Now Playing page
    m_parent->switchMainWidget(MainWindow::MainNowPlaying);
}

void ActionsManager::addToSavedAudioList(QAction *addAction)
{
    //Get list of selected items to add
    QList<MediaItem> mediaList = selectedMediaItems();
    
    //Add to saved list
    if (mediaList.count() > 0) {
        QString audioListName = addAction->data().toString();
        m_application->savedListsManager()->saveMediaList(mediaList, audioListName, "Audio", true);
    }
}

void ActionsManager::addToSavedVideoList(QAction *addAction)
{
    //Get list of selected items to add
    QList<MediaItem> mediaList = selectedMediaItems();
    
    //Add to saved list
    if (mediaList.count() > 0) {
        QString videoListName = addAction->data().toString();
        m_application->savedListsManager()->saveMediaList(mediaList, videoListName, "Video", true);
    }
}

void ActionsManager::loadSelectedSources()
{
    m_application->mediaListsManager()->addListToHistory();
    QList<MediaItem> mediaList;
    if (m_contextMenuSource == MainWindow::InfoBox) {
        mediaList = m_application->infoManager()->selectedInfoBoxMediaItems();
    } else {
        QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            QSortFilterProxyModel* proxyModel = ui->mediaView->filterProxyModel();
            QModelIndex sourceIndex = proxyModel->mapToSource(selectedRows.at(i));
            mediaList.append(m_application->browsingModel()->mediaItemAt(sourceIndex.row()));
        }
    }
    m_application->browsingModel()->clearMediaListData();
    m_application->browsingModel()->loadSources(mediaList);
}

void ActionsManager::showInfoForNowPlaying()
{
    if (m_application->mainWindow()->currentMainWidget() != MainWindow::MainNowPlaying) {
        return;
    }

    if (m_application->playlist()->nowPlayingModel()->rowCount() == 0) {
        return;
    }
    //FIXME: Remove this since "Hide Information" overrides this on the menu builder functions
    if (m_application->playlist()->mediaObject()->hasVideo()) {
        if (m_application->mainWindow()->videoSize() == MainWindow::Normal) {
            m_application->mainWindow()->setVideoSize(MainWindow::Mini);
            action("show_now_playing_info")->setText(i18n("Restore video size"));
            action("show_now_playing_info")->setIcon(KIcon("transform-scale"));
        } else {
            m_application->mainWindow()->setVideoSize(MainWindow::Normal);
            action("show_now_playing_info")->setText(i18n("Show information"));
            action("show_now_playing_info")->setIcon(KIcon("help-about"));
        }
    } else {
        NowPlayingDelegate * delegate = (NowPlayingDelegate *)ui->nowPlayingView->itemDelegate();
        delegate->setShowInfo(!delegate->showingInfo());
    }
}

const QList<MediaItem> ActionsManager::selectedMediaItems()
{
    QList<MediaItem> mediaList;
    QTreeView *view = (m_contextMenuSource == MainWindow::Playlist) ?
        (QTreeView *) ui->playlistView : (QTreeView *) ui->mediaView;
    MediaSortFilterProxyModel * proxy = (MediaSortFilterProxyModel *) view->model();
    MediaItemModel *model = (MediaItemModel *) proxy->sourceModel();
    
    if (m_contextMenuSource == MainWindow::InfoBox ||
        m_contextMenuSource == MainWindow::Default) {
        mediaList = m_application->infoManager()->selectedInfoBoxMediaItems();
    }
    if (m_contextMenuSource == MainWindow::MediaList ||
        m_contextMenuSource == MainWindow::Playlist ||
        (m_contextMenuSource == MainWindow::Default && mediaList.count() == 0)
        ) {
        QModelIndexList selection = view->selectionModel()->selectedIndexes();
        for (int i = 0; i < selection.count(); ++i) {
            QModelIndex _index = selection.at(i);
            QModelIndex index;
            index = proxy->mapToSource(_index);
            if (index.column() == 0) {
                mediaList.append(model->mediaItemAt(index.row()));
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
    m_application->nowPlayingManager()->setShowRemainingTime(!m_application->nowPlayingManager()->showingRemainingTime());
    if (m_application->nowPlayingManager()->showingRemainingTime()) {
        action("toggle_show_remaining_time")->setText(i18n("Show elapsed time"));
    } else {
        action("toggle_show_remaining_time")->setText(i18n("Show remaining time"));
    }
}

void ActionsManager::addBookmarkSlot()
{
    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        QString nowPlayingUrl = m_application->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        qint64 time = m_application->playlist()->mediaObject()->currentTime();
        int newBookmarkIndex = m_application->bookmarksManager()->bookmarks(nowPlayingUrl).count() + 1;
        QString name = i18n("Bookmark-%1", newBookmarkIndex);
        m_application->bookmarksManager()->addBookmark(nowPlayingUrl, name, time);
        if (m_application->bookmarksManager()->bookmarks(nowPlayingUrl).count() > 0) {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        }
    }
}

void ActionsManager::activateBookmark(QAction *bookmarkAction)
{
    QString bookmark = bookmarkAction->data().toString();
    if (bookmark.isEmpty())
        return;
    if (bookmark.startsWith("Activate:")) {
        bookmark.remove(0,9);
        qint64 time = m_application->bookmarksManager()->bookmarkTime(bookmark);
        m_application->playlist()->mediaObject()->seek(time);
    } else if (bookmark.startsWith("Chapter:")) {
        int no = bookmark.remove(0,8).toInt();
        m_application->playlist()->mediaController()->setCurrentChapter(no);
    }
}

void ActionsManager::removeBookmark(QAction *bookmarkAction)
{
    QString bookmark = bookmarkAction->data().toString();
    if (!bookmark.isEmpty() && bookmark.startsWith("Remove:")) {
        bookmark.remove(0,7);
        QString nowPlayingUrl = m_application->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        m_application->bookmarksManager()->removeBookmark(nowPlayingUrl, bookmark);
        if (m_application->bookmarksManager()->bookmarks(nowPlayingUrl).count() == 0) {
            ui->seekTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }
    }
}

void ActionsManager::toggleFilter()
{
    QFrame *frame = m_application->mainWindow()->currentFilterFrame();
    KFilterProxySearchLine *filter = m_application->mainWindow()->currentFilterProxyLine();
    bool visible = frame->isVisible();
    QString *restore;
    if (m_parent->currentMainWidget() == MainWindow::MainMediaList) {
        restore = &m_mediaListRestoreFilter;
        if ( !visible && !ui->mediaView->sourceModel()->containsPlayable() )
            return;
    } else {
        restore = &m_playlistRestoreFilter;
    }
    frame->setVisible(!visible);
    if(!visible) {
        if(!restore->isEmpty()) {
           filter->setText( *restore );
           restore->clear();
        }
        filter->lineEdit()->setFocus(); //the user can start immediately to search
        filter->lineEdit()->selectAll();
    } else {
        if(!filter->lineEdit()->text().isEmpty()) {
            *restore = filter->lineEdit()->text();
            filter->setText( "" );
        }
    }
    updateToggleFilterText();
}

void ActionsManager::updateOntologies()
{
    KGuiItem updateOntologies;
    updateOntologies.setText(i18n("Update Ontologies"));
    KGuiItem cancel;
    cancel.setText(i18n("Cancel"));
    if (KMessageBox::questionYesNo(m_parent, i18n("Updating ontologies ensures that media information is stored in a way that makes it most accessible to other desktop applications.  This is only necessary if you recently upgraded Bangarang or your KDE software compilation. <br><br>This may take several minutes."), QString(), updateOntologies, cancel) == KMessageBox::Yes) {
        QDialog *dialog = new QDialog(m_parent, Qt::Dialog);
        dialog->setModal(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setWindowTitle(i18n("Update Ontologies"));
        QHBoxLayout *layout = new QHBoxLayout;
        QLabel * label = new QLabel;
        QPushButton * stopButton = new QPushButton;
        stopButton->setText(i18n("Stop"));
        stopButton->setIcon(KIcon("process-stop"));
        QPushButton * closeButton = new QPushButton;
        closeButton->setText(i18n("Close"));
        closeButton->setIcon(KIcon("dialog-close"));
        layout->addWidget(label);
        layout->addWidget(stopButton);
        layout->addWidget(closeButton);
        dialog->setLayout(layout);
        closeButton->setVisible(false);
        OntologyUpdater *updater = new OntologyUpdater(this);
        connect(updater, SIGNAL(infoMessage(QString)), label, SLOT(setText(QString)));
        connect(stopButton, SIGNAL(clicked()), updater, SLOT(stopUpdate()));
        connect(updater, SIGNAL(done()), stopButton, SLOT(hide()));
        connect(updater, SIGNAL(done()), closeButton, SLOT(show()));
        connect(closeButton, SIGNAL(clicked()), dialog, SLOT(hide()));
        connect(dialog, SIGNAL(rejected()), updater, SLOT(stopUpdate()));
        dialog->show();
        updater->start();
    }
}

void ActionsManager::updateToggleFilterText()
{
    QString txt;
    if (m_parent->currentMainWidget() == MainWindow::MainMediaList) {
        if (ui->mediaListFilter->isVisible())
            txt = i18n("Hide filter");
        else
            txt = i18n("Show filter");
    } else {
        if (ui->playlistFilter->isVisible())
            txt = i18n("Hide filter");
        else
            txt = i18n("Show filter");
    }
    action("toggle_filter")->setText(txt);
}

void ActionsManager::addTemporaryAudioStreams()
{
    QList<MediaItem> selected = selectedMediaItems();
    QList<MediaItem> tempStreams;
    foreach(const MediaItem &item, selected) {
        if (Utilities::isTemporaryAudioStream(item)) {
            tempStreams << item;
        }
    }
    if (tempStreams.count() <= 0)
        return;
    m_application->playlist()->playlistModel()->updateSourceInfo(tempStreams, true);
}

void ActionsManager::toggleInfoView()
{
    m_application->infoManager()->toggleInfoView();
    if (m_application->infoManager()->infoViewVisible()) {
        action("show_info")->setText(i18n("Hide Info View"));
    } else {
        action("show_info")->setText(i18n("Show Info View"));
    }
}

void ActionsManager::selectAll()
{
    if (ui->mediaView->hasFocus()) {
        ui->mediaView->selectAll();
    } else if (ui->playlistView->hasFocus()) {
        ui->playlistView->selectAll();
    }
}

void ActionsManager::handleAltS()
{
    if (m_application->mainWindow()->currentMainWidget() == MainWindow::MainMediaList) {
        ui->Filter->setFocus();
    } else {
        m_application->nowPlayingManager()->toggleShuffle();
    }
}

void ActionsManager::handleAltLeft()
{
    if (m_application->mainWindow()->currentMainWidget() == MainWindow::MainMediaList) {
        m_application->mediaListsManager()->loadPreviousList();
    } else {
        m_application->mediaObject()->seek(-5000);
    }
}

void ActionsManager::handleCtrlM()
{
    if (m_application->mainWindow()->currentMainWidget() == MainWindow::MainMediaList) {
        m_application->mediaListsManager()->showMenu();
    } else {
        m_application->nowPlayingManager()->showMenu();
    }
}

void ActionsManager::addShortcuts()
{
    //Show Audio Lists Action
    KAction *action = new KAction(i18n("Show Audio Lists"), this);
    action->setShortcut(Qt::ALT + Qt::Key_A);
    connect(action, SIGNAL(triggered()), m_application->mediaListsManager(), SLOT(selectAudioList()));
    ui->collectionPage->addAction(action);

    //Show Video Lists Action
    action = new KAction(i18n("Show Video Lists"), this);
    action->setShortcut(Qt::ALT + Qt::Key_V);
    connect(action, SIGNAL(triggered()), m_application->mediaListsManager(), SLOT(selectVideoList()));
    ui->collectionPage->addAction(action);

    //Search/Shuffle Action
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_S);
    connect(action, SIGNAL(triggered()), this, SLOT(handleAltS()));
    m_application->mainWindow()->addAction(action);

    //Open Previous List/ Seek backward action
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_Left);
    connect(action, SIGNAL(triggered()), this, SLOT(handleAltLeft()));
    m_application->mainWindow()->addAction(action);

    //Open menu
    action = new KAction(QString(), this);
    action->setShortcut(Qt::CTRL + Qt::Key_M);
    connect(action, SIGNAL(triggered()), this, SLOT(handleCtrlM()));
    m_application->mainWindow()->addAction(action);

    //Show Media Lists/Now Playing
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_N);
    connect(action, SIGNAL(triggered()), m_application->mainWindow(), SLOT(toggleMainWidget()));
    m_application->mainWindow()->addAction(action);
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_M);
    connect(action, SIGNAL(triggered()), m_application->mainWindow(), SLOT(toggleMainWidget()));
    m_application->mainWindow()->addAction(action);

    //Show Time/Bookmark menu
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_T);
    connect(action, SIGNAL(triggered()), m_application->bookmarksManager(), SLOT(showBookmarksMenu()));
    ui->nowPlayingPage->addAction(action);
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_B);
    connect(action, SIGNAL(triggered()), m_application->bookmarksManager(), SLOT(showBookmarksMenu()));
    ui->nowPlayingPage->addAction(action);

    //Show/Hide playlist
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_P);
    connect(action, SIGNAL(triggered()), m_application->nowPlayingManager(), SLOT(togglePlaylist()));
    ui->nowPlayingPage->addAction(action);

    //View upcoming
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_U);
    connect(action, SIGNAL(triggered()), m_application->nowPlayingManager(), SLOT(toggleQueue()));
    ui->nowPlayingPage->addAction(action);

    //Clear Playlist
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_C);
    connect(action, SIGNAL(triggered()), m_application->nowPlayingManager(), SLOT(clearPlaylist()));
    ui->nowPlayingPage->addAction(action);

    //Toggle Repeat
    action = new KAction(QString(), this);
    action->setShortcut(Qt::ALT + Qt::Key_R);
    connect(action, SIGNAL(triggered()), m_application->nowPlayingManager(), SLOT(toggleRepeat()));
    ui->nowPlayingPage->addAction(action);

    //Show Now Playing Info
    action = new KAction(KIcon("help-about"), i18n("Show information"), m_parent);
    action->setShortcut(Qt::ALT + Qt::Key_I);
    connect(action, SIGNAL(triggered()), this, SLOT(showInfoForNowPlaying()));
    ui->nowPlayingPage->addAction(action);

    //Play/Pause Action
    action = new KAction(QString(), this);
    action->setShortcut(Qt::Key_Space);
    connect(action, SIGNAL(triggered()), this, SLOT(simplePlayPause()));
    ui->nowPlayingPage->addAction(action);

    //Play Next
    action = new KAction(QString(), this);
    action->setShortcut(Qt::Key_Right);
    connect(action, SIGNAL(triggered()), m_application->playlist(), SLOT(playNext()));
    ui->nowPlayingPage->addAction(action);

    //Play Previous
    action = new KAction(QString(), this);
    action->setShortcut(Qt::Key_Left);
    connect(action, SIGNAL(triggered()), m_application->playlist(), SLOT(playPrevious()));
    ui->nowPlayingPage->addAction(action);

}
