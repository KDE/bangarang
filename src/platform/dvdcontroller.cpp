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

#include "dvdcontroller.h"
#include "utilities/utilities.h"

#include <KLocalizedString>
#include <phonon/ObjectDescription>

DvdController::DvdController(QWidget* parent, Phonon::MediaController* mediaController)
             : QObject(parent)
{
    m_parent = parent;
    
    m_menu = new QMenu(i18n("DVD Menu"), m_parent);
  
    m_audioChannelGroup = NULL;
    m_subtitleGroup = NULL;
    m_angleGroup = NULL;
    m_titleGroup = NULL;
    m_chapterGroup = NULL;
    
    m_audioChannelMenu = NULL;
    m_subtitleMenu = NULL;
    m_angleMenu = NULL;
    m_titleMenu = NULL;
    m_chapterMenu = NULL;
    
    m_showAngleMenu = true;
    m_showChapterMenu = true;
    m_showTitleMenu = true;
    m_showAudioChannelMenu = true;
    m_showSubtitleMenu = true;
    
    setMediaController(mediaController);
}

void DvdController::setMediaController(Phonon::MediaController* mediaController)
{
    m_mediaController = mediaController;

    connect(m_mediaController, SIGNAL(availableAnglesChanged(int)), this, SLOT(updateAngles(int)));
    connect(m_mediaController, SIGNAL(angleChanged(int)), this, SLOT(updateCurrentAngle(int)));
    
    connect(m_mediaController, SIGNAL(availableTitlesChanged(int)), this, SLOT(updateTitles(int)));
    connect(m_mediaController, SIGNAL(titleChanged(int)), this, SLOT(updateCurrentTitle(int)));
    
    connect(m_mediaController, SIGNAL(availableChaptersChanged(int)), this, SLOT(updateChapters(int)));
    connect(m_mediaController, SIGNAL(chapterChanged(int)), this, SLOT(updateCurrentChapter(int)));
    
    connect(m_mediaController, SIGNAL(availableAudioChannelsChanged()), this, SLOT(updateAudioChannels()));
    
    connect(m_mediaController, SIGNAL(availableSubtitlesChanged()), this, SLOT(updateSubtitles()));
    
    updateAngles(m_mediaController->availableAngles());
    updateChapters(m_mediaController->availableChapters());
    updateTitles(m_mediaController->availableTitles());
    updateAudioChannels();
    updateSubtitles();
}

void DvdController::updateMenu()
{
    m_menu->clear();
    if (m_audioChannelMenu && m_showAudioChannelMenu)
        m_menu->addMenu(m_audioChannelMenu);
    if (m_subtitleMenu && m_showSubtitleMenu)
        m_menu->addMenu(m_subtitleMenu);
    if (m_titleMenu && m_showTitleMenu)
        m_menu->addMenu(m_titleMenu);
    if (m_chapterMenu && m_showChapterMenu)
        m_menu->addMenu(m_chapterMenu);
    if (m_angleMenu && m_showAngleMenu)
        m_menu->addMenu(m_angleMenu);
}

void DvdController::setAngle(int angle)
{
    m_mediaController->setCurrentAngle(angle);
    updateCurrentAngle(angle);
}

void DvdController::setChapter(int chapter)
{
    m_mediaController->setCurrentChapter(chapter);
    updateCurrentChapter(chapter);
}

void DvdController::setTitle(int title)
{
    //as the user selected the title to be played we have to enable autoplayTitles and then reset
    //it or phonon wouldn't start the playback
    //we also try to restore the selected subtitles and audio channel. As the title can have different
    //audio channels the SubtitleDescription and AudioDescription will differ from the old ones. So
    //we will check if a subtitle/audiochannel with the same name and description exists and we 
    //will set them if so
    bool oldAutoplay = m_mediaController->autoplayTitles();
    Phonon::AudioChannelDescription oldAudio = m_mediaController->currentAudioChannel();
    Phonon::SubtitleDescription oldSub = m_mediaController->currentSubtitle();
    
    m_mediaController->setAutoplayTitles(true);
    m_mediaController->setCurrentTitle( title );
    m_mediaController->setAutoplayTitles(oldAutoplay);
    
    QList<Phonon::AudioChannelDescription> chans = m_mediaController->availableAudioChannels();
    foreach(Phonon::AudioChannelDescription cur, chans) {
         if ( cur.name() == oldAudio.name() && cur.description() == oldAudio.description())
             m_mediaController->setCurrentAudioChannel(cur);
    }
    
    QList<Phonon::SubtitleDescription> subs = m_mediaController->availableSubtitles();
    foreach(Phonon::SubtitleDescription cur, subs) {
         if ( cur.name() == oldSub.name() && cur.description() == oldSub.description())
             m_mediaController->setCurrentSubtitle(cur);
    }
    updateCurrentTitle(title);
}

