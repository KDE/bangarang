#ifndef SENSIBLEWIDGETS_H
#define SENSIBLEWIDGETS_H

#include <QObject>
#include <QToolButton>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>

class SToolButton : public QToolButton
{
    Q_OBJECT
public:
    SToolButton(QWidget * parent = 0);
    ~SToolButton();
    void setHoverDelay(int i);
    int hoverDelay();
private:
    bool m_hovered;
    int m_hoverDelay;

Q_SIGNALS:
    void entered();
    void exited();

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

protected Q_SLOTS:
    void hoverTimeout();
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
#endif // SENSIBLEWIDGETS_H




 