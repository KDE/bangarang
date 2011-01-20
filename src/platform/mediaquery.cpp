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

#include "mediaquery.h"
#include "mediavocabulary.h"
#include <kdeversion.h>
#include <KDebug>
#include <nepomuk/tag.h>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>


MediaQuery::MediaQuery()
{
    m_queryPrefix = QString("PREFIX xesam: <%1> "
                    "PREFIX rdf: <%2> "
                    "PREFIX xls: <%3> "
                    "PREFIX rdfs: <%4> "
                    "PREFIX nmm: <http://www.semanticdesktop.org/ontologies/nmm#> "
                    "PREFIX nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> "
                    "PREFIX nfo: <http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#> ")
                    .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
                    .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
                    .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString())
                    .arg(Soprano::Vocabulary::RDFS::rdfsNamespace().toString());
                    
    m_filterOperators << "!=" << ">=" << "<=" << "=" << ">" << "<" << ".contains.";
    m_filterOperatorConstraint["!="] = NotEqual;
    m_filterOperatorConstraint[">="] = GreaterThanOrEqual;
    m_filterOperatorConstraint["<="] = LessThanOrEqual;
    m_filterOperatorConstraint["="] = Equal;
    m_filterOperatorConstraint[">"] = GreaterThan;
    m_filterOperatorConstraint["<"] = LessThan;
    m_filterOperatorConstraint[".contains."] = Contains;
    
    fieldBindingDictionary["artist"] = MediaVocabulary::musicArtistNameBinding();
    fieldBindingDictionary["album"] = MediaVocabulary::musicAlbumTitleBinding();
    fieldBindingDictionary["genre"] = MediaVocabulary::genreBinding();
    fieldBindingDictionary["tag"] = MediaVocabulary::tagBinding();
    fieldBindingDictionary["seriesName"] = MediaVocabulary::videoSeriesTitleBinding();
    fieldBindingDictionary["season"] = MediaVocabulary::videoSeasonBinding();
    fieldBindingDictionary["actor"] = MediaVocabulary::videoActorBinding();
    fieldBindingDictionary["director"] = MediaVocabulary::videoDirectorBinding();
    
    
}

MediaQuery::~MediaQuery()
{
}

void MediaQuery::select(const QStringList &bindings, SelectType selectType)
{
    m_querySelect += "SELECT ";
    if (selectType == Distinct) {
        m_querySelect += "DISTINCT ";
    }
    for (int i = 0; i < bindings.count(); i++) {
        if (bindings.at(i).contains("SUM(") ||
            bindings.at(i).contains("COUNT(") ||
            bindings.at(i).contains("AVG(") ||
            bindings.at(i).contains("MIN(") ||
            bindings.at(i).contains("MAX(") ) {
            m_querySelect += bindings.at(i);
        } else {
            m_querySelect += QString("?%1 ").arg(bindings.at(i));
        }
    }
}

void MediaQuery::startWhere()
{
    m_queryWhere += "WHERE { ";
}

void MediaQuery::addCondition(const QString &condition)
{
    m_queryCondition += condition;
}

void MediaQuery::startFilter()
{
    m_queryCondition += "FILTER ( ";
}

void MediaQuery::startSubFilter()
{
    m_queryCondition += "( ";
}

void MediaQuery::addFilterOr()
{
    m_queryCondition += "|| ";
}

void MediaQuery::addFilterAnd()
{
    m_queryCondition += "&& ";
}

void MediaQuery::addFilterConstraint(const QString &binding, const QString &test,
                                     MediaQuery::Constraint constraint)
{
    m_queryCondition += filterConstraint(binding, test, constraint);
}

void MediaQuery::addFilterConstraint(const QString &binding, int test,
                         MediaQuery::Constraint constraint)
{
    m_queryCondition += filterConstraint(binding, test, constraint);
}

void MediaQuery::addFilterConstraint(const QString &binding, QDate test,
                         MediaQuery::Constraint constraint)
{
    m_queryCondition += filterConstraint(binding, test, constraint);
}

void MediaQuery::addFilterConstraint(const QString &binding, QDateTime test,
                         MediaQuery::Constraint constraint)
{
    m_queryCondition += filterConstraint(binding, test, constraint);
}


