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

#include "audiostreamlistengine.h"
#include "mediaitemmodel.h"
#include "listenginefactory.h"
#include "mediavocabulary.h"
#include <KIcon>
#include <KUrl>
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

AudioStreamListEngine::AudioStreamListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

AudioStreamListEngine::~AudioStreamListEngine()
{
}

MediaItem AudioStreamListEngine::createMediaItem(Soprano::QueryResultIterator& it) {
    MediaItem mediaItem;
    QUrl url = it.binding("url").uri().isEmpty() ? 
                    it.binding("r").uri() :
                    it.binding("url").uri();
    mediaItem.url = url.toString();
    mediaItem.title = it.binding("title").literal().toString();
    mediaItem.fields["title"] = it.binding("title").literal().toString();
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
        }
    }
    
    mediaItem.type = "Audio";
    mediaItem.nowPlaying = false;
    mediaItem.artwork = KIcon("x-media-podcast");
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["genre"] = it.binding("genre").literal().toString();
    mediaItem.fields["rating"] = it.binding("rating").literal().toInt();
    mediaItem.fields["description"] = it.binding("description").literal().toString();
    mediaItem.fields["artworkUrl"] = it.binding("artwork").uri().toString();
    mediaItem.fields["audioType"] = "Audio Stream";

    return mediaItem;
}

void AudioStreamListEngine::run()
{
    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    
    if (m_nepomukInited) {
        if (engineArg.isEmpty()) {
            AudioStreamQuery audioStreamQuery = AudioStreamQuery(true);
            audioStreamQuery.selectResource();
            audioStreamQuery.selectTitle();
            audioStreamQuery.selectRating(true);
            audioStreamQuery.selectDescription(true);
            audioStreamQuery.selectArtwork(true);
            //audioStreamQuery.selectGenre(true);
            audioStreamQuery.orderBy("?title");
            
            //Execute Query
            Soprano::QueryResultIterator it = audioStreamQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 stream", "%1 streams", mediaList.count());
            
            MediaItem mediaItem;
            mediaItem.type = "Action";
            mediaItem.url = "audiostreams://";
            mediaItem.title = i18n("Create new audio stream item");
            mediaItem.artwork = KIcon("document-new");
            mediaList.append(mediaItem);
            
            m_mediaListProperties.type = QString("Sources");
            
        } else if (engineArg.toLower() == "search") {
            AudioStreamQuery audioStreamQuery = AudioStreamQuery(true);
            audioStreamQuery.selectResource();
            audioStreamQuery.selectTitle();
            audioStreamQuery.selectRating(true);
            audioStreamQuery.selectDescription(true);
            audioStreamQuery.selectArtwork(true);
            //audioStreamQuery.selectGenre(true);
            audioStreamQuery.searchString(engineFilter);
            audioStreamQuery.orderBy("?title");
            
            //Execute Query
            Soprano::QueryResultIterator it = audioStreamQuery.executeSelect(m_mainModel);
            
            //Build media list from results
            while( it.next() ) {
                MediaItem mediaItem = createMediaItem(it);
                mediaList.append(mediaItem);
            }
            
            m_mediaListProperties.summary = i18np("1 stream", "%1 streams", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        }
    }
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}

void AudioStreamListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return songs
    m_mediaListProperties.lri = QString("audiostreams://?%1").arg(engineFilter);
}

void AudioStreamListEngine::activateAction()
{
    MediaItem mediaItem;
    mediaItem.type = "Audio";
    mediaItem.url = QString();
    mediaItem.title = i18n("Untitled Audio Stream");
    mediaItem.subTitle = i18n("Select this item, click Info then Edit to enter audio stream info");
    mediaItem.artwork = KIcon("x-media-podcast");
    mediaItem.fields["title"] = "Untitled";
    mediaItem.fields["audioType"] = "Audio Stream";
    mediaItem.fields["isTemplate"] = true;
    
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    
    m_mediaListProperties.name = i18n("New Audio Stream");
    
    model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
}




