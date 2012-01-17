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


#include "timercounter.h"

TimerCounter::TimerCounter(int max, int interval, bool loop): QTimer()
{
    m_max = max;
    setInterval(interval);
    m_value = 0;
    m_loop = loop;
}

int TimerCounter::getValue()
{
    return m_value;
}

int TimerCounter::max()
{
    return m_max;
}

void TimerCounter::setMax(int max)
{
    m_max = max;
}


void TimerCounter::reset()
{
    m_value = 0;
    start();
}

void TimerCounter::timerEvent(QTimerEvent* event)
{
    m_value++;
    if ( m_value > m_max ) {
        if (m_loop) {
            m_value = 0;
        }
        m_value--; //do not loop, stay at max
        stop();
    }
    QTimer::timerEvent(event);
}
