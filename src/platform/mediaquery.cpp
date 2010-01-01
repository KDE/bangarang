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
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>


MediaQuery::MediaQuery()
{
    m_queryPrefix = QString("PREFIX xesam: <%1> "
                    "PREFIX rdf: <%2> "
                    "PREFIX xls: <%3> "
                    "PREFIX nmm: <http://www.semanticdesktop.org/ontologies/nmm#> "
                    "PREFIX nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> "
                    "PREFIX nfo: <http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#> ")
                    .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
                    .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
                    .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
}

MediaQuery::~MediaQuery()
{
}

void MediaQuery::select(const QStringList &bindings, SelectType selectType)
{
    m_queryForm += "SELECT ";
    if (selectType == Distinct) {
        m_queryForm += "DISTINCT ";
    }
    for (int i = 0; i < bindings.count(); i++) {
        m_queryForm += QString("?%1 ").arg(bindings.at(i));
    }
}

void MediaQuery::startWhere()
{
    m_queryForm += "WHERE { ";
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
    m_querySuffix += "} ";
}

void MediaQuery::orderBy(const QStringList &bindings, QList<MediaQuery::Order> order)
{
    m_querySuffix += "ORDER BY ";
    for (int i = 0; i < bindings.count(); i++) {
        QString orderPrefix;
        if (order.count() == 0) {
            orderPrefix = "ASC";
        } else if (order.at(i) == Ascending) {
            orderPrefix = "ASC";
        } else if (order.at(i) == Descending) {
            orderPrefix = "DESC";
        }
        m_querySuffix += QString("%1( ?%2 ) ")
                         .arg(orderPrefix)
                         .arg(bindings.at(i));
    }
}

void MediaQuery::addLimit(int limit)
{
    if (limit > 0) {
        m_querySuffix += QString("LIMIT %1 ").arg(limit);
    }
}

void MediaQuery::addOffset(int offset)
{
    if (offset > 0) {
        m_querySuffix += QString("OFFSET %1 ").arg(offset);
    }
}


void MediaQuery::addExtra(const QString &extra)
{
    m_querySuffix += extra + QString(" ");
}

QString MediaQuery::query()
{
    return m_queryPrefix + m_queryForm + m_queryCondition + m_querySuffix;
}

Soprano::QueryResultIterator MediaQuery::executeSelect(Soprano::Model* model)
{
    QString query = m_queryPrefix + m_queryForm + m_queryCondition + m_querySuffix;
    return model->executeQuery(query, Soprano::Query::QueryLanguageSparql);
}

bool MediaQuery::executeAsk(Soprano::Model* model)
{
    QString query = m_queryPrefix + QString("ASK { %1 } ").arg(m_queryCondition);
    return model->executeQuery(query, Soprano::Query::QueryLanguageSparql).boolValue();
}
