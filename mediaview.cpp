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

#include <KIcon>
#include <QMenu>
#include "mediaview.h"
#include "mainwindow.h"
#include "platform/mediaitemmodel.h"

MediaView::MediaView(QWidget * parent):QTreeView (parent) 
{
    
}

MediaView::~MediaView() 
{
}

void MediaView::setMainWindow(MainWindow * mainWindow)
{
    m_mainWindow = mainWindow;

    //Setup context menu actions
    playAllAction = new QAction(KIcon("media-playback-start"), tr("Play all"), this);
    connect(playAllAction, SIGNAL(triggered()), m_mainWindow, SLOT(playAll()));
    playSelectedAction = new QAction(KIcon("media-playback-start"), tr("Play selected"), this);
    connect(playSelectedAction, SIGNAL(triggered()), m_mainWindow, SLOT(playSelected()));    
    addSelectedToPlayListAction = new QAction(KIcon("mail-mark-notjunk"), tr("Add to playlist"), this);
    connect(addSelectedToPlayListAction, SIGNAL(triggered()), m_mainWindow, SLOT(addSelectedToPlaylist()));    
    removeSelectedToPlayListAction = new QAction(KIcon(), tr("Remove from playlist"), this);
    connect(removeSelectedToPlayListAction, SIGNAL(triggered()), m_mainWindow, SLOT(removeSelectedFromPlaylist()));    
}

void MediaView::contextMenuEvent(QContextMenuEvent * event)
{
    if (selectionModel()->selectedIndexes().count() != 0) {
        QModelIndex index = selectionModel()->selectedIndexes().at(0);
        QString type = index.data(MediaItem::TypeRole).toString();
        if ((type != "Action") && (type != "Message")) {
            QMenu menu(this);
            if ((type == "Audio") ||(type == "Video") || (type == "Image")) {
                menu.addAction(addSelectedToPlayListAction);
                menu.addAction(removeSelectedToPlayListAction);
            }
            menu.addSeparator();
            menu.addAction(playSelectedAction);
            menu.addAction(playAllAction);
            menu.exec(event->globalPos());
        }
    }
}