AudioStreamQuery::AudioStreamQuery(bool distinct) :
m_distinct(distinct),
m_selectResource(false),
m_selectTitle(false),
m_selectGenre(false)
{
}

void AudioStreamQuery::selectResource() {
    m_selectResource = true;
}

void AudioStreamQuery::selectTitle(bool optional) {
    m_selectTitle = true;
    m_titleCondition = addOptional(optional,
                                   QString("?r <%1> ?title . ")
                                   .arg(MediaVocabulary().title().toString()));
}

void AudioStreamQuery::selectGenre(bool optional) {
    m_selectGenre = true;
    m_genreCondition = addOptional(optional,
                                         QString("?r <%1> ?genre . ")
                                         .arg(MediaVocabulary().musicGenre().toString()));
}

void AudioStreamQuery::selectRating(bool optional) {
    m_selectRating = true;
    m_ratingCondition = addOptional(optional,
                                   QString("?r <%1> ?rating . ")
                                   .arg(Soprano::Vocabulary::NAO::numericRating().toString()));
}

void AudioStreamQuery::selectDescription(bool optional) {
    m_selectDescription = true;
    m_descriptionCondition = addOptional(optional,
                                    QString("?r <%1> ?description . ")
                                    .arg(MediaVocabulary().description().toString()));
}

void AudioStreamQuery::selectArtwork(bool optional) {
    m_selectArtwork = true;
    m_artworkCondition = addOptional(optional,
                                         QString("?r <%1> ?artwork . ")
                                         .arg(MediaVocabulary().artwork().toString()));
}

void AudioStreamQuery::searchString(QString str) {
    if (! str.isEmpty()) {
        m_searchCondition = QString(
        "FILTER (regex(str(?artist),\"%1\",\"i\") || " 
        "regex(str(?album),\"%1\",\"i\") || "
        "regex(str(?title),\"%1\",\"i\")) ")
        .arg(str);
    }
}


void AudioStreamQuery::orderBy(QString var) {
    if (!var.isEmpty()) {
        m_order = "ORDER BY " + var;
    }
}


QString AudioStreamQuery::addOptional(bool optional, QString str) {
    if (optional) {
        return QString("OPTIONAL { ") + str + "} . ";
    } else {
        return str;
    }
}

QString AudioStreamQuery::getPrefix() {
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

Soprano::QueryResultIterator AudioStreamQuery::executeSelect(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += "SELECT ";
    
    if (m_distinct)
        queryString += "DISTINCT ";
    if (m_selectResource)
        queryString += "?r ?url ";
    if (m_selectTitle)
        queryString += "?title ";
    if (m_selectGenre)
        queryString += "?genre ";
    if (m_selectRating)
        queryString += "?rating ";
    if (m_selectDescription)
        queryString += "?description ";
    if (m_selectArtwork)
        queryString += "?artwork ";
    
    //NOTE: nie:url is not in any released nie ontology that I can find.
    //      In future KDE will use nfo:fileUrl so this will need to be changed.
    queryString += QString("WHERE { ?r rdf:type <%1> . OPTIONAL { ?r nie:url ?url } . ")
    .arg(MediaVocabulary().typeAudioStream().toString());
    
    queryString += m_titleCondition;
    queryString += m_genreCondition;
    queryString += m_ratingCondition;
    queryString += m_searchCondition;
    queryString += m_descriptionCondition;
    queryString += m_artworkCondition;
    
    queryString += "} ";
    
    queryString += m_order;
    
    return model->executeQuery(queryString,
                               Soprano::Query::QueryLanguageSparql);
}

bool AudioStreamQuery::executeAsk(Soprano::Model* model) {
    QString queryString = getPrefix();
    queryString += QString("ASK { ?r rdf:type <%1> . ")
    .arg(MediaVocabulary().typeAudioStream().toString());
    
    queryString += m_titleCondition;
    queryString += m_genreCondition;
    queryString += m_ratingCondition;
    queryString += m_searchCondition;
    queryString += m_descriptionCondition;
    
    queryString += "} ";
    
    return model->executeQuery(queryString,
                               Soprano::Query::QueryLanguageSparql)
                               .boolValue();
}