void DvdController::setAudioChannel(int idx)
{
    Phonon::AudioChannelDescription selected = Phonon::AudioChannelDescription::fromIndex(idx);
    if ( selected != m_mediaController->currentAudioChannel() )
        m_mediaController->setCurrentAudioChannel( selected );
    updateCurrentAudioChannel();
}

void DvdController::setSubtitle(int idx)
{
    Phonon::SubtitleDescription selected = Phonon::SubtitleDescription::fromIndex(idx);
    if ( selected != m_mediaController->currentSubtitle() )
        m_mediaController->setCurrentSubtitle( selected );
    updateCurrentSubtitle();
}

void DvdController::updateAngles(int count)
{
    bool show = (count > 1);

    SAVE_DELETE_OBJ(m_angleGroup);
    SAVE_DELETE_OBJ(m_angleMenu);

    if (!show) {
        updateMenu();
        return;
    }
    m_angleGroup = new QActionGroup(this);
    m_angleGroup->setExclusive(true);
    m_angleMenu = new QMenu(i18n("Angles"));
    
    for (int i = 0; i < count; i++ ) {
        QAction *ac = m_angleGroup->addAction(QString("%1").arg( i + 1));
        ac->setCheckable(true);
        ac->setData(i);
        m_angleMenu->addAction(ac);
    }
    connect(m_angleGroup, SIGNAL(selected(QAction*)), this, SLOT(angleSelected(QAction*)));
    updateCurrentAngle(m_mediaController->currentAngle());
    updateMenu();
}

void DvdController::updateChapters(int count)
{
    bool show = (count > 1);
    
    SAVE_DELETE_OBJ(m_chapterGroup);
    SAVE_DELETE_OBJ(m_chapterMenu);
    
    if (!show) {
        updateMenu();
        return;
    }

    m_chapterGroup = new QActionGroup(this);
    m_chapterGroup->setExclusive(true);
    m_chapterMenu = new QMenu(i18n("Chapters"));
    
    for (int i = 0; i < count; i++ ) {
        QAction *ac = m_chapterGroup->addAction(QString("%1").arg( i + 1));
        ac->setCheckable(true);
        ac->setData(i);
        m_chapterMenu->addAction(ac);
    }
    connect(m_chapterGroup, SIGNAL(selected(QAction*)), this, SLOT(chapterSelected(QAction*)));
    updateCurrentChapter(m_mediaController->currentChapter());
    updateMenu();
}

void DvdController::updateTitles(int count)
{
    bool show = (count > 1);
    SAVE_DELETE_OBJ(m_titleGroup);
    SAVE_DELETE_OBJ(m_titleMenu);
    
    if (!show) {
        updateMenu();
        return;
    }
    
    m_titleGroup = new QActionGroup(this);
    m_titleGroup->setExclusive(true);
    m_titleMenu = new QMenu(i18n("Titles"));
    
    for (int i = 0; i < count; i++ ) {
        QAction *ac = m_titleGroup->addAction(QString("%1").arg( i + 1));
        ac->setCheckable(true);
        ac->setData(i);
        m_titleMenu->addAction(ac);
    }
    connect(m_chapterGroup, SIGNAL(selected(QAction*)), this, SLOT(titleSelected(QAction*)));
    updateCurrentTitle(m_mediaController->currentTitle());
    updateMenu();
}

