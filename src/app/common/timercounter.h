/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Stefan Burnicki (stefan.burnicki@gmx.de)
* <https://projects.kde.org/projects/playground/multimedia/bangarang>
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

#ifndef TIMERCOUNTER_H
#define TIMERCOUNTER_H

#include <QTimer>

class QWidget;
class QTimer;
class KIcon;
class QPaintEvent;

class TimerCounter: public QTimer
{
    Q_OBJECT
    
public:
    TimerCounter(int max, int interval, bool loop = true);
    int getValue();
    void setMax(int max);
    int max();
    void reset();
    
protected:
    virtual void timerEvent(QTimerEvent* event);

private:
    int m_max;
    int m_value;
    bool m_loop;
};

#endif // TIMERCOUNTER_H
