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
        void addExtra(const QString &extra);
        QString query();
        
        Soprano::QueryResultIterator executeSelect(Soprano::Model* model);
        bool executeAsk(Soprano::Model* model);
        
        static QString addOptional(const QString &str) {
            return QString("OPTIONAL { ") + str + "} . ";
        }

        static QString hasType(const QString &resourceBinding, const QUrl &type)
        {
            return QString("?%1 rdf:type <%2> . ")
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
                statement += QString(" (str(?%1) = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (str(?%1) != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::Contains) {
                statement += QString(" (regex(str(?%1), \"%2\", \"i\")) ")
                .arg(binding)
                .arg(test);
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (str(?%1) < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (str(?%1) > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (str(?%1) <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (str(?%1) >= %2) ")
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
                statement += QString(" (dT(?%1) = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (dT(?%1) != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (dT(?%1) < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (dT(?%1) > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (dT(?%1) <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (dT(?%1) >= %2) ")
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
                statement += QString(" (dT(?%1) = %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::NotEqual) {
                statement += QString(" (dT(?%1) != %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThan) {
                statement += QString(" (dT(?%1) < %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThan) {
                statement += QString(" (dT(?%1) > %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::LessThanOrEqual) {
                statement += QString(" (dT(?%1) <= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            } else if (constraint == MediaQuery::GreaterThanOrEqual) {
                statement += QString(" (dT(?%1) >= %2) ")
                .arg(binding)
                .arg(Soprano::Node::literalToN3(test));
            }
            return statement;
        }


    private:
        QString m_queryPrefix;
        QString m_queryForm;
        QString m_queryCondition;
        QString m_querySuffix;
};
#endif // MEDIAQUERY_H