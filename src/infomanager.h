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
class MediaItem;
class MediaItemDelegate;
class MediaIndexer;
class ArtworkWidget;

/*
 * This class provides a user interface for updating information associated with MediaItems
 */
class InfoManager : public QObject
{
    Q_OBJECT
    
    public:
        enum Format { NormalFormat = 0,
        TitleFormat = 1};
        InfoManager(MainWindow * parent);
        ~InfoManager();
        
        MediaItemModel *m_infoMediaItemsModel;
        MediaItemDelegate *m_infoItemDelegate;
        
    public slots:
        void saveInfoView();
        void showInfoView();
        void hideInfoView();
        void loadInfoView();
        void showInfoViewForMediaItem(const MediaItem &mediaItem);
        void removeSelectedItemsInfo();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        
        QStandardItemModel *m_infoItemModel;
        QItemDelegate *m_commonItemDelegate;
        bool m_nepomukInited;

        void addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable = false);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        void saveInfoToMediaModel();
        QList<int> m_rows;
        
        
    private slots:
        void updateInfoItemModel();
        void checkInfoItemChanged(QStandardItem *item);
        void saveInfoItemsToMediaModel();
        void saveFileMetaData();
};
#endif //INFOMANAGER_H
