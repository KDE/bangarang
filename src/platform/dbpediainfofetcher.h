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
#include <Soprano/BindingSet>

class DBPediaQuery;

/*
* This class uses DBpedia for fetching meta info for a provided media list.
 */
class DBPediaInfoFetcher : public InfoFetcher
{
    Q_OBJECT
    public:
        DBPediaInfoFetcher(QObject * parent = 0);
        ~DBPediaInfoFetcher();

    public slots:
        void fetchInfo(QList<MediaItem> mediaList);
        
    private:
        DBPediaQuery * m_dbPediaQuery;
        
    private slots:
        void gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey);
        
        
};
#endif // DBPEDIAINFOFETCHER_H
