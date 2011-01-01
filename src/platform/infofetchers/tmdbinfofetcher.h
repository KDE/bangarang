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

#ifndef TMDBINFOFETCHER_H
#define TMDBINFOFETCHER_H

#include "infofetcher.h"

#include <KUrl>

class Downloader;

class TMDBInfoFetcher : public InfoFetcher
{
    Q_OBJECT
    public:
        TMDBInfoFetcher(QObject *parent = 0);
        bool available(const QString &subType);

    private:
        QString m_apiKey;
        QString m_movieSearchAPI;
        QString m_movieInfoAPI;
        Downloader * m_downloader;
        QStringList m_requestKeys;
        QHash<int, QList<MediaItem> > m_fetchedMatches;
        QHash<QString, QString> m_thumbnailKeys;
        QHash<QString, QString> m_moreInfoKeys;

        void processOriginalRequest(const KUrl &from, const KUrl to);
        void processThumbnails(const KUrl &from, const KUrl to);
        void processMoreInfo(const KUrl &from, const KUrl to);

    private slots:
        void gotTMDBInfo(const KUrl &from, const KUrl &to);
        void timeout();

    signals:
        void download(const KUrl &from, const KUrl &to);

    public slots:
        void fetchInfo(QList<MediaItem> mediaList, bool updatedRequiredFields = true, bool updateArtwork = true);};

#endif // TMDBINFOFETCHER_H
