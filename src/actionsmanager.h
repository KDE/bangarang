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

#ifndef ACTIONSMANAGER_H
#define ACTIONSMANAGER_H

#include "videosettings.h"
#include "mainwindow.h"
#include <KMenu>
#include <KActionCollection>
#include <QObject>
#include <QAction>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;

/*
 * This class creates and manages all actions for bangarang. 
 */
class ActionsManager : public QObject
{
    Q_OBJECT
    
    public:
        ActionsManager(MainWindow * parent);
        ~ActionsManager();
        
        QAction *quit();
        QAction *playAll();
        QAction *playSelected();
        QAction *addSelectedToPlaylist();
        QAction *removeSelectedFromPlaylist();
        QAction *fullScreen();
        QAction *showHideControls();
        QAction *cancelFullScreenHideControls();
        QAction *editShortcuts();
        QAction *playPause();
        QAction *play();
        QAction *pause();
        QAction *playNext();
        QAction *playPrevious();
        QAction *mute();
        QAction *removeSelectedItemsInfo();
        QAction *refreshMediaView();
        QAction *showVideoSettings();
        QAction *removeFromSavedList();
        QAction *newAudioList();
        QAction *newVideoList();
        QAction *showItems();
        QAction *showNowPlayingInfo();
        QAction *showInfo();
        QAction *showScriptingConsole();
        QMenu *addToSavedAudioListMenu();
        QMenu *addToSavedVideoListMenu();

        QMenu * mediaViewMenu(bool showAbout = false, MainWindow::ContextMenuSource menuSource = MainWindow::Default);
        QMenu * nowPlayingContextMenu();
        KMenu * notifierMenu();
        void setContextMenuSource(MainWindow::ContextMenuSource menuSource);
        const QList<MediaItem> selectedMediaItems();
        
    public slots:
        void updateSavedListsMenus();
        
    private:

        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        VideoSettings *m_videoSettings;
	
        QAction *m_quit;
        QAction *m_playAllAction;
        QAction *m_playSelectedAction;
        QAction *m_addSelectedToPlayListAction;
        QAction *m_removeSelectedToPlayListAction;
        QAction *m_showHideControls;
        QAction *m_fullScreen;
        QAction *m_cancelFullScreenHideControls;
        QAction *m_editShortcuts;
        QAction *m_playPause;
        QAction *m_play;
        QAction *m_pause;
        QAction *m_playNext;
        QAction *m_playPrevious;
        QAction *m_mute;
        QAction *m_removeSelectedItemsInfo;
        QAction *m_refreshMediaView;
        QAction *m_showVideoSettings;
        QAction *m_removeFromSavedList;
        QAction *m_newAudioList;
        QAction *m_newVideoList;
        QAction *m_showItems;
        QAction *m_showInfo;
        QAction *m_showNowPlayingInfo;
        QAction *m_showScriptingConsole;
        QMenu *m_addToAudioSavedList;
        QMenu *m_addToVideoSavedList;
        QMenu *m_nowPlayingContextMenu;
        KMenu *m_notifierMenu;
        bool m_contextStackWasVisible;
        int m_previousContextStackIndex;

        KActionCollection *m_actionCollection;
        MainWindow::ContextMenuSource m_contextMenuSource;
        
    private slots:
        void fullScreenToggle();
        void toggleControls();
        void toggleVideoSettings();
        void cancelFSHC();
        void showShortcutsEditor();
        void hideShortcutsEditor();
        void simplePlayPause();
        void smartPlay();
        void muteAudio();
        void addSelectedToPlaylistSlot();
        void removeSelectedFromPlaylistSlot();
        void removeSelectedItemsInfoSlot();
        void playSelectedSlot();
        void playAllSlot();
        void addToSavedAudioList(QAction *addAction);
        void addToSavedVideoList(QAction *addAction);
        void loadSelectedSources();
        void showInfoForNowPlaying();
        void showScriptConsoleSlot();
};
#endif //ACTIONSMANAGER_H
