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

#include "artworkpainter.h"
#include <QPixmap>
#include <math.h>

ArtworkPainter::ArtworkPainter(QObject *parent) :
    QObject(parent)
{
    m_size = 128;
    for (int i = 0; i < 19; i++) {
        if (i%2) {
            m_artworkRotations.append(10);
        } else {
            m_artworkRotations.append(-10);
        }
    }
}

void ArtworkPainter::paint(QPainter *p, QRect rect, QList<QVariant> artworkList)
{
    p->save();
    int artworkSize  = 0.8 * m_size;
    if (artworkList.count() == 1) {
        artworkSize = m_size;
    }
    int spacing = (rect.width() - artworkSize - 30)/artworkList.count();
    int aTop = rect.top() + (rect.height()-artworkSize)/2;
    int startx = rect.left() + (rect.width()/2) - ((artworkSize/2) - (spacing/2)*(artworkList.count()-1));
    p->translate(startx, aTop);
    for (int i = artworkList.count()-1; i >= 0; i--) {
        qreal rot = m_artworkRotations.at(i);
        if (artworkList.count() == 1) {
            rot = 0;
        }
        double rotRad = (3.14159/180)*rot;
        qreal r = (sqrt(2.0*artworkSize*artworkSize))/2.0;
        int transX = (artworkSize/2) - r*cos(rotRad +(3.14159/4));
        int transY = r*sin(rotRad + (3.14159/4)) - (artworkSize/2);
        p->rotate(rot);
        p->translate(transX, -transY);
        QPixmap artwork = artworkList.at(i).value<QPixmap>().scaledToHeight(artworkSize);
        int ARxOffset = (artworkSize - artwork.width())/2;
        p->fillRect(ARxOffset, 0, artwork.width(), artwork.height(), Qt::white);
        p->drawPixmap(ARxOffset, 0, artwork.width(), artwork.height(), artwork);
        if (artworkList.count() > 1) {
            QColor outlineColor = QColor(Qt::black);
            outlineColor.setAlphaF(0.7);
            p->setPen(outlineColor);
            p->drawRect(ARxOffset, 0, artwork.width(), artwork.height());
        }
        p->translate(-transX, transY);
        p->rotate(-rot);
        p->translate(-spacing, 0);
    }
    p->restore();

}

void ArtworkPainter::setArtworkSize(int size)
{
    m_size = size;
}

void ArtworkPainter::paint(QPainter *p, QRect rect, QList<QPixmap> artworkList)
{
    QList<QVariant> artworks;
    for (int i = 0; i < artworkList.count(); i++) {
        artworks.append(artworkList.at(i));
    }
    paint(p, rect, artworks);
}

void ArtworkPainter::paint(QPainter *p, QRect rect, QPixmap artwork)
{
    QList<QVariant> artworks;
    artworks.append(artwork);
    paint(p, rect, artworks);
}
