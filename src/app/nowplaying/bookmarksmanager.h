/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#include <QtCore>
#include <QMenu>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItem;
class BangarangApplication;

/*
 * This class provides a user interface for adding and removing mediaItem bookmarks
 */
class BookmarksManager : public QObject
{
    Q_OBJECT
    
    public:
        BookmarksManager(MainWindow * parent);
        ~BookmarksManager();
        QStringList bookmarks(const QString &url);
        QString bookmarkName(const QString &bookmark);
        qint64 bookmarkTime(const QString &bookmark);
        QString bookmarkLookup(const QString & url, const QString &name);
        bool hasBookmarks(const MediaItem &mediaItem);
        
    public slots:
        void addBookmark(const QString &url, const QString &name, int time);
        void removeBookmark(const QString &url, const QString &bookmark);
        void removeBookmarks(const QString &url);
        void renameBookmark(const QString &url, const QString &oldName, const QString &newName);
        void showBookmarksMenu();
        void showAddBookmarkDialog();
        
    private:
        BangarangApplication* m_application;
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        bool m_nepomukInited;
        QFile *bookmarkFile(const QString &url, bool createIfMissing = false);
        void writeBookmarks(QFile *file, QStringList bookmarkList);
        QStringList bookmarkFileIndex();
        void writeBookmarkFileIndex(QStringList fileIndex);
        
};
#endif //BOOKMARKSMANAGER_H
