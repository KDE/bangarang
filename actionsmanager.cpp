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
    
    //Add standard quit shortcut
    m_quit = new QAction(this);
    m_quit->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(m_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    m_parent->addAction(m_quit);
    m_actionCollection->addAction(tr("Quit"), m_quit);
    
    //Play/Pause Action
    m_playPause = new QAction(this);
    m_playPause->setShortcut(Qt::Key_Space);
    connect(m_playPause, SIGNAL(triggered()), this, SLOT(simplePlayPause()));
    m_parent->addAction(m_playPause);
    
    //Play Next
    m_playNext = new QAction(KIcon("media-skip-forward"), tr("Play next"), this);
    m_playNext->setShortcut(Qt::Key_Right);
    connect(m_playNext, SIGNAL(triggered()), m_parent->playlist(), SLOT(playNext()));
    m_parent->addAction(m_playNext);

    //Play Previous
    m_playPrevious = new QAction(KIcon("media-skip-backward"), tr("Play previous"), this);
    m_playPrevious->setShortcut(Qt::Key_Left);
    connect(m_playPrevious, SIGNAL(triggered()), m_parent->playlist(), SLOT(playPrevious()));
    m_parent->addAction(m_playPrevious);
    
    //Mute
    m_mute = new QAction(this);
    m_mute->setShortcut(Qt::Key_M);
    connect(m_mute, SIGNAL(triggered()), this, SLOT(muteAudio()));
    m_parent->addAction(m_mute);
    
    //Play All Action
    m_playAllAction = new QAction(KIcon("media-playback-start"), tr("Play all"), this);
    connect(m_playAllAction, SIGNAL(triggered()), m_parent, SLOT(playAll()));
    m_actionCollection->addAction(tr("Play All"), m_playAllAction);
    
    //Play Selected Action
    m_playSelectedAction = new QAction(KIcon("media-playback-start"), tr("Play selected"), this);
    connect(m_playSelectedAction, SIGNAL(triggered()), m_parent, SLOT(playSelected()));
    m_actionCollection->addAction(tr("Play Selected"), m_playSelectedAction);
    
    //Add Selected To Playlist Action
    m_addSelectedToPlayListAction = new QAction(KIcon("mail-mark-notjunk"), tr("Add to playlist"), this);
    connect(m_addSelectedToPlayListAction, SIGNAL(triggered()), m_parent, SLOT(addSelectedToPlaylist()));  
    m_actionCollection->addAction(tr("Add to playlist"), m_addSelectedToPlayListAction);
    
    //Remove Selected From Playlist Action
    m_removeSelectedToPlayListAction = new QAction(KIcon(), tr("Remove from playlist"), this);
    connect(m_removeSelectedToPlayListAction, SIGNAL(triggered()), m_parent, SLOT(removeSelectedFromPlaylist()));
    m_actionCollection->addAction(tr("Remove from playlist"), m_removeSelectedToPlayListAction);
    
    //Show/Hide Controls Shortcut
    m_showHideControls = new QAction(KIcon("layer-visible-off"), tr("Hide controls"), this);
    m_showHideControls->setShortcut(Qt::CTRL + Qt::Key_H);
    connect(m_showHideControls, SIGNAL(triggered()), this, SLOT(toggleControls()));
    m_parent->addAction(m_showHideControls);
    m_actionCollection->addAction(tr("Hide controls"), m_showHideControls);
    
    
    //Full Screen
    m_fullScreen = new QAction(this);
    m_fullScreen->setShortcut(Qt::Key_F11);
    connect(m_fullScreen, SIGNAL(triggered()), this, SLOT(fullScreenToggle()));
    m_parent->addAction(m_fullScreen);
    m_actionCollection->addAction(tr("Toggle fullscreen"), m_fullScreen);
    
    //Cancel FullScreen/Cancel Hide Controls
    m_cancelFullScreenHideControls = new QAction(this);
    m_cancelFullScreenHideControls->setShortcut(Qt::Key_Escape);
    connect(m_cancelFullScreenHideControls, SIGNAL(triggered()), this, SLOT(cancelFSHC()));
    m_parent->addAction(m_cancelFullScreenHideControls);

    //Remove Info for Selected MediaItems
    m_removeSelectedItemsInfo = new QAction(KIcon("edit-delete-shred"), tr("Remove selected info"), this);
    connect(m_removeSelectedItemsInfo, SIGNAL(triggered()), m_parent->infoManager(), SLOT(removeSelectedItemsInfo()));
    m_parent->addAction(m_removeSelectedItemsInfo);

    //Refresh Media View
    m_refreshMediaView = new QAction(KIcon("view-refresh"), tr("Refresh"), this);
    m_refreshMediaView->setShortcut(Qt::Key_F5);
    connect(m_refreshMediaView, SIGNAL(triggered()), m_parent->m_mediaItemModel, SLOT(reload()));
    m_parent->addAction(m_refreshMediaView);
    
    //Edit Shortcuts
    //FIXME: Need to figure out how to use KShortcutsEditor
    m_editShortcuts = new QAction(KIcon("configure-shortcuts"), tr("Configure shortcuts..."), this);
    connect(m_editShortcuts, SIGNAL(triggered()), this, SLOT(showShortcutsEditor()));
    connect(ui->cancelEditShortcuts, SIGNAL(clicked()), this, SLOT(hideShortcutsEditor()));
    ui->shortcutsEditor->addCollection(m_actionCollection);
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

QMenu * ActionsManager::mediaViewMenu(bool showAbout)
{
    KHelpMenu * helpMenu = new KHelpMenu(m_parent, m_parent->aboutData(), false);
    helpMenu->menu();
    
    QMenu *menu = new QMenu(m_parent);
    QString type;
    bool selection = false;
    if (ui->mediaView->selectionModel()->selectedIndexes().count() != 0) {
        QModelIndex index = ui->mediaView->selectionModel()->selectedIndexes().at(0);
        type = index.data(MediaItem::TypeRole).toString();
        selection = true;
    } else if (m_parent->m_mediaItemModel->rowCount() > 0) {
        type = m_parent->m_mediaItemModel->mediaItemAt(0).type;
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
        } else {
            ui->widgetSet->setVisible(true);
            ui->nowPlayingToolbar->setVisible(true);
            m_showHideControls->setIcon(KIcon("layer-visible-off"));
        }
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
    } else if (m_parent->playlist()->mediaObject()->state() == Phonon::PausedState) {
        m_parent->playlist()->mediaObject()->play();
    }
}

void ActionsManager::muteAudio()
{
    bool muted = m_parent->audioOutput()->isMuted();
    m_parent->audioOutput()->setMuted(!muted);
}
