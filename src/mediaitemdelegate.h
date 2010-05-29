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

#include "platform/mediaitemmodel.h"

#include <KIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTreeView>

class MainWindow;
class MediaIndexer;

/*
 * This Item Delegate is responsible for painting items in a 
 * MediaItemModel for a View. It is also responsible for communicating ui
 * events to the model to activate categories or actions, etc.
 *
 */ 
class MediaItemDelegate : public QItemDelegate
{
    Q_OBJECT
    public:
        enum RenderMode {NormalMode = 0,
                          MiniMode = 1,
                          MiniPlaybackTimeMode = 2,
                          MiniRatingMode = 3,
                          MiniPlayCountMode = 4,
                          MiniAlbumMode = 5};
        MediaItemDelegate(QObject * parent = 0);
        ~MediaItemDelegate();
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        int columnWidth (int column, int viewWidth) const;
        bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& _index);
        void setView(QAbstractItemView * view);
        void setRenderMode(MediaItemDelegate::RenderMode mMode);
        MediaItemDelegate::RenderMode currentRenderMode();
        int heightForAllRows();
        void setUseProxy(bool b = true);
        bool useProxy() const { return m_useProxy; }

    protected:
        QRect ratingRect(const QRect *rect) const;
        MediaItemModel *mediaItemModelFromIndex(const QModelIndex* index) const;
    
    private:
        MainWindow * m_parent;
        QAbstractItemView * m_view;
        QAbstractItemView::SelectionMode m_defaultViewSelectionMode;
        QPixmap m_ratingCount;
        QPixmap m_ratingNotCount;
        KIcon m_showPlaying;
        KIcon m_showInPlaylist;
        KIcon m_showNotInPlaylist;
        int calcItemHeight() const;
        bool m_nepomukInited;
        bool m_useProxy;
        MediaIndexer * m_mediaIndexer;
        MediaItemDelegate::RenderMode m_renderMode;
        int m_starRatingSize;
        int m_durRatingSpacer;
        int m_padding;
        int m_iconSize;
        int m_textInner;
        
    Q_SIGNALS:
        void categoryActivated(QModelIndex index);
        void actionActivated(QModelIndex index);
};

#endif // MEDIAITEMDELEGATE_H
