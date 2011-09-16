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

#ifndef NOWPLAYINGDELEGATE_H
#define NOWPLAYINGDELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

class MainWindow;
class MediaIndexer;
class BangarangApplication;

/*
 * This ItemDelegate is responsible for painting the currently
 * playing MediaItem for the Now Playing View. It is
 * also responsible for updating the rating when the user changes 
 * the rating from the Now Playing View.
 */
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
        QAbstractItemView *view() { return m_view; }
        void setView(QAbstractItemView *view) { m_view = view; }
        void updateSizeHint();
        void setShowInfo(bool showInfo);
        bool showingInfo();
        void enableTouch();

    protected:
        QRect ratingRect(const QRect *rect) const;

    private:
        BangarangApplication * m_application;
        MainWindow * m_parent;
        int m_iconSize;
        int m_padding;
        int m_textInner;
        int m_starRatingSize;
        bool m_nepomukInited;
        QRect m_globalRatingRect;
        QAbstractItemView *m_view;
        MediaIndexer * m_mediaIndexer;
        bool m_showInfo;
        QFont m_infoFont;

        void paintInfo(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
        QRect infoRect(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
        
};

#endif // NOWPLAYINGDELEGATE_H
