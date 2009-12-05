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

#include <KActionCollection>
#include <QObject>
#include <QAction>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;

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
        QAction *playNext();
        QAction *playPrevious();
        QAction *mute();
        QAction *removeSelectedItemsInfo();
        QAction *refreshMediaView();
        
        QMenu * mediaViewMenu(bool showAbout = false);
        
    public slots:
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        
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
        QAction *m_playNext;
        QAction *m_playPrevious;
        QAction *m_mute;
        QAction *m_removeSelectedItemsInfo;
        QAction *m_refreshMediaView;
        KActionCollection *m_actionCollection;
        
    private slots:
        void fullScreenToggle();
        void toggleControls();
        void cancelFSHC();
        void showShortcutsEditor();
        void hideShortcutsEditor();
        void simplePlayPause();
        void muteAudio();
        
};
#endif //ACTIONSMANAGER_H