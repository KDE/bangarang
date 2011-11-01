/* BANGARANG MEDIA PLAYER
* Copyright (C) 2011 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef ARTWORKPAINTER_H
#define ARTWORKPAINTER_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QVariant>

class ArtworkPainter : public QObject
{
    Q_OBJECT
public:
    explicit ArtworkPainter(QObject *parent = 0);
    void setArtworkSize(int size);
    void paint(QPainter* p, QRect rect, QList<QVariant> artworkList);
    void paint(QPainter* p, QRect rect, QList<QPixmap> artworkList);
    void paint(QPainter* p, QRect rect, QPixmap artwork);

signals:

public slots:

private:
    int m_size;
    QList<qreal> m_artworkRotations;

};

#endif // ARTWORKPAINTER_H
