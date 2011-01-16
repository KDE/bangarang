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

#ifndef LASTFMINFOFETCHER_H
#define LASTFMINFOFETCHER_H

#include "infofetcher.h"

#include <KUrl>
#include <QDateTime>

class Downloader;

class LastfmInfoFetcher : public InfoFetcher
{
    Q_OBJECT
    public:
    enum RequestType {ArtistRequest = 0,
                      AlbumRequest = 1,
                      TrackRequest = 2};
        LastfmInfoFetcher(QObject *parent = 0);
        bool available(const QString &subType);

    private:
        QString m_apiKey;
        QString m_artistSearchAPI;
        QString m_artistInfoAPI;
        QString m_albumInfoAPI;
        QString m_trackInfoAPI;
        Downloader * m_downloader;
        QStringList m_requestKeys;
        QHash<int, QList<MediaItem> > m_fetchedMatches;
        QHash<QString, QString> m_thumbnailKeys;
        QHash<QString, QString> m_moreInfoKeys;
        RequestType m_requestType;
        QDateTime m_lastRequestTime;

        void processOriginalRequest(const KUrl &from, const KUrl to);
        void processThumbnails(const KUrl &from, const KUrl to);
        void processMoreInfo(const KUrl &from, const KUrl to);

    private slots:
        void gotLastfmInfo(const KUrl &from, const KUrl &to);
        void timeout();

    signals:
        void download(const KUrl &from, const KUrl &to);

    public slots:
        void fetchInfo(QList<MediaItem> mediaList, int maxMatches = 4, bool updatedRequiredFields = true, bool updateArtwork = true);};

#endif // LASTFMINFOFETCHER_H
