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
#include "flickcharm.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/utilities/utilities.h"
#include <KIcon>
#include <QMenu>
#include <QHeaderView>
#include <QToolTip>
#include <KDebug>

MediaView::MediaView(QWidget * parent):QTreeView (parent), LoadingAnimation(viewport(), 32)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_proxyModel = new MediaSortFilterProxyModel();
    setModel(m_proxyModel);
    m_mediaItemModel = new MediaItemModel(parent);
    m_mediaItemDelegate = NULL;
    setSourceModel(m_mediaItemModel);
    m_mode = MediaItemDelegate::NormalMode;
    m_loading = false;
    m_itemsAvailable = false;
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setAlternatingRowColors(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_scrolling = new QTime();
    m_scrollBarPressed = false;
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
    connect(this, SIGNAL(categoryActivated(QModelIndex)), m_mediaItemDelegate, SIGNAL(categoryActivated(QModelIndex)));
}

void MediaView::setMode(MediaItemDelegate::RenderMode mode)
{
    if (m_mediaItemDelegate) {
        m_mediaItemDelegate->setRenderMode(mode);
    }
    if ( m_mode != mode ) {
        switch ( mode ) {
            case MediaItemDelegate::NormalMode:
                setAnimationSize(32);
                break;
                
            default:
                setAnimationSize(8);
        }
    }
    m_mode = mode; 
}

MediaItemDelegate::RenderMode MediaView::mode()
{
    return m_mode;
}

void MediaView::setSourceModel(QAbstractItemModel * mediaItemModel)
{
    m_mediaItemModel = (MediaItemModel *)mediaItemModel;
    connect(m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_mediaItemModel, SIGNAL(loadingStateChanged(bool)), this, SLOT(loadingStateChanged(bool)));
    connect(m_mediaItemModel, SIGNAL(itemsAvailable(bool)), this, SLOT(itemsAvailable(bool)));
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

void MediaView::enableTouch()
{
    FlickCharm *charm = new FlickCharm(this);
    charm->activateOn(this);
    this->setDragEnabled(false);
    m_mediaItemDelegate->enableTouch();
}

void MediaView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Right) {
        QModelIndexList selectedRows = selectionModel()->selectedRows();
        if (selectedRows.count() > 0) {
            QModelIndex sourceIndex = filterProxyModel()->mapToSource(selectedRows.at(0));
            emit categoryActivated(sourceIndex);
        }
    }
    QTreeView::keyPressEvent(event);
}

void MediaView::loadingStateChanged(bool loading)
{
    if (loading == m_loading) {
        return;
    }
    if ( loading ) {
        checkForScrollOverlay();
    }
    m_loading = loading;
    if ( loading ) {
        startAnimation();
    } else {
        stopAnimation(); //should be done by itemsAvailable anyway, but we like to be sure
    }
    
}

void MediaView::paintEvent(QPaintEvent* event)
{
    if ( m_itemsAvailable ) { //we have items, simply draw them
        QTreeView::paintEvent(event);
        int scrollInterval = 500;
        int scrollLeft = scrollInterval - m_scrolling->elapsed();
        
        if (scrollLeft > 0 || m_scrollBarPressed ) { 
            //draw the letter of the topmost item semitransparent in the middle
            if ( scrollLeft > 0 ) {
                //make sure we redraw this after scrolling has ended
                QTimer::singleShot(scrollLeft, viewport(), SLOT(update()));
            }
            QPainter painter(viewport());
            QModelIndex idx = m_proxyModel->mapToSource(indexAt(viewport()->geometry().topLeft()));
            const QChar letter = m_mediaItemModel->mediaItemAt(idx.row()).title[0];
            QFont font = KGlobalSettings::largeFont();
            font.setPointSize(font.pointSize() - 8);
            font.setBold(true);
            QRect rect = QFontMetrics(font).boundingRect(letter);
            rect.setWidth(rect.width()+20);
            rect.setHeight(rect.height()+20);
            rect.moveCenter(viewport()->geometry().center());
            
            if ( event->rect().intersects(rect) ) {
                QColor background = QApplication::palette().color(QPalette::Foreground);
                QColor foreground = QApplication::palette().color(QPalette::Base);
                background.setAlpha(128);
                foreground.setAlpha(128);
                painter.setBrush(QBrush(background));
                painter.setPen(foreground);
                painter.drawRoundedRect(rect, 5.0, 5.0);
                foreground.setAlpha(190);
                painter.setPen(foreground);
                painter.setFont(font);
                painter.drawText(rect, Qt::AlignCenter, letter);
            }
        }
        
        return;
    }
    //no items, are we loading ?
    if ( m_loading ) {
        paintAnimation(event);
        return;
    }
    //so we have no results, draw a message
    //showNoResultsMessage
}

void MediaView::checkForScrollOverlay()
{
    //TODO: well this is kind of a workaround to disable ugly behavior where the current scrolling overlay
    //doesn't make sense
    
    QString engine = m_mediaItemModel->mediaListProperties().engine();
    //semantic engines have no "simple" order by item-title, so we leave it out. they are restricted
    //in their length anyway
    if (engine == "semantics://") {
        disconnect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved()));
        disconnect(verticalScrollBar(), SIGNAL(sliderPressed()), this, SLOT(scrollBarPressed()));
        disconnect(verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(scrollBarReleased()));   
        m_scrollBarPressed = false; //to be safe
    } else {
        connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved()));
        connect(verticalScrollBar(), SIGNAL(sliderPressed()), this, SLOT(scrollBarPressed()));
        connect(verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(scrollBarReleased()));   
    }
}

void MediaView::itemsAvailable(bool available)
{

    stopAnimation(); //definitely not loading anymore
    m_itemsAvailable = available;
}

void MediaView::scrollBarMoved()
{
    m_scrolling->restart();
}

void MediaView::scrollBarPressed()
{
    m_scrollBarPressed = true;
    viewport()->update();
}

void MediaView::scrollBarReleased()
{
    m_scrollBarPressed = false; 
    viewport()->update();
}

