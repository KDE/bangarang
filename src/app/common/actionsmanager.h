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
class BangarangApplication;
class VideoSettings;

/*
 * This class creates and manages all actions for bangarang. 
 */
class ActionsManager : public QObject
{
    Q_OBJECT
    
    public:
        ActionsManager(MainWindow * parent);
        ~ActionsManager();
        
        KActionCollection *shortcutsCollection() { return m_shortcutsCollection; }
        KActionCollection *othersCollection() { return m_othersCollection; }

        QAction *action( QString name, bool shortcutsOnly = false );

        QMenu *addToSavedAudioListMenu();
        QMenu *addToSavedVideoListMenu();

        QMenu * mediaViewMenu(bool showAbout = false, MainWindow::ContextMenuSource menuSource = MainWindow::Default);
        QMenu * playlistViewMenu();
        QMenu * nowPlayingContextMenu();

        KMenu * nowPlayingMenu();
        KMenu * notifierMenu();
        void setContextMenuSource(MainWindow::ContextMenuSource menuSource);
        const QList<MediaItem> selectedMediaItems();
        QMenu * bookmarksMenu();
        QMenu * infoMenu();
        void updateToggleFilterText();

        bool m_controlsVisible;
        
    public slots:
        void updateSavedListsMenus();
        
    private:
        BangarangApplication * m_application;
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        VideoSettings *m_videoSettings;
	
        QMenu *m_addToAudioSavedList;
        QMenu *m_addToVideoSavedList;
        QMenu *m_nowPlayingContextMenu;
        KMenu *m_nowPlayingMenu;
        bool m_contextStackWasVisible;
        int m_previousContextStackIndex;
        QMenu *m_bookmarksMenu;
        QMenu *m_removeBookmarksMenu;
        QMenu *m_infoMenu;
        
        //every actionn which is allowed to have a shortcut
        KActionCollection *m_shortcutsCollection;
        //shortcuts that make no sense to have a shortcut
        KActionCollection *m_othersCollection;

        MainWindow::ContextMenuSource m_contextMenuSource;
        KConfigGroup m_shortcutsConfig;
        QString m_playlistRestoreFilter;
        QString m_mediaListRestoreFilter;

    private slots:
        void mediaViewRefresh();
        void fullScreenToggle();
        void toggleControls();
        void toggleVideoSettings();
        void toggleAudioSettings();
        void cancelFSHC();
        void toggleShortcutsEditor();
        void saveShortcuts();
        void cancelShortcuts();
        void simplePlayPause();
        void smartPlay();
        void muteAudio();
        void addSelectedToPlaylistSlot();
        void addAfterNowPlaying();
        void removeSelectedFromPlaylistSlot();
        void removePlaylistSelectionFromPlaylistSlot();
        void removeSelectedItemsInfoSlot();
        void playSelectedSlot();
        void playAllSlot();
        void addToSavedAudioList(QAction *addAction);
        void addToSavedVideoList(QAction *addAction);
        void loadSelectedSources();
        void showInfoForNowPlaying();
        void toggleShowRemainingTimeSlot();
        void toggleFilter();
        void addBookmarkSlot();
        void activateBookmark(QAction *bookmarkAction);
        void removeBookmark(QAction *bookmarkAction);
        void updateOntologies();
        void addTemporaryAudioStreams();
        void toggleInfoView();
};
#endif //ACTIONSMANAGER_H
