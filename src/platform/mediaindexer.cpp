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
#include "utilities/utilities.h"
#include "mediavocabulary.h"

#include <KUrl>
#include <KDebug>
#include <KStandardDirs>
#include <KLocale>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>
#include <QTextStream>
#include <QProcess>
#include <QFile>
#include <QHash>

MediaIndexer::MediaIndexer(QObject * parent) : QObject(parent)
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    m_nepomukInited = Utilities::nepomukInited();
    m_state = Idle;
    m_percent = 0;
    m_writer = new KProcess(this);
    m_writer->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    m_writer->setWorkingDirectory(KStandardDirs::locateLocal("data", "bangarang/", true));
    connect(this, SIGNAL(startWriter(QStringList)), this, SLOT(startWriterSlot(QStringList)));
    connect(m_writer, SIGNAL(readyReadStandardOutput()), this, SLOT(processWriterOutput()));
    connect(m_writer, SIGNAL(started()), this, SIGNAL(started()));
    connect(m_writer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(m_writer, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
}

MediaIndexer::~MediaIndexer()
{
}

MediaIndexer::State MediaIndexer::state()
{
    return m_state;
}

void MediaIndexer::updateInfo(const QList<MediaItem> &mediaList)
{
    if (m_state == Running) {
        return;
    }
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
        m_mediaList = mediaList;
        m_urlList = urls;
        emit startWriter(QStringList(path));
        m_status["description"] = i18n("Starting update...");
        m_status["progress"] = 0;
        emit updateStatus(m_status);
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
    if (m_state == Running) {
        return;
    }
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
        emit startWriter(QStringList(path));
        m_status["description"] = i18n("Starting update...");
        m_status["progress"] = 0;
        emit updateStatus(m_status);
    }
}

void MediaIndexer::removeInfo(const MediaItem &mediaItem)
{
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    removeInfo(mediaList);
}

void MediaIndexer::updatePlaybackInfo(const QString &resourceUri, bool incrementPlayCount, const QDateTime &playDateTime)
{
    if (m_state == Running) {
        return;
    }
    if (m_nepomukInited && !resourceUri.isEmpty()) {
        kDebug() << "Updating playback info...";
        QString filename = QString("bangarang/%1.jb")
        .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        QString path = KStandardDirs::locateLocal("data", filename, true);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        QTextStream out(&file);
        out << "[" << resourceUri << "]\n";
        out << "lastPlayed = " << playDateTime.toString("yyyyMMddhhmmss") << "\n";
        if (incrementPlayCount) {
            int playCount = 0;
            Nepomuk::Resource res(resourceUri);
            if (res.exists()) {
                playCount = res.property(MediaVocabulary().playCount()).toInt();
            }   
            playCount = playCount + 1;
            out << "playCount = " << playCount << "\n";
        }
        out << "\n" << "\n";
        emit startWriter(QStringList(path));
    }
}

void MediaIndexer::updateRating(const QString & resourceUri, int rating)
{
    if (m_writer->state() == QProcess::Starting || m_state == Running) {
        return;
    }
    if (m_nepomukInited && !resourceUri.isEmpty()
        && (rating >= 0) && (rating <= 10)) {
        QString filename = QString("bangarang/%1.jb")
        .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        QString path = KStandardDirs::locateLocal("data", filename, true);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }
        QTextStream out(&file);
        out << "[" << resourceUri << "]\n";
        out << "rating = " << rating << "\n";
        out << "\n" << "\n";
        emit startWriter(QStringList(path));
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
    } else if (mediaItem.type == "Category") {
        out << "categoryType = " << mediaItem.fields["categoryType"].toString() << "\n";
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
        } else if (i.value().type() == QVariant::StringList){
            out << i.key() << " = " << i.value().toStringList().join("||")  << "\n";
        } else {
            out << i.key() << " = " << i.value().toString()  << "\n";
        }
    }
}

void MediaIndexer::startWriterSlot(const QStringList &args)
{
    m_writer->setProgram("bangarangnepomukwriter", args);
    m_writer->start();
    m_state = Running;
}

void MediaIndexer::processWriterOutput()
{
    m_writer->setReadChannel(QProcess::StandardOutput);
    while (!m_writer->atEnd() && m_writer->canReadLine()) {
        char buffer[1024];
        qint64 lineLength = m_writer->readLine(buffer, sizeof(buffer));
        if (lineLength == -1) {
            continue;
        }

        QString line = QUrl::fromPercentEncoding(buffer);
        if (line.startsWith("BangarangProgress:")) {
            m_percent = line.remove("BangarangProgress:").trimmed().toInt();
        } else if (line.startsWith("BangarangSignal:sourceInfoUpdated:")) {
            QString url = line.remove("BangarangSignal:sourceInfoUpdated:").trimmed();
            int index = m_urlList.indexOf(url);
            if (index == -1) {
                continue;
            }
            MediaItem mediaItem = m_mediaList.at(index);
            Nepomuk::Resource res(KUrl(mediaItem.url));
            if (res.exists()) {
                mediaItem.fields["resourceUri"] = res.resourceUri().toString();
            }
            emit sourceInfoUpdated(mediaItem);
            m_status["description"] = i18n("Updated: %1 - %2", mediaItem.title, mediaItem.subTitle);
            m_status["progress"] = m_percent;
            emit updateStatus(m_status);
        } else if (line.startsWith("BangrangSignal:urlInfoRemoved:")) {
            QString resourceUri = line.remove("BangrangSignal:urlInfoRemoved:").trimmed();
            m_status["description"] = i18n("Removing info...");
            m_status["progress"] = m_percent;
            emit updateStatus(m_status);
            emit urlInfoRemoved(resourceUri);
        } else if (line.startsWith("BangarangMessage:")) {
            QString message = line.remove("BangarangMessage:").trimmed();
            m_status["description"] = message;
            m_status["progress"] = m_percent;
            emit updateStatus(m_status);
        } else if (line.startsWith("BangarangDebug:")) {
            kDebug() << line.remove("BangarangDebug:");
        }
    }
}

void MediaIndexer::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_status["description"] = QString();
    m_status["progress"] = -1;
    emit updateStatus(m_status);
    emit finished();
    m_state = Idle;
    emit allFinished();
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}

void MediaIndexer::error(QProcess::ProcessError error)
{
    kDebug() << error;
}
#include "mediaindexer.moc"
