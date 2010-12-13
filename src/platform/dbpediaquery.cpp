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
#include <QFile>

DBPediaQuery::DBPediaQuery(QObject * parent) : QObject(parent)
{
    m_queryPrefix = QString("PREFIX owl: <http://www.w3.org/2002/07/owl#> "
                            "PREFIX xsd: <http://www.w3.org/2001/XMLSchema#> "
                            "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> "
                            "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
                            "PREFIX foaf: <http://xmlns.com/foaf/0.1/> "
                            "PREFIX dc: <http://purl.org/dc/elements/1.1/> "
                            "PREFIX dbr: <http://dbpedia.org/resource/> "
                            "PREFIX dbpedia2: <http://dbpedia.org/property/> "
                            "PREFIX dbpedia: <http://dbpedia.org/> "
                            "PREFIX skos: <http://www.w3.org/2004/02/skos/core#> "
                            "PREFIX dbo: <http://dbpedia.org/ontology/> ");
                            
     m_lang = KGlobal::locale()->language();
     if (m_lang.size() > 2) {
         m_lang = m_lang.left(2);
     }

}

DBPediaQuery::~DBPediaQuery()
{
}

void DBPediaQuery::getArtistInfo(const QString & artistName)
{
    //Create query url
    QString query = m_queryPrefix +
                    QString("SELECT DISTINCT str(?label) AS  ?name ?description ?thumbnail "
                    "WHERE { "
                    "{?person rdf:type dbo:Band . } "
                    "UNION "
                    "{?person rdf:type dbo:MusicalArtist . } "
                    "?person rdfs:label ?label . "
                    "?label bif:contains \"%1\" . "
                    "OPTIONAL {?person rdfs:comment ?description . "
                               "FILTER (lang(?description) ='%2') } "
                    "OPTIONAL {?person dbo:thumbnail ?thumbnail . } "
                    "} ")
                    .arg(artistName)
                    .arg(m_lang);
    
    //Create Request Key
    QString requestKey = QString("Artist:%1").arg(artistName);

    //Launch Query
    launchQuery(query, requestKey);
}

void DBPediaQuery::getAlbumInfo(const QString & albumName)
{
    Q_UNUSED(albumName)
}

void DBPediaQuery::getActorInfo(const QString & actorName)
{
    //Create query url
    QString query = m_queryPrefix +
                    QString("SELECT DISTINCT str(?label) AS  ?name ?description ?thumbnail "
                    "WHERE { "
                    "?person rdf:type dbo:Actor . "
                    "?person rdfs:label ?label . "
                    "?label bif:contains \"%1\" . "
                    "OPTIONAL {?person rdfs:comment ?description . "
                               "FILTER (lang(?description) ='%2') } "
                    "OPTIONAL {?person dbo:thumbnail ?thumbnail . } "
                    "} ")
                    .arg(actorName)
                    .arg(m_lang);
    
    //Create Request Key
    QString requestKey = QString("Actor:%1").arg(actorName);

    //Launch Query
    launchQuery(query, requestKey);
}

void DBPediaQuery::getDirectorInfo(const QString & directorName)
{
    //Create query url
    QString query = m_queryPrefix +
                    QString("SELECT DISTINCT str(?label) AS  ?name ?description ?thumbnail "
                    "WHERE { "
                    "?person rdf:type dbo:Director . "
                    "?person rdfs:label ?label . "
                    "?label bif:contains \"%1\" . "
                    "OPTIONAL {?person rdfs:comment ?description . "
                               "FILTER (lang(?description) ='%2') } "
                    "OPTIONAL {?person dbo:thumbnail ?thumbnail . } "
                    "} ")
                    .arg(directorName)
                    .arg(m_lang);
    
    //Create Request Key
    QString requestKey = QString("Director:%1").arg(directorName);

    //Launch Query
    launchQuery(query, requestKey);
}

