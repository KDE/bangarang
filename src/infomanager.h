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

#include <QtCore>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QAbstractItemView>


namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItemModel;
class InfoItemModel;
class MediaItem;
class InfoItemDelegate;
class InfoArtistModel;
class InfoArtistDelegate;
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
        
    public slots:
        void toggleInfoView();
        void showInfoView();
        void hideInfoView();
        void mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
        void saveItemInfo();
        void loadSelectedInfo();
        void showInfoViewForMediaItem(const MediaItem &mediaItem);
        void removeSelectedItemsInfo();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        bool m_nepomukInited;
        InfoItemModel *m_infoItemModel;
        InfoItemDelegate *m_infoItemDelegate;
        InfoArtistModel *m_infoArtistModel;
        InfoArtistDelegate *m_infoArtistDelegate;
        QList<MediaItem> m_infoMediaList;
        QString m_currentCategory;
        MediaItemModel *m_recentlyPlayedModel;
        MediaItemModel *m_highestRatedModel;
        MediaItemModel *m_frequentlyPlayedModel;
        
        void setCategoryToArtist();

    private slots:
        void updateViewsLayout();
        void cancelItemEdit();
        void infoDataChangedSlot(const QModelIndex &topleft, const QModelIndex &bottomright);
};
#endif //INFOMANAGER_H
