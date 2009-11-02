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

#ifndef SENSIBLEWIDGETS_H
#define SENSIBLEWIDGETS_H

#include <KUrl>
#include <QObject>
#include <QToolButton>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTimer>
#include <QHBoxLayout>

class SToolButton : public QToolButton
{
    Q_OBJECT
public:
    SToolButton(QWidget * parent = 0);
    ~SToolButton();
    void setHoverDelay(int i);
    int hoverDelay();
    void setHoldDelay(int i);
    int holdDelay();
    
private:
    bool m_hovered;
    bool m_pressed;
    int m_hoverDelay;
    int m_holdDelay;
    QTimer *m_timer;

Q_SIGNALS:
    void entered();
    void exited();
    void held();

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    
    
protected Q_SLOTS:
    void hoverTimeout();
    void holdTimeout();
    void pressedEvent();
    void releasedEvent();
};

class SFrame : public QFrame
{
    Q_OBJECT
public:
    SFrame(QWidget * parent = 0);
    ~SFrame();
    void setHoverDelay(int i);
    int hoverDelay();
private:
    bool m_hovered;
    int m_hoverDelay;

Q_SIGNALS:
    void entered();
    void exited();
    void mouseMoved();
    
protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    
protected Q_SLOTS:
    void hoverTimeout();
};

class SLabel : public QLabel
{
    Q_OBJECT
public:
    SLabel(QWidget * parent = 0);
    ~SLabel();
    void setHoverPixmap(QPixmap * pixmap);

private:
    QPixmap * m_hoverPixmap;
    QPixmap  m_pixmap;
    bool m_mousePressed;

Q_SIGNALS:
    void entered();
    void exited();
    void clicked();
protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};

class SListWidget : public QListWidget
{
    Q_OBJECT
public:
    SListWidget(QWidget * parent = 0);
    ~SListWidget();

Q_SIGNALS:
    void selected(QListWidgetItem * item);
    void unSelected(QListWidgetItem * item);

private:
    QList<QListWidgetItem *> lastSelectedItems;

protected Q_SLOTS:
    void compareSelectionChanges();
    void selectorEntered();
    void selectorExited();
};

class ArtworkWidget : public QWidget
{
    Q_OBJECT
    public:
        ArtworkWidget(QWidget * parent = 0);
        ~ArtworkWidget();
        
        KUrl url();
        const QPixmap * artwork();
        void setPixmap(QPixmap pixmap);
        void setUrl(KUrl url);
        
    private:
        QWidget * m_parent;
        QLabel * m_artworkLabel;
        QToolButton * m_openUrl;
        QHBoxLayout * m_layout;
        KUrl m_url;
        
    protected Q_SLOTS:
        void openUrl();
};
#endif // SENSIBLEWIDGETS_H




 