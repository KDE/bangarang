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

#include "playlistview.h"

#include <QHeaderView>

#include <mediaitemdelegate.h>
#include <actionsmanager.h>
#include <bangarangapplication.h>
#include <platform/utilities.h>
#include <ui_mainwindow.h>

PlaylistView::PlaylistView(QWidget* parent): QTreeView(parent)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_playlist = m_application->playlist();
    
    m_playlistModel = m_playlist->playlistModel();
    setModel(m_playlist->filterProxyModel());
    setSourceModel(m_playlistModel);
    
    connect(m_playlistModel, SIGNAL(mediaListChanged()), this, SLOT(playlistChanged()));
    connect(m_playlistModel, SIGNAL(mediaListPropertiesChanged()), this, SLOT(playlistChanged()));
    
    m_playlistItemDelegate = NULL;
    m_playlistName = NULL;
    m_playlistDuration = NULL;
    m_currentModel = Playlist::PlaylistModel;
}

void PlaylistView::setMainWindow(MainWindow* mainWindow)
{
    m_playlistItemDelegate = new MediaItemDelegate(mainWindow);
    m_playlistItemDelegate->setUseProxy(true);
    setItemDelegate(m_playlistItemDelegate);
    m_playlistItemDelegate->setView(this);
    m_playlistName = mainWindow->ui->playlistName;
    m_playlistDuration = mainWindow->ui->playlistDuration;
}

void PlaylistView::contextMenuEvent(QContextMenuEvent* event)
{
    if (selectionModel()->selectedIndexes().count() != 0) {
        MainWindow::ContextMenuSource contextMenuSource = MainWindow::Playlist;
        QMenu * menu = m_application->actionsManager()->playlistViewMenu(contextMenuSource);
        menu->exec(event->globalPos());
    }
}

void PlaylistView::setSourceModel(MediaItemModel* model)
{
    m_playlist->filterProxyModel()->setSourceModel((QAbstractItemModel *) model);
}

MediaItemModel *PlaylistView::sourceModel()
{
    return (MediaItemModel *)m_playlist->filterProxyModel()->sourceModel();
}

void PlaylistView::playlistChanged()
{
    if (model()->rowCount() > 0) {
        header()->setStretchLastSection(false);
        header()->setResizeMode(0, QHeaderView::Stretch);
        header()->setResizeMode(1, QHeaderView::ResizeToContents);
    }
    if (m_currentModel == Playlist::PlaylistModel) {
        m_playlistName->setText(i18n("<b>Playlist</b>"));
        if (m_playlist->playlistModel()->rowCount() > 0) {
            QString duration = Utilities::mediaListDurationText(m_playlist->playlistModel()->mediaList());
            m_playlistDuration->setText(i18np("1 item, %2", "%1 items, %2", m_playlist->playlistModel()->rowCount(), duration));
        } else {
            m_playlistDuration->setText(QString());
        }
    } else {
        m_playlistName->setText(i18n("<b>Playlist</b>(Upcoming)"));
    }
}

Playlist::Model PlaylistView::toggleModel()
{
    if ( m_currentModel == Playlist::PlaylistModel ) {
        m_currentModel = Playlist::QueueModel;
    } else {
        m_currentModel = Playlist::PlaylistModel;
    }
    if (m_currentModel == Playlist::QueueModel) {
        setSourceModel(m_playlist->queueModel());
        setDragDropMode(QAbstractItemView::InternalMove);
    } else {
        setSourceModel(m_playlist->playlistModel());
        setDragDropMode(QAbstractItemView::DragDrop);
    }
    playlistChanged();
    return m_currentModel;
}


