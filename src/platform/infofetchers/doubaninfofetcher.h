/* BANGARANG MEDIA PLAYER
 * Copyright (C) 2011 Ni Hui (shuizhuyuanluo@126.com)
 * <http://gitorious.org/bangarang>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOUBANINFOFETCHER_H
#define DOUBANINFOFETCHER_H

#include "infofetcher.h"
#include <KUrl>

class Downloader;

class DoubanInfoFetcher : public InfoFetcher
{
    Q_OBJECT
    public:
        explicit DoubanInfoFetcher(QObject* parent = 0);
        virtual ~DoubanInfoFetcher();
        virtual bool available(const QString &subType);
    public slots:
        virtual void fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields = true, bool fetchArtwork = true);
    signals:
        void download(const KUrl &from, const KUrl &to);
        void downloadThumbnail(const KUrl &from, const KUrl &to);
    protected slots:
        virtual void timeout();
    private slots:
        void processOriginalRequest(const KUrl &from, const KUrl to);
        void processThumbnails(const KUrl &from, const KUrl to);
    private:
        QString m_searchAPI;
        Downloader* m_downloader;
        Downloader* m_thumbnailDownloader;
        QHash<int, MediaItem> m_fetchedMatches;
        QStringList m_requestKeys;
        QHash<QString, int> m_thumbnailKeys;
        int m_toFetchCount;
        int m_fetchedCount;
        bool isFetchingThumbnail;
};

#endif // DOUBANINFOFETCHER_H
