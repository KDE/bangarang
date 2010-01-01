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

#include "semanticslistengine.h"
#include "mediaitemmodel.h"
#include "listenginefactory.h"
#include "mediavocabulary.h"
#include "mediaquery.h"
#include "utilities.h"
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
#include <QApplication>
#include <QTime>
#include <taglib/fileref.h>

SemanticsListEngine::SemanticsListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

SemanticsListEngine::~SemanticsListEngine()
{
}

void SemanticsListEngine::run()
{
    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QString mediaType;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    //Parse filter
    if (!engineFilter.isNull()) {
        QStringList argList = engineFilter.split("||");
        mediaType = argList.at(0);
    }
    
    if (m_nepomukInited) {
        if (engineArg.toLower() == "frequent") {
            if (!mediaType.isEmpty()) {
                MediaQuery query;
                QStringList bindings;
                bindings.append(mediaVocabulary.mediaResourceBinding());
                bindings.append(mediaVocabulary.mediaResourceUrlBinding());
                bindings.append(mediaVocabulary.playCountBinding());
                query.select(bindings, MediaQuery::Distinct);
                query.startWhere();
                if (mediaType == "audio") {
                    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                } else if (mediaType == "video") {
                    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                }
                query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Required));
                query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
                query.endWhere();
                QStringList orderByBindings;
                QList<MediaQuery::Order> order;
                orderByBindings.append(mediaVocabulary.playCountBinding());
                order.append(MediaQuery::Descending);
                orderByBindings.append(mediaVocabulary.lastPlayedBinding());
                order.append(MediaQuery::Descending);
                query.orderBy(orderByBindings, order);
                query.addLimit(20);
                
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

                //Build media list from results
                while( it.next() ) {
                    QUrl url = it.binding(mediaVocabulary.mediaResourceUrlBinding())
                                   .uri().isEmpty() ? it.binding(mediaVocabulary.mediaResourceBinding()).uri() :
                                   it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri();
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                    int playCount = it.binding(mediaVocabulary.playCountBinding()).literal().toInt();
                    mediaItem.fields["description"] = mediaItem.fields["description"].toString() + QString(" - Played %1 times").arg(playCount);
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = i18n("Frequently Played");
                m_mediaListProperties.type = QString("Sources");
            }
        }
        if (engineArg.toLower() == "recent") {
            if (!mediaType.isEmpty()) {
                MediaQuery query;
                QStringList bindings;
                bindings.append(mediaVocabulary.mediaResourceBinding());
                bindings.append(mediaVocabulary.mediaResourceUrlBinding());
                bindings.append(mediaVocabulary.lastPlayedBinding());
                query.select(bindings, MediaQuery::Distinct);
                query.startWhere();
                if (mediaType == "audio") {
                    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                } else if (mediaType == "video") {
                    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                }
                query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
                query.endWhere();
                QStringList orderByBindings;
                QList<MediaQuery::Order> order;
                orderByBindings.append(mediaVocabulary.lastPlayedBinding());
                order.append(MediaQuery::Descending);
                query.orderBy(orderByBindings, order);
                query.addLimit(20);
                
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    QUrl url = it.binding(mediaVocabulary.mediaResourceUrlBinding())
                                    .uri().isEmpty() ? 
                                    it.binding(mediaVocabulary.mediaResourceBinding()).uri() :
                                    it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri();
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                    QString lastPlayed = it.binding(mediaVocabulary.lastPlayedBinding()).literal().toDateTime().toString("ddd MMMM d yyyy h:mm:ss ap") ;
                    mediaItem.fields["description"] = mediaItem.fields["description"].toString() + QString(" - Last Played: %1").arg(lastPlayed);
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = i18n("Recently Played");
                m_mediaListProperties.type = QString("Sources");
            }
        }
        if (engineArg.toLower() == "highest") {
            if (!mediaType.isEmpty()) {
                MediaQuery query;
                QStringList bindings;
                bindings.append(mediaVocabulary.mediaResourceBinding());
                bindings.append(mediaVocabulary.mediaResourceUrlBinding());
                bindings.append(mediaVocabulary.ratingBinding());
                query.select(bindings, MediaQuery::Distinct);
                query.startWhere();
                if (mediaType == "audio") {
                    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                } else if (mediaType == "video") {
                    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                }
                query.addCondition(mediaVocabulary.hasRating(MediaQuery::Required));
                query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
                query.endWhere();
                QStringList orderByBindings;
                QList<MediaQuery::Order> order;
                orderByBindings.append(mediaVocabulary.ratingBinding());
                order.append(MediaQuery::Descending);
                orderByBindings.append(mediaVocabulary.playCountBinding());
                order.append(MediaQuery::Descending);
                query.orderBy(orderByBindings, order);
                query.addLimit(20);
                
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    QUrl url = it.binding(mediaVocabulary.mediaResourceUrlBinding())
                                .uri().isEmpty() ? 
                                it.binding(mediaVocabulary.mediaResourceBinding()).uri() :
                                it.binding(mediaVocabulary.mediaResourceUrlBinding()).uri();
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = i18n("Highest Rated");
                m_mediaListProperties.type = QString("Sources");
            }
        }
    }
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        model()->updateMediaItems(mediaItems);
    }
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}
