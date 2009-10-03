#ifndef MEDIAITEMDELEGATE_H
#define MEDIAITEMDELEGATE_H

#include <KIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTreeView>

class MainWindow;
class MediaItemDelegate : public QItemDelegate
{
    Q_OBJECT
    public:
        MediaItemDelegate(QObject * parent = 0);
        ~MediaItemDelegate();
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        int columnWidth (int column, int viewWidth) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
        void setView(QTreeView * view);
            
    private:
        MainWindow * m_parent;
        QTreeView * m_view;
        QAbstractItemView::SelectionMode m_defaultViewSelectionMode;
        QPixmap m_ratingCount;
        QPixmap m_ratingNotCount;
        KIcon m_showPlaying;
        KIcon m_showInPlaylist;
        KIcon m_showNotInPlaylist;
        int calcItemHeight(const QStyleOptionViewItem &option) const;
        
        
    Q_SIGNALS:
        void categoryActivated(QModelIndex index);
        void actionActivated(QModelIndex index);
};

#endif // MEDIAITEMDELEGATE_H
