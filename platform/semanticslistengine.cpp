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
#include "utilities.h"
#include <KIcon>
#include <KUrl>
#include <KDebug>
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
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QString mediaType;
    
    //Parse filter
    if (!engineFilter.isNull()) {
        QStringList argList = engineFilter.split("||");
        mediaType = argList.at(0);
    }
    
    if (m_nepomukInited) {
        if (engineArg.toLower() == "frequent") {
            SemanticsQuery query = SemanticsQuery(true);
            if (!mediaType.isEmpty()) {
                if (mediaType == "audio") {
                    query.selectAudioResource();
                } else if (mediaType == "video") {
                    query.selectVideoResource();
                }
                query.selectPlayCount();
                query.selectLastPlayed(true);
                query.orderBy("DESC(?playcount) DESC(?lastplayed)");
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

                //Build media list from results
                while( it.next() ) {
                    QUrl url = it.binding("url").uri().isEmpty() ? 
                                    it.binding("r").uri() :
                                    it.binding("url").uri();
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                    mediaItem.fields["description"] = mediaItem.fields["description"].toString() + QString(" - Played %1 times").arg(it.binding("playcount").literal().toInt());
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = "Frequently Played";
                m_mediaListProperties.type = QString("Sources");
            }
        }
        if (engineArg.toLower() == "recent") {
            SemanticsQuery query = SemanticsQuery(true);
            if (!mediaType.isEmpty()) {
                if (mediaType == "audio") {
                    query.selectAudioResource();
                } else if (mediaType == "video") {
                    query.selectVideoResource();
                }
                query.selectLastPlayed();
                query.orderBy("DESC(?lastplayed)");
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    QUrl url = it.binding("url").uri().isEmpty() ? 
                                    it.binding("r").uri() :
                                    it.binding("url").uri();
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                    QString lastPlayed = it.binding("lastplayed").literal().toDateTime().toString("ddd MMMM d yyyy h:mm:ss ap") ;
                    mediaItem.fields["description"] = mediaItem.fields["description"].toString() + QString(" - Last Played: %1").arg(lastPlayed);
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = "Recently Played";
                m_mediaListProperties.type = QString("Sources");
            }
        }
        if (engineArg.toLower() == "highest") {
            SemanticsQuery query = SemanticsQuery(true);
            if (!mediaType.isEmpty()) {
                if (mediaType == "audio") {
                    query.selectAudioResource();
                } else if (mediaType == "video") {
                    query.selectVideoResource();
                }
                query.selectRating();
                query.selectPlayCount(true);
                query.orderBy("DESC(?rating) DESC(?playcount) ");
                Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
                
                //Build media list from results
                while( it.next() ) {
                    QUrl url = it.binding("url").uri().isEmpty() ? 
                                    it.binding("r").uri() :
                                    it.binding("url").uri();
                    MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = "Highest Rated";
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

SemanticsQuery::SemanticsQuery(bool distinct) :
m_distinct(distinct),
m_selectAudioResource(false),
m_selectVideoResource(false),
m_selectRating(false),
m_selectPlayCount(false),
m_selectLastPlayed(false)
{
}

void SemanticsQuery::selectAudioResource() {
    m_selectAudioResource = true;
    //NOTE: nie:url is not in any released nie ontology that I can find.
    //      In future KDE will use nfo:fileUrl so this will need to be changed.
    m_audioResourceCondition = addOptional(false,
                                    QString(" { ?r rdf:type <%1> } "
                                            " UNION  "
                                            " { ?r rdf:type <%2> } "
                                            " UNION "
                                            " { ?r rdf:type <%3> } "
                                            " OPTIONAL { ?r nie:url ?url } . ")
                                    .arg(MediaVocabulary().typeAudio().toString())
                                    .arg(MediaVocabulary().typeAudioMusic().toString())
                                    .arg(MediaVocabulary().typeAudioStream().toString()));
}

void SemanticsQuery::selectVideoResource() {
    m_selectVideoResource = true;
    //NOTE: nie:url is not in any released nie ontology that I can find.
    //      In future KDE will use nfo:fileUrl so this will need to be changed.
    m_videoResourceCondition = QString("?r rdf:type <%1> . "
                                    " OPTIONAL { ?r <%2> %3 } "
                                    " OPTIONAL { ?r <%4> %5 } "
                                    " OPTIONAL { ?r nie:url ?url } . ")
                                    .arg(MediaVocabulary().typeVideo().toString())
                                    .arg(MediaVocabulary().videoIsMovie().toString())
                                    .arg(Soprano::Node::literalToN3(true))
                                    .arg(MediaVocabulary().videoIsTVShow().toString())
                                    .arg(Soprano::Node::literalToN3(true));
}

void SemanticsQuery::selectRating(bool optional) {
    m_selectRating = true;
    m_ratingCondition = addOptional(optional,
                                   QString("?r <%1> ?rating . ")
                                   .arg(Soprano::Vocabulary::NAO::numericRating().toString()));
}

void SemanticsQuery::selectPlayCount(bool optional) {
    m_selectPlayCount = true;
    m_playCountCondition = addOptional(optional,
                                    QString("?r <%1> ?playcount . ")
                                    .arg(MediaVocabulary().playCount().toString()));
}

void SemanticsQuery::selectLastPlayed(bool optional) {
    m_selectLastPlayed = true;
    m_lastPlayedCondition = addOptional(optional,
                                       QString("?r <%1> ?lastplayed . ")
                                       .arg(MediaVocabulary().lastPlayed().toString()));
}

void SemanticsQuery::searchString(QString str) {
    if (! str.isEmpty()) {
        //FIXME for rating search, etc.
        m_searchCondition = QString(
        "FILTER (regex(str(?artist),\"%1\",\"i\") || " 
        "regex(str(?album),\"%1\",\"i\") || "
        "regex(str(?title),\"%1\",\"i\")) ")
        .arg(str);
    }
}


void SemanticsQuery::orderBy(QString var) {
    if (!var.isEmpty()) {
        m_order = "ORDER BY " + var;
    }
}


QString SemanticsQuery::addOptional(bool optional, QString str) {
    if (optional) {
        return QString("OPTIONAL { ") + str + "} . ";
    } else {
        return str;
    }
}

QString SemanticsQuery::getPrefix() {
    return QString("PREFIX xesam: <%1> "
    "PREFIX rdf: <%2> "
    "PREFIX nmm: <%3> "
    "PREFIX xls: <%4> "
    "PREFIX nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> ")
    .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
    .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
    .arg("http://www.semanticdesktop.org/ontologies/nmm#")
    .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
}

Soprano::QueryResultIterator SemanticsQuery::executeSelect(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += "SELECT ";
    
    if (m_distinct)
        queryString += "DISTINCT ";
    if (m_selectAudioResource || m_selectVideoResource)
        queryString += "?r ?url ";
    if (m_selectRating)
        queryString += "?rating ";
    if (m_selectPlayCount)
        queryString += "?playcount ";
    if (m_selectLastPlayed)
        queryString += "?lastplayed ";
    
    queryString += QString("WHERE { ");
    
    queryString += m_audioResourceCondition;
    queryString += m_videoResourceCondition;
    queryString += m_ratingCondition;
    queryString += m_playCountCondition;
    queryString += m_lastPlayedCondition;
    
    queryString += "} ";
    
    queryString += m_order;
    queryString += " LIMIT 20 ";
    
    return model->executeQuery(queryString,
                               Soprano::Query::QueryLanguageSparql);
}

bool SemanticsQuery::executeAsk(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += QString("ASK { ");
    
    queryString += m_audioResourceCondition;
    queryString += m_videoResourceCondition;
    queryString += m_ratingCondition;
    queryString += m_playCountCondition;
    queryString += m_lastPlayedCondition;
    
    queryString += "} ";
    
    return model->executeQuery(queryString,
                               Soprano::Query::QueryLanguageSparql)
                               .boolValue();
}
