/* BANGARANG MEDIA PLAYER
* Copyright (C) 2011 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef MEDIALISTSMANAGER_H
#define MEDIALISTSMANAGER_H

#include "../../platform/devicemanager.h"
#include "../common/mainwindow.h"
#include <QObject>
#include <QItemSelection>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItem;
class MediaItemModel;
class MediaListProperties;
class BangarangApplication;

class MediaListsManager : public QObject
{
    Q_OBJECT

public:
    enum MediaListSelection{ AudioList = 0, VideoList = 1 };

    MediaListsManager(MainWindow* parent);
    void loadMediaList(MediaItemModel* listsModel, int row);
    void addListToHistory();
    void showMediaList(MediaListSelection listSelection);
    MediaListSelection currentMediaListSelection();
    MediaItemModel* audioListsModel();
    MediaItemModel* videoListsModel();
    void showMenu();

public slots:
    void loadPreviousList();
    void selectAudioList();
    void selectVideoList();
    void loadSearch();
    void closeMediaListFilter();
    void delayedNotificationHide();

private:
    BangarangApplication* m_application;
    MediaItemModel* m_audioListsModel;
    MediaItemModel* m_videoListsModel;
    MediaListSelection m_mediaListSelection;
    QList< QList<MediaItem> > m_mediaListHistory;
    QList<MediaListProperties> m_mediaListPropertiesHistory;
    QList<int> m_mediaListScrollHistory;
    int m_loadingProgress;

    void hidePlayButtons();

private slots:
    void audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void audioListsChanged();
    void videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void videoListsChanged();
    void mediaListChanged();
    void mediaListPropertiesChanged();
    void mediaListLoading();
    void mediaListLoadingStateChanged(bool);
    void updateListHeader();
    void mediaListCategoryActivated(QModelIndex index);
    void mediaListActionActivated(QModelIndex index);
    void mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void updateDeviceList(DeviceManager::RelatedType type);
    void showMediaListLoading();
    void playSelected();
    void playAll();
    void browsingModelStatusUpdated();
    void updateSeekTime(qint64 time);
    void nowPlayingChanged();
    void defaultListLoad(MainWindow::MainWidget which);
};

#endif // MEDIALISTSMANAGER_H
