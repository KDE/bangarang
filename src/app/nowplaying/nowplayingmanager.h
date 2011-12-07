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

#ifndef NOWPLAYINGMANAGER_H
#define NOWPLAYINGMANAGER_H

#include <QObject>
#include <QStandardItemModel>
#include <phonon/mediaobject.h>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItem;
class MediaItemModel;
class BangarangApplication;

class NowPlayingManager : public QObject
{
    Q_OBJECT
public:
    explicit NowPlayingManager(MainWindow* parent);

    void connectPhononWidgets();
    void disconnectPhononWidgets();
    void setShowRemainingTime(bool show);
    bool showingRemainingTime();
    bool newPlaylistNotification(QString text, QObject* receiver = NULL, const char* slot = NULL);

signals:
    void playlistNotificationResult(bool confirmed);

public slots:
    void showApplicationBanner();
    void showMenu();
    void showErrorMessage(QString error);
    void togglePlaylist();
    void clearPlaylist();
    void toggleShuffle();
    void toggleRepeat();
    void toggleQueue();

private:
    BangarangApplication* m_application;
    bool m_playWhenPlaylistChanges;
    bool m_showRemainingTime;
    bool m_pausePressed;
    bool m_stopPressed;
    int m_loadingProgress;

private slots:
    void mediaPlayPausePressed();
    void mediaPlayPauseHeld();
    void mediaPlayPauseReleased();
    void playPlaylistItem(const QModelIndex & index);
    void updateSeekTime(qint64 time);
    void mediaStateChanged(Phonon::State newstate, Phonon::State oldstate);
    void showLoading();
    void updateMuteStatus(bool muted);
    void playlistFinished();
    void shuffleModeChanged(bool repeat);
    void repeatModeChanged(bool shuffle);
    void skipForward(int i);
    void skipBackward(int i);
    void nowPlayingChanged();
    void playlistLoading();
    void closePlaylistNotification();
    void selectPlaylistNotificationNo();
    void selectPlaylistNotificationYes();
    void closePlaylistFilter();
};

#endif // NOWPLAYINGMANAGER_H