void DvdController::updateAudioChannels()
{
    QList<Phonon::AudioChannelDescription> descs = m_mediaController->availableAudioChannels();
    int count = descs.count();
    bool show = (count > 1);
    
    SAVE_DELETE_OBJ(m_audioChannelGroup);
    SAVE_DELETE_OBJ(m_audioChannelMenu);
      
    if (!show) {
        updateMenu();
        return;
    }  
    
    m_audioChannelGroup = new QActionGroup(this);
    m_audioChannelGroup->setExclusive(true);
    m_audioChannelMenu = new QMenu(i18n("Audio Channels"));
    
    foreach(Phonon::AudioChannelDescription cur, descs) {
        QString name = cur.name();
        if ( !cur.description().isEmpty() )
            name += " (" + cur.description() + ")";
        QAction *ac = m_audioChannelGroup->addAction(name);
        ac->setData(cur.index());
        ac->setCheckable(true);
        m_audioChannelMenu->addAction(ac);
    }
    connect(m_audioChannelGroup, SIGNAL(selected(QAction*)), this, SLOT(audioChannelSelected(QAction*)));
    updateCurrentAudioChannel();  
    updateMenu();
}

void DvdController::updateSubtitles()
{
    QList<Phonon::SubtitleDescription> descs = m_mediaController->availableSubtitles();
    int count = descs.count();
    bool show = (count > 0);

    SAVE_DELETE_OBJ(m_subtitleGroup);
    SAVE_DELETE_OBJ(m_subtitleMenu);
    
    if (!show) {
        updateMenu();
        return;
    }
    
    m_subtitleGroup = new QActionGroup(this);
    m_subtitleGroup->setExclusive(true);
    m_subtitleMenu = new QMenu(i18n("Subtitles"));
    
    QAction *ac = m_subtitleGroup->addAction(i18n("Disable"));
    ac->setData(-1);
    ac->setCheckable(true);
    m_subtitleMenu->addAction(ac);
    foreach(Phonon::SubtitleDescription cur, descs) {
        QString name = cur.name();
        if ( !cur.description().isEmpty() )
            name += " (" + cur.description() + ")";
        ac = m_subtitleGroup->addAction(name);
        ac->setData(cur.index());
        ac->setCheckable(true);
        m_subtitleMenu->addAction(ac);
    }
    connect(m_subtitleGroup, SIGNAL(selected(QAction*)), this, SLOT(subtitleSelected(QAction*)));
    updateCurrentSubtitle();  
    updateMenu();
}


void DvdController::updateCurrentAngle(int idx)
{
    if (m_angleMenu) {
        QList<QAction *> acs = m_angleMenu->actions();
        if (acs.count() > idx )
            acs.at(idx)->setChecked(true);
    }
}

void DvdController::updateCurrentChapter(int idx)
{
    if (m_chapterMenu) {
        QList<QAction *> acs = m_chapterMenu->actions();
        if (acs.count() > idx )
            acs.at(idx)->setChecked(true);
    }
}

void DvdController::updateCurrentTitle(int idx)
{
    if (m_titleMenu) {
        QList<QAction *> acs = m_titleMenu->actions();
        if (acs.count() > idx )
            acs.at(idx)->setChecked(true);
    }
}

void DvdController::updateCurrentAudioChannel()
{
    if (m_audioChannelMenu) {
        QList<QAction *> acs = m_audioChannelMenu->actions();
        int idx = m_mediaController->currentAudioChannel().index();
        if (acs.count() > idx )
            acs.at(idx)->setChecked(true);
    }
}

void DvdController::updateCurrentSubtitle()
{
    if (m_subtitleMenu) {
        QList<QAction *> acs = m_subtitleMenu->actions();
        int idx = m_mediaController->currentSubtitle().index() + 1;// +1 because of the "disable" entry
        if (acs.count() > idx )
            acs.at(idx)->setChecked(true);
    }
}

void DvdController::angleSelected(QAction* action)
{
    QVariant data = action->data();
    if ( data.isValid() )
        setAngle(data.toInt());
}

void DvdController::chapterSelected(QAction* action)
{
    QVariant data = action->data();
    if ( data.isValid() )
        setChapter(data.toInt());
}

void DvdController::titleSelected(QAction* action)
{
    QVariant data = action->data();
    if ( data.isValid() )
        setTitle(data.toInt());
}

void DvdController::audioChannelSelected(QAction* action)
{
    QVariant data = action->data();
    if ( data.isValid() )
        setAudioChannel(data.toInt());
}

void DvdController::subtitleSelected(QAction* action)
{
    QVariant data = action->data();
    if ( data.isValid() )
        setSubtitle(data.toInt());
}









