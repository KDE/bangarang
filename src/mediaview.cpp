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
#include <KDebug>

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
    } else if (m_mode == MiniMode) {
        m_mediaItemDelegate->setRenderMode(MediaItemDelegate::MiniMode);
    } else if (m_mode == MiniAlbumMode) {
        m_mediaItemDelegate->setRenderMode(MediaItemDelegate::MiniAlbumMode);
    }
}

MediaView::RenderMode MediaView::mode()
{
    return m_mode;
}

void MediaView::contextMenuEvent(QContextMenuEvent * event)
{
    if (selectionModel()->selectedIndexes().count() != 0) {
        //NOTE:The context menu source determination here depends on mini modes only being used for infoboxes.
        MainWindow::ContextMenuSource contextMenuSource;
        if (m_mode == NormalMode) {
            contextMenuSource = MainWindow::MediaList;
        } else {
            contextMenuSource = MainWindow::InfoBox;
        }
        bool showAbout = false;
        QMenu * menu = m_mainWindow->actionsManager()->mediaViewMenu(showAbout, contextMenuSource);
        menu->exec(event->globalPos());
    }
    
}

void MediaView::mediaListChanged()
{
    if (m_mediaItemModel->rowCount() > 0 && (m_mode == NormalMode || m_mode == MiniAlbumMode)) {
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
        if (m_mediaItemModel->rowCount() > 0 &&  m_mode != MiniAlbumMode) {
            header()->hideSection(1);
            header()->setStretchLastSection(true);
            header()->setResizeMode(QHeaderView::ResizeToContents);
        }
        setMinimumHeight(m_mediaItemDelegate->heightForAllRows());
        setMaximumHeight(m_mediaItemDelegate->heightForAllRows());
        
        //Add more info to each tooltip in mini modes
        QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
        for (int i = 0; i < mediaList.count(); i++) {
            QString tooltip = QString("<b>%1</b>").arg(mediaList.at(i).title);
            if (!mediaList.at(i).subTitle.isEmpty()) {
                tooltip += QString("<br>%1").arg(mediaList.at(i).subTitle);
            }
            if (!mediaList.at(i).semanticComment.isEmpty()) {
                tooltip += QString("<br><i>%3</i>").arg(mediaList.at(i).semanticComment);
            }
            m_mediaItemModel->item(i)->setData(tooltip, Qt::ToolTipRole);
        }
    }
}
