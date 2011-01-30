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

#ifndef MEDIAINDEXER_H
#define MEDIAINDEXER_H

#include <QtCore>
#include <KProcess>

#include "mediaitemmodel.h"

/**
* MediaIndexer provides a way to asynchronous write to the nepomuk datastore.
*/
class MediaIndexer : public QObject
{
    Q_OBJECT
    
    public:
        enum State {Idle = 0, Running = 1};
        /**
         * Constructor
         */
        MediaIndexer(QObject *parent = 0);
        
        /**
         *Destructor
         */
        ~MediaIndexer();
        
        /**
         * Update information in the Nepomuk datastore using the specified
         * mediaList.
         *
         * @param mediaList List of MediaItems containing updated information.
         */
        void updateInfo(const QList<MediaItem> &mediaList);
        
        /**
        * Update information in the Nepomuk datastore using the specified
        * MediaItem.
        *
        * @param mediaItem MediaItem containing updated information.
        */
        void updateInfo(const MediaItem &mediaItem);
        
        /**
         * Remove media information from the Nepomuk datastore corresponding
         * specified medialist.
         *
         * @param mediaList List of MediaItems whose information should be
         *                  removed from the datastore.
         */
        void removeInfo(const QList<MediaItem> &mediaList);
        
        /**
        * Remove media information from the Nepomuk datastore corresponding
        * specified MediaItem.
        *
        * @param mediaItem MediaItem whose information should be
        *                  removed from the datastore.
        */
        void removeInfo(const MediaItem &mediaItem);
        
        /**
         * Update the playback time and/or play count for the specified url.
         *
         * @param url url of media item to updateInfo
         * @param incrementPlayCount if true, the play count will be incremented
         * @param playDateTime DateTime of playback
         */
        void updatePlaybackInfo(const QString &resourceUri, bool incrementPlayCount, const QDateTime &playDateTime);
        
        /**
         * Update the rating of the specified url.
         *
         * @param url Url of MediaItem
         * @param rating Rating: and integer between 0 and 10
         */
        void updateRating(const QString &resourceUri, int rating);
        
        State state();
        
    Q_SIGNALS:
        /**
         * Emitted when the update/removal has started
         */
        void started();
        
        /**
         * Emitted when the update/removal has finished
         */
        void finished();
        
        /**
         * Emitted when all update/removal tasks managed by this MediaIndexer
         * is finished.
         */
        void allFinished();
        
        /**
         * Emitted when media information for the url has been removed
         */
        void urlInfoRemoved(QString url);
        
        /**
         * Emitted when media information for MediaItem has been updated
         */
        void sourceInfoUpdated(MediaItem mediaItem);
        
        /**
         * Emitted when media information has been updated/removed.
         */
        void updateStatus(QHash<QString, QVariant> updatedStatus);

        void startWriter(const QStringList &args);

    private:
        bool m_nepomukInited;
        KProcess * m_writer;
        QList<MediaItem> m_mediaList;
        QStringList m_urlList;
        State m_state;
        int m_percent;
        QHash<QString, QVariant> m_status;
        void writeRemoveInfo(MediaItem mediaItem, QTextStream &out);
        void writeUpdateInfo(MediaItem mediaItem, QTextStream &out);
        
    private Q_SLOTS:
        void startWriterSlot(const QStringList &args);
        void processWriterOutput();
        void finished(int exitCode, QProcess::ExitStatus exitStatus);
        void error(QProcess::ProcessError error);
};
#endif // MEDIAINDEXER_H
