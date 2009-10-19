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

#ifndef SAVEDLISTSMANAGER_H
#define SAVEDLISTSMANAGER_H

#include <QObject>
#include <QItemSelection>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItem;

class SavedListsManager : public QObject
{
    Q_OBJECT
    
    public:
        SavedListsManager(MainWindow * parent);
        ~SavedListsManager();
        void saveMediaList(QList<MediaItem> mediaList, QString name, QString type);
        void saveView(QString name, QString type);
        
    public slots:
        void showAudioListSave();
        void showVideoListSave();
        void hideAudioListSave();
        void hideVideoListSave();
        void saveAudioList();
        void saveVideoList();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        int m_startRow;
        QList<int> m_savedAudioListRows;
        QList<int> m_savedVideoListRows;
        QList<QString> m_savedAudioLists;
        QList<QString> m_savedVideoLists;
        void updateSavedListsIndex();
        
    private slots:
        void enableValidSave(QString newText = QString());
        void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
        void audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void mediaListChanged();
        void removeAudioList();
        void removeVideoList();
        void loadSavedListsIndex();
        

};
#endif //SAVEDLISTSMANAGER_H