void DBPediaQuery::getMovieInfo(const QString & movieName)
{
    //Create query url
    QString query = m_queryPrefix + 
                    QString("SELECT DISTINCT str(?label) AS  ?title ?description ?thumbnail ?duration ?releaseDate ?actor ?writer ?director ?producer "
                    "WHERE { "
                    "{ ?work rdf:type dbo:Film . } "
                    "?work rdfs:label ?label . "
                    "?label bif:contains \"%1\" . "
                    "?work rdfs:comment ?description . "
                    "OPTIONAL { ?work dbo:starring ?actorres . "
                    "?actorres foaf:name ?actor . } "
                    "OPTIONAL { ?work dbo:director ?directorres . "
                    "?directorres foaf:name ?director . } "
                    "OPTIONAL { ?work dbo:writer ?writerres . " 
                    "?writerres foaf:name ?writer . } "
                    "OPTIONAL { ?work dbo:producer ?producerres . "
                    "?producerres foaf:name ?producer . } "
                    "OPTIONAL { ?work dbo:duration ?duration . } "
                    "OPTIONAL {?work foaf:depiction ?thumbnail . } "
                    "OPTIONAL {?work dbo:releaseDate ?releaseDate . } "
                    "FILTER (lang(?description) ='%2') } ")
                    .arg(movieName)
                    .arg(m_lang);

    //Create Request Key
    QString requestKey = QString("Movie:%1").arg(movieName);

    //Launch Query
    launchQuery(query, requestKey);
}

void DBPediaQuery::launchQuery(const QString &query, const QString &requestKey)
{
    //Construct dbpedia url
    QString dbPediaSPARQL = QString(QUrl::toPercentEncoding(query));
    QString dbPediaUrlString= QString("http://dbpedia.org/sparql/?format=application/xml&query=%1").arg(dbPediaSPARQL);
    KUrl dbPediaUrl = KUrl(dbPediaUrlString);
    
    //Add query url to request collection
    m_requests.insert(requestKey, dbPediaUrl);
        
    //Prepare download target location
    QString targetFileName = QString("bangarang/%1.tmp")
                                .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
    KUrl dbPediaDownloadUrl = KUrl(KStandardDirs::locateLocal("data", targetFileName, true));
    QFile downloadTarget(dbPediaDownloadUrl.path());
    downloadTarget.remove();
    
    //Launch query
    KIO::CopyJob *copyJob = KIO::copy(dbPediaUrl, dbPediaDownloadUrl, KIO::Overwrite | KIO::HideProgressInfo);
    copyJob->setAutoDelete(true);
    connect (copyJob, 
             SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
             this,
             SLOT(resultsReturned(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
    copyJob->setUiDelegate(0);
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text) ||
        requestKey.isEmpty()) {
        kDebug() << QString("Couldn't open dbpedia query result file:%1").arg(from.path());
        if (requestKey.startsWith("Artist")) {
            emit gotArtistInfo(false, resultsBindingSets, requestKey);
        } else if (requestKey.startsWith("Album")) {
            emit gotAlbumInfo(false, resultsBindingSets, requestKey);
        } else if (requestKey.startsWith("Actor")) {
            emit gotActorInfo(false, resultsBindingSets, requestKey);
        } else if (requestKey.startsWith("Director")) {
            emit gotDirectorInfo(false, resultsBindingSets, requestKey);
        } else if (requestKey.startsWith("Movie")) {
            emit gotMovieInfo(false, resultsBindingSets, requestKey);
        }        
        return;
    }
    
    //Results file is an XML document
    QDomDocument resultsDoc("queryResult");
    resultsDoc.setContent(&file);
    kDebug() << "Got results for " << requestKey;

    //Iterate through result nodes of the XML document
    QDomNodeList results = resultsDoc.elementsByTagName("result");
    for (int i = 0; i < results.count(); i++) {
        QDomNodeList resultBindings = results.at(i).childNodes();
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
    m_requests.remove(requestKey);
    
    //Check type of request and emit appropriate results signal
    if (requestKey.startsWith("Artist")) {
        emit gotArtistInfo(true, resultsBindingSets, requestKey);
    } else if (requestKey.startsWith("Album")) {
        emit gotAlbumInfo(true, resultsBindingSets, requestKey);
    } else if (requestKey.startsWith("Actor")) {
        emit gotActorInfo(true, resultsBindingSets, requestKey);
    } else if (requestKey.startsWith("Director")) {
        emit gotDirectorInfo(true, resultsBindingSets, requestKey);
    } else if (requestKey.startsWith("Movie")) {
        emit gotMovieInfo(true, resultsBindingSets, requestKey);
    }

    //Remove results file
    file.remove();
}
