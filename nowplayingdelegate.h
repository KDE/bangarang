#ifndef NOWPLAYINGDELEGATE_H
#define NOWPLAYINGDELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

class MainWindow;
class NowPlayingDelegate : public QItemDelegate
{
    Q_OBJECT
    public:
        NowPlayingDelegate(QObject * parent = 0);
        ~NowPlayingDelegate();
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        int columnWidth (int column, int viewWidth) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
                    
    private:
        MainWindow * m_parent;  
        QPixmap m_ratingCount;
        QPixmap m_ratingNotCount;
        
};

#endif // NOWPLAYINGDELEGATE_H
