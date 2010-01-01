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
#include <nepomuk/variant.h>
#include <QApplication>
#include <QTime>

AudioStreamListEngine::AudioStreamListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

AudioStreamListEngine::~AudioStreamListEngine()
{
}

MediaItem AudioStreamListEngine::createMediaItem(Soprano::QueryResultIterator& it) {
    MediaItem mediaItem;
    QUrl url = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().isEmpty() ? 
    it.binding(MediaVocabulary::mediaResourceBinding()).uri() :
    it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri();
    mediaItem.url = url.toString();
    mediaItem.title = it.binding(MediaVocabulary::titleBinding()).literal().toString();
    mediaItem.fields["title"] = it.binding(MediaVocabulary::titleBinding()).literal().toString();
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
    mediaItem.fields["genre"] = it.binding(MediaVocabulary::genreBinding()).literal().toString();
    mediaItem.fields["rating"] = it.binding(MediaVocabulary::ratingBinding()).literal().toInt();
    mediaItem.fields["description"] = it.binding(MediaVocabulary::descriptionBinding()).literal().toString();
    mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
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
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            //bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioStream(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
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
            MediaQuery query;
            QStringList bindings;
            bindings.append(mediaVocabulary.mediaResourceBinding());
            bindings.append(mediaVocabulary.mediaResourceUrlBinding());
            bindings.append(mediaVocabulary.titleBinding());
            bindings.append(mediaVocabulary.ratingBinding());
            bindings.append(mediaVocabulary.descriptionBinding());
            bindings.append(mediaVocabulary.artworkBinding());
            //bindings.append(mediaVocabulary.genreBinding());
            query.select(bindings, MediaQuery::Distinct);
            query.startWhere();
            query.addCondition(mediaVocabulary.hasTypeAudioStream(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Required));
            query.addCondition(mediaVocabulary.hasRating(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasDescription(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
            query.startFilter();
            query.addFilterConstraint(mediaVocabulary.titleBinding(), engineFilter, MediaQuery::Contains);
            query.addFilterOr();
            query.addFilterConstraint(mediaVocabulary.descriptionBinding(), engineFilter, MediaQuery::Contains);
            query.endFilter();
            query.endWhere();
            QStringList orderByBindings;
            orderByBindings.append(mediaVocabulary.titleBinding());
            query.orderBy(orderByBindings);
            
            Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);
            
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
