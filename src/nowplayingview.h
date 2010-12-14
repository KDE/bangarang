/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Stefan Burnicki (stefan.burnicki@gmx.de)
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

#ifndef NOWPLAYINGVIEW_H
#define NOWPLAYINGVIEW_H

#include <QtCore>
#include <QTreeView>
#include <QAction>
#include "platform/mediaitemmodel.h"

class MainWindow;
class MediaItem;
class NowPlayingDelegate;
class BangarangApplication;
class MediaItemModel;

/*
 * This class is mostly to provide custom context menus for the NowPlaying TreeView
 */
class NowPlayingView : public QTreeView
{
    Q_OBJECT
    public:
        NowPlayingView(QWidget * parent = 0);
        ~NowPlayingView();
        void setMainWindow(MainWindow * mainWindow);

    public Q_SLOTS:
        void tidyHeader();
        void showInfo();
        void hideInfo();
        
    protected:
        void contextMenuEvent (QContextMenuEvent * event);
        void resizeEvent(QResizeEvent *event);
        
    private:
        MediaItemModel *m_nowPlayingModel;
        BangarangApplication * m_application;
        NowPlayingDelegate *m_nowPlayingDelegate;
          
};
#endif // NOWPLAYINGVIEW_H
