/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Stefan Burnicki (stefan.burnicki@gmx.de)
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
#ifndef DVDCONTROLLER_H
#define DVDCONTROLLER_H

#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include <phonon/MediaController>

class DvdController : public QObject
{
    Q_OBJECT
    public:

        DvdController(QWidget* parent, Phonon::MediaController* mediaController);
        
        void setMediaController(Phonon::MediaController *mediaController);
        Phonon::MediaController * mediaController() { return m_mediaController; }
        
        QMenu *menu() { return m_menu; }
        void updateMenu();
        
        QMenu *angleMenu() { return m_angleMenu; }
        QMenu *chapterMenu() { return m_chapterMenu; }
        QMenu *titleMenu() { return m_titleMenu; }
        QMenu *audioChannelMenu() { return m_audioChannelMenu; }
        QMenu *subtitleMenu() { return m_subtitleMenu; }
        
        void setShowAngleMenu(bool show) { m_showAngleMenu = show; }
        void setShowChapterMenu(bool show) { m_showChapterMenu = show; }
        void setShowTitleMenu(bool show) { m_showTitleMenu = show; }
        void setShowAudioChannelMenu(bool show) { m_showAudioChannelMenu = show; }
        void setShowSubtitleMenu(bool show) { m_showSubtitleMenu = show; }
        
        bool showAngleMenu() { return m_showAngleMenu; }
        bool showChapterMenu() { return m_showChapterMenu; }
        bool showTitleMenu() { return m_showTitleMenu; }
        bool showAudioChannelMenu() { return m_showAudioChannelMenu; }
        bool showSubtitleMenu() { return m_showSubtitleMenu; }
        
        void setAngle(int angle);
        void setChapter(int chapter);
        void setTitle(int title);
        void setAudioChannel(int idx);
        void setSubtitle(int idx);  
        
    protected:
        Phonon::MediaController *m_mediaController;
        QMenu *m_menu;
        QWidget *m_parent;
        
        bool m_showAngleMenu;
        bool m_showChapterMenu;
        bool m_showTitleMenu;
        bool m_showAudioChannelMenu;
        bool m_showSubtitleMenu;
        
        QMenu *m_angleMenu;
        QMenu *m_chapterMenu;
        QMenu *m_titleMenu;
        QMenu *m_audioChannelMenu;
        QMenu *m_subtitleMenu;
        
        QActionGroup *m_angleGroup;
        QActionGroup *m_chapterGroup;
        QActionGroup *m_titleGroup;
        QActionGroup *m_audioChannelGroup;
        QActionGroup *m_subtitleGroup;
        
    protected slots:
        void updateAngles(int count);
        void updateChapters(int count);
        void updateTitles(int count);
        void updateAudioChannels();
        void updateSubtitles();
             
        void updateCurrentAngle(int idx);
        void updateCurrentChapter(int idx);
        void updateCurrentTitle(int idx);
        void updateCurrentAudioChannel();
        void updateCurrentSubtitle();
        
        void angleSelected(QAction *action);
        void chapterSelected(QAction *action);    
        void titleSelected(QAction *action);
        void audioChannelSelected(QAction *action);
        void subtitleSelected(QAction *action);
};

#endif // DVDCONTROLLER_H
