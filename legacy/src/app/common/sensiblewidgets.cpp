/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
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

#include "sensiblewidgets.h"
#include "ratingdelegate.h"
#include "starrating.h"
#include "timercounter.h"

#include <KIcon>
#include <KFileDialog>
#include <QStylePainter>
#include <QPaintEvent>

SToolButton::SToolButton(QWidget * parent):QToolButton (parent) 
{
    m_hoverDelay = 0;
    m_holdEnabled = true;
    m_timer = new QTimer(this);
    m_holdIcon = NULL;
    m_animateHold = false;
    m_animationTimer = new TimerCounter(0, 50, false);
    connect(m_animationTimer, SIGNAL(timeout()), this, SLOT(repaint()));
    connect(this, SIGNAL(pressed()), this, SLOT(pressedEvent()));
    connect(this, SIGNAL(released()), this, SLOT(releasedEvent()));
}
SToolButton::~SToolButton() 
{
    delete m_timer;
}
void SToolButton::setHoverDelay(int i)
{
    m_hoverDelay = i;
}
int SToolButton::hoverDelay()
{
    return m_hoverDelay;
}
void SToolButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hovered = true;
    if (m_hoverDelay > 0) {
        QTimer::singleShot(m_hoverDelay, this, SLOT(hoverTimeout()));
    } else {   
        emit this->entered(); 
    }
}
void SToolButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hovered = false;
    emit this->exited(); 
}
void SToolButton::hoverTimeout()
{
    if (m_hovered) {
        emit this->entered();
    }
}
void SToolButton::setHoldDelay(int i)
{
    m_holdDelay = i;
    m_animationTimer->setMax(i/m_animationTimer->interval());
}
int SToolButton::holdDelay()
{
    return m_holdDelay;
}
void SToolButton::setHoldIcon(KIcon icon, bool animation)
{
    delete m_holdIcon;
    m_holdIcon = new KIcon(icon); //copy it
    m_animateHold = animation;
}
void SToolButton::enableHold(bool enable)
{
    m_holdEnabled = enable; //no animation if no icon was set
}
void SToolButton::disableHold()
{
    m_holdEnabled = false;
}
void SToolButton::pressedEvent()
{
    m_pressed = true;
    if (m_holdDelay <= 0 || !m_holdEnabled) { //nothing else to do
        return;
    }
    if (m_animateHold) {
        m_animationTimer->reset();
    }
 
    //start timer when hold timeout exceeds
    m_timer->singleShot(m_holdDelay, this, SLOT(holdTimeout()));
}
void SToolButton::holdTimeout()
{
    if (m_animateHold || m_animationTimer->isActive() ){
        m_animationTimer->stop();
    }
    if (!m_pressed) {
        return; //not pressed anymore, no actions required
    }
    if (m_holdIcon) {
        setIcon(*m_holdIcon);
    }
    emit this->held();
}
void SToolButton::releasedEvent()
{
    m_pressed = false;
    m_timer->stop();
}
void SToolButton::paintEvent(QPaintEvent* event)
{
    QToolButton::paintEvent(event);
    if (m_animateHold && m_pressed && m_holdEnabled) {
        qreal progress = (qreal) m_animationTimer->getValue() / (qreal) m_animationTimer->max();
        //progress *= progress;
        const QPixmap &pxm = m_holdIcon->pixmap(iconSize());
        QRect pmRect = pxm.rect();
        pmRect.moveCenter(rect().center());
        if (pmRect.intersects(event->rect())) {
            QPainter painter(this);
            painter.setOpacity(progress);
            painter.drawPixmap(pmRect.left(), pmRect.top(), pxm);
        }
    }
}


SFrame::SFrame(QWidget * parent):QFrame (parent) 
{
    m_hoverDelay = 0;
}
SFrame::~SFrame() 
{
}
void SFrame::setHoverDelay(int i)
{
    m_hoverDelay = i;
}
int SFrame::hoverDelay()
{
    return m_hoverDelay;
}
void SFrame::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hovered = true;
    if (m_hoverDelay > 0) {
        QTimer::singleShot(m_hoverDelay, this, SLOT(hoverTimeout()));
    } else {   
        emit this->entered(); 
    }
}
void SFrame::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hovered = false;
    emit this->exited(); 
}
void SFrame::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoved();
    Q_UNUSED(event);
}
void SFrame::resizeEvent(QResizeEvent *)
{
    emit resized();
}
void SFrame::hoverTimeout()
{
    if (m_hovered) {
        emit this->entered();
    }
}

SLabel::SLabel(QWidget * parent):QLabel (parent) 
{
    m_hoverPixmap = new QPixmap();
    m_mousePressed = false;
}
SLabel::~SLabel() 
{
}
void SLabel::setHoverPixmap(QPixmap * pixmap)
{
    m_hoverPixmap = pixmap;
}
void SLabel::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (!m_hoverPixmap->isNull()) {
        m_pixmap = *this->pixmap();
        this->setPixmap(*m_hoverPixmap);
    }
    emit this->entered(); 
}
void SLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
   if (!m_hoverPixmap->isNull()) {
        this->setPixmap(m_pixmap);
    }
    emit this->exited(); 
}

SListWidget::SListWidget(QWidget * parent):QListWidget (parent) 
{
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(compareSelectionChanges()));
}
SListWidget::~SListWidget() 
{
}
void SListWidget::compareSelectionChanges()
{
    QList<QListWidgetItem *> currentSelectedItems = this->selectedItems();
    for (int i = 0; i < currentSelectedItems.count(); ++i) {
        if (lastSelectedItems.indexOf(currentSelectedItems.at(i)) == -1) {
            emit selected(currentSelectedItems.at(i));
        }
    }
    for (int i = 0; i < lastSelectedItems.count(); ++i) {
        if (currentSelectedItems.indexOf(lastSelectedItems.at(i)) == -1) {
            emit unSelected(lastSelectedItems.at(i));
        }
    }
    lastSelectedItems = currentSelectedItems;
}

void SListWidget::selectorEntered()
{
    this->setSelectionMode(QAbstractItemView::MultiSelection);
}

void SListWidget::selectorExited()
{
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

SRatingCombo::SRatingCombo(QWidget * parent) : QComboBox(parent)
{
    this->setItemDelegate(new RatingDelegate(this));
}

void SRatingCombo::paintEvent (QPaintEvent * e)
{
    QVariant ratingData = this->itemData(this->currentIndex(),Qt::UserRole).toInt();
    if (ratingData.isValid()) {
        QStylePainter p(this);
        QStyleOptionComboBox option;
        option.initFrom(this);

        p.drawComplexControl(QStyle::CC_ComboBox, option);

        QRect subRect = p.style()->subElementRect(QStyle::SE_ComboBoxFocusRect, &option);
        const int left = subRect.left();
        const int top = subRect.top();
        const int height = subRect.height();

        int rating = ratingData.toInt();
        StarRating starRating = StarRating(rating, StarRating::Medium);
        starRating.setRating(rating);
        QSize ratingSize = starRating.sizeHint();
        int ratingLeft = left + 2;
        int ratingTop = top + (height - ratingSize.height())/2;
        QRect ratingRect = QRect(QPoint(ratingLeft, ratingTop), ratingSize);
        starRating.setPoint(ratingRect.topLeft());
        if (!this->isEnabled()) {
            starRating.setHoverAtPosition(ratingRect.topLeft());
        }
        starRating.paint(&p);
    } else {
        QComboBox::paintEvent(e);
    }

}

#include "sensiblewidgets.moc"
