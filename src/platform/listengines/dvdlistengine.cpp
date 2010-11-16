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
#include "dvdlistengine.h"
#include "listenginefactory.h"
#include "../mediaindexer.h"
#include "../utilities/utilities.h"
#include "../mediavocabulary.h"

#include <QApplication>
#include <KIcon>
#include <KFileDialog>
#include <KMimeType>
#include <KLocale>
#include <KDebug>
#include <phonon/mediacontroller.h>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/Block>

DVDListEngine::DVDListEngine(ListEngineFactory * parent) : ListEngine(parent)
{
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
    QThread::setTerminationEnabled(true);
    m_stop = false;

    QString udi = m_mediaListProperties.engineArg();
    Solid::Device device = Solid::Device( udi );
    const Solid::Block* block = device.as<const Solid::Block>();
    
    QList<MediaItem> mediaList;
    if (!block->isValid())
        return;
    QString dev_str = block->device();
    if (!m_loadWhenReady) {
        m_mediaObject->setCurrentSource(Phonon::MediaSource(Phonon::Dvd, dev_str));
        m_loadWhenReady = true;
    }
    if (m_mediaObject->state() != Phonon::StoppedState)
        return;
    QString discTitle = Utilities::deviceName(udi, m_mediaObject);
    Phonon::MediaController *mediaController = new Phonon::MediaController(m_mediaObject);
    int trackCount = mediaController->availableTitles();
    //int duration;
    for (int i = 1; i <= trackCount; i++) {
        if (m_stop) {
            return;
        }
        KUrl url = Utilities::deviceUrl("dvd", udi, discTitle, "Video", i);
        MediaItem mediaItem = Utilities::mediaItemFromUrl(url);
        if (discTitle.isEmpty())
            mediaItem.subTitle = i18nc("%1=Total number of tracks on the DVD", "DVD Video - %1 Titles", trackCount);
        mediaList << mediaItem;
    }
    delete mediaController;

    emit results(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
    m_requestSignature = QString();
    m_subRequestSignature = QString();
    m_loadWhenReady = false;
}

void DVDListEngine::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    if ((oldState == Phonon::LoadingState) && m_loadWhenReady) {
        start();
    }
    Q_UNUSED(newState);
}
