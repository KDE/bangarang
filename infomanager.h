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

#include <QObject>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItemModel;
class MediaItemDelegate;
class MediaIndexer;

class InfoManager : public QObject
{
    Q_OBJECT
    
    public:
        InfoManager(MainWindow * parent);
        ~InfoManager();
        
        MediaItemModel *m_infoMediaItemsModel;
        MediaItemDelegate *m_infoItemDelegate;
        
    public slots:
        void saveInfoView();
        void loadInfoView();
        
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        MediaIndexer *m_mediaIndexer;
        QVariant commonValue(QString field);
        QStringList valueList(QString field);
        void saveMusicInfoToFiles();
        void saveInfoToMediaModel();
        QList<int> m_rows;
        void showAudioType(int index);
        void showAudioFields();
        void showAudioMusicFields();
        void showAudioStreamFields();
        void showVideoType(int index);
        void showVideoFields();
        void showVideoMovieFields();
        void showVideoSeriesFields();
        bool multipleVideoTypes();
        bool multipleAudioTypes();
        
    private slots:
        void mediaTypeChanged(int index);
};
#endif //INFOMANAGER_H