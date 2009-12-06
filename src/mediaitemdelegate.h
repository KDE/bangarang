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
        bool m_nepomukInited;
        
        
    Q_SIGNALS:
        void categoryActivated(QModelIndex index);
        void actionActivated(QModelIndex index);
};

#endif // MEDIAITEMDELEGATE_H
