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
#include "../common/actionsmanager.h"

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
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_contextMenu = NULL;
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
  if(event->button() == Qt::LeftButton && !m_application->isTouchEnabled()){
    if (m_fullscreen) {
      emit fullscreenChanged(false);
      setIsFullscreen(false);
    }
    else {
      emit fullscreenChanged(true);
      setIsFullscreen(true);
    }
  }
}

void BangarangVideoWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_application->isTouchEnabled()) {
        m_application->actionsManager()->action("toggle_controls")->trigger();
    }
    Q_UNUSED(event);
}

void
BangarangVideoWidget::setIsFullscreen(bool isFullscreen)
{ 
  m_fullscreen = isFullscreen;
}

void
BangarangVideoWidget::contextMenuEvent ( QContextMenuEvent * event ) 
{
  /*
  * NOTE: at least at a bangarang 2.x release we should set a fixed menu. the nowPlayingContextMenu
  * is currently rebuild at any call, because the available subtitles/audiotracks and so on could have changed.
  * But as I spotted the MediaController provides signals that these have changed. So we need to implement
  * slots for these in a class (don't know which, maybe just a separate one existing only for the DVD menu).
  * Then the menu should be _changed_ not recreated as this function can keep the pointer to the contextMenu
  */
  Q_UNUSED(event);
  if ( m_contextMenu != NULL )
    m_contextMenu->exec(QCursor::pos());
  else
    m_application->actionsManager()->nowPlayingContextMenu()->exec(QCursor::pos());
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
