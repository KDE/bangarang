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

#include "downloader.h"
#include <KDebug>
#include <QFile>

Downloader::Downloader(QObject * parent):QObject (parent) 
{
    m_dirLister = new KDirLister(this);
}

Downloader::~Downloader() 
{
}

void Downloader::download(const KUrl &from, const KUrl &to)
{
    if (to.isLocalFile()) {
        QFile fileTarget(to.path());
        fileTarget.remove();
    }
    KIO::CopyJob *copyJob = KIO::copyAs(from, to, KIO::Overwrite | KIO::HideProgressInfo);
    copyJob->setUiDelegate(0);
    copyJob->setAutoDelete(true);
    connect (copyJob, 
             SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
             this,
             SLOT(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
}

KDirLister * Downloader::dirLister()
{
    return m_dirLister;
}

void Downloader::listDir(const KUrl &url)
{
    connect(m_dirLister, SIGNAL(completed(KUrl)), this, SLOT(listDirComplete(KUrl)));
    m_dirLister->openUrl(url);
}

void Downloader::copyingDone(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);
    disconnect (job,
             SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
             this,
             SLOT(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
    
    emit downloadComplete(from, to);
}

void Downloader::listDirComplete(const KUrl &url)
{
    m_dirLister->stop();
    disconnect(m_dirLister,SIGNAL(completed(KUrl)), this, SLOT(listDirComplete(KUrl)));
    emit listingComplete(url);
}