void MediaQuery::addFilterString(const QString &filterString)
{
    m_queryCondition += filterString;
}

void MediaQuery::endSubFilter()
{
    m_queryCondition += ") ";
}

void MediaQuery::endFilter()
{
    m_queryCondition += ") ";
}

void MediaQuery::endWhere()
{
    m_queryCondition += "} ";
}

void MediaQuery::orderBy(const QStringList &bindings, QList<MediaQuery::Order> order)
{
    QString queryOrder = "ORDER BY ";
    for (int i = 0; i < bindings.count(); i++) {
        QString orderPrefix;
        if (order.count() == 0) {
            orderPrefix = "ASC";
        } else if (order.at(i) == Ascending) {
            orderPrefix = "ASC";
        } else if (order.at(i) == Descending) {
            orderPrefix = "DESC";
        }
        queryOrder += QString("%1( ?%2 ) ")
                         .arg(orderPrefix)
                         .arg(bindings.at(i));
    }
    m_queryOrder = queryOrder;
}

void MediaQuery::addLimit(int limit)
{
    if (limit > 0) {
        m_queryLimit = QString("LIMIT %1 ").arg(limit);
    }
}

void MediaQuery::addOffset(int offset)
{
    if (offset > 0) {
        m_queryOffset = QString("OFFSET %1 ").arg(offset);
    }
}

void MediaQuery::addSubQuery(MediaQuery subQuery)
{
    bool excludePrefix = true;
    m_queryCondition += QString("{ %1 } ").arg(subQuery.query(excludePrefix));
}

void MediaQuery::addExtra(const QString &extra)
{
    m_querySuffix += extra + QString(" ");
}

QString MediaQuery::query(bool excludePrefix)
{
    QString query;
    if (excludePrefix) {
        query = m_querySelect + m_queryWhere + m_queryCondition + m_queryOffset + m_queryOrder + m_queryLimit + m_querySuffix;
    } else {
        query = m_queryPrefix + m_querySelect + m_queryWhere + m_queryCondition + m_queryOrder + m_queryOffset + m_queryLimit + m_querySuffix;
    }
    return query;
}

Soprano::QueryResultIterator MediaQuery::executeSelect(Soprano::Model* model)
{
    return model->executeQuery(query(), Soprano::Query::QueryLanguageSparql);
}

bool MediaQuery::executeAsk(Soprano::Model* model)
{
    QString query = m_queryPrefix + QString("ASK { %1 } ").arg(m_queryCondition);
    return model->executeQuery(query, Soprano::Query::QueryLanguageSparql).boolValue();
}

void MediaQuery::addLRIFilterConditions(const QStringList &lriFilterList, MediaVocabulary mediaVocabulary)
{
    for (int i = 0; i < lriFilterList.count(); i++) {
        QString lriFilter = lriFilterList.at(i).trimmed();
        addLRIFilterCondition(lriFilter, mediaVocabulary);
    }
}

