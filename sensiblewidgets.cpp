#include "sensiblewidgets.h"
#include <QTimer>

SToolButton::SToolButton(QWidget * parent):QToolButton (parent) 
{
    m_hoverDelay = 0;
}
SToolButton::~SToolButton() 
{
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
#include "sensiblewidgets.moc"
