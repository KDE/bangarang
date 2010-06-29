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
class BangarangApplication;

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
        QMenu * playlistViewMenu(MainWindow::ContextMenuSource menuSource = MainWindow::Default);
        QMenu * nowPlayingContextMenu();
        KMenu * notifierMenu();
        void setContextMenuSource(MainWindow::ContextMenuSource menuSource);
        const QList<MediaItem> selectedMediaItems();
        QMenu * bookmarksMenu();
        QMenu * dvdMenu();

        bool m_controlsVisible;
        bool m_playlistFilterVisible;
        
    public slots:
        void updateSavedListsMenus();
        
    private:
        void hideShortcutsEditor();
        
        BangarangApplication * m_application;
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        VideoSettings *m_videoSettings;
	
        QMenu *m_addToAudioSavedList;
        QMenu *m_addToVideoSavedList;
        QMenu *m_nowPlayingContextMenu;
        QMenu *m_dvdMenu;
        KMenu *m_notifierMenu;
        bool m_contextStackWasVisible;
        int m_previousContextStackIndex;
        QMenu *m_bookmarksMenu;
        QMenu *m_removeBookmarksMenu;
        
        QActionGroup *m_audioChannelGroup;
        QActionGroup *m_subtitleGroup;
        QActionGroup *m_angleGroup;
        QActionGroup *m_titleGroup;
        QActionGroup *m_chapterGroup;

        //every actionn which is allowed to have a shortcut
        KActionCollection *m_shortcutsCollection;
        //shortcuts that make no sense to have a shortcut
        KActionCollection *m_othersCollection;

        MainWindow::ContextMenuSource m_contextMenuSource;
        KConfigGroup m_shortcutsConfig;
        QString m_restoreFilter;

    private slots:
        void fullScreenToggle();
        void toggleControls();
        void toggleVideoSettings();
        void toggleAudioSettings();
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
        void addToSavedAudioList(QAction *addAction);
        void addToSavedVideoList(QAction *addAction);
        void loadSelectedSources();
        void showInfoForNowPlaying();
        void toggleShowRemainingTimeSlot();
        void togglePlaylistFilter();
        void addBookmarkSlot();
        void activateBookmark(QAction *bookmarkAction);
        void removeBookmark(QAction *bookmarkAction);
        void updateOntologies();
        void audioChannelChanged(QAction *action);
        void subtitleChanged(QAction *action);
        void angleChanged(QAction *action);
        void titleChanged(QAction *action);
        void chapterChanged(QAction *action);
};
#endif //ACTIONSMANAGER_H
