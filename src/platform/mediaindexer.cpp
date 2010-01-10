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

#include "mediaindexer.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <KUrl>
#include <KDebug>
#include <KStandardDirs>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include <QTextStream>
#include <QProcess>
#include <QFile>
#include <QHash>

MediaIndexer::MediaIndexer(QObject * parent) : QObject(parent)
{
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true; //resource manager inited successfully
    } else {
        m_nepomukInited = false; //no resource manager
    }
    m_state = Idle;
}

MediaIndexer::~MediaIndexer()
{
}

void MediaIndexer::updateInfo(const QList<MediaItem> &mediaList)
{
    if (m_nepomukInited && (mediaList.count() > 0)) {
        QString filename = QString("bangarang/%1.jb")
                                .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        QString path = KStandardDirs::locateLocal("data", filename, true);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        QTextStream out(&file);
        out << "#Count = " << mediaList.count() << "\n";
        QList<QString> urls;
        for (int i = 0; i < mediaList.count(); i++) {
            writeUpdateInfo(mediaList.at(i), out);
            out << "\n";
            urls << mediaList.at(i).url;
        }
        out << "\n" <<"\n";
        file.close();
        KProcess * writer = new KProcess();
        writer->setProgram("bangarangnepomukwriter", QStringList(path));
        writer->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        connect(writer, SIGNAL(readyReadStandardOutput()), this, SLOT(processWriterOutput()));
        connect(writer, SIGNAL(started()), this, SIGNAL(started()));
        connect(writer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
        connect(writer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        m_mediaLists.insert(m_writers.count(), mediaList);
        m_urlLists.insert(m_writers.count(), urls);
        m_writers.append(writer);
        writer->start();
        m_state = Running;
        emit percentComplete(0);
    }
}

void MediaIndexer::updateInfo(const MediaItem &mediaItem)
{
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    updateInfo(mediaList);
}

void MediaIndexer::removeInfo(const QList<MediaItem> &mediaList)
{
    if (m_nepomukInited && (mediaList.count() > 0)) {
        QString filename = QString("bangarang/%1.jb")
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        QString path = KStandardDirs::locateLocal("data", filename, true);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        QTextStream out(&file);
        out << "#Count = " << mediaList.count() << "\n";
        for (int i = 0; i < mediaList.count(); i++) {
            writeRemoveInfo(mediaList.at(i), out);
            out << "\n";
        }
        out << "\n" <<"\n";
        file.close();
        KProcess * writer = new KProcess();
        writer->setProgram("bangarangnepomukwriter", QStringList(path));
        writer->setWorkingDirectory(KStandardDirs::locateLocal("data", "bangarang/", true));
        writer->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        connect(writer, SIGNAL(readyReadStandardOutput()), this, SLOT(processWriterOutput()));
        connect(writer, SIGNAL(started()), this, SIGNAL(started()));
        connect(writer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
        connect(writer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        m_writers.append(writer);
        writer->start();
        m_state = Running;
        emit percentComplete(0);
    }
}

void MediaIndexer::removeInfo(const MediaItem &mediaItem)
{
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    removeInfo(mediaList);
}

void MediaIndexer::updatePlaybackInfo(const QString &url, bool incrementPlayCount, const QDateTime &playDateTime)
{
    if (m_nepomukInited && !url.isEmpty()) {
        QString filename = QString("bangarang/%1.jb")
        .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        QString path = KStandardDirs::locateLocal("data", filename, true);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        QTextStream out(&file);
        out << "[" << url << "]\n";
        out << "lastPlayed = " << playDateTime.toString("yyyyMMddhhmmss") << "\n";
        if (incrementPlayCount) {
            int playCount = 0;
            Nepomuk::Resource res(url);
            if (res.exists()) {
                playCount = res.property(MediaVocabulary().playCount()).toInt();
            }   
            playCount = playCount + 1;
            out << "playCount = " << playCount << "\n";
        }
        out << "\n" << "\n";
        KProcess * writer = new KProcess();
        writer->setProgram("bangarangnepomukwriter", QStringList(path));
        writer->setWorkingDirectory(KStandardDirs::locateLocal("data", "bangarang/", true));
        writer->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        connect(writer, SIGNAL(started()), this, SIGNAL(started()));
        connect(writer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
        connect(writer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        m_writers.append(writer);
        writer->start();
        m_state = Running;
        emit percentComplete(0);
    }
}

void MediaIndexer::updateRating(const QString &url, int rating)
{
    if (m_nepomukInited && !url.isEmpty()
        && (rating >= 0) && (rating <= 10)) {
        QString filename = QString("bangarang/%1.jb")
        .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        QString path = KStandardDirs::locateLocal("data", filename, true);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        QTextStream out(&file);
        out << "[" << url << "]\n";
        out << "rating = " << rating << "\n";
        out << "\n" << "\n";
        KProcess * writer = new KProcess();
        writer->setProgram("bangarangnepomukwriter", QStringList(path));
        writer->setWorkingDirectory(KStandardDirs::locateLocal("data", "bangarang/", true));
        writer->setOutputChannelMode(KProcess::OnlyStdoutChannel);
        connect(writer, SIGNAL(started()), this, SIGNAL(started()));
        connect(writer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
        connect(writer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        m_writers.append(writer);
        writer->start();
        m_state = Running;
        emit percentComplete(0);
    }
}

void MediaIndexer::writeRemoveInfo(MediaItem mediaItem, QTextStream &out)
{
    out << "[" << mediaItem.fields["resourceUri"].toString()  << "]\n";;
    out << "type = " << mediaItem.type  << "\n";;
    if (mediaItem.type == "Audio") {
        out << "audioType = " << mediaItem.fields["audioType"].toString() << "\n";
    } else if (mediaItem.type == "Video") {
        out << "videoType = " << mediaItem.fields["videoType"].toString() << "\n";
    }
    out << "url = " << mediaItem.fields["url"].toString() << "\n";
    out << "removeInfo = true" << "\n";
}

void MediaIndexer::writeUpdateInfo(MediaItem mediaItem, QTextStream &out)
{
    out << "[" << mediaItem.fields["resourceUri"].toString()  << "]\n";;
    out << "type = " << mediaItem.type  << "\n";;
    
    QHashIterator<QString, QVariant> i(mediaItem.fields);
    while (i.hasNext()) {
        i.next();
        if (i.value().type() == QVariant::DateTime) {
            out << i.key() << " = " << i.value().toDateTime().toString("yyyyMMddhhmmss") << "\n";
        } else if (i.value().type() == QVariant::DateTime) {
            out << i.key() << " = " << i.value().toDateTime().toString("yyyyMMdd") << "\n";
        } else {
            out << i.key() << " = " << i.value().toString()  << "\n";
        }
    }
}

void MediaIndexer::processWriterOutput()
{
    for (int i = 0; i < m_writers.count(); i++) {
        m_writers.at(i)->setReadChannel(QProcess::StandardOutput);
        while (!m_writers.at(i)->atEnd()) {
            if (m_writers.at(i)->canReadLine()) {
                char buffer[1024];
                qint64 lineLength = m_writers.at(i)->readLine(buffer, sizeof(buffer));
                if (lineLength != -1) {
                    QString line = QUrl::fromPercentEncoding(buffer);
                    if (line.startsWith("BangarangProgress:")) {
                        int percent = line.remove("BangarangProgress:").trimmed().toInt();
                        emit percentComplete(percent);
                    } else if (line.startsWith("BangarangSignal:sourceInfoUpdated:")) {
                        QString url = line.remove("BangarangSignal:sourceInfoUpdated:").trimmed();
                        QList<QString> urls = m_urlLists[i];
                        int index = urls.indexOf(url);
                        if (index != -1) {
                            MediaItem mediaItem = m_mediaLists[i].at(index);
                            emit sourceInfoUpdated(mediaItem);
                        }
                    } else if (line.startsWith("BangrangSignal:urlInfoRemoved:")) {
                        QString url = line.remove("BangrangSignal:urlInfoRemoved:").trimmed();
                        emit urlInfoRemoved(url);
                    }
                }
            }
        }
    }                     
}

void MediaIndexer::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished();
    bool isAllFinished = true;
    for (int i = 0; i < m_writers.count(); i++) {
        if (m_writers.at(i)->state() == QProcess::Running ||
            m_writers.at(i)->state() == QProcess::Starting) {
            isAllFinished = false;
        }
    }
    
    if (isAllFinished) {
        m_state = Idle;
        emit allFinished();
    }
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}

void MediaIndexer::error(QProcess::ProcessError error)
{
    kDebug() << error;
}
