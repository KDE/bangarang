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

#include "feedinfofetcher.h"
#include "../downloader.h"
#include "../mediaitemmodel.h"

#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <Solid/Networking>

#include <QDomDocument>
#include <QFile>

FeedInfoFetcher::FeedInfoFetcher(QObject *parent) :
    InfoFetcher(parent)
{
    m_name = i18n("Feed Info");
    m_icon = KIcon("application-rss+xml");
    m_about = i18n("This fetcher gets information for the feed at the specified location.");

    m_downloader = new Downloader(this);
    connect(this, SIGNAL(download(KUrl,KUrl)), m_downloader, SLOT(download(KUrl,KUrl)));
    connect(m_downloader, SIGNAL(downloadComplete(KUrl,KUrl)), this, SLOT(gotFeedInfo(KUrl,KUrl)));

    //Define fetchable fields
    m_fetchableFields["Audio Feed"] = QStringList() << "artwork" << "title" << "description";
    m_fetchableFields["Video Feed"] = QStringList() << "artwork" << "title" << "description";

    //Define required fields
    m_requiredFields["Audio Feed"] = QStringList() << "url";
    m_requiredFields["Video Feed"] = QStringList() << "url";

}

FeedInfoFetcher::~FeedInfoFetcher()
{
}

bool FeedInfoFetcher::available(const QString &subType)
{
    //Available if connected to network
    bool networkConnected = (Solid::Networking::status() == Solid::Networking::Connected);
    bool handlesType = (m_requiredFields[subType].count() > 0);
    return (networkConnected && handlesType);
}

void FeedInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    Q_UNUSED(maxMatches);
    m_updateRequiredFields = updateRequiredFields;
    m_mediaList.clear();
    m_requestKeys.clear();
    m_updateArtwork = updateArtwork;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        QString feedUrlStr = mediaItem.fields["url"].toString();

        KUrl feedUrl(feedUrlStr);
        if (!feedUrl.isEmpty()) {
            m_mediaList.append(mediaItem);
            m_requestKeys.append(feedUrl.prettyUrl());

            //Retrieve feed info
            QString feedTargetFile = QString("bangarang/temp/%1").arg(feedUrl.fileName());
            KUrl feedTargetUrl = KUrl(KStandardDirs::locateLocal("data", feedTargetFile, true));
            QFile feedTarget(feedTargetUrl.path());
            feedTarget.remove();
            setFetching();
            emit download(feedUrl, feedTargetUrl);
        }
    }
}

void FeedInfoFetcher::gotFeedInfo(const KUrl &from, const KUrl &to)
{
    //Determine if we got feed or thumbnail
    bool gotThumbnail = false;
    if (m_requestKeys.indexOf(from.prettyUrl()) == -1) {
        if (!m_thumbnailKeys.key(from.prettyUrl()).isEmpty()) {
            gotThumbnail = true;
        }
    }

    if (!gotThumbnail) {
        //Find item corresponding to fetched info
        int foundIndex = m_requestKeys.indexOf(from.prettyUrl());
        if (foundIndex == -1) {
            return;
        }

        MediaItem mediaItem = m_mediaList.at(foundIndex);

        QFile file(to.path());
        QDomDocument feedDoc("feed");
        feedDoc.setContent(&file);

        //Iterate through item nodes of the XML document
        bool mediaItemUpdated = false;
        KUrl thumbnailUrl;
        QDomNodeList channels = feedDoc.elementsByTagName("channel");
        for (int i = 0; i < channels.count(); i++) {
            QDomNodeList nodes = channels.at(i).childNodes();
            for (int j = 0; j < nodes.count(); j++) {
                if (nodes.at(j).isElement()) {
                    QDomElement element = nodes.at(j).toElement();
                    if (element.tagName() == "title") {
                        mediaItem.title = element.text();
                        mediaItem.fields["title"] = mediaItem.title;
                        mediaItemUpdated = true;
                    } else if (element.tagName() == "description") {
                        mediaItem.fields["description"] = element.text();
                        mediaItemUpdated = true;
                    } else if (element.tagName() == "itunes:image") {
                        if (thumbnailUrl.isEmpty() && m_updateArtwork) {
                            thumbnailUrl = KUrl(element.attribute("href"));
                        }
                    } else if (element.tagName() == "image") {
                        QDomNodeList imageNodes = nodes.at(j).childNodes();
                        for (int k = 0; k < imageNodes.count(); k++) {
                            QDomElement imageElement = imageNodes.at(k).toElement();
                            if (imageElement.tagName() == "url") {
                                if (thumbnailUrl.isEmpty() && m_updateArtwork) {
                                    thumbnailUrl = KUrl(imageElement.text());
                                }
                            }
                        }
                    }
                }
            }
        }
        QFile(to.path()).remove();

        if (mediaItemUpdated) {
            m_mediaList.replace(foundIndex, mediaItem);
            QList<MediaItem> fetchedMatches;
            fetchedMatches.append(mediaItem);
            emit infoFetched(fetchedMatches);
        }

        //Fetch Thumbnail
        if (thumbnailUrl.isValid()) {
            m_thumbnailKeys[from.prettyUrl()] = thumbnailUrl.prettyUrl();
            QString thumbnailTargetFile = QString("bangarang/thumbnails/%1-%2-%3")
                                          .arg(mediaItem.subType())
                                          .arg(mediaItem.title)
                                          .arg(thumbnailUrl.fileName());
            KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
            QFile downloadTarget(thumbnailTargetUrl.path());
            downloadTarget.remove();
            emit download(thumbnailUrl, thumbnailTargetUrl);
        } else {
            m_requestKeys.removeAll(from.prettyUrl());
        }

    } else {
        //Find item corresponding to fetched thumbnail
        QString requestKey = m_thumbnailKeys.key(from.prettyUrl());
        int foundIndex = m_requestKeys.indexOf(requestKey);
        if (foundIndex == -1) {
            return;
        }

        //Update artwork
        QString thumbnailFile = to.path();
        QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!thumbnail.isNull())  {
            MediaItem mediaItem = m_mediaList.at(foundIndex);
            mediaItem.artwork = QIcon(thumbnail);
            mediaItem.fields["artworkUrl"] = to.prettyUrl();
            mediaItem.hasCustomArtwork = true;
            m_mediaList.replace(foundIndex, mediaItem);
            QList<MediaItem> fetchedMatches;
            fetchedMatches.append(mediaItem);
            emit infoFetched(fetchedMatches);
        }
        m_requestKeys.removeAll(requestKey);
    }

    if (m_requestKeys.count() == 0) {
        m_isFetching = false;
        if (!m_timeout) {
            m_timer->stop();
            emit fetchComplete();
            emit fetchComplete(this);
        }
    } else if (!m_timeout){
        m_timer->start(m_timeoutLength);
    }
}
