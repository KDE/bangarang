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

#include "sensiblewidgets.h"

#include <KIcon>
#include <KFileDialog>

SToolButton::SToolButton(QWidget * parent):QToolButton (parent) 
{
    m_hoverDelay = 0;
    m_timer = new QTimer(this);
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
}
int SToolButton::holdDelay()
{
    return m_holdDelay;
}
void SToolButton::pressedEvent()
{
    m_pressed = true;
    if (m_holdDelay > 0) {
        m_timer->singleShot(m_holdDelay, this, SLOT(holdTimeout()));
    }
}
void SToolButton::holdTimeout()
{
    if (m_pressed) {
        emit this->held();
    }
}
void SToolButton::releasedEvent()
{
    m_pressed = false;
    m_timer->stop();
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

ArtworkWidget::ArtworkWidget(QWidget * parent):QWidget (parent) 
{
    m_parent = parent;
    m_openUrl = new QToolButton();
    m_openUrl->setIcon(KIcon("document-open"));
    m_artworkLabel = new QLabel();
    m_artworkLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_artworkLabel->setMargin(8);
    m_artworkLabel->setMinimumHeight(136);
    m_layout = new QHBoxLayout;
    m_layout->addWidget(m_artworkLabel);
    m_layout->addWidget(m_openUrl);
    this->setLayout(m_layout);
    
    connect(m_openUrl, SIGNAL(clicked()), this, SLOT(openUrl()));
}

ArtworkWidget::~ArtworkWidget() 
{
    delete m_openUrl;
    delete m_artworkLabel;
    delete m_layout;
}

KUrl ArtworkWidget::url()
{
    return m_url;
}

const QPixmap * ArtworkWidget::artwork()
{
    return m_artworkLabel->pixmap();
}

void ArtworkWidget::openUrl()
{
    KUrl url = KFileDialog::getImageOpenUrl(KUrl(), m_parent, tr("Open artwork file"));
    if (!url.isEmpty()) {
        setUrl(url);
    }
}

void ArtworkWidget::setPixmap(QPixmap pixmap)
{
    m_artworkLabel->setPixmap(pixmap);
}

void ArtworkWidget::setUrl(KUrl url)
{
    QPixmap pixmap = QPixmap(url.path()).scaled(QSize(128,128), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_artworkLabel->setPixmap(pixmap);
    m_url = url;
}

#include "sensiblewidgets.moc"
