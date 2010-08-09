/* BANGARANG MEDIA PLAYER
 * Copyright (C) 2010 Ni Hui (shuizhuyuanluo@126.com)
 * <http://gitorious.org/bangarang>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusobjects.h"
#include "bangarangapplication.h"
#include "mainwindow.h"

#include "platform/playlist.h"
#include "platform/mediaitemmodel.h"
#include "platform/utilities.h"

#include <QDBusMetaType>
#include <KAboutData>
#include <KApplication>
#include <KUrl>

QDBusArgument& operator<<(QDBusArgument& argument, const MprisStatusStruct& statusStruct)
{
    argument.beginStructure();
    argument << statusStruct.state;
    argument << statusStruct.random;
    argument << statusStruct.repeatTrack;
    argument << statusStruct.repeatPlaylist;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, MprisStatusStruct& statusStruct)
{
    argument.beginStructure();
    argument >> statusStruct.state;
    argument >> statusStruct.random;
    argument >> statusStruct.repeatTrack;
    argument >> statusStruct.repeatPlaylist;
    argument.endStructure();
    return argument;
}

QDBusArgument& operator<<(QDBusArgument& argument, const MprisVersionStruct& versionStruct)
{
    argument.beginStructure();
    argument << versionStruct.major << versionStruct.minor;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, MprisVersionStruct& versionStruct)
{
    argument.beginStructure();
    argument >> versionStruct.major >> versionStruct.minor;
    argument.endStructure();
    return argument;
}

MprisRootObject::MprisRootObject(QObject *parent) : QObject(parent)
{
    qDBusRegisterMetaType<MprisVersionStruct>();
}

MprisRootObject::~MprisRootObject()
{
}

QString MprisRootObject::Identity()
{
    const KAboutData *aboutData = KGlobal::mainComponent().aboutData();
    return aboutData->programName() + ' ' + aboutData->version();
}

void MprisRootObject::Quit()
{
    kapp->quit();
}

MprisVersionStruct MprisRootObject::MprisVersion()
{
    MprisVersionStruct versionStruct;
    versionStruct.major = 1;
    versionStruct.minor = 0;
    return versionStruct;
}

MprisPlayerObject::MprisPlayerObject(BangarangApplication *app_)
: QObject(app_), app(app_)
{
    qDBusRegisterMetaType<MprisStatusStruct>();
    connect(app->playlist()->mediaObject(), SIGNAL(currentSourceChanged(const Phonon::MediaSource&)), this, SLOT(slotTrackChange()));
    connect(app->playlist(), SIGNAL(shuffleModeChanged(bool)), this, SLOT(slotStatusChange()));
    connect(app->playlist(), SIGNAL(repeatModeChanged(bool)), this, SLOT(slotStatusChange()));
    connect(app->playlist()->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(slotMediaStateChange(Phonon::State,Phonon::State)));
}

MprisPlayerObject::~MprisPlayerObject()
{
}

void MprisPlayerObject::Next()
{
    app->playlist()->playNext();
}

void MprisPlayerObject::Prev()
{
    app->playlist()->playPrevious();
}

void MprisPlayerObject::Pause()
{
    if (app->playlist()->mediaObject()->state() == Phonon::PlayingState) {
        app->playlist()->mediaObject()->pause();
    } else {
        /// unpause may start the playlist as well when no media is paused
        Play();
    }
}

void MprisPlayerObject::Stop()
{
    app->playlist()->stop();
}

void MprisPlayerObject::Play()
{
    if (app->playlist()->mediaObject()->state() == Phonon::PausedState) {
        app->playlist()->mediaObject()->play();
    } else if (app->playlist()->mediaObject()->state() != Phonon::PlayingState) {
        app->playlist()->start();
    }
}

void MprisPlayerObject::Repeat(bool repeat)
{
    app->playlist()->setRepeatMode(repeat);
}

MprisStatusStruct MprisPlayerObject::GetStatus()
{
    MprisStatusStruct statusStruct;
    
    if (app->playlist()->mediaObject()->state() == Phonon::PausedState) {
        statusStruct.state = 1;
    } else if (app->playlist()->mediaObject()->state() == Phonon::PlayingState) {
        statusStruct.state = 0;
    } else {
        statusStruct.state = 2;
    }
    
    if (app->playlist()->shuffleMode()) {
        statusStruct.random = 1;
    } else {
        statusStruct.random = 0;
    }
    
    statusStruct.repeatTrack = 0; // FIXME track repeat not implemented yet
    
    if (app->playlist()->repeatMode()) {
        statusStruct.repeatPlaylist = 1;
    } else {
        statusStruct.repeatPlaylist = 0;
    }
    
    return statusStruct;
}

QVariantMap MprisPlayerObject::GetMetadata()
{
    MediaItem item = app->playlist()->nowPlayingModel()->mediaItemAt(0);
    if ( item.type != "Audio" && item.type != "Video" )
        return QVariantMap();
    QVariantMap map;
    map["location"] = item.url;
    map["title"] = item.title;
    map["artist"] = item.fields.value("artist");
    map["album"] = item.fields.value("album");
    map["tracknumber"] = item.fields.value("trackNumber");
    map["time"] = item.fields.value("duration");
    map["genre"] = item.fields.value("genre");
//     map["rating"] = item.fields.value("rating");
    map["year"] = item.fields.value("year");
//     map["arturl"] = item.fields.value("artworkUrl");
    return map;// TODO: more metadata
}

int MprisPlayerObject::GetCaps()
{
    int capabilities =
    (1 << 0) | // CAN_GO_NEXT
    (1 << 1) | // CAN_GO_PREV
    (1 << 3) | // CAN_PLAY
    (1 << 4) | // CAN_SEEK
    (1 << 5) | // CAN_PROVIDE_METADATA
    (1 << 6); // CAN_HAS_TRACKLIST
    
    if (app->playlist()->mediaObject()->state() == Phonon::PlayingState) {
        capabilities |= (1 << 2); // CAN_PAUSE
    }
    
    return capabilities;
}

void MprisPlayerObject::VolumeSet(int volume)
{
    app->mainWindow()->audioOutput()->setVolume(qreal(volume)/100);
}

int MprisPlayerObject::VolumeGet()
{
    return static_cast<int>(app->mainWindow()->audioOutput()->volume()*100);
}

void MprisPlayerObject::PositionSet(int position)
{
    app->playlist()->mediaObject()->seek(position);
}

int MprisPlayerObject::PositionGet()
{
    return app->playlist()->mediaObject()->currentTime();
}

void MprisPlayerObject::slotTrackChange()
{
    emit TrackChange( GetMetadata() );
    emit StatusChange( GetStatus() ); // also emit status change when track changes
}

void MprisPlayerObject::slotStatusChange()
{
    emit StatusChange( GetStatus() );
}

void MprisPlayerObject::slotMediaStateChange(Phonon::State newstate, Phonon::State oldstate)
{
    if ( newstate == Phonon::PausedState || oldstate == Phonon::PausedState ) {
        emit StatusChange( GetStatus() );
        return;
    }
    if ( newstate == Phonon::PlayingState || oldstate == Phonon::PlayingState ) {
        emit CapsChange( GetCaps() );
        emit StatusChange( GetStatus() );
    }
}

MprisTrackListObject::MprisTrackListObject(BangarangApplication *app_)
: QObject(app_), app(app_)
{
    connect(app->playlist()->playlistModel(), SIGNAL(mediaListChanged()), this, SLOT(slotTrackListChange()));
}

MprisTrackListObject::~MprisTrackListObject()
{
}

QVariantMap MprisTrackListObject::GetMetadata(int index)
{
    if ( index < 0 || index >= GetLength() || GetLength() == 0 )
        return QVariantMap();
    MediaItem item = app->playlist()->nowPlayingModel()->mediaList().value(index);
    if ( item.type != "Audio" && item.type != "Video" )
        return QVariantMap();
    QVariantMap map;
    map["location"] = item.url;
    map["title"] = item.title;
    map["artist"] = item.fields.value("artist");
    map["album"] = item.fields.value("album");
    map["tracknumber"] = item.fields.value("trackNumber");
    map["time"] = item.fields.value("duration");
    map["genre"] = item.fields.value("genre");
//     map["rating"] = item.fields.value("rating");
    map["year"] = item.fields.value("year");
//     map["arturl"] = item.fields.value("artworkUrl");
    return map;// TODO: more metadata
}

int MprisTrackListObject::GetCurrentTrack()
{
    return app->playlist()->rowOfNowPlaying();
}

int MprisTrackListObject::GetLength()
{
    return app->playlist()->nowPlayingModel()->rowCount();
}

int MprisTrackListObject::AddTrack(const QString &url, bool playImmediately)
{
    MediaItem item = Utilities::mediaItemFromUrl( KUrl( url ) );
    if ( playImmediately ) {
        // insert the item at the first place
        QList<MediaItem> mediaList = app->browsingModel()->mediaList();
        mediaList.insert( 0, item );
        app->playlist()->playlistModel()->setMediaListProperties(app->browsingModel()->mediaListProperties());
        app->playlist()->playMediaList(mediaList);
        return 0;
    }
    app->playlist()->addMediaItem( item );
    return 0;
}

void MprisTrackListObject::DelTrack(int index)
{
    app->playlist()->removeMediaItemAt(index);
}

void MprisTrackListObject::SetLoop(bool loop)
{
    app->playlist()->setRepeatMode(loop);
}

void MprisTrackListObject::SetRandom(bool random)
{
    app->playlist()->setShuffleMode(random);
}

void MprisTrackListObject::slotTrackListChange()
{
    emit TrackListChange( GetLength() );
}
