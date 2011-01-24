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
#include "listenginefactory.h"
#include "../mediaitemmodel.h"
#include "../mediavocabulary.h"
#include "../utilities/utilities.h"
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

void AudioStreamListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

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
                if (m_stop) {
                    return;
                }
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Audio Stream"), m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.summary = i18np("1 stream", "%1 streams", mediaList.count());
            
            MediaItem mediaItem;
            mediaItem.type = "Audio";
            mediaItem.url = QString();
            mediaItem.title = i18n("New Audio Stream");
            mediaItem.subTitle = i18n("Edit info to create new audio stream");
            mediaItem.artwork = KIcon("text-html");
            mediaItem.fields["title"] = i18n("Untitled");
            mediaItem.fields["audioType"] = "Audio Stream";
            mediaItem.fields["isTemplate"] = true;
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
                MediaItem mediaItem = Utilities::mediaItemFromIterator(it, QString("Audio Stream"), m_mediaListProperties.lri);
                if (!mediaItem.url.startsWith("nepomuk:/")) {
                    mediaList.append(mediaItem);
                }
            }
            
            m_mediaListProperties.summary = i18np("1 stream", "%1 streams", mediaList.count());
            m_mediaListProperties.type = QString("Sources");
        }
    }
    
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}

void AudioStreamListEngine::setFilterForSources(const QString& engineFilter)
{
    //Always return streams
    m_mediaListProperties.lri = QString("audiostreams://?%1").arg(engineFilter);
}

void AudioStreamListEngine::activateAction()
{
}
