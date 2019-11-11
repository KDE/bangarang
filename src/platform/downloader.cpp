/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@gmail.com)
* <https://commits.kde.org/bangarang>
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
#include <QDebug>
#include <QFile>

Downloader::Downloader(QObject * parent):QObject (parent) 
{
    m_dirLister = new KDirLister(this);
}

Downloader::~Downloader() 
{
}

void Downloader::download(const QUrl &from, const QUrl &to)
{
    if (to.isLocalFile()) {
        QFile fileTarget(to.path());
        fileTarget.remove();
    }
    KIO::CopyJob *copyJob = KIO::copyAs(from, to, KIO::Overwrite | KIO::HideProgressInfo);
    copyJob->setUiDelegate(0);
    copyJob->setAutoDelete(true);
    connect (copyJob, 
             SIGNAL(copyingDone(KIO::Job*,QUrl,QUrl,time_t,bool,bool)),
             this,
             SLOT(copyingDone(KIO::Job*,QUrl,QUrl,time_t,bool,bool)));
}

KDirLister * Downloader::dirLister()
{
    return m_dirLister;
}

void Downloader::listDir(const QUrl &url)
{
    connect(m_dirLister, SIGNAL(completed(QUrl)), this, SLOT(listDirComplete(QUrl)));
    m_dirLister->openUrl(url);
}

void Downloader::copyingDone(KIO::Job *job, const QUrl &from, const QUrl &to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);
    disconnect (job,
             SIGNAL(copyingDone(KIO::Job*,QUrl,QUrl,time_t,bool,bool)),
             this,
             SLOT(copyingDone(KIO::Job*,QUrl,QUrl,time_t,bool,bool)));
    
    emit downloadComplete(from, to);
}

void Downloader::listDirComplete(const QUrl &url)
{
    m_dirLister->stop();
    disconnect(m_dirLister,SIGNAL(completed(QUrl)), this, SLOT(listDirComplete(QUrl)));
    emit listingComplete(url);
}
