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

#include <KLineEdit>
#include <KUrlRequester>
#include <QComboBox>
#include <QSpinBox>
#include <QObject>
#include <QTreeWidgetItem>
#include <QTextEdit>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItemModel;
class MediaItemDelegate;
class MediaIndexer;
class ArtworkWidget;

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
        void editInfoView();
        void removeSelectedItemsInfo();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        MediaIndexer *m_mediaIndexer;
        bool m_editToggle;
        void loadInfoView();
        QVariant commonValue(QString field);
        QStringList valueList(QString field);
        void saveInfoToMediaModel();
        QList<int> m_rows;
        void showFields(bool edit = false);
        void showCommonFields(bool edit = false);
        void showAudioType(int index, bool edit = false);
        void showAudioFields();
        void showAudioMusicFields(bool edit = false);
        void showAudioStreamFields(bool edit = false);
        void showVideoType(int index, bool edit = false);
        void showVideoFields();
        void showVideoMovieFields(bool edit = false);
        void showVideoTVShowFields(bool edit = false);
        bool multipleVideoTypes();
        bool multipleAudioTypes();
        void setLabel(int row, QString label, int format = NormalFormat);
        void setInfo(int row, QString info, int format = NormalFormat);
        void setInfo(int row, QPixmap pixmap);
        void setEditWidget(int row, KLineEdit *lineEdit, QString value = QString());
        void setEditWidget(int row, QTextEdit *textEdit, QString value = QString());
        void setEditWidget(int row, QComboBox *comboBox, QString value = QString(), QStringList list = QStringList(), bool editable = false);
        void setEditWidget(int row, KUrlRequester *urlRequester, QString value = QString());
        void setEditWidget(int row, QSpinBox *spinBox, int value = 0);
        void setEditWidget(int row, ArtworkWidget *artworkWidget, QPixmap pixmap);
        
        
    private slots:
        void mediaViewHolderChanged(int index);
        void audioTypeChanged(int index);
        void videoTypeChanged(int index);
};
#endif //INFOMANAGER_H