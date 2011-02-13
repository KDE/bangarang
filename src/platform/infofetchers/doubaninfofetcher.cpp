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

#include "doubaninfofetcher.h"
#include "../downloader.h"
#include "../mediaitemmodel.h"

#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <Solid/Networking>

#include <QDomDocument>
#include <QFile>

DoubanInfoFetcher::DoubanInfoFetcher(QObject* parent) :
        InfoFetcher(parent)
{
    m_name = i18n("Douban");
    m_icon = KIcon("bangarang-douban");
    m_url = KUrl("http://douban.com");
    m_about = i18n("Note: This fetcher uses the Douban API but is not endorsed or certified by Douban.");

    m_searchAPI = "http://api.douban.com/music/subjects?q=%1&max-results=%2";
    m_timeout = 20000;

    m_downloader = new Downloader(this);
    connect(this, SIGNAL(download(KUrl,KUrl)), m_downloader, SLOT(download(KUrl,KUrl)));
    connect(m_downloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(processOriginalRequest(KUrl,KUrl)));
    m_thumbnailDownloader = new Downloader(this);
    connect(this, SIGNAL(downloadThumbnail(KUrl,KUrl)), m_thumbnailDownloader, SLOT(download(KUrl,KUrl)));
    connect(m_thumbnailDownloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(processThumbnails(KUrl,KUrl)));

    //Define fetchable fields
    m_fetchableFields["Music"] = QStringList() << "artwork" << "title" << "description" << "artist" << "album" << "trackNumber" << "genre" << "year" << "duration";
//     m_fetchableFields["Artist"] = QStringList() << "artwork" << "title" << "description";
//     m_fetchableFields["Album"] = QStringList() << "artwork" << "title" << "description";

    //Define required fields
    m_requiredFields["Music"] = QStringList() << "title" << "artist";
//     m_requiredFields["Artist"] = QStringList() << "title";
//     m_requiredFields["Album"] = QStringList() << "title" << "artist";
}

DoubanInfoFetcher::~DoubanInfoFetcher()
{
}

bool DoubanInfoFetcher::available(const QString &subType)
{
    Solid::Networking::Status state = Solid::Networking::status();
    bool networkConnected = ( state == Solid::Networking::Connected ||
                              state == Solid::Networking::Unknown );
    bool handlesType = (m_requiredFields[subType].count() > 0);
    return (networkConnected && handlesType);
}

void DoubanInfoFetcher::timeout()
{
    //Return any remaining matches that have not yet been returned
    emit infoFetched(m_fetchedMatches.values());
    InfoFetcher::timeout();
}

void DoubanInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool fetchArtwork)
{
    Q_UNUSED(maxMatches);
    Q_UNUSED(updateRequiredFields);
    Q_UNUSED(fetchArtwork);
    m_mediaList.clear();
    m_requestKeys.clear();
    m_fetchedMatches.clear();
    m_thumbnailKeys.clear();
    m_toFetchCount = mediaList.count();
    m_fetchedCount = 0;

    for (int i = 0; i < m_toFetchCount; i++) {
        MediaItem mediaItem = mediaList.at(i);
        QString DoubanUrlStr;

        if (mediaItem.subType() == "Album") {
            QString album = mediaItem.fields["title"].toString();
            QString artist = mediaItem.fields["artist"].toString();
            DoubanUrlStr = m_searchAPI.arg(album).arg(artist).arg(1);
        }
        else if (mediaItem.subType() == "Music") {
            QString title = mediaItem.fields["title"].toString();
            QString artist = mediaItem.fields["artist"].toString();
            QString album = mediaItem.fields["album"].toString();
            QString searchTerm = title + ',' + artist;
            if (!album.isEmpty())
                searchTerm += ',' + album;
            DoubanUrlStr = m_searchAPI.arg(searchTerm).arg(1);
        }

        if (DoubanUrlStr.isEmpty())
            continue;

        KUrl DoubanUrl(DoubanUrlStr);

        m_mediaList.append(mediaItem);
        m_requestKeys.append(DoubanUrl.prettyUrl());

        //Retrieve Douban info
        QString DoubanTargetFile = QString("bangarang/temp/Douban%1.xml").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzzz"));
        KUrl DoubanTargetUrl = KUrl(KStandardDirs::locateLocal("data", DoubanTargetFile, true));
        QFile DoubanTarget(DoubanTargetUrl.path());
        DoubanTarget.remove();
        setFetching();
        emit download(DoubanUrl, DoubanTargetUrl);
    }
}

