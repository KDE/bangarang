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

#include "starrating.h"
#include <QPainter>
#include <QSize>
#include <QRect>

StarRating::StarRating(int rating, int size, QPoint point, QPoint hoverPos) {
    m_rating = rating;
    m_point = point;
    setHoverAtPosition(hoverPos);
    setSize(size);
}

void StarRating::paint(QPainter *painter) {
    painter->save();
    bool hover = (m_hoverRating != InvalidRating && m_rating != m_hoverRating);
    int lowRating = m_rating;
    int highRating = m_hoverRating;
    bool lower = ( hover && m_hoverRating < m_rating);
    if (lower) {
        lowRating = m_hoverRating;
        highRating = m_rating;
    }

    painter->translate(m_point + QPoint(StarRating::Margin, StarRating::Margin));
    int rating = lowRating;
    QPixmap *star = &m_starNormal;
    QPixmap *star_next = hover ? &m_starHover : &m_starInactive;
    for (int i = 1; i <= MaxRating; i++) {
        if ( i > rating ) {
            if (hover) {
                rating = highRating;
                star = &m_starHover;
                star_next = &m_starInactive;
                hover = false;
            } else {
                rating = MaxRating;
                star = &m_starInactive;
                star_next = &m_starInactive;
            }
        }
        if (i % 2 == 0) {
            painter->drawPixmap((((i / 2) - 1) * (m_starSize + 2)), 0, *star);
        } else if( i == rating) {
            int halfWidth = (m_starSize / 2);
            int pos = (int) (i / 2);
            QRect srcRect = QRect(QPoint(0, 0),
                                QSize(halfWidth, m_starSize));
            painter->drawPixmap(QPoint((pos * (m_starSize + 2)), 0), *star, srcRect);
            srcRect = QRect(QPoint(halfWidth, 0),
                            QSize(halfWidth, m_starSize));
            painter->drawPixmap(QPoint((pos * (m_starSize + 2)) + halfWidth, 0), *star_next, srcRect);
            i++;
        }
    }

    painter->restore();
}

void StarRating::setSize(int size) {
    int px = (size < Small) ? Small : size;
    QSize sz = QSize(px, px);
    KIcon icon = KIcon("rating");
    m_starNormal = icon.pixmap(sz);
    m_starSize = m_starNormal.size().width(); //maybe we didn't get the full size
    sz = QSize(m_starSize, m_starSize);
    m_starInactive = icon.pixmap(sz, QIcon::Disabled);

    m_starHover = QPixmap(sz);
    QColor transBlack = Qt::black;
    transBlack.setAlpha(100);
    m_starHover.fill(transBlack);
    QPainter p(&m_starHover);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.drawPixmap(QPoint(0,0), m_starNormal);
    p.end();
}
