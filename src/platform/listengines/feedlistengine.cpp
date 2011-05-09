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

#include "../mediaitemmodel.h"
#include "feedlistengine.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

#include <QApplication>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>
#include <QFile>
#include <Nepomuk/ResourceManager>

FeedListEngine::FeedListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
    m_fetchingThumbnails = false;
}

FeedListEngine::~FeedListEngine()
{
}

void FeedListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }

    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QStringList engineFilterList = m_mediaListProperties.engineFilterList();
    
    if (m_nepomukInited) {
        if (engineArg == "audiofeeds") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioFeed(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString title = it.binding(mediaVocabulary.titleBinding()).literal().toString().trimmed();
                QString description = it.binding(mediaVocabulary.descriptionBinding()).literal().toString().trimmed();
                QString feedUrl = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().toString();
                MediaItem mediaItem;
                mediaItem.url = QString("feeds://audio?feedUrl=%1").arg(feedUrl);
                mediaItem.type = "Category";
                mediaItem.fields["categoryType"] = QString("Audio Feed");
                mediaItem.artwork = KIcon("application-rss+xml");
                mediaItem.title = title;
                mediaItem.fields["title"] = title;
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                mediaItem.fields["description"] = description;
                mediaItem.fields["url"] = feedUrl;
                mediaItem.fields["resourceUri"] = it.binding(MediaVocabulary::mediaResourceBinding()).uri().toString();
                mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
                mediaItem = Utilities::makeSubtitle(mediaItem);
                mediaList.append(mediaItem);
            }
            MediaItem mediaItem;
            mediaItem.title = i18n("New audio feed");
            mediaItem.subTitle = i18n("Edit info to create new audio feed");
            mediaItem.fields["title"] = "Untitled";
            mediaItem.fields["url"] = QString();
            mediaItem.type = QString("Category");
            mediaItem.fields["categoryType"] = QString("Audio Feed");
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            mediaItem.fields["isTemplate"] = true;
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaList.append(mediaItem);
            
            m_mediaListProperties.summary = i18np("1 feed", "%1 feeds", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
            //Return results
            emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
            
        } else if (engineArg == "videofeeds") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoFeed(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString title = it.binding(mediaVocabulary.titleBinding()).literal().toString().trimmed();
                QString description = it.binding(mediaVocabulary.descriptionBinding()).literal().toString().trimmed();
                QString feedUrl = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().toString();
                MediaItem mediaItem;
                mediaItem.url = QString("feeds://video?feedUrl=%1").arg(feedUrl);
                mediaItem.type = "Category";
                mediaItem.fields["categoryType"] = QString("Video Feed");
                mediaItem.artwork = KIcon("application-rss+xml");
                mediaItem.title = title;
                mediaItem.fields["title"] = title;
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                mediaItem.fields["description"] = description;
                mediaItem.fields["url"] = feedUrl;
                mediaItem.fields["resourceUri"] = it.binding(MediaVocabulary::mediaResourceBinding()).uri().toString();
                mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
                mediaItem = Utilities::makeSubtitle(mediaItem);
                mediaList.append(mediaItem);
            }
            MediaItem mediaItem;
            mediaItem.title = i18n("New video feed");
            mediaItem.subTitle = i18n("Edit info to create new video feed");
            mediaItem.fields["title"] = i18n("Untitled");
            mediaItem.fields["url"] = QString();
            mediaItem.type = QString("Category");
            mediaItem.fields["categoryType"] = QString("Video Feed");
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            mediaItem.fields["isTemplate"] = true;
            mediaItem.artwork = KIcon("application-rss+xml");
            mediaList.append(mediaItem);
            
            m_mediaListProperties.summary = i18np("1 feed", "%1 feeds", mediaList.count());
            m_mediaListProperties.type = QString("Categories");
            
            //Return results
            emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
            
        } else if (engineArg == "audio" || engineArg == "video") {
            //Get feed url
            QString feedUrlStr = m_mediaListProperties.filterFieldValue("feedUrl");
            
            KUrl feedUrl(feedUrlStr);
            if (!feedUrl.isEmpty()) {
                QString feedTargetFile = QString("bangarang/temp/%1").arg(feedUrl.fileName());
                KUrl feedTargetUrl = KUrl(KStandardDirs::locateLocal("data", feedTargetFile, true));
                connectDownloader();
                emit download(feedUrl, feedTargetUrl);
            }  
            
            //Start event loop to wait for feed results
            exec();
        }
        
    }
}

