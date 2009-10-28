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

#include <KStandardDirs>
#include <KMessageBox>
#include <QFile>

ActionsManager::ActionsManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
    
    //Add standard quit shortcut
    m_quit = new QAction(this);
    m_quit->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(m_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    m_parent->addAction(m_quit);

    //Play All Action
    m_playAllAction = new QAction(KIcon("media-playback-start"), tr("Play all"), this);
    connect(m_playAllAction, SIGNAL(triggered()), m_parent, SLOT(playAll()));
    
    //Play Selected Action
    m_playSelectedAction = new QAction(KIcon("media-playback-start"), tr("Play selected"), this);
    connect(m_playSelectedAction, SIGNAL(triggered()), m_parent, SLOT(playSelected()));
    
    //Add Selected To Playlist Action
    m_addSelectedToPlayListAction = new QAction(KIcon("mail-mark-notjunk"), tr("Add to playlist"), this);
    connect(m_addSelectedToPlayListAction, SIGNAL(triggered()), m_parent, SLOT(addSelectedToPlaylist()));    
    
    //Remove Selected From Playlist Action
    m_removeSelectedToPlayListAction = new QAction(KIcon(), tr("Remove from playlist"), this);
    connect(m_removeSelectedToPlayListAction, SIGNAL(triggered()), m_parent, SLOT(removeSelectedFromPlaylist()));  
    
    //Hide Controls Shortcut
    m_showHideControls = new QAction(this);
    m_showHideControls->setShortcut(Qt::CTRL + Qt::Key_H);
    connect(m_showHideControls, SIGNAL(triggered()), this, SLOT(toggleControls()));
    m_parent->addAction(m_showHideControls);
    
    //Full Screen
    m_fullScreen = new QAction(this);
    m_fullScreen->setShortcut(Qt::Key_F11);
    connect(m_fullScreen, SIGNAL(triggered()), this, SLOT(fullScreenToggle()));
    m_parent->addAction(m_fullScreen);
    
    //Cancel FullScreen/Cancel Hide Controls
    m_cancelFullScreenHideControls = new QAction(this);
    m_cancelFullScreenHideControls->setShortcut(Qt::Key_Escape);
    connect(m_cancelFullScreenHideControls, SIGNAL(triggered()), this, SLOT(cancelFSHC()));
    m_parent->addAction(m_cancelFullScreenHideControls);
    
}

ActionsManager::~ActionsManager()
{
}

QAction * ActionsManager::quit()
{
    return m_quit;
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
        } else {
            ui->widgetSet->setVisible(true);
            ui->nowPlayingToolbar->setVisible(true);
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
        }
    }
}