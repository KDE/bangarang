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

#include "dbpediaquery.h"
#include <kdeversion.h>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <kio/copyjob.h>
#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QDomDocument>

DBPediaQuery::DBPediaQuery(QObject * parent) : QObject(parent)
{
    m_queryPrefix = QString("PREFIX owl: <http://www.w3.org/2002/07/owl#> "
                            "PREFIX xsd: <http://www.w3.org/2001/XMLSchema#> "
                            "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> "
                            "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
                            "PREFIX foaf: <http://xmlns.com/foaf/0.1/> "
                            "PREFIX dc: <http://purl.org/dc/elements/1.1/> "
                            "PREFIX : <http://dbpedia.org/resource/> "
                            "PREFIX dbpedia2: <http://dbpedia.org/property/> "
                            "PREFIX dbpedia: <http://dbpedia.org/> "
                            "PREFIX skos: <http://www.w3.org/2004/02/skos/core#> "
                            "PREFIX dbo: <http://dbpedia.org/ontology/> ");
                            
     m_artistInfoBindingsCount = 3;
     m_albumInfoBindingsCount = 3;
}

DBPediaQuery::~DBPediaQuery()
{
}

void DBPediaQuery::getArtistInfo(const QString & artistName)
{
    //Create query url
    QString query = m_queryPrefix + 
                    QString("SELECT DISTINCT ?artistName ?description ?thumbnail "
                    "WHERE { "
                    "{ ?person rdf:type dbo:Band . } "
                    "UNION "
                    "{?person rdf:type dbo:MusicalArtist . } "
                    "?person foaf:name ?artistName . "
                    "?person rdfs:comment ?description . "
                    "OPTIONAL {?person dbo:thumbnail ?thumbnail . } "
                    "FILTER (?artistName ='%1') . "
                    "} ")
                    .arg(artistName);
    QString dbPediaSPARQL = QString(QUrl::toPercentEncoding(query));
    QString dbPediaUrlString= QString("http://dbpedia.org/sparql/?format=application/xml&query=%1").arg(dbPediaSPARQL);
    KUrl dbPediaUrl = KUrl(dbPediaUrlString);
    
    //Add query url to request collection
    m_requests.insert(QString("Artist:%1").arg(artistName), dbPediaUrl);
    
    //Prepare download target location
    QString targetFileName = QString("bangarang/%1%2.tmp")
                                .arg(artistName.trimmed())
                                .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
    KUrl dbPediaDownloadUrl = KUrl(KStandardDirs::locateLocal("data", targetFileName, true));
    QFile downloadTarget(dbPediaDownloadUrl.path());
    downloadTarget.remove();
    
    //Launch query
    KIO::CopyJob *copyJob = KIO::copy(dbPediaUrl, dbPediaDownloadUrl, KIO::Overwrite | KIO::HideProgressInfo);
    connect (copyJob, 
             SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
             this,
             SLOT(resultsReturned(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
}

void DBPediaQuery::getAlbumInfo(const QString & albumName)
{
    Q_UNUSED(albumName)
}

void DBPediaQuery::resultsReturned(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);

    QList<Soprano::BindingSet> resultsBindingSets;
    QString requestKey = m_requests.key(from);
    QFile file(to.path());
    
    //Check to see if result file can be opened
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kDebug() << QString("Couldn't open dbpedia query result file:%1").arg(from.path());
        if (requestKey.startsWith("Artist")) {
            emit gotArtistInfo(false, resultsBindingSets, requestKey);
        } else if (requestKey.startsWith("Album")) {
            emit gotAlbumInfo(false, resultsBindingSets, requestKey);
        }        
        return;
    }
    
    //Results file is an XML document
    QDomDocument resultsDoc("queryResult");
    resultsDoc.setContent(&file);
    
    //Iterate through result nodes of the XML document
    QDomNodeList results = resultsDoc.elementsByTagName("result");
    for (int i = 0; i < results.count(); i++) {
        QDomNodeList resultBindings = results.at(i).childNodes();
        bool addResult = true;
        
        //Check bindings for explicit language attributes and only add
        //result if explicit language attribute matches system default;
        for (int j = 0; j < resultBindings.count(); j++) {
            QDomNode currentBinding = resultBindings.at(j);
            QDomElement currentBindingContent = currentBinding.firstChild().toElement();
            if (currentBindingContent.tagName() == "literal") {
                if (currentBindingContent.hasAttribute("xml:lang")) {
                    addResult = false;
                    QString language = currentBindingContent.attribute("xml:lang");
                    if (language == KLocale::defaultLanguage()) {
                        addResult = true;
                    } else if (language.startsWith(KLocale::defaultLanguage()) ||
                        KLocale::defaultLanguage().startsWith(language)) {
                        addResult = true;
                    }
                }
            }
        }
        
        //Add results
        if (addResult) {
            Soprano::BindingSet bindingSet = Soprano::BindingSet();
            for (int j = 0; j < resultBindings.count(); j++) {
                QDomElement currentBinding = resultBindings.at(j).toElement();
                QString bindingName = currentBinding.attribute("name");
                Soprano::Node value;
                QDomElement currentBindingContent = currentBinding.firstChild().toElement();
                if (currentBindingContent.tagName() == "uri") {
                    value = Soprano::Node(QUrl(currentBindingContent.text()));
                } else if (currentBindingContent.tagName() == "literal") {
                    if (currentBindingContent.attribute("datatype") == "http://www.w3.org/2001/XMLSchema#date") {
                        QDate dateValue = QDate::fromString(currentBindingContent.text(), "yyyy-MM-dd");
                        value = Soprano::Node(Soprano::LiteralValue(dateValue));
                    } else {
                        value = Soprano::Node(Soprano::LiteralValue(currentBindingContent.text()));
                    }
                }
                bindingSet.insert(bindingName, value);
            }
            resultsBindingSets.append(bindingSet);
        }
    }
    
    //Check type of request and emit appropriate results signal
    if (requestKey.startsWith("Artist")) {
        emit gotArtistInfo(true, resultsBindingSets, requestKey);
    } else if (requestKey.startsWith("Album")) {
        emit gotAlbumInfo(true, resultsBindingSets, requestKey);
    }

    //Remove results file
    file.remove();
}