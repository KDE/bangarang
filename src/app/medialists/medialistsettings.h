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

#ifndef MEDIALISTSETTINGS_H
#define MEDIALISTSETTINGS_H

#include <QObject>
#include <QItemSelection>
#include <QStringList>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItem;
class MediaItemModel;
class BangarangApplication;

/*
 * This class provides a user interface for saving and removing media lists
 * FIXME: Need interface to rename (or configure?) media lists
 */
class MediaListSettings : public QObject
{
    Q_OBJECT
    
    public:
        MediaListSettings(MainWindow * parent);
        ~MediaListSettings();
        
    public slots:
        void showMediaListSettings();
        void hideMediaListSettings();
        void saveMediaListSettings();
        
    private:
        BangarangApplication* m_application;
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        void showMediaListSettingsForLri(const QString &lri);
        QStringList configEntryForLRI(const QString &lri);
        void readConfigEntryForLRI(const QString &lri);
        void writeConfigEntryForLRI(const QString &lri);

    private slots:
        void moreSelected(bool);

};
#endif //MEDIALISTSETTINGS_H
