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
#include "videosettings.h"

#include <KStandardDirs>
#include <KMessageBox>
#include <KHelpMenu>
#include <KMenu>
#include <KDebug>
#include <QFile>

ActionsManager::ActionsManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
    
    m_actionCollection = new KActionCollection(this);
    m_contextMenuSource = MainWindow::Default;
    
    //Add standard quit shortcut
    m_quit = new QAction(this);
    m_quit->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(m_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    m_parent->addAction(m_quit);
    m_actionCollection->addAction(i18n("Quit"), m_quit);
    
    //Play/Pause Action
    m_playPause = new QAction(KIcon("media-playback-start"), i18n("Play/Pause"), this);
    m_playPause->setShortcut(Qt::Key_Space);
    connect(m_playPause, SIGNAL(triggered()), this, SLOT(simplePlayPause()));
    m_parent->addAction(m_playPause);
    
    //Play Action
    m_play = new QAction(KIcon("media-playback-start"), i18n("Play"), this);
    connect(m_play, SIGNAL(triggered()), this, SLOT(smartPlay()));
    
    //Pause Action
    m_pause = new QAction(KIcon("media-playback-pause"), i18n("Pause"), this);
    connect(m_pause, SIGNAL(triggered()), m_parent->playlist()->mediaObject(), SLOT(pause()));
    
    //Play Next
    m_playNext = new QAction(KIcon("media-skip-forward"), i18n("Play next"), this);
    m_playNext->setShortcut(Qt::Key_Right);
    connect(m_playNext, SIGNAL(triggered()), m_parent->playlist(), SLOT(playNext()));
    m_parent->addAction(m_playNext);

    //Play Previous
    m_playPrevious = new QAction(KIcon("media-skip-backward"), i18n("Play previous"), this);
    m_playPrevious->setShortcut(Qt::Key_Left);
    connect(m_playPrevious, SIGNAL(triggered()), m_parent->playlist(), SLOT(playPrevious()));
    m_parent->addAction(m_playPrevious);
    
    //Mute
    m_mute = new QAction(this);
    m_mute->setShortcut(Qt::Key_M);
    connect(m_mute, SIGNAL(triggered()), this, SLOT(muteAudio()));
    m_parent->addAction(m_mute);
    
    //Play All Action
    m_playAllAction = new QAction(KIcon("media-playback-start"), i18n("Play all"), this);
    connect(m_playAllAction, SIGNAL(triggered()), this, SLOT(playAllSlot()));
    m_actionCollection->addAction(i18n("Play All"), m_playAllAction);
    
    //Play Selected Action
    m_playSelectedAction = new QAction(KIcon("media-playback-start"), i18n("Play selected"), this);
    connect(m_playSelectedAction, SIGNAL(triggered()), this, SLOT(playSelectedSlot()));
    m_actionCollection->addAction(i18n("Play Selected"), m_playSelectedAction);
    
    //Add Selected To Playlist Action
    m_addSelectedToPlayListAction = new QAction(KIcon("mail-mark-notjunk"), i18n("Add to playlist"), this);
    connect(m_addSelectedToPlayListAction, SIGNAL(triggered()), this, SLOT(addSelectedToPlaylistSlot()));  
    m_actionCollection->addAction(i18n("Add to playlist"), m_addSelectedToPlayListAction);
    
    //Remove Selected From Playlist Action
    m_removeSelectedToPlayListAction = new QAction(KIcon(), i18n("Remove from playlist"), this);
    connect(m_removeSelectedToPlayListAction, SIGNAL(triggered()), this, SLOT(removeSelectedFromPlaylistSlot()));
    m_actionCollection->addAction(i18n("Remove from playlist"), m_removeSelectedToPlayListAction);
    
    //Show/Hide Controls Shortcut
    m_showHideControls = new QAction(KIcon("layer-visible-off"), i18n("Hide controls"), this);
    m_showHideControls->setShortcut(Qt::CTRL + Qt::Key_H);
    connect(m_showHideControls, SIGNAL(triggered()), this, SLOT(toggleControls()));
    m_parent->addAction(m_showHideControls);
    m_actionCollection->addAction(i18n("Hide controls"), m_showHideControls);
   
    //Show VideoSettings
    m_showVideoSettings = new QAction(KIcon("video-display"),tr("Show Video Settings"),this);
    m_showVideoSettings->setShortcut(Qt::CTRL + Qt::Key_V);
    connect(m_showVideoSettings, SIGNAL(triggered()), this, SLOT(toggleVideoSettings()));
    m_parent->addAction(m_showVideoSettings);
    m_actionCollection->addAction(i18n("Show Video Settings"),m_showVideoSettings); 
    
    //Full Screen
    m_fullScreen = new QAction(this);
    m_fullScreen->setShortcut(Qt::Key_F11);
    connect(m_fullScreen, SIGNAL(triggered()), this, SLOT(fullScreenToggle()));
    m_parent->addAction(m_fullScreen);
    m_actionCollection->addAction(i18n("Toggle fullscreen"), m_fullScreen);
    
    //Cancel FullScreen/Cancel Hide Controls
    m_cancelFullScreenHideControls = new QAction(this);
    m_cancelFullScreenHideControls->setShortcut(Qt::Key_Escape);
    connect(m_cancelFullScreenHideControls, SIGNAL(triggered()), this, SLOT(cancelFSHC()));
    m_parent->addAction(m_cancelFullScreenHideControls);

    //Remove Info for Selected MediaItems
    m_removeSelectedItemsInfo = new QAction(KIcon("edit-delete-shred"), i18n("Remove selected info"), this);
    connect(m_removeSelectedItemsInfo, SIGNAL(triggered()), m_parent->infoManager(), SLOT(removeSelectedItemsInfo()));
    m_parent->addAction(m_removeSelectedItemsInfo);

    //Refresh Media View
    m_refreshMediaView = new QAction(KIcon("view-refresh"), i18n("Refresh"), this);
    m_refreshMediaView->setShortcut(Qt::Key_F5);
    connect(m_refreshMediaView, SIGNAL(triggered()), m_parent->m_mediaItemModel, SLOT(reload()));
    m_parent->addAction(m_refreshMediaView);
    
    //Remove selected from playlist
    m_removeFromSavedList = new QAction(i18n("Remove from list"), this);
    connect(m_removeFromSavedList, SIGNAL(triggered()), m_parent->savedListsManager(), SLOT(removeSelected()));
    
    //Add selected to saved audio list
    m_addToAudioSavedList = new QMenu(i18n("Add to list"), m_parent);
    connect(m_addToAudioSavedList, SIGNAL(triggered(QAction *)), this, SLOT(addToSavedAudioList(QAction *)));
    
    //Add selected to saved video list
    m_addToVideoSavedList = new QMenu(i18n("Add to list "), m_parent);
    connect(m_addToVideoSavedList, SIGNAL(triggered(QAction *)), this, SLOT(addToSavedVideoList(QAction *)));
    
    //Add a new audio list
    m_newAudioList = new QAction(KIcon("list-add"), i18n("New list"), m_parent);
    connect(m_newAudioList, SIGNAL(triggered()), m_parent->savedListsManager(), SLOT(showAudioListSave()));
    
    //Add a new video list
    m_newVideoList = new QAction(KIcon("list-add"), i18n("New list"), m_parent);
    connect(m_newVideoList, SIGNAL(triggered()), m_parent->savedListsManager(), SLOT(showVideoListSave()));
    
    //Show Items
    m_showItems = new QAction(KIcon("bangarang-category-browse"), i18n("Show items"), m_parent);
    connect(m_showItems, SIGNAL(triggered()), this, SLOT(loadSelectedSources()));

    //Show Now Playing Info
    m_showNowPlayingInfo = new QAction(KIcon("help-about"), i18n("Show Information"), m_parent);
    connect(m_showNowPlayingInfo, SIGNAL(triggered()), this, SLOT(showInfoForNowPlaying()));
    
    //Show Info View
    m_showInfo = new QAction(KIcon("help-about"), i18n("Show Information"), m_parent);
    connect(m_showInfo, SIGNAL(triggered()), m_parent->infoManager(), SLOT(showInfoView()));
  
    //Edit Shortcuts
    //FIXME: Need to figure out how to use KShortcutsEditor
    m_editShortcuts = new QAction(KIcon("configure-shortcuts"), i18n("Configure shortcuts..."), this);
    connect(m_editShortcuts, SIGNAL(triggered()), this, SLOT(showShortcutsEditor()));
    connect(ui->cancelEditShortcuts, SIGNAL(clicked()), this, SLOT(hideShortcutsEditor()));
    ui->shortcutsEditor->addCollection(m_actionCollection);
    
    //Show the Scripting Console
    m_showScriptingConsole = new QAction(KIcon("applications-development"),i18n("Show Scripting-Console"),m_parent);
    connect(m_showScriptingConsole,SIGNAL(triggered()),this,SLOT(showScriptConsoleSlot()));
        
    m_nowPlayingContextMenu = new QMenu(m_parent);
    m_notifierMenu = new KMenu(m_parent);
}

