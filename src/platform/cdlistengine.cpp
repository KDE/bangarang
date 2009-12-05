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
#include "cdlistengine.h"
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
#include <KLocale>
#include <Phonon/MediaController>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/OpticalDisc>

CDListEngine::CDListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setCurrentSource(Phonon::Cd);
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    m_loadWhenReady = false;
}

CDListEngine::~CDListEngine()
{
    delete m_mediaObject;
}

void CDListEngine::run()
{
    //check if Audio CD is present
    bool audioCDFound = false;
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString()))
    {
        const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc> ();
        if (disc->availableContent() & Solid::OpticalDisc::Audio) {
            audioCDFound = true;
        }
    }
    
    QList<MediaItem> mediaList;
    if (audioCDFound) {
        if (!m_loadWhenReady) {
            m_mediaObject->setCurrentSource(Phonon::Cd);
            m_loadWhenReady = true;
        }
        if (m_mediaObject->state() == Phonon::StoppedState) {
            Phonon::MediaController *mediaController = new Phonon::MediaController(m_mediaObject);
            MediaItem mediaItem;
            int trackCount = mediaController->availableTitles();
            QString album = "Audio CD";
            QString title;
            QString artist;
            //int duration;
            for (int i = 1; i <= trackCount; i++) {
                title = i18n("Track %1", i);
                mediaItem.url = QString("CDTRACK%1").arg(i);
                mediaItem.artwork = KIcon("media-optical-audio");
                mediaItem.title = title;
                mediaItem.subTitle = i18n("Audio CD - %1 Tracks", trackCount);
                mediaItem.type = "Audio";
                mediaItem.fields["url"] = mediaItem.url;
                mediaItem.fields["title"] = mediaItem.title;
                mediaItem.fields["audioType"] = "CD Track";
                mediaItem.fields["trackNumber"] = i;
                mediaList << mediaItem;
            }

            m_mediaListProperties.summary = i18np("1 track", "%1 tracks", mediaList.count());
            model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
            m_requestSignature = QString();
            m_subRequestSignature = QString();
            m_loadWhenReady = false;
            delete mediaController;
        }
    } else {
        model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
        m_requestSignature = QString();
        m_subRequestSignature = QString();
        m_loadWhenReady = false;
    }
    //exec();    
}


void CDListEngine::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::LoadingState) && m_loadWhenReady) {
        start();
    }
    Q_UNUSED(newState);
}
