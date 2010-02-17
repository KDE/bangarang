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

#include "bangarangvideowidget.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QMenu> 
#include <phonon/videowidget.h>
#include <QList>
#include <QAction>
#include <KLocale>
#include <KDebug>
class BangarangVideoWidgetPrivate
{
  public: 
    BangarangVideoWidgetPrivate()
    { }
};

BangarangVideoWidget::BangarangVideoWidget(QWidget * parent) : Phonon::VideoWidget(parent) ,
							       d(new BangarangVideoWidgetPrivate)
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(makeCustomContext(QPoint)));
  m_contextMenu = new QMenu();
}

BangarangVideoWidget::~BangarangVideoWidget()
{
  delete d;
}

void 
BangarangVideoWidget::wheelEvent(QWheelEvent *event)
{
  if(event->delta() > 0 ) {
    emit skipBackward(event->delta());
  } else if (event->delta() < 0 ) {
    emit skipForward(event->delta());
  }
  Phonon::VideoWidget::wheelEvent(event);
}

void
BangarangVideoWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton){
    if (fullscreen) {
      emit fullscreenChanged(false);
      setIsFullscreen(false);
    }
    else {
      emit fullscreenChanged(true);
      setIsFullscreen(true);
    }
  }
}
void
BangarangVideoWidget::setIsFullscreen(bool isFullscreen)
{ 
  fullscreen = isFullscreen;
}

void
BangarangVideoWidget::makeCustomContext(QPoint pos) 
{
  Q_UNUSED(pos);
  m_contextMenu->exec(QCursor::pos());
}

QMenu* BangarangVideoWidget::contextMenu()
{
  return m_contextMenu;
}

void BangarangVideoWidget::setContextMenu(QMenu * menu)
{
  m_contextMenu = menu;
}
#include "bangarangvideowidget.moc"