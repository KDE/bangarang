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
        enum Size { Small = 8, Big = 16 };
        enum MinMax{ InvalidRating = -1, MinRating = 0, MaxRating = 10 };

        StarRating(int rating = 0, Size size = Small, QPoint point = QPoint(0, 0), int hoverX = -1);

        void setRating(int rating) { m_rating = rating; }
        void setSize(int size);
        void setPoint(QPoint point) { m_point = point; }
        void setHoverAtPosition(int hoverX) { m_hoverRating = ratingAtPosition(hoverX); }

        bool valid(int rating) const { return (rating >= MinRating && rating <= MaxRating); }
        int rating() { return m_rating; }
        QSize sizeHint() const;
        int ratingAtPosition(int x);
        void paint(QPainter *painter);

    protected:
        QPixmap m_starHover;
        QPixmap m_starNormal;
        QPixmap m_starInactive;

        int m_rating;
        int m_hoverRating;
        QPoint m_point;
        QSize m_starSize;
        QSize m_margin;
};

Q_DECLARE_METATYPE( StarRating )

#endif