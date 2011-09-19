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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QListView>
#include <QLabel>
#include "../../platform/playlist.h"
#include "../common/mainwindow.h"

class MediaItemDelegate;

class PlaylistView : public QListView
{
    Q_OBJECT
    public:
        PlaylistView(QWidget * parent = 0);
        void setMainWindow(MainWindow *mainWindow);
        void setSourceModel(MediaItemModel* model);
        MediaItemModel *sourceModel();
        Playlist::Model toggleModel();
        Playlist::Model currentModelType() { return m_currentModel; }
        void setupActions();
        void enableTouch();
        
    protected:
        void contextMenuEvent (QContextMenuEvent * event);
        void dragMoveEvent(QDragMoveEvent *e);
        void dropEvent(QDropEvent *e);
        
    private:
        Playlist *m_playlist;
        BangarangApplication *m_application;
        MediaItemModel * m_playlistModel;
        MediaItemDelegate * m_playlistItemDelegate;
        QLabel* m_playlistName;
        QLabel* m_playlistDuration;
        Playlist::Model m_currentModel;
        
    private slots:
        void playlistChanged();
};

#endif // PLAYLISTVIEW_H