ActionsManager::~ActionsManager()
{
}

QAction * ActionsManager::quit()
{
    return m_quit;
}

QAction * ActionsManager::playPause()
{
    return m_playPause;
}

QAction * ActionsManager::play()
{
    return m_play;
}

QAction * ActionsManager::pause()
{
    return m_pause;
}

QAction * ActionsManager::playNext()
{
    return m_playNext;
}

QAction * ActionsManager::playPrevious()
{
    return m_playPrevious;
}

QAction * ActionsManager::mute()
{
    return m_mute;
}

QAction * ActionsManager::playAll()
{
    return m_playAllAction;
}

QAction * ActionsManager::playSelected()
{
    return m_playSelectedAction;
}

QAction * ActionsManager::addSelectedToPlaylist()
{
    return m_addSelectedToPlayListAction;
}

QAction * ActionsManager::removeSelectedFromPlaylist()
{
    return m_removeSelectedToPlayListAction;
}

QAction * ActionsManager::showHideControls()
{
    return m_showHideControls;
}

QAction * ActionsManager::fullScreen()
{
    return m_fullScreen;
}

QAction * ActionsManager::cancelFullScreenHideControls()
{
    return m_cancelFullScreenHideControls;
}

QAction * ActionsManager::editShortcuts()
{
    return m_editShortcuts;
}

