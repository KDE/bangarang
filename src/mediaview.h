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

#ifndef MEDIAVIEW_H
#define MEDIAVIEW_H

#include <QtCore>
#include <QTreeView>
#include <QAction>
#include "mediaitemdelegate.h"

class MainWindow;
class MediaItemModel;
class BangarangApplication;
class MediaSortFilterProxyModel;
class QSortFilterProxyModel;

/*
 * This class is mostly to provide custom context menus for the QTreeView
 * used to display media lists.
 */
class MediaView : public QTreeView
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

    protected:
        void contextMenuEvent(QContextMenuEvent * event);
        bool viewportEvent(QEvent * event); 
        
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
    
    private slots:
        void mediaListChanged();
};
#endif // MEDIAVIEW_H
