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

#ifndef INFOARTISTDELEGATE_H
#define INFOARTISTDELEGATE_H

#include <KIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTableView>

class MainWindow;
class MediaIndexer;

/*
 * This Item Delegate is responsible for painting items in a 
 * values model for items' infoview. It is also responsible for 
 * providing editing widgets.
 *
 */ 
class InfoArtistDelegate : public QItemDelegate
{
    Q_OBJECT
    public:
        InfoArtistDelegate(QObject * parent = 0);
        ~InfoArtistDelegate();
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        int columnWidth (int column) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
        QWidget *createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const;
        void setView(QTableView * view);
            
    private:
        MainWindow * m_parent;
        QTableView * m_view;
        QAbstractItemView::SelectionMode m_defaultViewSelectionMode;
        bool m_nepomukInited;
        
        
    Q_SIGNALS:
};

#endif // INFOARTISTDELEGATE_H