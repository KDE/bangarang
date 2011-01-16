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

#ifndef FEEDINFOFETCHER_H
#define FEEDINFOFETCHER_H

#include "infofetcher.h"

#include <KUrl>

class Downloader;

class FeedInfoFetcher : public InfoFetcher
{
    Q_OBJECT
    public:
        FeedInfoFetcher(QObject *parent = 0);
        ~FeedInfoFetcher();
        bool available(const QString &subType);

    private:
        Downloader * m_downloader;
        QStringList m_requestKeys;
        QHash<QString, QString> m_thumbnailKeys;

    private slots:
        void gotFeedInfo(const KUrl &from, const KUrl &to);

    signals:
        void download(const KUrl &from, const KUrl &to);

    public slots:
        void fetchInfo(QList<MediaItem> mediaList, int maxMatches = 4, bool updatedRequiredFields = true, bool updateArtwork = true);

};

#endif // FEEDINFOFETCHER_H
