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

#ifndef INFOITEMDELEGATE_H
#define INFOITEMDELEGATE_H

#include <KIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>

class MainWindow;
class MediaIndexer;
class InfoItemView;

/*
 * This Item Delegate is responsible for painting items in a 
 * values model for items' infoview. It is also responsible for 
 * providing editing widgets.
 *
 */ 
class InfoItemDelegate : public QItemDelegate
{
    Q_OBJECT
    public:
        InfoItemDelegate(QObject * parent = 0);
        ~InfoItemDelegate();
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
        QWidget *createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
        void setEditorData ( QWidget * editor, const QModelIndex & index ) const;
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const;
        void setView(InfoItemView * view);
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        int rowHeight(int row) const;
        int heightForAllRows();
        void resetEditMode();

    private:
        InfoItemView * m_view;
        QAbstractItemView::SelectionMode m_defaultViewSelectionMode;
        bool m_nepomukInited;
        int heightForWordWrap(QFont font, int width, QString text) const;
        QList<qreal> m_artworkRotations;
        QPoint m_mousePos;
        int m_stringListIndexEditing;
        int m_rowOfNewValue;
        QIcon m_drillIcon;
        QIcon m_drillIconHighlight;
        int m_padding;
        bool m_isEditing;
        QRect fieldDataRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        int stringListIndexAtMousePos(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QRect stringListRectAtMousePos(const QStyleOptionViewItem &option, const QModelIndex &index) const;

        
    Q_SIGNALS:

    private Q_SLOTS:
        void endEditing(QWidget * editor);
};

#endif // INFOITEMDELEGATE_H
