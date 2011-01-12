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
#include "bangarangapplication.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "platform/mediaitemmodel.h"
#include "platform/utilities/utilities.h"
#include <KIcon>
#include <QMenu>
#include <QHeaderView>
#include <QToolTip>
#include <KDebug>

MediaView::MediaView(QWidget * parent):QTreeView (parent) 
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_proxyModel = new MediaSortFilterProxyModel();
    setModel(m_proxyModel);
    m_mediaItemModel = new MediaItemModel(parent);
    setSourceModel(m_mediaItemModel);
    m_mode = MediaItemDelegate::NormalMode;
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
    m_mediaItemDelegate = new MediaItemDelegate(mainWindow);
    m_mediaItemDelegate->setUseProxy(true);
    setItemDelegate(m_mediaItemDelegate);
    m_mediaItemDelegate->setView(this);
    setDragEnabled(true);
}

void MediaView::setMode(MediaItemDelegate::RenderMode mode)
{
    m_mode = mode;
    m_mediaItemDelegate->setRenderMode(mode);
}

MediaItemDelegate::RenderMode MediaView::mode()
{
    return m_mode;
}

void MediaView::setSourceModel(QAbstractItemModel * mediaItemModel)
{
    m_mediaItemModel = (MediaItemModel *)mediaItemModel;
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    m_proxyModel->setSourceModel((QAbstractItemModel *) m_mediaItemModel);
}
void MediaView::contextMenuEvent(QContextMenuEvent * event)
{
    if (selectionModel()->selectedIndexes().count() != 0) {
        //NOTE:The context menu source determination here depends on mini modes only being used for infoboxes.
        MainWindow::ContextMenuSource contextMenuSource;
        if (m_mode == MediaItemDelegate::NormalMode) {
            contextMenuSource = MainWindow::MediaList;
        } else {
            contextMenuSource = MainWindow::InfoBox;
        }
        bool showAbout = false;
        QMenu * menu = m_application->actionsManager()->mediaViewMenu(showAbout, contextMenuSource);
        menu->exec(event->globalPos());
    }
    
}

void MediaView::mediaListChanged()
{
    if (m_mediaItemModel->rowCount() > 0 &&
        (m_mode == MediaItemDelegate::NormalMode || m_mode == MediaItemDelegate::MiniAlbumMode)) {
        header()->setStretchLastSection(true);
    }
    else if (m_mode != MediaItemDelegate::NormalMode) {
        if (m_mediaItemModel->rowCount() > 0 &&  m_mode != MediaItemDelegate::MiniAlbumMode) {
            header()->setStretchLastSection(true);
            header()->setResizeMode(QHeaderView::ResizeToContents);
        }
        int height = m_mediaItemDelegate->heightForAllRows();
        setMinimumHeight(height);
        setMaximumHeight(height);
        
        //Add more info to each tooltip in mini modes
        QList<MediaItem> mediaList = m_mediaItemModel->mediaList();
        for (int i = 0; i < mediaList.count(); i++) {
            const MediaItem& mi = mediaList.at(i);
            QString tooltip = QString("<b>%1</b>").arg(mi.title);
            if (!mi.subTitle.isEmpty()) {
                tooltip += QString("<br>%1").arg(mi.subTitle);
            }
            if (!mi.semanticComment.isEmpty()) {
                tooltip += QString("<br><i>%3</i>").arg(mi.semanticComment);
            }
            m_mediaItemModel->item(i)->setData(tooltip, Qt::ToolTipRole);
        }
    }
}

bool MediaView::viewportEvent(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QPoint mousePos = helpEvent->pos();
        QModelIndex index = indexAt(mousePos);
        QRect visRect = visualRect(index);
        QRect area = m_mediaItemDelegate->addRmPlaylistRect(&visRect);
        if (area.contains(mousePos)) {
            QString url = index.data(MediaItem::UrlRole).toString();
            QString type = index.data(MediaItem::TypeRole).toString();
            if (!url.isEmpty()) {
                QString tipText;
                if (Utilities::isMedia(type)) {
                    tipText = i18n("Add to playlist/Remove from playlist");
                } else if (Utilities::isCategory(type)) {
                    if (url.startsWith("music://songs")) {
                        tipText = i18n( "Show Songs" );
                    } else if (url.startsWith("music://albums")) {
                        tipText = i18n( "Show Albums" );
                    } else if (url.startsWith("music://artists")) {
                        tipText = i18n( "Show Artists" );
                    }   
                }
                if ( !tipText.isEmpty() ) {
                    QToolTip::showText(helpEvent->globalPos(), tipText, this, area);
                    return true;
                }
            }
        }
    }
    return QTreeView::viewportEvent(event);
}