void FeedListEngine::downloadComplete(const KUrl &from, const KUrl &to)
{
    if (!m_fetchingThumbnails) {
        m_mediaList.clear();
        m_artworkUrlList.clear();
        QFile file(to.path());
        QDomDocument feedDoc("feed");
        feedDoc.setContent(&file);

        //Determine if RSS or Atom based feed
        bool isRSS = (feedDoc.elementsByTagName("rss").count() > 0);
        bool isAtom = (feedDoc.elementsByTagName("feed").count() > 0);
        QString itemTagName = isRSS ? "item" : (isAtom ? "entry" : QString());

        //Specify tag preference order
        QStringList titleTagPref;
        titleTagPref << "media:title" << "title";
        QStringList descriptionTagPref;
        descriptionTagPref << "media:description" << "itunes:summary" << "description";
        QStringList contentTagPref;
        contentTagPref << "media:content" << "enclosure";
        QStringList releaseDateTagPref = QStringList() << "pubDate" << "published";
        
        //Iterate through item nodes of the XML document
        QDomNodeList items = feedDoc.elementsByTagName(itemTagName);
        for (int i = 0; i < items.count(); i++) {
            MediaItem mediaItem;
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
            bool isAudio = false;
            bool isVideo = false;
            QDomNodeList itemNodes = items.at(i).childNodes();
            QDomElement releaseDateElement = getPreferredTag(itemNodes, releaseDateTagPref);
            mediaItem.fields["releaseDate"] = releaseDateElement.text();
            mediaItem.semanticComment = releaseDateElement.text();
            QList<QDomElement> relatedTos = getPreferredTags(itemNodes, QStringList("link"));
            for (int j = 0; j < relatedTos.count(); j++) {
                QDomElement relatedTo = relatedTos.at(j);
                if (!relatedTo.hasAttribute("rel") ||
                    (relatedTo.hasAttribute("rel") && relatedTo.attribute("rel").toLower() == "alternate")) {
                    if (relatedTo.hasAttribute("href")) {
                        mediaItem.fields["relatedTo"] = QStringList(relatedTo.attribute("href"));
                    } else {
                        mediaItem.fields["relatedTo"] = QStringList(relatedTo.text());
                    }
                }
            }
            //Only use media:group nodes in Atom feeds
            if (isAtom) {
                QDomElement mediaGroup = getPreferredTag(itemNodes, QStringList("media:group"));
                if (!mediaGroup.isNull()) {
                    itemNodes = mediaGroup.childNodes();
                }
            }
            QDomElement titleElement = getPreferredTag(itemNodes, titleTagPref);
            mediaItem.title = titleElement.text();
            mediaItem.fields["title"] = titleElement.text();
            QDomElement descriptionElement = getPreferredTag(itemNodes, descriptionTagPref);
            if (!descriptionElement.text().trimmed().startsWith("<") &&
                !descriptionElement.text().trimmed().endsWith(">")) { //ignore html descriptions
                mediaItem.fields["description"] = descriptionElement.text();
            }
            QList<QDomElement> contentElements = getPreferredTags(itemNodes, contentTagPref);
            for (int j = 0; j < contentElements.count(); j++) {
                QDomElement contentElement = contentElements.at(j);
                if (contentElement.tagName() == "media:content") {
                    int duration = contentElement.attribute("duration").toInt();
                    if (duration != 0 ) {
                        mediaItem.duration = Utilities::durationString(duration);
                        mediaItem.fields["duration"] = duration;
                    }
                }
                KMimeType::Ptr type = KMimeType::mimeType(contentElement.attribute("type").toLower().trimmed());
                if (!type.isNull()) {
                    if (Utilities::isAudioMimeType(type)) {
                        isAudio = true;
                        mediaItem.url = contentElement.attribute("url");
                        mediaItem.fields["url"] = mediaItem.url;
                        mediaItem.type = "Audio";
                        mediaItem.fields["audioType"] = "Audio Clip";
                        mediaItem.artwork = KIcon("audio-x-generic");
                    }
                    if (Utilities::isVideoMimeType(type)) {
                        isVideo = true;
                        mediaItem.url = contentElement.attribute("url");
                        mediaItem.fields["url"] = mediaItem.url;
                        mediaItem.type = "Video";
                        mediaItem.fields["videoType"] = "Video Clip";
                        mediaItem.artwork = KIcon("video-x-generic");
                    }
                }
            }
            if (mediaItem.duration.isEmpty()) {
                QDomElement durationElement = getPreferredTag(itemNodes, QStringList("itunes:duration"));
                if (durationElement.text().contains(":")) {
                    mediaItem.duration = durationElement.text();
                } else {
                    int duration = durationElement.text().toInt();
                    if (duration != 0 ) {
                        mediaItem.duration = Utilities::durationString(duration);
                        mediaItem.fields["duration"] = duration;
                    }
                }
            }
            QDomElement thumbnailElement = getPreferredTag(itemNodes, QStringList("media:thumbnail"));
            
            mediaItem = Utilities::makeSubtitle(mediaItem);
            if (isAudio && m_mediaListProperties.engineArg() == "audio") {
                m_mediaList.append(mediaItem);
                m_artworkUrlList.append(KUrl(thumbnailElement.attribute("url")));
            } else if (isVideo && m_mediaListProperties.engineArg() == "video") {
                m_mediaList.append(mediaItem);
                m_artworkUrlList.append(KUrl(thumbnailElement.attribute("url")));
            }
        }
        file.remove();
        
        m_mediaListProperties.summary = i18np("1 item", "%1 items", m_mediaList.count());
        m_mediaListProperties.type = QString("Sources");
        
        //Return results
        emit results(m_requestSignature, m_mediaList, m_mediaListProperties, true, m_subRequestSignature);
        
        //Launch thumbnail downloads
        for (int i = 0; i < m_artworkUrlList.count(); i++) {
            KUrl artworkUrl = m_artworkUrlList.at(i);
            if (!artworkUrl.isEmpty()) {
                m_fetchingThumbnails = true;
                QString artworkTargetFile = QString("bangarang/thumbnails/%1").arg(artworkUrl.fileName());
                KUrl artworkTargetUrl = KUrl(KStandardDirs::locateLocal("data", artworkTargetFile, true));
                emit download(artworkUrl, artworkTargetUrl);
            }
        }
        
    } else {
        //Update feed item artwork with thumbnail downloads
        int index = m_artworkUrlList.indexOf(from);
        if (index != -1) {
            MediaItem mediaItem = m_mediaList.at(index);
            mediaItem.fields["artworkUrl"] = to.prettyUrl();
            QImage artwork = Utilities::getArtworkImageFromMediaItem(mediaItem);
            if (!artwork.isNull()) {
                mediaItem.hasCustomArtwork = true;
                emit updateArtwork(artwork, mediaItem);
            }
            m_mediaList.removeAt(index);
            m_artworkUrlList.removeAt(index);
            if (m_mediaList.count() == 0) {
                m_fetchingThumbnails = false;
                disconnectDownloader();
            }
        }
    }
    
    if (!m_fetchingThumbnails) {
        //Exit event loop
        quit();
    }

}

QDomElement FeedListEngine::getPreferredTag(const QDomNodeList &itemNodes, const QStringList &tagPref)
{
    QDomElement preferredElement;
    for (int i = 0; i < tagPref.count(); i++) {
        QString tag = tagPref.at(i);
        for (int j = 0; j < itemNodes.count(); j++) {
            if (itemNodes.at(j).isElement()) {
                QDomElement itemElement = itemNodes.at(j).toElement();
                if (itemElement.tagName() == tag) {
                    return itemElement;
                }
            }
        }
    }
    return preferredElement;
}

QList<QDomElement> FeedListEngine::getPreferredTags(const QDomNodeList &itemNodes, const QStringList &tagPref)
{
    QList<QDomElement> preferredElements;
    bool foundPreferred = false;
    for (int i = 0; i < tagPref.count(); i++) {
        QString tag = tagPref.at(i);
        for (int j = 0; j < itemNodes.count(); j++) {
            if (itemNodes.at(j).isElement()) {
                QDomElement itemElement = itemNodes.at(j).toElement();
                if (itemElement.tagName() == tag) {
                    preferredElements.append(itemElement);
                    foundPreferred = true;
                }
            }
        }
        if (foundPreferred) {
            break;
        }
    }
    return preferredElements;
}
