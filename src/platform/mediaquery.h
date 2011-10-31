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

#ifndef MEDIAQUERY_H
#define MEDIAQUERY_H

#include <QtCore>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>

class MediaVocabulary;

class MediaQuery {
    
    public:
        MediaQuery();
        ~MediaQuery();
        
        enum SelectType{NonDistinct = 0, Distinct = 1};
        enum Constraint {Equal = 0, 
                         NotEqual = 1,
                         Contains = 2, 
                         GreaterThan = 3, 
                         LessThan = 4,
                         GreaterThanOrEqual = 5,
                         LessThanOrEqual = 6,
                         Bound = 7,
                         NotBound = 8};
        
        enum Match {Required = 0, Optional = 1};
        
        enum Order{Ascending = 0, Descending = 1};
        enum Aggregate{Sum = 0, Count = 1, Average = 2, Min = 3, Max = 4,
                        CountAverage = 5};
                        
        QHash<QString, QString> fieldBindingDictionary;
       
        void select(const QStringList &bindings, SelectType selectType = NonDistinct);
        void startWhere();
        void addCondition(const QString &condition);
        void startFilter();
        void startSubFilter();
        void addFilterOr();
        void addFilterAnd();
        void addFilterConstraint(const QString &binding, const QString &test,
                                 MediaQuery::Constraint constraint);
        void addFilterConstraint(const QString &binding, int test,
                               MediaQuery::Constraint constraint);
        void addFilterConstraint(const QString &binding, QDate test,
                               MediaQuery::Constraint constraint);
        void addFilterConstraint(const QString &binding, QDateTime test,
                               MediaQuery::Constraint constraint);
        void addFilterString(const QString & filterString);
        void endSubFilter();
        void endFilter();
        void endWhere();
        void orderBy(const QStringList &bindings, 
                     QList<Order> order = QList<Order>());
        void addLimit(int limit);
        void addOffset(int offset);
        void addSubQuery(MediaQuery subQuery);
        void addExtra(const QString &extra);
        QString query(bool excludePrefix = false);
        
        Soprano::QueryResultIterator executeSelect(Soprano::Model* model);
        bool executeAsk(Soprano::Model* model);
        void addLRIFilterCondition(const QString &lriFilter, MediaVocabulary mediaVocabulary);
        void addLRIFilterConditions(const QStringList &lriFilterList, MediaVocabulary mediaVocabulary);
        
        static QString aggregateBinding(QString binding, Aggregate agg) {
            if (agg == Sum) {
                return QString("SUM(?%1*1) as ?%1_sum ").arg(binding);
            } else if (agg == Count) {
                return QString("COUNT(?%1) as ?%1_count ").arg(binding);
            } else if (agg == Average) {
                return QString("AVG(?%1*1) as ?%1_avg ").arg(binding);
            } else if (agg == Min) {
                return QString("MIN(?%1) as ?%1_min ").arg(binding);
            } else if (agg == Max) {
                return QString("MAX(?%1) as ?%1_max ").arg(binding);
            } else if (agg == CountAverage) {
                return QString("(AVG(?%1*1)*COUNT(?%1)) as ?%1_countavg ").arg(binding);
            } else {
                return QString("dummy");
            }
        }
        
        
        static QString addOptional(const QString &str) {
            return QString("OPTIONAL { ") + str + "} . ";
        }

        static QString hasType(const QString &resourceBinding, const QUrl &type)
        {
            return QString("?%1 rdf:type <%2> . ")
            .arg(resourceBinding)
            .arg(type.toString());
        }

        static QString excludeType(const QString &resourceBinding, const QUrl &type)
        {
            return QString("OPTIONAL { ?%1 rdf:type <%2> . "
                           "?%1 rdf:type ?excludeType . } "
                           "FILTER ( !bound(?excludeType) ) ")
            .arg(resourceBinding)
            .arg(type.toString());
        }

        static QString hasProperty(const QString &resourceBinding, const QUrl &property, const QString &propertyBinding)
        {
            return QString("?%1 <%2> ?%3 . ")
            .arg(resourceBinding)
            .arg(property.toString())
            .arg(propertyBinding);
        }
                
        static QString filterConstraint(const QString &binding, const QString &test,
                                        MediaQuery::Constraint constraint)
        {
            QString statement;
            if (constraint == MediaQuery::Equal) {
                statement += QString(" (?%1 = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (?%1 != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::Contains) {
                statement += QString(" (regex(?%1, \"%2\", \"i\")) ")
                .arg(binding)
                .arg(test);
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (?%1 < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (?%1 > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (?%1 <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (?%1 >= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::Bound) {
                statement += QString(" bound(?%1) ")
                .arg(binding);
            } else if (constraint == MediaQuery::NotBound) {
                statement += QString(" !bound(?%1) ")
                .arg(binding);
            }
            return statement;
        }
        
        static QString filterConstraint(const QString &binding, int test,
                                        MediaQuery::Constraint constraint)
        {
            QString statement;
            if (constraint == MediaQuery::Equal) {
                statement += QString(" (?%1 = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (?%1 != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (?%1 < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (?%1 > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (?%1 <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (?%1 >= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            }
            return statement;
        }
        
        static QString filterConstraint(const QString &binding, QDate test,
                                        MediaQuery::Constraint constraint)
        {
            QString statement;
            if (constraint == MediaQuery::Equal) {
                statement += QString(" (?%1 = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (?%1 != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (?%1 < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (?%1 > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (?%1 <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (?%1 >= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            }
            return statement;
        }
        
        static QString filterConstraint(const QString &binding, QDateTime test,
                               MediaQuery::Constraint constraint)
        {
            QString statement;
            if (constraint == MediaQuery::Equal) {
                statement += QString(" (?%1 = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (?%1 != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (?%1 < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (?%1 > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (?%1 <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (?%1 >= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            }
            return statement;
        }
        static QString filterConstraint(const QString &binding, const QUrl &test,
                               MediaQuery::Constraint constraint)
        {
            QString statement;
            if (constraint == MediaQuery::Equal) {
                statement += QString(" (?%1 = <%2>) ")
                .arg(binding)
                .arg(test.toString());
            } else {
                statement += QString(" (?%1 != <%2>) ")
                .arg(binding)
                .arg(test.toString());
            }
            return statement;
        }


    private:
        QString m_queryPrefix;
        QString m_querySelect;
        QString m_queryWhere;
        QString m_queryCondition;
        QString m_queryLimit;
        QString m_queryOffset;
        QString m_queryOrder;
        QString m_querySuffix;
        QStringList m_filterOperators;
        QHash<QString, Constraint> m_filterOperatorConstraint;
        QHash<QString, QString> m_fieldBindingDictionary;
};
#endif // MEDIAQUERY_H
