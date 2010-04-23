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
#include <KConfigGroup>
#include <KGlobalAccel>
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
        
        KAction *quit();
        KAction *playAll();
        KAction *playSelected();
        KAction *addSelectedToPlaylist();
        KAction *removeSelectedFromPlaylist();
        KAction *fullScreen();
        KAction *showHideControls();
        KAction *cancelFullScreenHideControls();
        KAction *editShortcuts();
        KAction *toggleShowRemainingTime();
        KAction *playPause();
        KAction *play();
        KAction *pause();
        KAction *playNext();
        KAction *playPrevious();
        KAction *mute();
        KAction *removeSelectedItemsInfo();
        KAction *refreshMediaView();
        KAction *showVideoSettings();
        KAction *removeFromSavedList();
        KAction *newAudioList();
        KAction *newVideoList();
        KAction *showItems();
        KAction *showNowPlayingInfo();
        KAction *showInfo();
        KAction *addBookmark();
        KAction *showScriptingConsole();
        KAction *showHidePlaylistFilter();
        QMenu *addToSavedAudioListMenu();
        QMenu *addToSavedVideoListMenu();

        QMenu * mediaViewMenu(bool showAbout = false, MainWindow::ContextMenuSource menuSource = MainWindow::Default);
        QMenu * nowPlayingContextMenu();
        KMenu * notifierMenu();
        void setContextMenuSource(MainWindow::ContextMenuSource menuSource);
        const QList<MediaItem> selectedMediaItems();
        QMenu * bookmarksMenu();

        bool m_controlsVisible;
        bool m_playlistFilterVisible;
        
    public slots:
        void updateSavedListsMenus();
        
    private:

        void hideShortcutsEditor();

        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        VideoSettings *m_videoSettings;
	
        KAction *m_quit;
        KAction *m_playAllAction;
        KAction *m_playSelectedAction;
        KAction *m_addSelectedToPlayListAction;
        KAction *m_removeSelectedToPlayListAction;
        KAction *m_showHideControls;
        KAction *m_fullScreen;
        KAction *m_cancelFullScreenHideControls;
        KAction *m_editShortcuts;
        KAction *m_toggleShowRemainingTime;
        KAction *m_playPause;
        KAction *m_play;
        KAction *m_pause;
        KAction *m_playNext;
        KAction *m_playPrevious;
        KAction *m_mute;
        KAction *m_removeSelectedItemsInfo;
        KAction *m_refreshMediaView;
        KAction *m_showVideoSettings;
        KAction *m_removeFromSavedList;
        KAction *m_newAudioList;
        KAction *m_newVideoList;
        KAction *m_showItems;
        KAction *m_showInfo;
        KAction *m_showNowPlayingInfo;
        KAction *m_addBookmark;
        KAction *m_showScriptingConsole;
        QMenu *m_addToAudioSavedList;
        QMenu *m_addToVideoSavedList;
        QMenu *m_nowPlayingContextMenu;
        KMenu *m_notifierMenu;
        bool m_contextStackWasVisible;
        int m_previousContextStackIndex;
        QMenu *m_bookmarksMenu;
        QMenu *m_removeBookmarksMenu;
        KAction* m_showHidePlaylistFilter;

        KActionCollection *m_actionCollection;
        MainWindow::ContextMenuSource m_contextMenuSource;
        KConfigGroup m_shortcutsConfig;

    private slots:
        void fullScreenToggle();
        void toggleControls();
        void toggleVideoSettings();
        void cancelFSHC();
        void showShortcutsEditor();
        void saveShortcuts();
        void cancelShortcuts();
        void simplePlayPause();
        void smartPlay();
        void muteAudio();
        void addSelectedToPlaylistSlot();
        void removeSelectedFromPlaylistSlot();
        void removeSelectedItemsInfoSlot();
        void playSelectedSlot();
        void playAllSlot();
        void addToSavedAudioList(KAction *addAction);
        void addToSavedVideoList(KAction *addAction);
        void loadSelectedSources();
        void showInfoForNowPlaying();
        void showScriptConsoleSlot();
        void toggleShowRemainingTimeSlot();
        void togglePlaylistFilter();
        void addBookmarkSlot();
        void activateBookmark(KAction *bookmarkAction);
        void removeBookmark(KAction *bookmarkAction);
};
#endif //ACTIONSMANAGER_H
