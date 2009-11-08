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

#include <KIcon>
#include <KAboutData>
#include <KHelpMenu>
#include <Phonon/AudioOutput>
#include <Phonon/MediaController>
#include <Phonon/MediaObject>
#include <Phonon/VideoPlayer>
#include <Phonon/VideoWidget>
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

class MediaItem;
class MediaListProperties;
class MediaItemModel;
class Playlist;
class MediaItemDelegate;
class NowPlayingDelegate;
class InfoManager;
class SavedListsManager;
class ActionsManager;

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
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    MediaItemModel * m_audioListsModel;
    MediaItemModel * m_videoListsModel;
    MediaItemModel * m_mediaItemModel;
    MediaItemModel * m_currentPlaylist;
    MediaItemModel * m_nowPlaying;
    Playlist * m_playlist;
    void addListToHistory();
    Ui::MainWindowClass *ui;
    QList< QList<MediaItem> > m_mediaListHistory;
    QList<MediaListProperties> m_mediaListPropertiesHistory;
    ActionsManager * actionsManager();
    void setAboutData(KAboutData *aboutData);
    Playlist * playlist();
    Phonon::AudioOutput * audioOutput();
    
    
private:
    Phonon::VideoPlayer *m_player;
    MediaItemDelegate * m_itemDelegate;
    MediaItemDelegate * m_playlistItemDelegate;
    NowPlayingDelegate * m_nowPlayingDelegate;
    Phonon::VideoWidget *m_videoWidget;
    Phonon::AudioOutput *m_audioOutput;
    Phonon::MediaObject *m_media;
    Phonon::MediaController *m_mediaController;
    QGraphicsScene *m_Scene;
    QString m_addItemsMessage;
    QTime m_messageTime;
    bool playWhenPlaylistChanges;
    bool showRemainingTime;
    void setupModel();
    QList<MediaItem> m_mediaList;
    QList<int> m_mediaListScrollHistory;
    KIcon addItemsIcon();
    void setupIcons();
    void setupActions();
    void showApplicationBanner();
    KIcon turnIconOff(KIcon icon, QSize size);
    bool m_showQueue;
    bool m_repeat;
    bool m_shuffle;
    QDateTime m_lastMouseMoveTime;
    InfoManager * m_infoManager;
    SavedListsManager * m_savedListsManager;
    ActionsManager * m_actionsManager;
    bool m_pausePressed;
    bool m_stopPressed;
    QList<QString> m_devicesAdded;
    void updateCachedDevicesList();
    int m_loadingProgress;
    KAboutData *m_aboutData;
    KHelpMenu *m_helpMenu;
    KMenu *m_menu;
    
    QAction * playAllAction;
    QAction * playSelectedAction;
    
public slots:
    void playAll();
    void playSelected();
    void addSelectedToPlaylist();
    void removeSelectedFromPlaylist();
    void on_fullScreen_toggled(bool fullScreen);
    
private slots:
    void on_nowPlaying_clicked();
    void on_collectionButton_clicked();
    void on_mediaPlayPause_pressed();
    void on_mediaPlayPause_held();
    void on_mediaPlayPause_released();
    void updateSeekTime(qint64 time);
    void updateMuteStatus(bool muted);
    void on_previous_clicked();
    void mediaStateChanged(Phonon::State newstate, Phonon::State oldstate);
    void mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void on_playAll_clicked();
    void on_playSelected_clicked();
    void on_mediaLists_currentChanged(int i);
    void on_showPlaylist_clicked(bool checked);
    void mediaListChanged();
    void audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void audioListsChanged();
    void videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void videoListsChanged();
    void playlistChanged();
    void nowPlayingChanged();
    void playlistFinished();
    void hidePlayButtons();
    void updateListTitle();
    void on_clearPlaylist_clicked();
    void on_playlistView_doubleClicked(const QModelIndex & index);
    void on_seekTime_clicked();
    void on_shuffle_clicked();
    void on_repeat_clicked();
    void on_showQueue_clicked();
    void on_Filter_returnPressed();
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
    void showLoading();
    void on_showMenu_clicked();
    
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    //void keyPressEvent(QKeyEvent *event);
};

class MouseMoveDetector : public QObject
{
    Q_OBJECT
    MouseMoveDetector(QObject * parent = 0) : QObject(parent){}
    ~MouseMoveDetector(){}
    
    Q_SIGNALS:
    void mouseMoved();
    
    protected:
        bool eventFilter(QObject *obj, QEvent *event)
        {
            if (event->type() == QEvent::MouseMove) {
                emit mouseMoved();
            }
            // standard event processing
            return QObject::eventFilter(obj, event);
        }
        
};
#endif // MAINWINDOW_H