void MediaQuery::addLRIFilterCondition(const QString &lriFilter, MediaVocabulary mediaVocabulary)
{
    //Parse filter
    QString field;
    Constraint constraint = Equal;
    QString value;
    for (int j = 0; j < m_filterOperators.count(); j ++) {
        QString oper = m_filterOperators.at(j);
        if (lriFilter.indexOf(oper) != -1) {
            constraint = m_filterOperatorConstraint[oper];
            field = lriFilter.left(lriFilter.indexOf(oper)).trimmed();
            value = lriFilter.mid(lriFilter.indexOf(oper) + oper.length()).trimmed();
            break;
        }
    }
    
    //Special handling for groupBy field
    if (field == "groupBy") {
        m_querySelect += QString("?%1 ").arg(fieldBindingDictionary[value]);
        
        //Set the field to ensure SPARQL triple match is also added to the query condition
        field = value;
        value = QString();
    }
    
    
    //Add filter condition
    if (field == "type" && constraint == Equal) {
        if (value.toLower() == "audio") {
            addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
        } else if (value.toLower() == "video") {
            addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
        }
    } else if (field == "audioType" && constraint == Equal) {
        if (value.toLower() == "audio clip") {
            addCondition(mediaVocabulary.hasTypeAudio(MediaQuery::Required));
        } else if (value.toLower() == "music") {
            addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
        } else if (value.toLower() == "audio stream") {
            addCondition(mediaVocabulary.hasTypeAudioStream(MediaQuery::Required));
        }          
    } else if (field == "videoType") {
        if (value.toLower() == "video clip") {
            addCondition(mediaVocabulary.hasTypeVideo(MediaQuery::Required));
        } else if (value.toLower() == "movie") {
            addCondition(mediaVocabulary.hasTypeVideoMovie(MediaQuery::Required));
        } else if (value.toLower() == "tv show") {
            addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
        }          
    } else if (field == "title") {
        addCondition(mediaVocabulary.hasTitle(MediaQuery::Required,
                                                        value,
                                                        constraint));
    } else if (field == "tag") {
        addCondition(mediaVocabulary.hasTag(MediaQuery::Required,
                                            value,
                                            constraint));
    } else if (field == "desription") {
        addCondition(mediaVocabulary.hasDescription(MediaQuery::Required,
                                                        value,
                                                        constraint));
    } else if (field == "duration") {
        addCondition(mediaVocabulary.hasDuration(MediaQuery::Required,
                                                        value.toInt(),
                                                        constraint));
    } else if (field == "lastPlayed") {
        QDateTime valueDateTime = QDateTime::fromString(value, "yyyyMMddHHmmss");
        addCondition(mediaVocabulary.hasLastPlayed(MediaQuery::Required,
                                                        valueDateTime,
                                                        constraint));
    } else if (field == "playCount") {
        MediaQuery::Match match = MediaQuery::Required;
        addCondition(mediaVocabulary.hasPlayCount(match,
                                                  value.toInt(),
                                                  constraint));
    } else if (field == "created") {
        QDate valueDate = QDate::fromString(value, "yyyyMMdd");
        addCondition(mediaVocabulary.hasCreated(MediaQuery::Required,
                                                        valueDate,
                                                        constraint));
    } else if (field == "genre") {
        addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                              value,
                                              constraint));
    } else if (field == "releaseDate") {
        QDate valueDate = QDate::fromString(value, "yyyyMMdd");
        addCondition(mediaVocabulary.hasReleaseDate(MediaQuery::Required,
                                                    valueDate,
                                                    constraint));
    } else if (field == "rating") {
        MediaQuery::Match match = MediaQuery::Required;
        addCondition(mediaVocabulary.hasRating(match,
                                               value.toInt(),
                                               constraint));
    } else if (field == "artist") {
        addCondition(mediaVocabulary.hasMusicAnyArtistName(MediaQuery::Required,
                                                        value,
                                                        constraint));
    } else if (field == "composer") {
        addCondition(mediaVocabulary.hasMusicComposerName(MediaQuery::Required,
                                                        value,
                                                        constraint));
    } else if (field == "album") {
        addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required,
                                                       value,
                                                       constraint));
    } else if (field == "albumYear") {
        addCondition(mediaVocabulary.hasMusicAlbumYear(MediaQuery::Required,
                                                       value.toInt(),
                                                       constraint));
    } else if (field == "trackNumber") {
        addCondition(mediaVocabulary.hasMusicTrackNumber(MediaQuery::Required,
                                              value.toInt(),
                                              constraint));
    } else if (field == "seriesName") {
        if (value != "~") {
            addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required,
                                            value,
                                            constraint));
        }
    } else if (field == "synopsis") {
        addCondition(mediaVocabulary.hasVideoSynopsis(MediaQuery::Required,
                                            value,
                                            constraint));
    } else if (field == "season") {
        if (value.toInt() != -1) {
            addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Required,
                                                      value.toInt(),
                                                      constraint));
        }
    } else if (field == "episodeNumber") {
        addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Required,
                                                      value.toInt(),
                                                      constraint));
    } else if (field == "audienceRating") {
        addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "writer") {
        addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "director") {
        addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "assistantDirector") {
        addCondition(mediaVocabulary.hasVideoAssistantDirector(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "producer") {
        addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "actor") {
        addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "cinematographer") {
        addCondition(mediaVocabulary.hasVideoCinematographer(MediaQuery::Required,
                                                      value,
                                                      constraint));
    } else if (field == "limit") {
        addLimit(value.toInt());
    }
}