QAction * ActionsManager::removeSelectedItemsInfo()
{
    return m_removeSelectedItemsInfo;
}

QAction * ActionsManager::refreshMediaView()
{
    return m_refreshMediaView;
}

QAction * ActionsManager::showVideoSettings()
{
  return m_showVideoSettings;
}

QAction * ActionsManager::showNowPlayingInfo()
{
    return m_showNowPlayingInfo;
}

QAction * ActionsManager::showInfo()
{
    return m_showInfo;
}

QAction * ActionsManager::showScriptingConsole()
{
  return m_showScriptingConsole;
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
            menu->addAction(addSelectedToPlaylist());
            menu->addAction(removeSelectedFromPlaylist());
            menu->addSeparator();
        }
        if (selection) menu->addAction(playSelected());
        menu->addAction(playAll());
        menu->addSeparator();
        if (selection && isCategory) {
            menu->addAction(m_showItems);
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
                menu->addAction(removeFromSavedList());
            }
            menu->addSeparator();
        }
        if (selection && isMedia) {
            menu->addAction(removeSelectedItemsInfo());
            menu->addSeparator();
        }
	
        menu->addAction(refreshMediaView());
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
        m_nowPlayingContextMenu->addAction(m_playPrevious);
        if (m_parent->playlist()->mediaObject()->state() == Phonon::PlayingState) {
            m_nowPlayingContextMenu->addAction(m_pause);
        } else {
            m_nowPlayingContextMenu->addAction(m_play);
        }
    } else {
        m_nowPlayingContextMenu->addAction(m_play);
    }
    m_nowPlayingContextMenu->addAction(m_playNext);
    m_nowPlayingContextMenu->addSeparator();
    if (m_parent->playlist()->mediaObject()->hasVideo()) {
        m_nowPlayingContextMenu->addAction(m_showVideoSettings);
    }
    m_nowPlayingContextMenu->addAction(m_showHideControls);
    return m_nowPlayingContextMenu;
}

