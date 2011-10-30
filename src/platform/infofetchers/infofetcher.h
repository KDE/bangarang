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

#include <KUrl>
#include <QtCore>
#include <QIcon>
#include "../mediaitemmodel.h"

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
        KUrl url();
        QString about();
        virtual QStringList fetchableFields(const QString &subType)
        {
            return m_fetchableFields[subType];
        }
        virtual QStringList requiredFields(const QString &subType)
        {
            return m_requiredFields[subType];
        }
        virtual bool available(const QString &subType)
        {
            Q_UNUSED(subType);
            return false;
        }
        virtual bool isFetching()
        {
            return m_isFetching;
        }

    public slots:
        virtual void fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields = true, bool fetchArtwork = true)
        {
            Q_UNUSED(mediaList);
            Q_UNUSED(maxMatches);
            Q_UNUSED(updateRequiredFields);
            Q_UNUSED(fetchArtwork);
        }
        
    protected:
        QString m_name;
        QIcon m_icon;
        KUrl m_url;
        QList<MediaItem> m_mediaList;
        QHash<QString, QStringList> m_fetchableFields;
        QHash<QString, QStringList> m_requiredFields;
        bool m_isFetching;
        bool m_updateRequiredFields;
        bool m_updateArtwork;
        bool m_timeout;
        QString m_about;
        QTimer *m_timer;
        int m_timeoutLength;

        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        void setFetching();

    protected slots:
        virtual void timeout();
        
    signals:
        void infoFetched(QList<MediaItem> fetchedMatches);
        void updateFetchedInfo(int index, MediaItem match);
        void fetching();
        void fetchComplete();
        void fetchComplete(InfoFetcher *infoFetcher);
        void noResults(InfoFetcher *infoFetcher);
};
#endif // INFOFETCHER_H
