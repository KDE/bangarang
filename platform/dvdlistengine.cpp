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

#include "mediaitemmodel.h"
#include "dvdlistengine.h"
#include "listenginefactory.h"
#include "mediaindexer.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KIcon>
#include <KFileDialog>
#include <KMimeType>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Phonon/MediaController>

DVDListEngine::DVDListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;
    
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        //resource manager inited successfully
        m_mediaIndexer = new MediaIndexer(this);
    } else {
        //no resource manager
    };
    
    m_mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    
    //m_mediaListProperties.dataEngine = "files://";
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();

    m_cdObject = new KCompactDisc::KCompactDisc();
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setCurrentSource(Phonon::Dvd);
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    m_loadWhenReady = false;
}

DVDListEngine::~DVDListEngine()
{
    delete m_mediaObject;
}

void DVDListEngine::run()
{
    Phonon::MediaController *mediaController = new Phonon::MediaController(m_mediaObject);
    QList<MediaItem> mediaList;
    m_loadWhenReady = true;
    if (m_mediaObject->state() == Phonon::StoppedState) {
        MediaItem mediaItem;
        int trackCount = mediaController->availableTitles();
        QString album = "DVD Video";
        QString title;
        QString artist;
        //int duration;
        for (int i = 1; i <= trackCount; i++) {
            title = QString("Title %1").arg(i);
            mediaItem.url = QString("DVDTRACK%1").arg(i);
            mediaItem.artwork = KIcon("media-optical-dvd");
            mediaItem.title = title;
            mediaItem.subTitle = QString("DVD Video - %1 Titles").arg(trackCount);
            mediaItem.type = "Video";
            mediaItem.fields["url"] = mediaItem.url;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["videoType"] = "DVD Title";
            mediaItem.fields["trackNumber"] = i;
            mediaList << mediaItem;
        }

        /*mediaItem.url = "-";
        mediaItem.title = QString("Number of tracks: %1").arg(trackCount);
        mediaItem.type = "Audio";
        mediaList << mediaItem;*/
        
        model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
        m_requestSignature = QString();
        m_subRequestSignature = QString();
        m_loadWhenReady = false;
    }
    //exec();    
    delete mediaController;
}

void DVDListEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties DVDListEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void DVDListEngine::setFilterForSources(QString engineFilter)
{
    Q_UNUSED(engineFilter);
}

void DVDListEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void DVDListEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void DVDListEngine::activateAction()
{
        
}

void DVDListEngine::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::LoadingState) && m_loadWhenReady) {
        start();
    }
}
