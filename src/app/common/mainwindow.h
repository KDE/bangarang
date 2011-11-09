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

#include "../nowplaying/bangarangvideowidget.h"
#include "../medialists/audiolistsstack.h"
#include "../medialists/videolistsstack.h"
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

class MediaListSettings;
class BangarangApplication;
class KFilterProxySearchLine;

namespace Ui
{
    class MainWindowClass;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    
    enum ContextMenuSource{Default = 0, MediaList = 1, InfoBox = 2, Playlist = 3};
    enum MainWidget{ MainMediaList = 0, MainNowPlaying = 1 };
    enum VideoSize{Normal = 0, Mini = 1 };
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void completeSetup();
    AudioListsStack *audioListsStack();
    VideoListsStack *videoListsStack();
    MediaListSettings *mediaListSettings();
    BangarangVideoWidget * videoWidget();
    void setVideoSize(VideoSize size = Normal);
    VideoSize videoSize();

    void switchMainWidget(MainWindow::MainWidget which);
    MainWidget currentMainWidget();
    QFrame *currentFilterFrame();
    KFilterProxySearchLine* currentFilterProxyLine();
    void enableTouch();
    void resetTabOrder();
    void stopMenuTimer();

    Ui::MainWindowClass *ui;

signals:
    void switchedMainWidget(MainWindow::MainWidget which);
    
public slots:
    void on_fullScreen_toggled(bool fullScreen);
    void toggleMainWidget();

private:
    void setupIcons();
    void setupActions();
    
    BangarangVideoWidget *m_videoWidget;
    QDateTime m_lastMouseMoveTime;
    bool m_nepomukInited;
    BangarangApplication * m_application;
    VideoSize m_videoSize;
    QTimer *m_menuTimer;
    AudioListsStack *m_audioListsStack;
    VideoListsStack *m_videoListsStack;
    MediaListSettings *m_mediaListSettings;

private slots:
    void on_nowPlayingHolder_resized();
    void on_nowPlaying_clicked();
    void on_collectionButton_clicked();
    void on_showPlaylist_clicked();
    void on_showPlaylist_2_clicked();
    void on_seekTime_clicked();
    void on_showMenu_clicked();
    void on_showMenu_2_clicked();
    void on_showMediaViewMenu_clicked();
    void updateCustomColors();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINWINDOW_H
