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

#ifndef BANGARANGNOTIFIERITEM_H
#define BANGARANGNOTIFIERITEM_H

#include <QIcon>
#include <phonon/phononnamespace.h>

#include <KStatusNotifierItem>

/**
* Bangarang KStatusNotifierIcon.
*
* Represents the app in the systray.
*
* If Phonon::State is playing or paused a respective overlay icon is shown.
*
* If the user performs an mouse scroll action on the icon, either a changeTrackRequest or
* changeVolumeRequest is emitted.
* If the icon recognizes a middle click the changeStateRequest will be emitted.
*
* @author Steve Fischer <fischenderHobbit@web.de>
*/
class BangarangNotifierItem : public KStatusNotifierItem
{
  Q_OBJECT
    
  public:
    BangarangNotifierItem(QObject* parent = 0);
    virtual ~BangarangNotifierItem();
    
    Phonon::State state() const;
    void setState(Phonon::State state);
    
  signals:
    /**
    * Inform the associated window that the state should changed either from playing to paused or 
    * from paused to playing.
    * @param newState either 'Playing' or 'Paused'
    */
    void changeStateRequested(Phonon::State newState);
    
    /**
    * Inform the associated window that the volume should be increased or decreased.
    * @param delta increase/decrease value
    */
    void changeVolumeRequested(int delta);
    
  private:
    
    Phonon::State m_currentState;
    
  private slots:
    void handleMiddleClick();
    void handleScrollRequested(int delta, Qt::Orientation orientation);
};

#endif // BANGARANGNOTIFIERITEM_H
