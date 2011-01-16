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

#ifndef DBPEDIAINFOFETCHER_H
#define DBPEDIAINFOFETCHER_H

#include "infofetcher.h"
#include <KUrl>
#include <Soprano/BindingSet>
#include <QtCore>

class DBPediaQuery;
class Downloader;

/*
* This class uses DBpedia for fetching meta info for a provided media list.
 */
class DBPediaInfoFetcher : public InfoFetcher
{
    Q_OBJECT
    public:
        DBPediaInfoFetcher(QObject * parent = 0);
        ~DBPediaInfoFetcher();
        bool available(const QString &subType);

    public slots:
        void fetchInfo(QList<MediaItem> mediaList, int maxMatches = 4, bool updatedRequiredFields = true, bool updateArtwork = true);
        
    private:
        DBPediaQuery * m_dbPediaQuery;
        Downloader * m_downloader;
        QHash<QString, QString> m_thumbnailKeys;
        QStringList m_requestKeys;
        QHash<int, QList<MediaItem> > m_fetchedMatches;
        QList<int> m_indexesWithThumbnail;

        bool allThumbnailsFetchedForIndex(int index);
        void checkComplete();
        
    private slots:
        void gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey);
        void gotThumbnail(const KUrl &from, const KUrl &to);
        void gotPersonInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey);

    protected slots:
        void timeout();

    signals:
        void download(const KUrl &from, const KUrl &to);

        
        
};
#endif // DBPEDIAINFOFETCHER_H
