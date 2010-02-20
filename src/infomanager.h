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


namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItemModel;
class InfoItemModel;
class MediaItem;
class InfoItemDelegate;

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
        void saveItemInfo();
        void showInfoView();
        void hideInfoView();
        void loadSelectedInfo();
        void showInfoViewForMediaItem(const MediaItem &mediaItem);
        void removeSelectedItemsInfo();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        InfoItemModel *m_infoItemModel;
        InfoItemDelegate *m_infoItemDelegate;
        bool m_nepomukInited;
        QList<MediaItem> m_infoMediaList;

    private slots:
        void saveFileMetaData(QList<MediaItem> mediaList);
        void updateItemViewLayout();
        void cancelItemEdit();
};
#endif //INFOMANAGER_H