void DoubanInfoFetcher::processOriginalRequest(const KUrl &from, const KUrl to)
{
    //Process results
    int originalRequestIndex = m_requestKeys.indexOf(from.prettyUrl());
    MediaItem mediaItem = m_mediaList.at(originalRequestIndex);

    QFile file(to.path());
    QDomDocument DoubanDoc("Douban");
    DoubanDoc.setContent(&file);

    QDomNode entry = DoubanDoc.elementsByTagName("entry").at(0);

    QDomElement titleElem = entry.firstChildElement("title");
    if (!titleElem.isNull()) {
        if (mediaItem.subType() == "Album") {
            mediaItem.title = titleElem.text();
            mediaItem.fields["title"] = mediaItem.title;
        }
        else if (mediaItem.subType() == "Music") {
            mediaItem.fields["album"] = titleElem.text();
        }
    }

    QDomElement dbattrElem = entry.firstChildElement("db:attribute");
    while (!dbattrElem.isNull()) {
        if (dbattrElem.attribute("name") == "pubdate") {
            mediaItem.fields["year"] = dbattrElem.text().section('-', 0, 0);
        }
        else if (dbattrElem.attribute("name") == "singer") {
            mediaItem.fields["artist"] = QStringList() << dbattrElem.text();
        }
        dbattrElem = dbattrElem.nextSiblingElement("db:attribute");
    }

    //Add fetched matches to collection of fetched matches
    m_fetchedMatches.insert(originalRequestIndex, mediaItem);

    QString link;
    QDomElement linkElem = entry.firstChildElement("link");
    while (!linkElem.isNull()) {
        link = linkElem.attribute("href");
        if (link.endsWith("jpg"))
            break;
        linkElem = linkElem.nextSiblingElement("link");
    }
    QString imageUrl = link.replace("spic","lpic");
    KUrl thumbnailUrl = KUrl(imageUrl);

    QFile(to.path()).remove();

    //Launch Thumbnail requests
    QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-Douban-%2-%3")
                                    .arg(mediaItem.subType())
                                    .arg(mediaItem.fields["DoubanID"].toString())
                                    .arg(thumbnailUrl.fileName());
    KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
    QFile downloadTarget(thumbnailTargetUrl.path());
    downloadTarget.remove();

    m_thumbnailKeys[thumbnailUrl.prettyUrl()] = originalRequestIndex;

    emit downloadThumbnail(thumbnailUrl, thumbnailTargetUrl);
}

void DoubanInfoFetcher::processThumbnails(const KUrl &from, const KUrl to)
{
    //Process results
    int originalRequestIndex = m_thumbnailKeys.value(from.prettyUrl());
    MediaItem mediaItem = m_fetchedMatches.value(originalRequestIndex);

    QString thumbnailFile = to.path();
    qWarning() << "thumbnailFile" << thumbnailFile;
    QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!thumbnail.isNull()) {
        mediaItem.artwork = QIcon(thumbnail);
        mediaItem.fields["artworkUrl"] = to.prettyUrl();
        mediaItem.hasCustomArtwork = true;
    }

    m_fetchedMatches[originalRequestIndex] = mediaItem;

    m_fetchedCount++;
    if (m_toFetchCount == m_fetchedCount) {
        emit infoFetched(m_fetchedMatches.values());
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
            emit fetchComplete(this);
        }
        m_fetchedCount = 0;
        m_toFetchCount = 0;
    }
}

