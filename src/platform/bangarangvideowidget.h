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

#ifndef BANGARANGVIDEOWIDGET_H
#define BANGARANGVIDEOWIDGET_H

#include <phonon/videowidget.h>
#include <QWheelEvent>

/**
 * This class is there to override the Phonon::VideoWidget to get the QWheelEvent
 * and thus be able to skip over parts of a video.
 * 
 * @short overrides the QWheelEvent
 * @author Andreas Marschke <xxtjaxx@gmail.com> 
 **/

class BangarangVideoWidgetPrivate; 
class BangarangVideoWidget : public Phonon::VideoWidget
{

  Q_OBJECT 

  public:
    /**
     * create new instance 
     **/
    BangarangVideoWidget(QWidget * parent = 0);

    /**
     * delete instance
     **/
    ~BangarangVideoWidget();

    /**
     * override of wheelEvent. If mousewheel is vertically scrolled it emits @func skip().
     **/
    virtual void wheelEvent ( QWheelEvent * event );
 
   /**
     * @param set if @param fullscreen should be true or false
     * This emits @SIGNAL fullscreenChanged() so  the rest of Bangarang can Hook into it.
     **/
    void setIsFullscreen(bool fullscreen);

 protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    
 Q_SIGNALS:
    void skipForward(int i);
    void skipBackward(int i); 
    void fullscreenChanged(bool);
 private:
    friend class BangarangVideoWidgetPrivate;
    BangarangVideoWidgetPrivate* const d;
    bool fullscreen;
};

#endif //BANGARANGVIDEOWIDGET_H

