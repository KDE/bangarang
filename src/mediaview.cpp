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

#include "mediaview.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "mediaitemdelegate.h"
#include "platform/mediaitemmodel.h"
#include <KIcon>
#include <QMenu>
#include <QHeaderView>

MediaView::MediaView(QWidget * parent):QTreeView (parent) 
{
    m_mediaItemModel = new MediaItemModel(parent);
    setModel(m_mediaItemModel);
    m_mode = NormalMode;
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setAlternatingRowColors(true);
}

MediaView::~MediaView() 
{
}

void MediaView::setMainWindow(MainWindow * mainWindow)
{
    m_mainWindow = mainWindow;
    m_mediaItemDelegate = new MediaItemDelegate(mainWindow);
    setItemDelegate(m_mediaItemDelegate);
    m_mediaItemDelegate->setView(this);
    connect(m_mediaItemDelegate, SIGNAL(categoryActivated(QModelIndex)), m_mediaItemModel, SLOT(categoryActivated(QModelIndex)));
    connect(m_mediaItemDelegate, SIGNAL(actionActivated(QModelIndex)), m_mediaItemModel, SLOT(actionActivated(QModelIndex)));
}

void MediaView::setMode(RenderMode mode)
{
    m_mode = mode;
    if (m_mode == NormalMode) {
        m_mediaItemDelegate->setRenderMode(MediaItemDelegate::NormalMode);
    } else if (m_mode == MiniPlaybackTimeMode) {
        m_mediaItemDelegate->setRenderMode(MediaItemDelegate::MiniPlaybackTimeMode);
    } else if (m_mode == MiniRatingMode) {
        m_mediaItemDelegate->setRenderMode(MediaItemDelegate::MiniRatingMode);
    } else if (m_mode == MiniPlayCountMode) {
        m_mediaItemDelegate->setRenderMode(MediaItemDelegate::MiniPlayCountMode);
    }
}

MediaView::RenderMode MediaView::mode()
{
    return m_mode;
}

void MediaView::contextMenuEvent(QContextMenuEvent * event)
{
    if (selectionModel()->selectedIndexes().count() != 0) {
        QMenu * menu = m_mainWindow->actionsManager()->mediaViewMenu();
        menu->exec(event->globalPos());
    }
    
}

void MediaView::mediaListChanged()
{
    if (m_mediaItemModel->rowCount() > 0 && m_mode == NormalMode) {
        header()->setStretchLastSection(false);
        header()->setResizeMode(0, QHeaderView::Stretch);
        header()->setResizeMode(1, QHeaderView::ResizeToContents);
        
        QString listItemType = m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            if (!m_mediaItemModel->mediaItemAt(0).fields["isTemplate"].toBool()) {
                header()->showSection(1);
            }
        } else if (listItemType == "Category") {
            header()->showSection(1);
        } else if ((listItemType == "Action") || (listItemType == "Message")) {
            header()->hideSection(1);
        }
    }
    
    if (m_mode != NormalMode) {
        header()->setResizeMode(QHeaderView::ResizeToContents);
        if (m_mediaItemModel->rowCount() > 0) {
            header()->hideSection(1);
        }
        setMinimumHeight(m_mediaItemDelegate->heightForAllRows());
        setMaximumHeight(m_mediaItemDelegate->heightForAllRows());
    }
}
