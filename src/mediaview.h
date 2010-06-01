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

class MainWindow;
class MediaItemModel;
class MediaItemDelegate;
class BangarangApplication;

/*
 * This class is mostly to provide custom context menus for the QTreeView
 * used to display media lists.
 */
class MediaView : public QTreeView
{
    Q_OBJECT
    public:
        enum RenderMode {NormalMode = 0,
                          MiniMode = 1,
                          MiniPlaybackTimeMode = 2,
                          MiniRatingMode = 3,
                          MiniPlayCountMode = 4,
                          MiniAlbumMode = 5};
        MediaView(QWidget * parent = 0);
        ~MediaView();
        void setMainWindow(MainWindow * mainWindow);
        void setMode(RenderMode Mode);
        RenderMode mode();
        void setModel(QAbstractItemModel * mediaItemModel);

    protected:
        void contextMenuEvent (QContextMenuEvent * event);
        
    private:
        BangarangApplication * m_application;
        //MainWindow * m_mainWindow;
        MediaItemModel * m_mediaItemModel;
        MediaItemDelegate * m_mediaItemDelegate;
        RenderMode m_mode;
        QAction * playAllAction;
        QAction * playSelectedAction;   
        QAction * addSelectedToPlayListAction;
        QAction * removeSelectedToPlayListAction;
    
    private slots:
        void mediaListChanged();
};
#endif // MEDIAVIEW_H