KMenu *ActionsManager::notifierMenu()
{
    m_notifierMenu->clear();
    m_notifierMenu->addAction(m_playPrevious);
    m_notifierMenu->addAction(m_playPause);
    m_notifierMenu->addAction(m_playNext);
    return m_notifierMenu;
}

QAction *ActionsManager::removeFromSavedList()
{
    return m_removeFromSavedList;
}

QAction *ActionsManager::newAudioList()
{
    return m_newAudioList;
}

QAction *ActionsManager::newVideoList()
{
    return m_newVideoList;
}

QMenu *ActionsManager::addToSavedAudioListMenu()
{
    return m_addToAudioSavedList;
}

QMenu *ActionsManager::addToSavedVideoListMenu()
{
    return m_addToVideoSavedList;
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
    if ((!m_parent->isFullScreen()) && (ui->stackedWidget->currentIndex() == 1)) {
        if (ui->widgetSet->isVisible()) {
            ui->widgetSet->setVisible(false);
            ui->nowPlayingToolbar->setVisible(false);
            m_showHideControls->setIcon(KIcon("layer-visible-on"));
	    m_showHideControls->setText(i18n("Show Controls"));
        } else {
            ui->widgetSet->setVisible(true);
            ui->nowPlayingToolbar->setVisible(true);
            m_showHideControls->setIcon(KIcon("layer-visible-off"));
	    m_showHideControls->setText(i18n("Hide Controls"));
        }
    }
}

void ActionsManager::toggleVideoSettings()
{
    if(ui->contextStack->currentIndex() != 1 ) {
        m_contextStackWasVisible = ui->contextStack->isVisible();
        m_previousContextStackIndex = ui->contextStack->currentIndex();
        ui->contextStack->setCurrentIndex(1);
        ui->contextStack->setVisible(true);
        m_showVideoSettings->setText(i18n("Hide Video Settings"));
    } else {
        ui->contextStack->setVisible(m_contextStackWasVisible);
        ui->contextStack->setCurrentIndex(m_previousContextStackIndex);
        m_showVideoSettings->setText(i18n("Show Video Settings"));
    }
}

void ActionsManager::cancelFSHC()
{
    if (m_parent->isFullScreen()) {
        m_parent->on_fullScreen_toggled(false);
    } else {
        if (ui->stackedWidget->currentIndex() == 1) {
            ui->widgetSet->setVisible(true);
            ui->nowPlayingToolbar->setVisible(true);
            m_showHideControls->setIcon(KIcon("layer-visible-off"));
        }
    }
}

void ActionsManager::showShortcutsEditor()
{
    ui->contextStack->setCurrentIndex(2);
    ui->contextStack->setVisible(true);
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
    m_addToAudioSavedList->addAction(m_newAudioList);
    QStringList audioListNames = m_parent->savedListsManager()->savedListNames("Audio");
    for (int i = 0; i < audioListNames.count(); i++) {
        if (!((audioListNames.at(i) == m_parent->m_mediaItemModel->mediaListProperties().name)
            && (m_parent->m_mediaItemModel->mediaListProperties().lri.startsWith("savedlists://")))) { 
            QAction * addToSavedList = new QAction(KIcon("view-list-text"), audioListNames.at(i), m_addToAudioSavedList);
            addToSavedList->setData(audioListNames.at(i));
            m_addToAudioSavedList->addAction(addToSavedList);
        }
    }
    
    m_addToVideoSavedList->clear();
    m_addToVideoSavedList->addAction(m_newVideoList);
    QStringList videoListNames = m_parent->savedListsManager()->savedListNames("Video");
    for (int i = 0; i < videoListNames.count(); i++) {
        if (!((videoListNames.at(i) == m_parent->m_mediaItemModel->mediaListProperties().name)
            && (m_parent->m_mediaItemModel->mediaListProperties().lri.startsWith("savedlists://")))) { 
            QAction * addToSavedList = new QAction(KIcon("view-list-text"), videoListNames.at(i), m_addToVideoSavedList);
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
