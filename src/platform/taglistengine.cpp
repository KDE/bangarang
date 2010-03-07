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

#include "taglistengine.h"
#include "mediaitemmodel.h"
#include "listenginefactory.h"
#include "mediavocabulary.h"
#include "mediaquery.h"
#include "utilities.h"
#include <KIcon>
#include <KUrl>
#include <KDebug>
#include <KLocale>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/variant.h>
#include <nepomuk/tag.h>
#include <QApplication>
#include <QTime>
#include <taglib/fileref.h>

TagListEngine::TagListEngine(ListEngineFactory * parent) : NepomukListEngine(parent)
{
}

TagListEngine::~TagListEngine()
{
}

void TagListEngine::run()
{
    if (m_updateSourceInfo || m_removeSourceInfo) {
        NepomukListEngine::run();
        return;
    }
    
    //Create media list based on engine argument and filter
    QList<MediaItem> mediaList;
    
    QString engineArg = m_mediaListProperties.engineArg();
    QString engineFilter = m_mediaListProperties.engineFilter();
    QString mediaType;
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    //Parse filter
    if (!engineFilter.isNull()) {
        QStringList argList = engineFilter.split("||");
        mediaType = argList.at(0);
    }
    
    if (m_nepomukInited) { 
      if (engineArg.isEmpty()) { //without arguments, list tags 
	foreach(Nepomuk::Tag tag, Nepomuk::Tag::allTags()) {
	      MediaItem mediaItem;
	      mediaItem.title = tag.label();
	      mediaItem.type = "Category";
	      mediaItem.url = QString("tag://%1?%2").arg(mediaItem.title).arg(engineFilter);
	      mediaList.append(mediaItem);
	}
	    m_mediaListProperties.name = i18n("List of Tags");
	    m_mediaListProperties.type = QString("Sources");
      }
      else {
            if (!mediaType.isEmpty()) {
                MediaQuery query;
                QStringList bindings;
                bindings.append(mediaVocabulary.mediaResourceBinding());
                bindings.append(mediaVocabulary.mediaResourceUrlBinding());
                query.select(bindings, MediaQuery::Distinct);
                query.startWhere();
                if (mediaType == "audio") {
                    query.addCondition(mediaVocabulary.hasTypeAnyAudio(MediaQuery::Required));
                } else if (mediaType == "video") {
                    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
                } 
                query.addCondition(mediaVocabulary.hasTag(MediaQuery::Required,Soprano::Node::resourceToN3(Nepomuk::Tag(engineArg).resourceUri())));
		query.endWhere();
		Soprano::QueryResultIterator it = query.executeSelect(m_mainModel);

                //Build media list from results
                while( it.next() ) {
                    Nepomuk::Resource res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
                    MediaItem mediaItem = Utilities::mediaItemFromNepomuk(res);
                    mediaItem.fields["description"] = i18n("%1", mediaItem.fields["description"].toString());
                    mediaList.append(mediaItem);
                }
                m_mediaListProperties.name = i18n("Files with Tag '%1'",engineArg);
                m_mediaListProperties.type = QString("Sources");
	    }
      }
          
    }
    
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    
    //Check if MediaItems in mediaList exist
    QList<MediaItem> mediaItems = Utilities::mediaItemsDontExist(mediaList);
    if (mediaItems.count() > 0) {
        emit updateMediaItems(mediaItems);
    }
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();
}
