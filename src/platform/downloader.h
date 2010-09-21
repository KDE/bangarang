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

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <KIO/CopyJob>
#include <KDirLister>
#include <QtCore>

/*
 * This class provides a simple downloader accessible via signals/slots.
 */
class Downloader : public QObject
{
    Q_OBJECT
    public:
        Downloader(QObject * parent = 0);
        ~Downloader();
        KDirLister *dirLister();

    public slots:
        void download(const KUrl &from, const KUrl &to);
        void listDir(const KUrl &url);

    private:
        KDirLister * m_dirLister;
        
    private slots:
        void copyingDone(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed);
        void listDirComplete(const KUrl & url);

    signals:
        void downloadComplete(const KUrl &from, const KUrl &to);
        void listingComplete(const KUrl &url);
};
#endif // DOWNLOADER_H
