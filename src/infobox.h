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

#ifndef INFOBOX_H
#define INFOBOX_H

#include <QtCore>
#include <QWidget>
#include <QLabel>
#include <QModelIndex>

class MainWindow;
class MediaView;
/*
 * This class is mostly to provide custom infobox widget used in the Info view
 * used to display media lists.
 */
class InfoBox : public QWidget
{
    Q_OBJECT
    public:
        InfoBox(QWidget * parent = 0);
        ~InfoBox();
        void setMainWindow(MainWindow * mainWindow);
        QLabel * title();
        MediaView * mediaView();
        void setInfo(const QString &title, const QString & lri);
        void setTitle(const QString &title);
        
    private:
        MediaView * m_mediaView;
        MainWindow * m_mainWindow;
        QLabel * m_icon;
        QLabel * m_title;
        QWidget * m_titleBar;
        
    private slots:
        void updateTitleColors();
        void mediaListChanged();
        void categoryActivated(QModelIndex index);

};
#endif // INFOBOX_H
