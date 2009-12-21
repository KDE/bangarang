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

class MediaItem;

class MediaIndexer : public QObject
{
    Q_OBJECT
    
    public:
        enum State {Idle = 0, Running = 1};
        MediaIndexer(QObject *parent);
        ~MediaIndexer();
        void updateInfo(QList<MediaItem> mediaList);
        void removeInfo(QList<MediaItem> mediaList);
        void updatePlaybackInfo(QString url, bool incrementPlayCount, QDateTime playDateTime);
        void updateRating(QString url, int rating);
        void state();
        
    private:
        bool m_nepomukInited;
        QList<KProcess *> m_writers;
        QHash<int, QList<MediaItem> > m_mediaLists;
        QHash<int, QList<QString> > m_urlLists;
        State m_state;
        void writeRemoveInfo(MediaItem mediaItem, QTextStream &out);
        void writeUpdateInfo(MediaItem mediaItem, QTextStream &out);
        
    Q_SIGNALS:
        void started();
        void finished();
        void allFinished();
        void urlInfoRemoved(QString url);
        void sourceInfoUpdated(MediaItem mediaItem);
        void percentComplete(int percent);
    
    public Q_SLOTS:
        void processWriterOutput();
        void finished(int exitCode, QProcess::ExitStatus exitStatus);
        void error(QProcess::ProcessError error);
};
#endif // MEDIAINDEXER_H
