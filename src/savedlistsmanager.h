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
#include <QStringList>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItem;

/*
 * This class provides a user interface for saving and removing media lists
 * FIXME: Need interface to rename (or configure?) media lists
 */
class SavedListsManager : public QObject
{
    Q_OBJECT
    
    public:
        SavedListsManager(MainWindow * parent);
        ~SavedListsManager();
        void saveMediaList(QList<MediaItem> mediaList, QString name, QString type, bool append = false);
        void saveView(QString name, QString type);
        QStringList savedListNames(QString type);
        QString savedListLriName(QString lri);
        
    signals:
        void savedListsChanged();
        
    public slots:
        void showAudioListSave();
        void showVideoListSave();
        void returnToAudioList();
        void returnToVideoList();
        void saveAudioList();
        void saveVideoList();
        void removeSelected();
        void saveAudioListSettings();
        void saveVideoListSettings();
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        int m_startRow;
        QList<int> m_savedAudioListRows;
        QList<int> m_savedVideoListRows;
        QStringList m_savedAudioLists;
        QStringList m_savedVideoLists;
        void updateSavedListsIndex();
        bool m_nepomukInited;
        
    private slots:
        void enableValidSave(QString newText = QString());
        void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
        void audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void mediaListChanged();
        void removeAudioList();
        void removeVideoList();
        void loadSavedListsIndex();
        void showAudioSavedListSettings();
        void showVideoSavedListSettings();

};
#endif //SAVEDLISTSMANAGER_H
