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

#ifndef INFOFETCHER_H
#define INFOFETCHER_H

#include <QtCore>
#include <QIcon>
class MediaItem;

/*
 * This is a base clase for fetching meta info for a provided media list.
 */
class InfoFetcher : public QObject
{
    Q_OBJECT
    public:
        InfoFetcher(QObject * parent = 0);
        ~InfoFetcher();
        QString name();
        QIcon icon();

    public slots:
        virtual void fetchInfo(QList<MediaItem> mediaList)
        {
            Q_UNUSED(mediaList);
        }
        
    private:
        QString m_name;
        QIcon m_icon;
        QList<MediaItem> m_mediaList;
        
        void setValue(const QString &field, const QVariant &value);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        
    signals:
        void infoFetched(QList<MediaItem> mediaList);
};
#endif // INFOFETCHER_H
