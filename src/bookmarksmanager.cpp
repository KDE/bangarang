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

#include "bookmarksmanager.h"
#include "platform/utilities/utilities.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"
#include "platform/playlist.h"

#include <KStandardDirs>
#include <KMessageBox>
#include <KDebug>
#include <QFile>

BookmarksManager::BookmarksManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
}

BookmarksManager::~BookmarksManager()
{
}

QStringList BookmarksManager::bookmarks(const QString &url)
{
    QStringList bookmarksList;
    
    if( url == "-" )
        return bookmarksList;
    
    //Load bookmarks for specified url
    QFile *file = bookmarkFile(url);
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream bmin(file);
        while (!bmin.atEnd()) {
            QString line = bmin.readLine();
            QStringList nameTime = line.split(":::");
            if (nameTime.count() == 2) {
                bookmarksList.append(line);
            }
        }
    }
    return bookmarksList;
}

QString BookmarksManager::bookmarkName(const QString &bookmark)
{
    QString name;
    QStringList nameTime = bookmark.split(":::");
    if (nameTime.count() == 2) {
        name = nameTime.at(0);
    }
    return name; 
}

qint64 BookmarksManager::bookmarkTime(const QString &bookmark)
{
    qint64 time = 0;
    QStringList nameTime = bookmark.split(":::");
    if (nameTime.count() == 2) {
        time = nameTime.at(1).toInt();
    }
    return time;
}

QString BookmarksManager::bookmarkLookup(const QString & url, const QString &name)
{
    QString bookmark;
    QStringList currentBookmarks = bookmarks(url);
    for (int i = 0; i < currentBookmarks.count(); i++) {
        if (bookmarkName(currentBookmarks.at(i)) == name) {
            bookmark = currentBookmarks.at(i);
            break;
        }
    }
    return bookmark;
}

bool BookmarksManager::hasBookmarks(const MediaItem &mediaItem)
{
    return (bookmarks(mediaItem.url).count() > 0);
}

void BookmarksManager::addBookmark(const QString &url, const QString &name, int time)
{
    QStringList currentBookmarks = bookmarks(url);
    currentBookmarks.append(QString("%1:::%2").arg(name).arg(time));
    writeBookmarks(bookmarkFile(url, true), currentBookmarks);
}

void BookmarksManager::removeBookmark(const QString &url, const QString &bookmark)
{
    QStringList currentBookmarks = bookmarks(url);
    if (currentBookmarks.count() > 0) {
        int indexOfBookmark = currentBookmarks.indexOf(bookmark);
        if (indexOfBookmark != -1) {
            currentBookmarks.removeAt(indexOfBookmark);
        }
        if (currentBookmarks.count() > 0) {
            writeBookmarks(bookmarkFile(url), currentBookmarks);
        } else {
            removeBookmarks(url);
        }
    }
}

void BookmarksManager::removeBookmarks(const QString &url)
{
    QFile *file = bookmarkFile(url);
    file->remove();
    QStringList fileIndex = bookmarkFileIndex();
    QStringList newFileIndex;
    for (int i = 0; i < fileIndex.count(); i++) {
        QStringList urlBookmarkFile = fileIndex.at(i).split(":::");
        if (urlBookmarkFile.count() == 2) {
            if (urlBookmarkFile.at(0) != url) {
                newFileIndex.append(fileIndex.at(i));
            }
        }
    }
    writeBookmarkFileIndex(newFileIndex);
}

void BookmarksManager::renameBookmark(const QString &url, const QString &oldName, const QString &newName)
{
    QStringList currentBookmarks = bookmarks(url);
    for (int i = 0; i < currentBookmarks.count(); i++) {
        if (bookmarkName(currentBookmarks.at(i)) == oldName) {
            QString newBookMark = QString("%1:::%2").arg(newName).arg(bookmarkTime(currentBookmarks.at(i)));
            currentBookmarks.replace(i,newBookMark);
            writeBookmarks(bookmarkFile(url), currentBookmarks);
            break;
        }
    }
}

void BookmarksManager::showAddBookmarkDialog()
{
    /*if (m_parent->playlist()->nowPlayingModel()->rowCount() > 0) {
        QString nowPlayingUrl = m_parent->playlist()->nowPlayingModel()->mediaItemAt(0).url;
        int currentTime = m_parent->playlist()->mediaObject()->currentTime();
    }*/
}

QFile *BookmarksManager::bookmarkFile(const QString &url, bool createIfMissing)
{
    QFile *file = new QFile("");
    bool urlIsMissing = true;
    
    //Find bookmark file for the specified url
    QStringList fileIndex = bookmarkFileIndex();
    QStringList indexOfUrl = fileIndex.filter(url);
    if (indexOfUrl.count() > 0) {
        QStringList urlBookmarkFile = indexOfUrl.at(0).split(":::");
        if (urlBookmarkFile.count() == 2) {
            file = new QFile(urlBookmarkFile.at(1));
            urlIsMissing = false;
        }
    }
    
    //Add new bookmark file for the specified url to the index if none exists
    if (urlIsMissing && createIfMissing) {
        QString bookmarkFileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        QString bookmarkFilePath = KStandardDirs::locateLocal("data", QString("bangarang/bookmarks/%1").arg(bookmarkFileName), true);
        fileIndex.append(QString("%1:::%2").arg(url).arg(bookmarkFilePath));
        writeBookmarkFileIndex(fileIndex);
        file = new QFile(bookmarkFilePath);
    }
    return file;
}

void BookmarksManager::writeBookmarks(QFile *file, QStringList bookmarkList)
{
    file->remove();
    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream bmout(file);
        for (int i = 0; i < bookmarkList.count(); i ++) {
            bmout << bookmarkList.at(i) << "\r\n";
        }
        file->close();
    }
}

QStringList BookmarksManager::bookmarkFileIndex()
{
    QStringList fileIndex;
    //Find bookmark file for the specified url
    QFile bookmarksFile(KStandardDirs::locateLocal("data", "bangarang/bookmarks/bookmarks", true));
    if (bookmarksFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&bookmarksFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList urlBookmarkFile = line.split(":::");
            if (urlBookmarkFile.count() == 2) {
                fileIndex << line;
            }
        }
        bookmarksFile.close();
    }
    return fileIndex;
}

void BookmarksManager::writeBookmarkFileIndex(QStringList fileIndex)
{
    QFile bookmarkIndex(KStandardDirs::locateLocal("data", "bangarang/bookmarks/bookmarks", true));
    bookmarkIndex.remove();
    if (bookmarkIndex.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream bmout(&bookmarkIndex);
        for (int i = 0; i < fileIndex.count(); i++) {
            bmout << fileIndex.at(i) << "\r\n";
        }
        bookmarkIndex.close();
    }
}
