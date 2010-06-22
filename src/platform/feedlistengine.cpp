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

#include "mediaitemmodel.h"
#include "feedlistengine.h"
#include "listenginefactory.h"
#include "mediaindexer.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <QApplication>
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>
#include <QFile>
#include <QDomDocument>
#include <QDomNodeList>
#include <Nepomuk/ResourceManager>

FeedListEngine::FeedListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

FeedListEngine::~FeedListEngine()
{
}

void FeedListEngine::run()
{
    QThread::setTerminationEnabled(true);
    
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
                mediaItem.subTitle = description;
                mediaItem.fields["description"] = description;
                mediaItem.fields["url"] = feedUrl;
                mediaItem.fields["resourceUri"] = it.binding(MediaVocabulary::mediaResourceBinding()).uri().toString();
                mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
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
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeVideoFeed(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings = bindings;
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
            //Build media list from results
            while( it.next() ) {
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
                mediaItem.subTitle = description;
                mediaItem.fields["description"] = description;
                mediaItem.fields["url"] = feedUrl;
                mediaItem.fields["resourceUri"] = it.binding(MediaVocabulary::mediaResourceBinding()).uri().toString();
                mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
                mediaList.append(mediaItem);
            }
            MediaItem mediaItem;
            mediaItem.title = i18n("New video feed");
            mediaItem.subTitle = i18n("Edit info to create new video feed");
            mediaItem.type = QString("Category");
            mediaItem.fields["categoryType"] = QString("Video Feed");
            mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
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
        
        //Get local feed artwork
        MediaVocabulary mediaVocabulary;
        for (int i = 0; i < mediaList.count(); i++) {
            MediaItem mediaItem = mediaList.at(i);
            if (mediaItem.fields["categoryType"].toString() == "Audio Feed" ||
                mediaItem.fields["categoryType"].toString() == "Video Feed") {
                mediaItem.fields["artworkUrl"] = mediaItem.fields["artworkUrl"];
                QImage artwork = Utilities::getArtworkImageFromMediaItem(mediaItem);
                if (!artwork.isNull()) {
                    emit updateArtwork(artwork, mediaItem);
                    break;
                }
            }
        }
    }
}

void FeedListEngine::downloadComplete(const KUrl &from, const KUrl &to)
{
    Q_UNUSED(from);
    
    disconnectDownloader();
    
    QList<MediaItem> mediaList;
    QFile file(to.path());
    QDomDocument feedDoc("feed");
    feedDoc.setContent(&file);
    
    //Iterate through item nodes of the XML document
    QDomNodeList items = feedDoc.elementsByTagName("item");
    for (int i = 0; i < items.count(); i++) {
        MediaItem mediaItem;
        mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
        bool isAudio = false;
        bool isVideo = false;
        QDomNodeList itemNodes = items.at(i).childNodes();
        for (int j = 0; j < itemNodes.count(); j++) {
            if (itemNodes.at(j).isElement()) {
                QDomElement itemElement = itemNodes.at(j).toElement();
                if (itemElement.tagName() == "title") {
                    mediaItem.title = itemElement.text();
                    mediaItem.fields["title"] = itemElement.text();
                } else if (itemElement.tagName() == "description") {
                    mediaItem.subTitle = QString("%1...").arg(itemElement.text().left(50));
                    mediaItem.fields["description"] = itemElement.text();
                } else if (itemElement.tagName() == "itunes:duration") {
                    mediaItem.duration = itemElement.text();
                } else if (itemElement.tagName() == "pubDate") {
                    mediaItem.fields["releaseDate"] = itemElement.text();
                } else if (itemElement.tagName() == "enclosure") {
                    mediaItem.url = itemElement.attribute("url");
                    mediaItem.fields["url"] = mediaItem.url;
                    KMimeType::Ptr type = KMimeType::mimeType(itemElement.attribute("type"));
                    if (Utilities::isAudioMimeType(type)) {
                        isAudio = true;
                        mediaItem.type = "Audio";
                        mediaItem.fields["audioType"] = "Audio Clip";
                        mediaItem.artwork = KIcon("audio-x-generic");
                    } else if (Utilities::isVideoMimeType(type)) {
                        isVideo = true;
                        mediaItem.type = "Video";
                        mediaItem.fields["videoType"] = "Video Clip";
                        mediaItem.artwork = KIcon("video-x-generic");
                    }
                }
            }
        }
        if (mediaItem.type == "Audio" && m_mediaListProperties.engineArg() == "audio") {
            mediaList.append(mediaItem);
        } else if (mediaItem.type == "Video" && m_mediaListProperties.engineArg() == "video") {
            mediaList.append(mediaItem);
        }
    }
    file.remove();
    
    m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
    m_mediaListProperties.type = QString("Sources");
    
    //Return results
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Exit event loop
    quit();
}
