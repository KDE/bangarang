/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://projects.kde.org/projects/playground/multimedia/bangarang>
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

#ifndef MEDIAVIEW_H
#define MEDIAVIEW_H

#include <QtCore/QTextCodec>
#include <QtGui/QTreeView>
#include <QtGui/QAction>
#include <QtGui/QScrollBar>
#include "mediaitemdelegate.h"
#include "loadinganimation.h"

class MainWindow;
class MediaItemModel;
class BangarangApplication;
class MediaSortFilterProxyModel;
class QSortFilterProxyModel;

/*
 * This class is mostly to provide custom context menus for the QTreeView
 * used to display media lists.
 */
class MediaView : public QTreeView, protected LoadingAnimation
{
    Q_OBJECT
    public:
        MediaView(QWidget * parent = 0);
        ~MediaView();
        void setMainWindow(MainWindow * mainWindow);
        void setMode(MediaItemDelegate::RenderMode Mode);
        MediaItemDelegate::RenderMode mode();
        void setSourceModel(QAbstractItemModel * mediaItemModel);
        MediaItemModel *sourceModel() { return m_mediaItemModel; }
        QSortFilterProxyModel *filterProxyModel() { return (QSortFilterProxyModel *) m_proxyModel; }
        void enableTouch();

    Q_SIGNALS:
        void categoryActivated(QModelIndex index);

    protected:
        void contextMenuEvent(QContextMenuEvent * event);
        bool viewportEvent(QEvent * event);
        void keyPressEvent(QKeyEvent *event);
        void paintEvent(QPaintEvent *event);
        void checkForScrollOverlay();
        void paintScrollOverlay(QPaintEvent* event);
        
    private:
        BangarangApplication * m_application;
        MediaItemModel * m_mediaItemModel;
        MediaItemDelegate * m_mediaItemDelegate;
        MediaSortFilterProxyModel *m_proxyModel;
        MediaItemDelegate::RenderMode m_mode;
        QAction * playAllAction;
        QAction * playSelectedAction;   
        QAction * addSelectedToPlayListAction;
        QAction * removeSelectedToPlayListAction;
        bool m_loading;
        bool m_itemsAvailable;
        QTime *m_scrolling;
        bool m_scrollBarPressed;
    
    private slots:
        void mediaListChanged();
        void loadingStateChanged(bool loading);
        void itemsAvailable(bool available);
        void scrollBarMoved();
        void scrollBarPressed();
        void scrollBarReleased();

};
#endif // MEDIAVIEW_H
