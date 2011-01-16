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

#include "../mediaitemmodel.h"
#include "cdlistengine.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <QApplication>
#include <KIcon>
#include <KFileDialog>
#include <KMimeType>
#include <KLocale>
#include <phonon/mediacontroller.h>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/Block>

CDListEngine::CDListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_mediaObject = new Phonon::MediaObject(this);
    m_mediaObject->setCurrentSource(Phonon::Cd);
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
    m_loadWhenReady = false;
    qRegisterMetaType<Phonon::MediaSource>("MediaSource");
}

CDListEngine::~CDListEngine()
{
    delete m_mediaObject;
}

void CDListEngine::run()
{
    QThread::setTerminationEnabled(true);
    m_stop = false;

    //check if Audio CD is present
    QString udi = m_mediaListProperties.engineArg();
    Solid::Device device = Solid::Device( udi );
    const Solid::Block* block = device.as<const Solid::Block>();
    
    QList<MediaItem> mediaList;
    if (block->isValid()) {
        QString dev_str = block->device();
        if (!m_loadWhenReady) {
            m_mediaObject->setCurrentSource(Phonon::MediaSource(Phonon::Cd, dev_str));
            m_loadWhenReady = true;
        }
        forever {
            if (m_stop) {
                return;
            }
            if (m_mediaObject->state() == Phonon::LoadingState) {
                msleep(100);
                continue;
            }
            Phonon::MediaController *mediaController = new Phonon::MediaController(m_mediaObject);
            int trackCount = mediaController->availableTitles();
            //int duration;
            for (int i = 1; i <= trackCount; i++) {
                if (m_stop) {
                    return;
                }
                KUrl url = Utilities::deviceUrl("cd", udi, QString(), "Audio", i);
                MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
                mediaItem.subTitle = i18nc("%1=Total number of tracks on the CD", "Audio CD - %1 Tracks", trackCount);
                mediaList << mediaItem;
            }

            m_mediaListProperties.summary = i18np("1 track", "%1 tracks", mediaList.count());
            delete mediaController;
            break;
        }
    }
    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    m_loadWhenReady = false;
    //exec();    
}


void CDListEngine::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::LoadingState) && m_loadWhenReady) {
        start();
    }
    Q_UNUSED(newState);
}
