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

#ifndef STARRATINGWIDGET_H
#define STARRATINGWIDGET_H

#include <KIcon>
#include <QIcon>
#include <QMetaType>

/*
 This file will provide the StarRatingWidget, which is a QWidget and the StarRating which is
 responsible for painting rating stars. It will support ratings from 1 to 10 in different sizes
*/
#include <QPainter>

class StarRating {

    public:
        enum Size { Small = 8, Medium = 12, Big = 16 };
        enum MinMax{ InvalidRating = -1, MinRating = 0, MaxRating = 10 };

        static const int Margin = 1;

        StarRating(int rating = 0, int size = Small,
                   QPoint point = QPoint(0, 0), QPoint hoverPos = QPoint(-1, -1));

        void setRating(int rating) { m_rating = rating; }
        void setSize(int size);
        void setPoint(QPoint point) { m_point = point; }
        void setHoverAtPosition(QPoint point) { m_hoverRating = ratingAtPosition(point); }

        bool valid(int rating) const { return (rating >= MinRating && rating <= MaxRating); }
        int rating() { return m_rating; }
        QSize sizeHint() const { return StarRating::SizeHint(m_starSize); }
        int ratingAtPosition(QPoint point) const { return StarRating::RatingAtPosition(point, m_starSize, m_point); }
        void paint(QPainter *painter);

        static QSize SizeHint(int size) {
            // *__*__*__*__*   +  StarRating::Margin around it
            int bothMargin = (StarRating::Margin * 2);
            return QSize( 5 * (size + 2) - 2, size) + QSize(bothMargin, bothMargin);
        }

        static int RatingAtPosition(QPoint point, int starSize, QPoint offset) {
            QPoint p = point - offset; //relative to the rating pic now
            QSize sz = StarRating::SizeHint(starSize);
            if (p.x() < 0 || p.y() < 0 || p.x() > sz.width() || p.y() > sz.height())
                return StarRating::InvalidRating;
            int in_x = p.x() - StarRating::Margin + starSize / 4;
            int real_width = sz.width() - StarRating::Margin * 2;
            int rating = (int) ((StarRating::MaxRating * in_x) / real_width);
            if (rating > StarRating::MaxRating || rating < StarRating::MinRating)
                rating = StarRating::InvalidRating;
            return rating;
        }

    protected:
        QPixmap m_starHover;
        QPixmap m_starNormal;
        QPixmap m_starInactive;

        int m_rating;
        int m_hoverRating;
        QPoint m_point;
        int m_starSize;
};

Q_DECLARE_METATYPE( StarRating )

#endif
