/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Florian Weik (f.weik@web.de)
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

#include "taglistengine.h"
#include "../mediaitemmodel.h"
#include "listenginefactory.h"
#include "../mediavocabulary.h"
#include "../mediaquery.h"
#include "../utilities/utilities.h"
#include <KIcon>
#include <KUrl>
#include <KDebug>
#include <KLocale>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/variant.h>
#include <nepomuk/tag.h>
#include <QApplication>
#include <QTime>
#include <taglib/fileref.h>
#include <QDebug>

TagListEngine::TagListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

TagListEngine::~TagListEngine()
{
}

void TagListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QStringList engineFilterList = m_mediaListProperties.engineFilterList();
    QString mediaType;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    if (m_nepomukInited) { 
        if (engineArg == "audiotags" || engineArg == "videotags") {
            QString mediaType;
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.tagBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            if (engineArg == "audiotags") {
                query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                mediaType = "audio";
            } else if (engineArg == "videotags") {
                query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                mediaType = "video";
            }
            query.addCondition(mediaVocabulary.hasTag(MediaQuery::Required)); 
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.tagBinding());
            query.orderBy(orderByBindings);
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                QString tag = it.binding(mediaVocabulary.tagBinding()).literal().toString().trimmed();
                MediaItem mediaItem;
                mediaItem.url = QString("tag://%1?tag=%2").arg(mediaType).arg(tag);
                mediaItem.type = "Category";
                if (mediaType == "audio") {
                    mediaItem.fields["categoryType"] = "AudioTag";
                } else if (mediaType == "video") {
                    mediaItem.fields["categoryType"] = "VideoTag";
                }
                mediaItem.title = tag;
                mediaItem.fields["title"] = tag;
                mediaItem.fields["sourceLri"] = m_mediaListProperties.lri;
                mediaItem.artwork = KIcon("view-pim-notes");
                mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?%1||limit=4||tag=%2").arg(mediaType).arg(tag));
                mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?%1||limit=4||tag=%2").arg(mediaType).arg(tag));
                mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?%1||limit=4||tag=%2").arg(mediaType).arg(tag));
                mediaList.append(mediaItem);
            }
            mediaList = Utilities::sortMediaList(mediaList);
            m_mediaListProperties.name = i18n("Tags");
            m_mediaListProperties.summary = i18np("1 tag", "%1 tags", mediaList.count());
            m_mediaListProperties.type = QString("Categories");

        } else if (engineArg == "audio" || engineArg == "video") {
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            if (engineArg == "audio") {
                query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
            } else if (engineArg == "video") {
                query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
            } 
            query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
            query.endWhere();

            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

            //Build media list from results
            while( it.next() ) {
                if (m_stop) {
                    return;
                }
                Nepomuk::Resource res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
                MediaItem mediaItem = Utilities::mediaItemFromNepomuk(res, m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    mediaList.append(mediaItem);
                }
            }
            mediaList = Utilities::sortMediaList(mediaList);
            m_mediaListProperties.summary = i18np("1 item", "%1 items", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        }
    }
          
    
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        emit updateMediaItems(mediaItems);
    }
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}
