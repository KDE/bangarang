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

#ifndef DBPEDIAQUERY_H
#define DBPEDIAQUERY_H

#include <KUrl>
#include <kio/copyjob.h>
#include <Soprano/BindingSet>
#include <QtCore>

class DBPediaQuery : public QObject 
{
    
    Q_OBJECT
    public:
        DBPediaQuery(QObject *parent = 0);
        ~DBPediaQuery();
        
        void getArtistInfo(const QString & artistName);
        void getAlbumInfo(const QString & albumName);
        void getActorInfo(const QString & actorName);
        void getDirectorInfo(const QString & actorName);
        void getMovieInfo(const QString & movieName);
        
    private:
        QString m_queryPrefix;
        QHash<QString, KUrl> m_requests;
        QString m_lang;
        void launchQuery(const QString &query, const QString &requestKey);
        
    Q_SIGNALS:
        void gotArtistInfo(bool successful, const QList<Soprano::BindingSet> results, const QString requestKey);
        void gotAlbumInfo(bool successful, const QList<Soprano::BindingSet> results, const QString requestKey);
        void gotActorInfo(bool successful, const QList<Soprano::BindingSet> results, const QString requestKey);
        void gotDirectorInfo(bool successful, const QList<Soprano::BindingSet> results, const QString requestKey);
        void gotMovieInfo(bool successful, const QList<Soprano::BindingSet> results, const QString requestKey);
        
    private Q_SLOTS:
        void resultsReturned(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed);
        
};
#endif // DBPEDIAQUERY_H
