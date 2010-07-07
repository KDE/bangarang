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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "platform/bangarangvideowidget.h"
#include <KIcon>
#include <KAboutData>
#include <KHelpMenu>
#include <KAction>
#include <phonon/audiooutput.h>
#include <phonon/mediacontroller.h>
#include <phonon/mediaobject.h>
#include <phonon/videoplayer.h>
#include <phonon/videowidget.h>
#include <QResizeEvent>
#include <QEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsItem>
#include <QFrame>
#include <QListWidgetItem>
#include <QAction>
#include <QDateTime>
#include <QMainWindow>
#include <kross/core/action.h>

class MediaItem;
class MediaListProperties;
class MediaItemModel;
class MediaListCache;
class Playlist;
class MediaItemDelegate;
class NowPlayingDelegate;
class InfoManager;
class SavedListsManager;
class ActionsManager;
class BookmarksManager;
class AudioSettings;
class MediaListSettings;
class KStatusNotifierItem;
class VideoSettings;
class BangarangApplication;
class KFilterProxySearchLine;

namespace Ui
{
    class MainWindowClass;
}

namespace ListChooser
{
    enum MediaListRole{NameRole = Qt::UserRole + 1,
                       LriRole = Qt::UserRole + 2,
                       TypeRole = Qt::UserRole + 3};
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    
    enum ContextMenuSource{Default = 0, MediaList = 1, InfoBox = 2, Playlist = 3};
    enum MainWidget{ MainMediaList = 0, MainNowPlaying = 1 };
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void completeSetup();
    Phonon::AudioOutput * audioOutput();
    void addListToHistory();
    Phonon::VideoWidget * videoWidget();
    bool showingRemainingTime();
    void switchMainWidget(MainWidget which);
    MainWidget currentMainWidget();
    QFrame *currentFilterFrame();
    KFilterProxySearchLine* currentFilterProxyLine();
    
    Ui::MainWindowClass *ui;
    MediaItemModel * m_audioListsModel;
    MediaItemModel * m_videoListsModel;
    QList< QList<MediaItem> > m_mediaListHistory;
    QList<MediaListProperties> m_mediaListPropertiesHistory;
    
    
public slots:
    void on_fullScreen_toggled(bool fullScreen);
    void setShowRemainingTime(bool showRemainingTime);
    
private:
    void setupModel();
    KIcon addItemsIcon();
    void setupIcons();
    void showApplicationBanner();
    void updateCachedDevicesList();
    void setupActions();
    
    Phonon::VideoPlayer *m_player;
    MediaItemDelegate * m_itemDelegate;
    MediaItemDelegate * m_playlistItemDelegate;
    BangarangVideoWidget *m_videoWidget;
    Phonon::AudioOutput *m_audioOutput;
    Phonon::AudioOutput *m_audioOutputMusicCategory;
    Phonon::AudioOutput *m_audioOutputVideoCategory;
    Phonon::Path m_audioPath;
    Phonon::Path m_videoPath;
    Phonon::MediaController *m_mediaController;
    QGraphicsScene *m_Scene;
    QString m_addItemsMessage;
    QTime m_messageTime;
    QList<MediaItem> m_mediaList;
    QList<int> m_mediaListScrollHistory;
    QDateTime m_lastMouseMoveTime;
    bool m_pausePressed;
    bool m_stopPressed;
    QList<QString> m_devicesAdded;
    int m_loadingProgress;
    bool m_nepomukInited;
    bool playWhenPlaylistChanges;
    bool m_showRemainingTime;
    qreal m_volume;
    KAction *playPause;    
    MediaListSettings *m_mediaListSettings;
    VideoSettings * m_videoSettings;
    BangarangApplication * m_application;

private slots:
    void on_nowPlaying_clicked();
    void on_configureAudioList_clicked();
    void on_configureVideoList_clicked();
    void on_collectionButton_clicked();
    void on_mediaPlayPause_pressed();
    void on_mediaPlayPause_held();
    void on_mediaPlayPause_released();
    void on_previous_clicked();
    void on_playAll_clicked();
    void on_playSelected_clicked();
    void on_mediaLists_currentChanged(int i);
    void on_showPlaylist_clicked(bool checked);
    void on_clearPlaylist_clicked();
    void on_playlistView_doubleClicked(const QModelIndex & index);
    void on_seekTime_clicked();
    void on_shuffle_clicked();
    void on_repeat_clicked();
    void on_showQueue_clicked();
    void on_Filter_returnPressed();
    void on_showMenu_clicked();
    void on_showMediaViewMenu_clicked();
    void on_closePlaylistFilter_clicked();
    void on_closeMediaListFilter_clicked();
    void updateSeekTime(qint64 time);
    void updateMuteStatus(bool muted);
    void mediaStateChanged(Phonon::State newstate, Phonon::State oldstate);
    void mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void mediaListChanged();
    void mediaListPropertiesChanged();
    void audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void audioListsChanged();
    void videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void videoListsChanged();
    void nowPlayingChanged();
    void playlistFinished();
    void repeatModeChanged(bool repeat);
    void shuffleModeChanged(bool shuffle);
    void hidePlayButtons();
    void updateListHeader();
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
    void showLoading();
    void showNotification();
    void delayedNotificationHide();
    void sourceInfoUpdated(const MediaItem &mediaItem);
    void sourceInfoRemoved(QString url);
    void updateCustomColors();
    void volumeChanged(qreal newVolume);
    void skipForward(int i);
    void skipBackward(int i);
    
    void mediaListCategoryActivated(QModelIndex index);
    void mediaListActionActivated(QModelIndex index);
    
 protected:
    bool eventFilter(QObject *obj, QEvent *event);
    
};

#endif // MAINWINDOW_H
