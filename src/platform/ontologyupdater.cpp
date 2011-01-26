/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#include "ontologyupdater.h"
#include "mediavocabulary.h"
#include "mediaquery.h"
#include "utilities/utilities.h"
#include <KDebug>
#include <KLocale>
#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>

OntologyUpdater::OntologyUpdater(QObject * parent) : QObject(parent)
{
}

OntologyUpdater::~OntologyUpdater()
{
}

void OntologyUpdater::start()
{
    QApplication::processEvents();
    Soprano::Model *m_mainModel;
    bool m_nepomukInited = Utilities::nepomukInited();
    if (m_nepomukInited) {
        m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    } else {
        return;
    }
    
    m_stopUpdate = false;
    
    MediaVocabulary mediaVocabulary;
    
    //Update audio
    QString queryPrefix = QString("PREFIX xesam: <%1> "
                    "PREFIX rdf: <%2> "
                    "PREFIX xls: <%3> "
                    "PREFIX nmm: <http://www.semanticdesktop.org/ontologies/nmm#> "
                    "PREFIX nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> "
                    "PREFIX nfo: <http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#> ")
                    .arg(Soprano::Vocabulary::Xesam::xesamNamespace().toString())
                    .arg(Soprano::Vocabulary::RDF::rdfNamespace().toString())
                    .arg(Soprano::Vocabulary::XMLSchema::xsdNamespace().toString());
    QString queryStr = queryPrefix + QString("SELECT ?r "
                       "WHERE { {?r rdf:type <http://www.semanticdesktop.org/ontologies/nfo#Audio>} "
                       "UNION "
                       "{?r rdf:type <http://www.semanticdesktop.org/ontologies/nmm#MusicPiece>} "
                       "UNION "
                       "{?r rdf:type <http://www.semanticdesktop.org/ontologies/nmm#DigitalRadio>} "
                       "UNION "
                       "{?r rdf:type <http://www.semanticdesktop.org/ontologies/nmm#MusicAlbum>} "
                       "UNION "
                       "{?r rdf:type <%1>} "
                       "UNION "
                       "{?r rdf:type <%2>} "
                       "UNION "
                       "{?r rdf:type <%3>} }")
                       .arg(mediaVocabulary.typeAudio().toString())
                       .arg(mediaVocabulary.typeAudioMusic().toString())
                       .arg(mediaVocabulary.typeAudioStream().toString());
    
    Soprano::QueryResultIterator it = m_mainModel->executeQuery(queryStr, Soprano::Query::QueryLanguageSparql);
    emit infoMessage(i18n("<b>Updating audio types and properties</b><br>0 items updated..."));
    QApplication::processEvents();
    int i = 0;
    while( it.next() && !m_stopUpdate) {
        QApplication::processEvents();
        i++;
        Nepomuk::Resource resource = Nepomuk::Resource(it.binding("r").uri());
        //Update types
        QUrl type = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Audio");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeAudio())) {
                resource.addType(mediaVocabulary.typeAudio());
            }
        }
        type = QUrl("http://www.semanticdesktop.org/ontologies/nmm#MusicPiece");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeAudioMusic())) {
                resource.addType(mediaVocabulary.typeAudioMusic());
            }
            //Update properties
            QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#musicAlbum");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.musicAlbum(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#trackNumber");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.musicTrackNumber(), value);
            }
        }
        type = QUrl("http://www.semanticdesktop.org/ontologies/nmm#DigitalRadio");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeAudioStream())) {
                resource.addType(mediaVocabulary.typeAudioStream());
            }
        }
        type = QUrl("http://www.semanticdesktop.org/ontologies/nmm#MusicAlbum");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeMusicAlbum())) {
                resource.addType(mediaVocabulary.typeMusicAlbum());
            }
        }
        
        //Update common properties
        QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/nfo#duration");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.duration(), value);
        }
        property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#artwork");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.artwork(), value);
        }
        property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#genre");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.genre(), value);
        }
        property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#releaseDate");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.releaseDate(), value);
        }
        property = Soprano::Vocabulary::Xesam::useCount();
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.playCount(), value);
        }
        property = Soprano::Vocabulary::Xesam::lastUsed();
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.lastPlayed(), value);
        }
        emit infoMessage(i18n("<b>Updating audio types and properties</b><br>%1 audio items done...", i));
        QApplication::processEvents();
    }


    //Update video
    queryStr = queryPrefix + QString("SELECT ?r "
                       "WHERE { {?r rdf:type <http://www.semanticdesktop.org/ontologies/nfo#Video>} "
                       "UNION "
                       "{?r rdf:type <http://www.semanticdesktop.org/ontologies/nmm#Movie>} "
                       "UNION "
                       "{?r rdf:type <http://www.semanticdesktop.org/ontologies/nmm#TVShow>} "
                       "UNION "
                       "{?r rdf:type <http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#TVSeries>} "
                       "UNION "
                       "{?r rdf:type <%1>} "
                       "UNION "
                       "{?r rdf:type <%2>} "
                       "UNION "
                       "{?r rdf:type <%3>} }")
                       .arg(mediaVocabulary.typeVideo().toString())
                       .arg(mediaVocabulary.typeVideoMovie().toString())
                       .arg(mediaVocabulary.typeVideoTVShow().toString());

    
    it = m_mainModel->executeQuery(queryStr, Soprano::Query::QueryLanguageSparql);
    emit infoMessage(i18n("<b>Updating audio types and properties</b><br>0 items updated..."));
    QApplication::processEvents();
    i = 0;
    while( it.next() && !m_stopUpdate) {
        QApplication::processEvents();
        i++;
        Nepomuk::Resource resource = Nepomuk::Resource(it.binding("r").uri());
        //Update types
        QUrl type = QUrl("http://www.semanticdesktop.org/ontologies/nfo#Video");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeVideo())) {
                resource.addType(mediaVocabulary.typeVideo());
            }
        }
        type = QUrl("http://www.semanticdesktop.org/ontologies/nmm#Movie");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeVideoMovie())) {
                resource.addType(mediaVocabulary.typeVideoMovie());
            }
            //Update properties
            QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#synopsis");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoSynopsis(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#writer");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoWriter(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#actor");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoActor(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#director");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoDirector(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#producer");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoProducer(), value);
            }
        }
        type = QUrl("http://www.semanticdesktop.org/ontologies/nmm#TVShow");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeVideoTVShow())) {
                resource.addType(mediaVocabulary.typeVideoTVShow());
            }
            //Update properties
            QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#series");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoSeries(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#synopsis");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoSynopsis(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#season");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoSeason(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#episodeNumber");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoEpisodeNumber(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#writer");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoWriter(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#actor");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoActor(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#director");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoDirector(), value);
            }
            property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#producer");
            if (resource.hasProperty(property)) {
                Nepomuk::Variant value = resource.property(property);
                resource.removeProperty(property);
                resource.setProperty(mediaVocabulary.videoProducer(), value);
            }
        }
        type = QUrl("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#TVSeries");
        if (resource.hasType(type)) {
            removeType(resource, type);
            if (!resource.hasType(mediaVocabulary.typeTVSeries())) {
                resource.addType(mediaVocabulary.typeTVSeries());
            }
        }
        
        //Update common properties
        QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/nfo#duration");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.duration(), value);
        }
        property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#artwork");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.artwork(), value);
        }
        property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#genre");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.genre(), value);
        }
        property = QUrl("http://www.semanticdesktop.org/ontologies/nmm#releaseDate");
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.releaseDate(), value);
        }
        property = Soprano::Vocabulary::Xesam::useCount();
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.playCount(), value);
        }
        property = Soprano::Vocabulary::Xesam::lastUsed();
        if (resource.hasProperty(property)) {
            Nepomuk::Variant value = resource.property(property);
            resource.removeProperty(property);
            resource.setProperty(mediaVocabulary.lastPlayed(), value);
        }
        
        emit infoMessage(i18n("<b>Updating video types and properties</b><br>%1 video items done...", i));
        QApplication::processEvents();
    }


    //Fix screwed up properties
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.ratingBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasRating(MediaQuery::Required, 10, MediaQuery::GreaterThan));
    query.endWhere();
    it = m_mainModel->executeQuery(query.query(), Soprano::Query::QueryLanguageSparql);
    emit infoMessage(i18n("<b>Updating audio types and properties</b><br>0 items updated..."));
    QApplication::processEvents();
    i = 0;
    while( it.next() && !m_stopUpdate) {
        QApplication::processEvents();
        i++;
        Nepomuk::Resource resource = Nepomuk::Resource(it.binding("r").uri());
        QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/2007/08/15/nao#numericRating");
        if (resource.hasProperty(property)) {
            int rating = resource.property(property).toInt();
            if (rating > 10) {
                resource.removeProperty(property);
            }
        }
        emit infoMessage(i18n("<b>Cleaning up erroneous audio properties</b><br>%1 audio items done...", i));
        QApplication::processEvents();
    }

    MediaQuery query1;
    bindings.clear();
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.ratingBinding());
    query1.select(bindings, MediaQuery::Distinct);
    query1.startWhere();
    query1.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
    query1.addCondition(mediaVocabulary.hasRating(MediaQuery::Required, 10, MediaQuery::GreaterThan));
    query1.endWhere();
    it = m_mainModel->executeQuery(query.query(), Soprano::Query::QueryLanguageSparql);
    emit infoMessage(i18n("<b>Updating audio types and properties</b><br>0 items updated..."));
    QApplication::processEvents();
    i = 0;
    while( it.next() && !m_stopUpdate) {
        QApplication::processEvents();
        i++;
        Nepomuk::Resource resource = Nepomuk::Resource(it.binding("r").uri());
        QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/2007/08/15/nao#numericRating");
        if (resource.hasProperty(property)) {
            int rating = resource.property(property).toInt();
            if (rating > 10) {
                resource.removeProperty(property);
            }
        }
        emit infoMessage(i18n("<b>Cleaning up erroneous video properties</b><br>%1 video items done...", i));
        QApplication::processEvents();
    }

    if (!m_stopUpdate) {
        emit infoMessage(i18n("<b>Update complete.</b>"));
    } else {
        emit infoMessage(i18n("<b>Update stopped.</b>"));
    }
    emit done();
}

void OntologyUpdater::stopUpdate()
{
    m_stopUpdate = true;
}

void OntologyUpdater::removeType(Nepomuk::Resource res, QUrl mediaType)
{
    QList<QUrl> types = res.types();
    for (int i = 0; i < types.count(); i++) {
        if (types.at(i).toString() == mediaType.toString()) {
            types.removeAt(i);
            break;
        }
    }
    res.setTypes(types);
}
