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

#ifndef INFOMANAGER_H
#define INFOMANAGER_H

#include "platform/mediaitemmodel.h"
#include "platform/infocategorymodel.h"
#include <QtCore>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QAbstractItemView>


namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class InfoItemModel;
class InfoItemDelegate;
class InfoCategoryDelegate;
class MediaItemDelegate;

/*
 * This class provides a user interface for updating information associated with MediaItems
 */
class InfoManager : public QObject
{
    Q_OBJECT
    
    public:
        InfoManager(MainWindow * parent);
        ~InfoManager();
        const QList<MediaItem> selectedInfoBoxMediaItems();
        
    public slots:
        void toggleInfoView();
        void showInfoView();
        void hideInfoView();
        void mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
        void saveItemInfo();
        void loadSelectedInfo();
        void showInfoViewForMediaItem(const MediaItem &mediaItem);
        void setContext(const MediaItem &category);
        void clearInfoBoxSelection();
        void mediaListPropertiesChanged();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        bool m_nepomukInited;
        InfoItemModel *m_infoItemModel;
        InfoItemDelegate *m_infoItemDelegate;
        InfoCategoryModel *m_infoCategoryModel;
        InfoCategoryDelegate *m_infoCategoryDelegate;
        QList<MediaItem> m_infoMediaList;
        MediaItemModel *m_recentlyPlayedModel;
        MediaItemModel *m_highestRatedModel;
        MediaItemModel *m_frequentlyPlayedModel;
        MediaItem m_contextCategory;
        QList<MediaItem> m_selectedInfoBoxMediaItems;
        
    private slots:
        void updateViewsLayout();
        void cancelItemEdit();
        void infoDataChangedSlot(const QModelIndex &topleft, const QModelIndex &bottomright);
        void infoBoxSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
        
    Q_SIGNALS:
        void infoBoxSelectionChanged(QList<MediaItem> selectedItems);
};
#endif //INFOMANAGER_H
