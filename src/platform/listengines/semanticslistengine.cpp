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
    
    //Parse filter
    QString mediaType;
    QString groupByCategoryType;
    QString groupByField;
    QString limitFilter;
    if (engineFilterList.count() != 0) {
        mediaType = engineFilterList.at(0);
        if (engineFilterList.filter("groupBy=").count() != 0) {
            QString groupByFilter = engineFilterList.filter("groupBy=").at(0);
            groupByField = groupByFilter.remove("groupBy=").trimmed();
            if (groupByField == "artist") {
                groupByCategoryType = "Artist";
            } else if (groupByField == "album") {
                groupByCategoryType = "Album";
            } else if (groupByField == "genre") {
                if (mediaType == "audio") {
                    groupByCategoryType = "AudioGenre";
                } else if (mediaType == "video") {
                    groupByCategoryType = "VideoGenre";
                }
            } else if (groupByField == "seriesName") {
                groupByCategoryType = "TV Series";
            } else if (groupByField == "actor") {
                groupByCategoryType = "Actor";
            } else if (groupByField == "director") {
                groupByCategoryType = "Director";
            } else if (groupByField == "tag") {
                if (mediaType == "audio") {
                    groupByCategoryType = "AudioTag";
                } else if (mediaType == "video") {
                    groupByCategoryType = "VideoTag";
                }
            }
        }
        if (engineFilterList.filter("limit=").count() !=0) {
            limitFilter = engineFilterList.filter("limit=").at(0);
        }
    }
    
    if (m_nepomukInited) {
        if (engineArg.toLower() == "frequent") {
            mediaList.clear();
            if (mediaType == "audio" || mediaType == "video") {
                MediaQuery query;
                bool ignoreZeros = false;
                if (groupByCategoryType.isEmpty()) {
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
                    query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
                    if (m_mediaListProperties.filterForField("playCount").isEmpty()) {
                        query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Required, 0, MediaQuery::GreaterThan));
                        ignoreZeros = true;
                    }
                    query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Optional));
                    query.endWhere();
                    QStringList orderByBindings;
                    QList<MediaQuery::Order> order;
                    orderByBindings.append(mediaVocabulary.playCountBinding());
                    order.append(MediaQuery::Descending);
                    orderByBindings.append(mediaVocabulary.lastPlayedBinding());
                    order.append(MediaQuery::Descending);
                    query.orderBy(orderByBindings, order);
                } else {
                    QStringList bindings;
                    //NOTE:query.addLRIFilterConditions will automatically add 
                    //the groupBy field name to the binding list.
                    QString groupByResourceBinding = MediaVocabulary::resourceBindingForCategory(groupByCategoryType);
                    if (!groupByResourceBinding.isEmpty()) {
                        bindings.append(groupByResourceBinding);
                    }
                    bindings.append(query.fieldBindingDictionary[groupByField]);
                    bindings.append(MediaQuery::aggregateBinding(mediaVocabulary.playCountBinding(), MediaQuery::Sum));
                    query.select(bindings, MediaQuery::Distinct);
                    query.startWhere();
                    MediaQuery subQuery;
                    QStringList subBindings;
                    subBindings.append(mediaVocabulary.playCountBinding());
                    subBindings.append(mediaVocabulary.mediaResourceBinding());
                    subQuery.select(subBindings, MediaQuery::Distinct);
                    subQuery.startWhere();
                    if (mediaType == "audio") {
                        subQuery.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                    } else if (mediaType == "video") {
                        subQuery.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                    }
                    if (m_mediaListProperties.filterForField("playCount").isEmpty()) {
                        subQuery.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Required, 0, MediaQuery::GreaterThan));
                        ignoreZeros = true;
                    }
                    QStringList subQueryLRIFilterList = engineFilterList;
                    subQueryLRIFilterList.removeAll(limitFilter);
                    subQuery.addLRIFilterConditions(subQueryLRIFilterList, mediaVocabulary);
                    subQuery.endWhere();
                    query.addSubQuery(subQuery);
                    query.endWhere();
                    query.addLRIFilterCondition(limitFilter,  mediaVocabulary);
                    QStringList orderByBindings;
                    QList<MediaQuery::Order> order;
                    orderByBindings.append(QString("%1_sum").arg(mediaVocabulary.playCountBinding()));
                    order.append(MediaQuery::Descending);
                    query.orderBy(orderByBindings, order);
                }

                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    if (m_stop) {
                        return;
                    }
                    MediaItem mediaItem;
                    if (groupByCategoryType.isEmpty()) {
                        Nepomuk::Resource res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
                        mediaItem = Utilities::mediaItemFromNepomuk(res, m_mediaListProperties.lri);
                        mediaItem.semanticComment = i18np("played once", "played %1 times", mediaItem.fields["playCount"].toInt());
                    } else {
                        mediaItem = Utilities::categoryMediaItemFromIterator(it, groupByCategoryType, m_mediaListProperties.lri);
                        int playCount = it.binding(QString("%1_sum").arg(mediaVocabulary.playCountBinding())).literal().toInt();
                        mediaItem.semanticComment = i18np("played once", "played %1 times", playCount);
                        mediaItem.fields["playCount"] = playCount;
                    }
                    if (!mediaItem.url.startsWith("nepomuk:/")) {
                        if ((ignoreZeros && mediaItem.fields["playCount"].toInt() > 0) ||
                            !ignoreZeros) {
                            mediaList.append(mediaItem);
                        }
                    }
                }
                m_mediaListProperties.name = i18n("Frequently Played");
                m_mediaListProperties.type = QString("Sources");
            }
        }
        if (engineArg.toLower() == "recent") {
            mediaList.clear();
            if (!mediaType.isEmpty()) {
                MediaQuery query;
                QStringList bindings;
                if (groupByCategoryType.isEmpty()) {
                    bindings.append(mediaVocabulary.mediaResourceBinding());
                    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
                    bindings.append(mediaVocabulary.lastPlayedBinding());
                } else {
                    //NOTE:query.addLRIFilterConditions will automatically add 
                    //the groupBy field name to the binding list.
                    QString groupByResourceBinding = MediaVocabulary::resourceBindingForCategory(groupByCategoryType);
                    if (!groupByResourceBinding.isEmpty()) {
                        bindings.append(groupByResourceBinding);
                    }
                    bindings.append(MediaQuery::aggregateBinding(mediaVocabulary.lastPlayedBinding(), MediaQuery::Max));
                } 
                query.select(bindings, MediaQuery::Distinct);
                query.startWhere();
                if (mediaType == "audio") {
                    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                } else if (mediaType == "video") {
                    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                }
                query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
                query.addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Required));
                query.endWhere();
                QStringList orderByBindings;
                QList<MediaQuery::Order> order;
                if (groupByCategoryType.isEmpty()) {
                    orderByBindings.append(mediaVocabulary.lastPlayedBinding());
                } else {
                    orderByBindings.append(QString("%1_max").arg(mediaVocabulary.lastPlayedBinding()));
                }
                order.append(MediaQuery::Descending);
                query.orderBy(orderByBindings, order);
                
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    if (m_stop) {
                        return;
                    }
                    MediaItem mediaItem;
                    if (groupByCategoryType.isEmpty()) {
                        Nepomuk::Resource res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
                        mediaItem = Utilities::mediaItemFromNepomuk(res, m_mediaListProperties.lri);
                        mediaItem.fields["lastPlayed"] = it.binding(mediaVocabulary.lastPlayedBinding()).literal().toDateTime();
                    } else {
                        mediaItem = Utilities::categoryMediaItemFromIterator(it, groupByCategoryType, m_mediaListProperties.lri);
                        mediaItem.fields["lastPlayed"] = it.binding(QString("%1_max").arg(mediaVocabulary.lastPlayedBinding())).literal().toDateTime();
                    }
                    mediaItem.semanticComment = Utilities::wordsForTimeSince(mediaItem.fields["lastPlayed"].toDateTime());
                    if (!mediaItem.url.startsWith("nepomuk:/")) {
                        mediaList.append(mediaItem);
                    }
                }
                m_mediaListProperties.name = i18n("Recently Played");
                m_mediaListProperties.type = QString("Sources");
            }
        }
        if (engineArg.toLower() == "highest") {
            mediaList.clear();
            if (!mediaType.isEmpty()) {
                bool ignoreZeros = false;
                MediaQuery query;
                QStringList bindings;
                if (groupByCategoryType.isEmpty()) {
                    bindings.append(mediaVocabulary.mediaResourceBinding());
                    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
                    bindings.append(mediaVocabulary.ratingBinding());
                } else {
                    //NOTE:query.addLRIFilterConditions will automatically add 
                    //the groupBy field name to the binding list.
                    QString groupByResourceBinding = MediaVocabulary::resourceBindingForCategory(groupByCategoryType);
                    if (!groupByResourceBinding.isEmpty()) {
                        bindings.append(groupByResourceBinding);
                    }
                    bindings.append(MediaQuery::aggregateBinding(mediaVocabulary.ratingBinding(), MediaQuery::CountAverage));
                    bindings.append(MediaQuery::aggregateBinding(mediaVocabulary.ratingBinding(), MediaQuery::Average));
                } 
                query.select(bindings, MediaQuery::Distinct);
                query.startWhere();
                if (mediaType == "audio") {
                    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                } else if (mediaType == "video") {
                    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                }
                query.addLRIFilterConditions(engineFilterList, mediaVocabulary);
                if (m_mediaListProperties.filterForField("rating").isEmpty()) {
                    query.addCondition(mediaVocabulary.hasRating(MediaQuery::Required, 0, MediaQuery::GreaterThan));
                    ignoreZeros = true;
                }
                query.addCondition(mediaVocabulary.hasPlayCount(MediaQuery::Optional));
                query.endWhere();
                QStringList orderByBindings;
                QList<MediaQuery::Order> order;
                if (groupByCategoryType.isEmpty()) {
                    orderByBindings.append(mediaVocabulary.ratingBinding());
                    order.append(MediaQuery::Descending);
                    orderByBindings.append(mediaVocabulary.playCountBinding());
                    order.append(MediaQuery::Descending);
                } else {
                    orderByBindings.append(QString("%1_countavg").arg(mediaVocabulary.ratingBinding()));
                    order.append(MediaQuery::Descending);
                    order.append(MediaQuery::Descending);
                }
                query.orderBy(orderByBindings, order);
                
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    if (m_stop) {
                        return;
                    }
                    MediaItem mediaItem;
                    if (groupByCategoryType.isEmpty()) {
                        Nepomuk::Resource res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
                        mediaItem = Utilities::mediaItemFromNepomuk(res, m_mediaListProperties.lri);
                    } else {
                        mediaItem = Utilities::categoryMediaItemFromIterator(it, groupByCategoryType, m_mediaListProperties.lri);
                        int rating = it.binding(QString("%1_avg").arg(mediaVocabulary.ratingBinding())).literal().toInt();
                        mediaItem.fields["rating"] = rating;
                    }
                    if (!mediaItem.url.startsWith("nepomuk:/")) {
                        if ((ignoreZeros && mediaItem.fields["rating"].toInt() > 0) ||
                            !ignoreZeros) {
                            mediaList.append(mediaItem);
                        }
                    }
                }
                m_mediaListProperties.name = i18n("Highest Rated");
                m_mediaListProperties.type = QString("Sources");
            }
        }
    }
    
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        emit updateMediaItems(mediaItems);
    } else {
        //Get any remaining metadata for mediaItems
        if (mediaType == "video") {
            for (int i = 0; i < mediaList.count(); i++) {
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::completeMediaItem(mediaList.at(i));
                emit updateMediaItem(mediaItem);
            }
        }
        
    }
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}
