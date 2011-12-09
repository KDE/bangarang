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

#ifndef BACKGROUNDANIMATION_H
#define BACKGROUNDANIMATION_H

#include <QList>
#include <qtimer.h>

class QWidget;
class QTimer;
class KIcon;
class QPaintEvent;

class TimerCounter: public QTimer
{
    Q_OBJECT
    
public:
    TimerCounter(int max, int interval);
    int getValue();
    
protected:
    virtual void timerEvent(QTimerEvent* event);

private:
    int m_max;
    int m_value;
};


class LoadingAnimation
{

public:
    LoadingAnimation(QWidget *displayWidget, int size);
    ~LoadingAnimation();
    
    void setAnimationSize(int size);
    void startAnimation();
    void stopAnimation();
    
    void paintAnimation(QPaintEvent *event);
    
    
private:
    QWidget *m_display;
    QList<KIcon *> *m_frames;
    TimerCounter *m_counter;
    bool m_running;
    int m_frameCount;
    int m_size;
};

#endif // BACKGROUNDANIMATION_H
