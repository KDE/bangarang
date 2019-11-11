/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Stefan Burnicki (stefan.burnicki@gmx.de)
* <https://commits.kde.org/bangarang>
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


#include "loadinganimation.h"
#include "timercounter.h"

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <KIcon>

LoadingAnimation::LoadingAnimation(QWidget* displayWidget, int size)
{
    m_display = displayWidget;
    m_frameCount = 8;
    m_size = size;
    m_frames = new QList<KIcon *>();
    m_counter = new TimerCounter(m_frameCount - 1, 100);
    QObject::connect(m_counter, SIGNAL(timeout()), m_display, SLOT(repaint()));
    
    for (int i = 0; i < m_frameCount; i++) {
        KIcon *ico = new KIcon(QString("bangarang-loading-%1").arg(i));
        m_frames->append(ico);
    }
}

LoadingAnimation::~LoadingAnimation()
{
    foreach (KIcon *ico, *m_frames) {
        delete ico;
    }
}

void LoadingAnimation::setAnimationSize(int size)
{
    m_size = size;
}

void LoadingAnimation::startAnimation()
{
    m_counter->start();
}

void LoadingAnimation::stopAnimation()
{
    m_counter->stop();
    m_display->repaint();
}

void LoadingAnimation::paintAnimation(QPaintEvent* event)
{
    int val = m_counter->getValue();
    if (!m_counter->isActive() || m_frames->size() <= val) {
        return;
    }
    const QPixmap &cur = m_frames->at(val)->pixmap(m_size, m_size);
    QRect pmRect = cur.rect();
    pmRect.moveCenter(m_display->rect().center());
    if (pmRect.intersects(event->rect())) {
        QPainter painter(m_display);
        painter.drawPixmap(pmRect.left(), pmRect.top(), cur);
    }
}
