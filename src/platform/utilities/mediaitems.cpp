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

#ifndef UTILITIES_MEDIAITEMS_CPP
#define UTILITIES_MEDIAITEMS_CPP

#include "mediaitems.h"
#include "filetags.h"
#include "typechecks.h"
#include "general.h"
#include "../mediaitemmodel.h"
#include "../mediavocabulary.h"
#include "../mediaquery.h"

#include <KUrl>
#include <KIcon>
#include <KLocale>
#include <KDebug>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <kio/netaccess.h>
#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/Model>
#include <Nepomuk/Resource>
#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>

#include <QFile>
#include <QTime>

#include <taglib/mpegfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/speexfile.h>
#include <taglib/flacfile.h>
#include <taglib/mpcfile.h>
#include <taglib/trueaudiofile.h>
#include <taglib/wavpackfile.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/attachedpictureframe.h>

MediaItem Utilities::getArtistCategoryItem(const QString &artist)
{
    MediaItem mediaItem;
    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.musicArtistNameBinding());
    bindings.append(mediaVocabulary.musicArtistDescriptionBinding());
    bindings.append(mediaVocabulary.musicArtistArtworkBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasMusicAnyArtistName(MediaQuery::Required, artist, MediaQuery::Equal));
    query.addCondition(mediaVocabulary.hasMusicAnyArtistDescription(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasMusicArtistArtwork(MediaQuery::Optional));
    query.endWhere();
    query.addLimit(1);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    while( it.next() ) {
        QString artist = it.binding(mediaVocabulary.musicArtistNameBinding()).literal().toString().trimmed();
        if (!artist.isEmpty()) {
            mediaItem.url = QString("music://albums?artist=%1").arg(artist);
            mediaItem.title = artist;
            mediaItem.type = QString("Category");
            mediaItem.fields["categoryType"] = QString("Artist");
            mediaItem.nowPlaying = false;
            mediaItem.artwork = KIcon("system-users");
            mediaItem.fields["title"] = artist;
            mediaItem.fields["description"] = it.binding(mediaVocabulary.musicArtistDescriptionBinding()).literal().toString().trimmed();
            mediaItem.fields["artworkUrl"] = it.binding(mediaVocabulary.musicArtistArtworkBinding()).uri().toString();
        }
    }
    return mediaItem;

}


MediaItem Utilities::mediaItemFromUrl(KUrl url, bool preferFileMetaData)
{
    MediaItem mediaItem;
    if(isDisc(url)) {
        bool dvd = isDvd(url);
        QString album = dvd ? "DVD Video" : "Audio CD";
        QString discTitle = deviceNameFromUrl(url);
        int track = deviceTitleFromUrl(url);
        QString title;
        if (track != invalidTitle())
            title = i18n(dvd ? "Title %1" : "Track %1", track);
        else
            title = i18n("Full Disc");
        mediaItem.url = url.url();
        mediaItem.artwork = dvd ? KIcon("media-optical-dvd") : KIcon("media-optical-audio");
        mediaItem.title = title;
        mediaItem.fields["url"] = mediaItem.url;
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.fields["discTitle"] = discTitle;
        if ( dvd )
            mediaItem.fields["videoType"] = "DVD Title";
        else
            mediaItem.fields["audioType"] = "CD Track";
        mediaItem.fields["album"] = album;
        mediaItem.type = dvd ? "Video" : "Audio";
        mediaItem.fields["trackNumber"] = track;
        mediaItem = makeSubtitle(mediaItem);
        return mediaItem;
    }

    if (url.prettyUrl().startsWith("filex:/")) {
        url = urlForFilex(url);
    }

    MediaVocabulary mediaVocabulary = MediaVocabulary();

    if (url.isLocalFile() && (Utilities::isM3u(url.url()) || Utilities::isPls(url.url()))) {
        mediaItem.artwork = KIcon("audio-x-scpls");
        mediaItem.url = QString("savedlists://%1").arg(url.url());
        mediaItem.title = url.fileName();
        mediaItem.fields["title"] = url.fileName();
        mediaItem.subTitle = i18n("Playlist");
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = "Basic+Artwork";
        return mediaItem;
    }

    mediaItem.url = url.prettyUrl();
    mediaItem.title = url.fileName();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["title"] = mediaItem.title;

    //Determine type of file - nepomuk is primary source
    bool foundInNepomuk = false;
    if (nepomukInited()) {
        //Try to find the corresponding resource in Nepomuk
        Nepomuk::Resource res = mediaResourceFromUrl(url);
        if (res.exists() && (res.hasType(mediaVocabulary.typeAudio()) ||
            res.hasType(mediaVocabulary.typeAudioMusic()) ||
            res.hasType(mediaVocabulary.typeAudioStream()) ||
            res.hasType(mediaVocabulary.typeVideo()) ||
            res.hasType(mediaVocabulary.typeVideoMovie()) ||
            res.hasType(mediaVocabulary.typeVideoTVShow())) ) {
            mediaItem = mediaItemFromNepomuk(res);
            foundInNepomuk = true;
        }

        //Get mediaItem from Nepomuk if file Metadata is not preferred OR
        // is of type for which we are unable to read metadata
        if (foundInNepomuk && (!preferFileMetaData ||
                               res.hasType(mediaVocabulary.typeVideo()) ||
                               res.hasType(mediaVocabulary.typeVideoMovie()) ||
                               res.hasType(mediaVocabulary.typeVideoTVShow()))) {
            mediaItem = mediaItemFromNepomuk(res);
        }
    }

    if (!foundInNepomuk || mediaItem.type.isEmpty()) {
        mediaItem.type == "Audio"; // default to Audio;
        if (isAudio(mediaItem.url)) {
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Audio Clip";
        }
        if (isMusic(mediaItem.url)) {
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Music";
        }
        if (isVideo(mediaItem.url)){
            mediaItem.type = "Video";
            mediaItem.fields["videoType"] = "Video Clip";
        }
        if (!url.isLocalFile()) {
            //Audio streams are mostly internet radio and so on
            //It's nicer for the user to see the server he's getting the stream from than anything
            //else as e.g. radios have their own website/servers
            mediaItem.title = url.host();
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Audio Stream";
            mediaItem.fields["sourceLri"] = "audiostreams://"; //Set sourceLri so that any MediaItemModel will know how to save info
        }
    }

    if (mediaItem.type == "Audio") {
        if (mediaItem.fields["audioType"] == "Audio Clip") {
            mediaItem.artwork = KIcon("audio-x-generic");
        } else if (mediaItem.fields["audioType"] == "Music") {
            mediaItem.artwork = KIcon("audio-mp4");
            if (!foundInNepomuk || preferFileMetaData) {
                mediaItem = Utilities::getAllInfoFromTag(mediaItem.url, mediaItem);
            }
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            mediaItem.artwork = KIcon("text-html");
            if (mediaItem.title.isEmpty()) {
                mediaItem.title = url.prettyUrl();
            }
        }
    } else if (mediaItem.type == "Video") {
        if (mediaItem.fields["videoType"] == "Video Clip") {
            mediaItem.artwork = KIcon("video-x-generic");
        }
    }

    //Lookup nepomuk metadata not stored with file
    if (nepomukInited() && preferFileMetaData) {
        Nepomuk::Resource res(KUrl(mediaItem.url));
        QUrl nieUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url");
        mediaItem.fields["rating"] = res.rating();
        QStringList tags;
        foreach (Nepomuk::Tag tag, res.tags()) {
            tags.append(tag.label());
        }
        mediaItem.fields["tags"] = tags;
        mediaItem.fields["playCount"] = res.property(mediaVocabulary.playCount()).toInt();
        if (res.property(mediaVocabulary.lastPlayed()).isValid()) {
            QDateTime lastPlayed = res.property(mediaVocabulary.lastPlayed()).toDateTime();
            if (lastPlayed.isValid()) {
                mediaItem.fields["lastPlayed"] = lastPlayed;
            }
        }
        mediaItem.fields["artworkUrl"] = res.property(mediaVocabulary.artwork()).toResource()
                                         .property(nieUrl).toString();
        mediaItem.fields["relatedTo"] = Utilities::getLinksForResource(res);

    }
    return mediaItem;
}

QStringList Utilities::mediaListUrls(const QList<MediaItem> &mediaList)
{
    QStringList urls;
    for (int i = 0; i < mediaList.count(); i++) {
        urls << mediaList.at(i).url;
    }
    return urls;
}

int Utilities::mediaListDuration(const QList<MediaItem> &mediaList)
{
    int duration = 0;
    for (int i = 0; i < mediaList.count(); i++) {
        duration += mediaList.at(i).fields["duration"].toInt();
    }
    return duration;
}

QString Utilities::mediaListDurationText(const QList<MediaItem> &mediaList)
{
    int duration = mediaListDuration(mediaList);
    if (duration == 0) {
        return QString();
    }
    int hours = duration/3600;
    int minutes = (duration - (hours*3600))/60;
    int seconds = duration - (hours*3600) - (minutes*60);
    QString min = minutes < 10 ? QString("0%1").arg(minutes): QString("%1").arg(minutes);
    QString sec = seconds < 10 ? QString("0%1").arg(seconds): QString("%1").arg(seconds);

    return QString("%1:%2:%3").arg(hours).arg(min).arg(sec);
}

QList<MediaItem> Utilities::mediaItemsDontExist(const QList<MediaItem> &mediaList)
{
    QList<MediaItem> items;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        bool dvdNotFound = false;
        QString url_string = mediaItem.url;
        if (isDisc(url_string)) {
            bool dvd = isDvd(url_string);
            continue;
            dvdNotFound = true;
            Q_UNUSED(dvd);
        }
        KUrl url = KUrl(url_string);
        if (dvdNotFound ||
            (url.isValid() && url.isLocalFile() && !QFile(url.path()).exists()) ||
            url_string.startsWith("trash:/")
           ) {
            mediaItem.exists = false;
            kDebug() << mediaItem.url << " missing";
            items << mediaItem;
        }
    }
    return items;
}

MediaItem Utilities::mediaItemFromNepomuk(Nepomuk::Resource res, const QString &sourceLri)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    QString type;
    //Check types beyond the current vocabulary to detect basic Audio type indexed by Strigi
    if (res.hasType(Soprano::Vocabulary::Xesam::Audio()) ||
        res.hasType(QUrl("http://www.semanticdesktop.org/ontologies/nfo#Audio"))) {
        type = "Audio Clip";
    }
    if (res.hasType(mediaVocabulary.typeAudioMusic())) {
        type = "Music";
    }
    if (res.hasType(mediaVocabulary.typeAudioStream())) {
        type = "Audio Stream";
    }
    //Check types beyond the current vocabulary to detect basic Video type indexed by Strigi
    if (res.hasType(mediaVocabulary.typeVideo()) ||
        res.hasType(QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Video")) ||
        res.hasType(Soprano::Vocabulary::Xesam::Video())) {
        type = "Video Clip";
    }
    if (res.hasType(mediaVocabulary.typeVideoMovie())) {
        type = "Movie";
    }
    if (res.hasType(mediaVocabulary.typeVideoTVShow())) {
        type = "TV Show";
    }

    QUrl nieUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url");
    KUrl url(res.property(nieUrl).toUrl());
    if (url.prettyUrl().startsWith("filex:/")) {
        url = urlForFilex(url);
    }

    //If nepomuk resource type is not recognized try recognition by mimetype
    if (type.isEmpty()) {
        if (isAudio(url.prettyUrl())) {
            type = "Audio Clip";
        }
        if (isMusic(url.prettyUrl())) {
            type = "Music";
        }
        if (isVideo(url.prettyUrl())){
            type = "Video Clip";
        }
    }

    MediaItem mediaItem;
    mediaItem.url = url.prettyUrl();
    mediaItem.exists = !url.prettyUrl().startsWith("filex:/"); //if url is still a filex:/ url mark not exists.
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["resourceUri"] = res.resourceUri().toString();
    mediaItem.fields["sourceLri"] = sourceLri;
    mediaItem.title = res.property(mediaVocabulary.title()).toString();
    mediaItem.fields["title"] = mediaItem.title;
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
            mediaItem.fields["title"] = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
            mediaItem.fields["title"] = mediaItem.url;
        }
    }
    mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
    int duration = res.property(mediaVocabulary.duration()).toInt();
    if (duration != 0) {
        mediaItem.duration = Utilities::durationString(duration);
        mediaItem.fields["duration"] = duration;
    }
    QStringList rawGenres =  res.property(mediaVocabulary.genre()).toStringList();
    mediaItem.fields["genre"] = genresFromRawTagGenres(rawGenres);
    mediaItem.fields["rating"] = res.rating();

    QStringList tags;
    foreach (Nepomuk::Tag tag, res.tags()) {
        tags.append(tag.label());
    }
    mediaItem.fields["tags"] = tags;
    mediaItem.fields["playCount"] = res.property(mediaVocabulary.playCount()).toInt();
    if (res.property(mediaVocabulary.lastPlayed()).isValid()) {
        QDateTime lastPlayed = res.property(mediaVocabulary.lastPlayed()).toDateTime();
        if (lastPlayed.isValid()) {
            mediaItem.fields["lastPlayed"] = lastPlayed;
        }
    }
    mediaItem.fields["artworkUrl"] = res.property(mediaVocabulary.artwork()).toResource()
                                     .property(nieUrl).toString();
    mediaItem.fields["relatedTo"] = Utilities::getLinksForResource(res);

    if (type == "Audio Clip" || type == "Audio Stream" || type == "Music") {
        mediaItem.type = "Audio";
        mediaItem.fields["audioType"] = type;
        mediaItem.artwork = KIcon("audio-x-wav");
        if (type == "Audio Stream") {
            mediaItem.artwork = KIcon("text-html");
        } else if (type == "Music") {
            mediaItem.artwork = KIcon("audio-mpeg");

            QStringList artists;
            QList<Nepomuk::Resource> artistResources = res.property(mediaVocabulary.musicArtist()).toResourceList();
            for (int i = 0; i < artistResources.count(); i++) {
                artists.append(artistResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            artistResources = res.property(mediaVocabulary.musicPerformer()).toResourceList();
            for (int i = 0; i < artistResources.count(); i++) {
                artists.append(artistResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            artists = cleanStringList(artists);
            mediaItem.fields["artist"] = artists;

            QStringList composers;
            QList<Nepomuk::Resource> composerResources = res.property(mediaVocabulary.musicComposer()).toResourceList();
            for (int i = 0; i < composerResources.count(); i++) {
                composers.append(composerResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            composers = cleanStringList(composers);
            mediaItem.fields["composer"] = composers;

            QString album = res.property(mediaVocabulary.musicAlbum()).toResource()
                            .property(mediaVocabulary.musicAlbumName()).toString();
            if (!album.isEmpty()) {
                mediaItem.fields["album"] = album;
            }
            if (res.property(mediaVocabulary.musicAlbumYear()).isValid()) {
                QDate yearDate = res.property(mediaVocabulary.musicAlbumYear()).toDate();
                if (yearDate.isValid()) {
                    mediaItem.fields["year"] = yearDate.year();
                } else {
                    mediaItem.fields["year"] = QVariant(QVariant::Int);
                }
            }

            int trackNumber = res.property(mediaVocabulary.musicTrackNumber()).toInt();
            if (trackNumber != 0) {
                mediaItem.fields["trackNumber"] = trackNumber;
            } else {
                mediaItem.fields["trackNumber"] = QVariant(QVariant::Int);
            }
        }
    } else if (type == "Video Clip" || type == "Movie" || type == "TV Show") {
        mediaItem.type = "Video";
        mediaItem.fields["videoType"] = type;
        mediaItem.artwork = KIcon("video-x-generic");
        if (type == "Movie" || type == "TV Show") {
            mediaItem.artwork = KIcon("tool-animator");
            if (res.property(mediaVocabulary.releaseDate()).isValid()) {
                QDate releaseDate = res.property(mediaVocabulary.releaseDate()).toDate();
                if (releaseDate.isValid()) {
                    mediaItem.fields["releaseDate"] = releaseDate;
                    mediaItem.fields["year"] = releaseDate.year();
                } else {
                    mediaItem.fields["releaseDate"] = QVariant(QVariant::Date);
                    mediaItem.fields["year"] = QVariant(QVariant::Int);
                }
            } else {
                mediaItem.fields["releaseDate"] = QVariant(QVariant::Date);
                mediaItem.fields["year"] = QVariant(QVariant::Int);
            }
            QStringList writers;
            QList<Nepomuk::Resource> writerResources = res.property(mediaVocabulary.videoWriter()).toResourceList();
            for (int i = 0; i < writerResources.count(); i++) {
                QString name = writerResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    writers.append(name);
                }
            }
            writers = cleanStringList(writers);
            mediaItem.fields["writer"] = writers;

            QStringList directors;
            QList<Nepomuk::Resource> directorResources = res.property(mediaVocabulary.videoDirector()).toResourceList();
            for (int i = 0; i < directorResources.count(); i++) {
                QString name = directorResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    directors.append(name);
                }
            }
            directors = cleanStringList(directors);
            mediaItem.fields["director"] = directors;

            QStringList producers;
            QList<Nepomuk::Resource> producerResources = res.property(mediaVocabulary.videoProducer()).toResourceList();
            for (int i = 0; i < producerResources.count(); i++) {
                QString name = producerResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    producers.append(name);
                }
            }
            producers = cleanStringList(producers);
            mediaItem.fields["producer"] = producers;

            QStringList actors;
            QList<Nepomuk::Resource> actorResources = res.property(mediaVocabulary.videoActor()).toResourceList();
            for (int i = 0; i < actorResources.count(); i++) {
                QString name = actorResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    actors.append(name);
                }
            }
            actors = cleanStringList(actors);
            mediaItem.fields["actor"] = actors;
            if (type == "TV Show") {
                mediaItem.artwork = KIcon("video-television");
                QString seriesName = res.property(mediaVocabulary.videoSeries()).toResource()
                                     .property(mediaVocabulary.videoSeriesTitle()).toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                }

                int season = res.property(mediaVocabulary.videoSeason()).toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                } else {
                    mediaItem.fields["season"] = QVariant(QVariant::Int);
                }

                int episodeNumber = res.property(mediaVocabulary.videoEpisodeNumber()).toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                } else {
                    mediaItem.fields["episodeNumber"] = QVariant(QVariant::Int);
                }
            }
        }
    }
    mediaItem = makeSubtitle(mediaItem);
    return mediaItem;
}

MediaItem Utilities::mediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &sourceLri)
{
    MediaItem mediaItem;
    MediaVocabulary mediaVocabulary;

    Nepomuk::Resource res(it.binding(MediaVocabulary::mediaResourceBinding()).uri());
    KUrl url = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().isEmpty() ?
    it.binding(MediaVocabulary::mediaResourceBinding()).uri() :
    it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri();
    if (url.prettyUrl().startsWith("filex:/")) {
        url = urlForFilex(url);
    }
    mediaItem.url = url.prettyUrl();
    mediaItem.exists = !url.prettyUrl().startsWith("filex:/"); //if url is still a filex:/ url mark not exists.
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["resourceUri"] = it.binding(MediaVocabulary::mediaResourceBinding()).uri().toString();
    mediaItem.fields["sourceLri"] = sourceLri;
    mediaItem.title = it.binding(MediaVocabulary::titleBinding()).literal().toString();
    mediaItem.fields["title"] = mediaItem.title;
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
            mediaItem.fields["title"] = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
            mediaItem.fields["title"] = mediaItem.url;
        }
    }
    mediaItem.fields["description"] = it.binding(MediaVocabulary::descriptionBinding()).literal().toString();
    int duration = it.binding(MediaVocabulary::durationBinding()).literal().toInt();
    if (duration != 0) {
        mediaItem.duration = Utilities::durationString(duration);
        mediaItem.fields["duration"] = duration;
    }
    QStringList rawGenres =  res.property(mediaVocabulary.genre()).toStringList();
    mediaItem.fields["genre"] = genresFromRawTagGenres(rawGenres);
    mediaItem.fields["rating"] = it.binding(MediaVocabulary::ratingBinding()).literal().toInt();
    QStringList tags;
    foreach (Nepomuk::Tag tag, res.tags()) {
        tags.append(tag.label());
    }
    mediaItem.fields["tags"] = tags;
    mediaItem.fields["playCount"] = it.binding(MediaVocabulary::playCountBinding()).literal().toInt();
    if (it.binding(MediaVocabulary::lastPlayedBinding()).isValid()) {
        QDateTime lastPlayed = it.binding(MediaVocabulary::lastPlayedBinding()).literal().toDateTime();
        if (lastPlayed.isValid()) {
            mediaItem.fields["lastPlayed"] = lastPlayed;
        }
    }
    mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
    //mediaItem.fields["relatedTo"] = Utilities::getLinksForResource(res);
    if (type == "Audio Clip" || type == "Audio Stream" || type == "Music") {
        mediaItem.type = "Audio";
        mediaItem.fields["audioType"] = type;
        mediaItem.artwork = KIcon("audio-x-wav");
        if (type == "Audio Stream") {
            mediaItem.artwork = KIcon("text-html");
        } else if (type == "Music") {
            mediaItem.artwork = KIcon("audio-mpeg");

            QStringList artists;
            QList<Nepomuk::Resource> artistResources = res.property(mediaVocabulary.musicArtist()).toResourceList();
            for (int i = 0; i < artistResources.count(); i++) {
                artists.append(artistResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            artistResources = res.property(mediaVocabulary.musicPerformer()).toResourceList();
            for (int i = 0; i < artistResources.count(); i++) {
                artists.append(artistResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            artists = cleanStringList(artists);
            mediaItem.fields["artist"] = artists;

            QStringList composers;
            QList<Nepomuk::Resource> composerResources = res.property(mediaVocabulary.musicComposer()).toResourceList();
            for (int i = 0; i < composerResources.count(); i++) {
                composers.append(composerResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            composers = cleanStringList(composers);
            mediaItem.fields["composer"] = composers;

            //TODO: For some reason virtuoso SPARQL corrupts string variable non-ascii characters when a filter is specified
            //WORKAROUND: Prefer album title using album resource for now
            QString album;
            Nepomuk::Resource albumRes(it.binding(MediaVocabulary::albumResourceBinding()).uri());
            if (res.exists()) {
                album = albumRes.property(mediaVocabulary.musicAlbumName()).toString();
            } else {
                album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            }
            if (!album.isEmpty()) {
                mediaItem.fields["album"] = album;
            }

            if (it.binding(MediaVocabulary::musicAlbumYearBinding()).isValid()) {
                QDate yearDate = it.binding(MediaVocabulary::musicAlbumYearBinding()).literal().toDate();
                if (yearDate.isValid()) {
                    mediaItem.fields["year"] = yearDate.year();
                } else {
                    mediaItem.fields["year"] = QVariant(QVariant::Int);
                }
            }

            int trackNumber = it.binding(MediaVocabulary::musicTrackNumberBinding()).literal().toInt();
            if (trackNumber != 0) {
                mediaItem.fields["trackNumber"] = trackNumber;
            } else {
                mediaItem.fields["trackNumber"] = QVariant(QVariant::Int);
            }
        }
    } else if (type == "Video Clip" || type == "Movie" || type == "TV Show") {
        mediaItem.type = "Video";
        mediaItem.fields["videoType"] = type;
        mediaItem.artwork = KIcon("video-x-generic");
        if (type == "Movie" || type == "TV Show") {
            mediaItem.artwork = KIcon("tool-animator");
            if (it.binding(MediaVocabulary::releaseDateBinding()).isValid()) {
                QDate releaseDate = it.binding(MediaVocabulary::releaseDateBinding()).literal().toDate();
                if (releaseDate.isValid()) {
                    mediaItem.fields["releaseDate"] = releaseDate;
                    mediaItem.fields["year"] = releaseDate.year();
                } else {
                    mediaItem.fields["releaseDate"] = QVariant(QVariant::Date);
                    mediaItem.fields["year"] = QVariant(QVariant::Int);
                }
            } else {
                mediaItem.fields["releaseDate"] = QVariant(QVariant::Date);
                mediaItem.fields["year"] = QVariant(QVariant::Int);
            }
            QStringList writers;
            QList<Nepomuk::Resource> writerResources = res.property(mediaVocabulary.videoWriter()).toResourceList();
            for (int i = 0; i < writerResources.count(); i++) {
                QString name = writerResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    writers.append(name);
                }
            }
            writers = cleanStringList(writers);
            mediaItem.fields["writer"] = writers;

            QStringList directors;
            QList<Nepomuk::Resource> directorResources = res.property(mediaVocabulary.videoDirector()).toResourceList();
            for (int i = 0; i < directorResources.count(); i++) {
                QString name = directorResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    directors.append(name);
                }
            }
            directors = cleanStringList(directors);
            mediaItem.fields["director"] = directors;

            QStringList producers;
            QList<Nepomuk::Resource> producerResources = res.property(mediaVocabulary.videoProducer()).toResourceList();
            for (int i = 0; i < producerResources.count(); i++) {
                QString name = producerResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    producers.append(name);
                }
            }
            producers = cleanStringList(producers);
            mediaItem.fields["producer"] = producers;

            QStringList actors;
            QList<Nepomuk::Resource> actorResources = res.property(mediaVocabulary.videoActor()).toResourceList();
            for (int i = 0; i < actorResources.count(); i++) {
                QString name = actorResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    actors.append(name);
                }
            }
            actors = cleanStringList(actors);
            mediaItem.fields["actor"] = actors;
            if (type == "TV Show") {
                mediaItem.artwork = KIcon("video-television");
                QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                }

                int season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                } else {
                    mediaItem.fields["season"] = QVariant(QVariant::Int);
                }

                int episodeNumber = it.binding(MediaVocabulary::videoEpisodeNumberBinding()).literal().toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                } else {
                    mediaItem.fields["episodeNumber"] = QVariant(QVariant::Int);
                }
            }
        }
    }
    mediaItem = makeSubtitle(mediaItem);
    return mediaItem;
}

MediaItem Utilities::categoryMediaItemFromNepomuk(Nepomuk::Resource &res, const QString &type, const QString &sourceLri)
{
    MediaVocabulary mediaVocabulary;
    MediaItem mediaItem;

    if (type == "Artist" ||
         type == "Album" ||
         type == "TV Series" ||
         type == "Actor" ||
         type == "Director") {
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = type;
        mediaItem.nowPlaying = false;
        mediaItem.fields["resourceUri"] = res.resourceUri().toString();
        mediaItem.fields["relatedTo"] = Utilities::getLinksForResource(res);

        if (type =="Artist") {
            QString artist = res.property(mediaVocabulary.musicArtistName()).toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString artworkUrl;
            if (res.hasProperty(mediaVocabulary.artwork())) {
                Nepomuk::Resource artworkResource = res.property(mediaVocabulary.artwork()).toResource();
                artworkUrl = artworkResource.property(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url")).toString();
            }
            mediaItem.url = QString("music://albums?%1")
                            .arg(artistFilter);
            mediaItem.title = artist;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("system-users");
            mediaItem.fields["artworkUrl"] = artworkUrl;
            mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
            //Provide context info for artist
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1").arg(artist));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1").arg(artist));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1").arg(artist));
        } else if (type == "Album") {
            QString album = res.property(mediaVocabulary.musicAlbumName()).toString();
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            //TODO: Get corresponding artist name
            mediaItem.url = QString("music://songs?%1")
                            .arg(albumFilter);
            mediaItem.title = album;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("media-optical-audio");
            //Provide context info for album
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||album=%1").arg(album));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||album=%1").arg(album));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||album=%1").arg(album));
        } else if (type == "TV Series") {
            QString seriesName = res.property(mediaVocabulary.videoSeriesTitle()).toString();
            QString seriesNameFilter = seriesName.isEmpty() ? QString(): QString("seriesName=%1").arg(seriesName);
            QString artworkUrl;
            if (res.hasProperty(mediaVocabulary.artwork())) {
                Nepomuk::Resource artworkResource = res.property(mediaVocabulary.artwork()).toResource();
                artworkUrl = artworkResource.property(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url")).toString();
            }
            mediaItem.url = QString("video://seasons?||%1")
                            .arg(seriesNameFilter);
            mediaItem.title = seriesName;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("video-television");
            mediaItem.fields["artworkUrl"] = artworkUrl;
            mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
            //Provide context info for TV series
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||seriesName=%1").arg(seriesName));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||seriesName=%1").arg(seriesName));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||seriesName=%1").arg(seriesName));
        } else if (type == "Actor") {
            QString actor = res.property(mediaVocabulary.ncoFullname()).toString();
            QString actorFilter = actor.isEmpty() ? QString(): QString("actor=%1").arg(actor);
            QString artworkUrl;
            if (res.hasProperty(mediaVocabulary.artwork())) {
                Nepomuk::Resource artworkResource = res.property(mediaVocabulary.artwork()).toResource();
                artworkUrl = artworkResource.property(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url")).toString();
            }
            mediaItem.url = QString("video://sources?||%1")
                            .arg(actorFilter);
            mediaItem.title = actor;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.fields["artworkUrl"] = artworkUrl;
            mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||actor=%1").arg(actor));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||actor=%1").arg(actor));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||actor=%1").arg(actor));
        } else if (type == "Director") {
            QString director = res.property(mediaVocabulary.ncoFullname()).toString();
            QString directorFilter = director.isEmpty() ? QString(): QString("director=%1").arg(director);
            QString artworkUrl;
            if (res.hasProperty(mediaVocabulary.artwork())) {
                Nepomuk::Resource artworkResource = res.property(mediaVocabulary.artwork()).toResource();
                artworkUrl = artworkResource.property(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url")).toString();
            }
            mediaItem.url = QString("video://sources?||%1")
                            .arg(directorFilter);
            mediaItem.title = director;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.fields["artworkUrl"] = artworkUrl;
            mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||director=%1").arg(director));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||director=%1").arg(director));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||director=%1").arg(director));
        }
        mediaItem.fields["sourceLri"] = sourceLri;
    }
    mediaItem = makeSubtitle(mediaItem);
    return mediaItem;
}

MediaItem Utilities::categoryMediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &lri, const QString &sourceLri)
{
    MediaItem mediaItem;

    if (type == "Artist" ||
         type == "Album" ||
         type == "AudioGenre" ||
         type == "AudioTag" ||
         type == "TV Series" ||
         type == "VideoGenre" ||
         type == "Actor" ||
         type == "Director"||
         type == "VideoTag") {
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = type;
        mediaItem.nowPlaying = false;
        if (!MediaVocabulary::resourceBindingForCategory(type).isEmpty()) {
            Nepomuk::Resource res(it.binding(MediaVocabulary::resourceBindingForCategory("Artist")).uri());
            if (res.exists()) {
                mediaItem.fields["relatedTo"] = Utilities::getLinksForResource(res);
            }
        }

        if (type =="Artist") {
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            mediaItem.url = QString("music://albums?%1||%2||%3")
                            .arg(artistFilter)
                            .arg(albumFilter)
                            .arg(genreFilter);
            mediaItem.title = artist;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("system-users");
            //Provide context info for artist
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
        } else if (type == "Album") {
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            mediaItem.url = QString("music://songs?%1||%2||%3")
                            .arg(artistFilter)
                            .arg(albumFilter)
                            .arg(genreFilter);
            mediaItem.title = album;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["artist"] = artist;
            mediaItem.artwork = KIcon("media-optical-audio");
            //Provide context info for album
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
        } else if (type == "AudioGenre") {
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            mediaItem.url = QString("music://artists?%1||%2||%3")
                            .arg(artistFilter)
                            .arg(albumFilter)
                            .arg(genreFilter);
            mediaItem.title = genre;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("flag-blue");
            //Provide context info for genre
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
        } else if (type == "AudioTag") {
            QString tag = it.binding(MediaVocabulary::tagBinding()).literal().toString();
            QString tagFilter = tag.isEmpty() ? QString(): QString("tag=%1").arg(tag);
            mediaItem.url = QString("tag://audio?%1")
                            .arg(tagFilter);
            mediaItem.title = tag;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-pim-notes");;
            //Provide context info for tag
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?audio||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?audio||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?audio||limit=4||tag=%1").arg(tag));
        } else if (type == "TV Series") {
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
            QString season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toString();
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            QString seriesNameFilter = seriesName.isEmpty() ? QString(): QString("seriesName=%1").arg(seriesName);
            QString seasonFilter = season.isEmpty() ? QString(): QString("season=%1").arg(season);
            mediaItem.url = QString("video://seasons?||%1||%2||%3")
                            .arg(genreFilter)
                            .arg(seriesNameFilter)
                            .arg(seasonFilter);
            mediaItem.title = seriesName;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("video-television");
            //Provide context info for TV series
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
        } else if (type == "TV Season") {
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
            QString season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toString();
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            QString seriesNameFilter = seriesName.isEmpty() ? QString(): QString("seriesName=%1").arg(seriesName);
            QString seasonFilter = season.isEmpty() ? QString(): QString("season=%1").arg(season);
            mediaItem.url = QString("video://seasons?||%1||%2||%3")
                            .arg(genreFilter)
                            .arg(seriesNameFilter)
                            .arg(seasonFilter);
            mediaItem.title = seriesName;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.fields["season"] = season;
            mediaItem.artwork = KIcon("video-television");
            //Provide context info for genre
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
        } else if (type == "VideoGenre") {
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            mediaItem.url = QString("video://sources?||genre=%1").arg(genre);
            mediaItem.title = genre;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("flag-green");
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||genre=%1").arg(genre));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||genre=%1").arg(genre));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||genre=%1").arg(genre));
        } else if (type == "Actor") {
            QString actor = it.binding(MediaVocabulary::videoActorBinding()).literal().toString();
            mediaItem.url = QString("video://sources?||actor=%1").arg(actor);
            mediaItem.title = actor;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||actor=%1").arg(actor));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||actor=%1").arg(actor));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||actor=%1").arg(actor));
        } else if (type == "Director") {
            QString director = it.binding(MediaVocabulary::videoDirectorBinding()).literal().toString();
            mediaItem.url = QString("video://sources?||director=%1").arg(director);
            mediaItem.title = director;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||director=%1").arg(director));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||director=%1").arg(director));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||director=%1").arg(director));
        } else if (type == "VideoTag") {
            QString tag = it.binding(MediaVocabulary::tagBinding()).literal().toString();
            QString tagFilter = tag.isEmpty() ? QString(): QString("tag=%1").arg(tag);
            mediaItem.url = QString("tag://video?%1")
                            .arg(tagFilter);
            mediaItem.title = tag;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-pim-notes");;
            //Provide context info for tag
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Frequently Played"),QString("semantics://frequent?video||limit=4||tag=%1").arg(tag));
        }
        /*if (!lri.isEmpty()) {
            mediaItem.url = lri;
        }*/
        Q_UNUSED(lri);
        mediaItem.fields["sourceLri"] = sourceLri;
    }
    mediaItem = makeSubtitle(mediaItem);
    return mediaItem;
}

Nepomuk::Resource Utilities::mediaResourceFromUrl(KUrl url)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasUrl(MediaQuery::Required, url.url()));
    query.endWhere();
    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);

    Nepomuk::Resource res = Nepomuk::Resource();
    while (it.next()) {
        res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
        if (res.exists() && (res.hasType(mediaVocabulary.typeAudio()) ||
            res.hasType(mediaVocabulary.typeAudioMusic()) ||
            res.hasType(mediaVocabulary.typeAudioStream()) ||
            res.hasType(mediaVocabulary.typeVideo()) ||
            res.hasType(mediaVocabulary.typeVideoMovie()) ||
            res.hasType(mediaVocabulary.typeVideoTVShow())) ) {
            break;//returns first media resource found
        }
    }
    return res;
}

QList<MediaItem> Utilities::mediaListFromSavedList(const MediaItem &savedListMediaItem)
{
    QList<MediaItem> mediaList;
    bool originNotLocal = false;

    //Download playlist if it is remote
    KUrl location = KUrl(savedListMediaItem.url);
    if (!location.isLocalFile() &&
        (Utilities::isPls(savedListMediaItem.url) || Utilities::isM3u(savedListMediaItem.url))) {
        originNotLocal = true;
        QString tmpFile;
        if( KIO::NetAccess::download(location, tmpFile, 0)) {
            location = KUrl(tmpFile);
        } else {
            return mediaList;
        }
    }
    QFile file(location.path());
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return mediaList;
        }
    }

    //Make sure it's a valid M3U or PLSfileref
    QTextStream in(&file);
    bool valid = false;
    bool isM3U = false;
    bool isPLS = false;
    if (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed() == "#EXTM3U") {
            valid = true;
            isM3U = true;
        } else if (line.trimmed() == "[playlist]") {
            valid = true;
            isPLS = true;
        }
    }

    //Create a MediaItem for each entry
    if (valid) {
        while (!in.atEnd()) {
            QString line = in.readLine();
            if ((isM3U)) {
                bool add = false;
                QString title;
                int duration = 0;
                QString url;
                //some internet radios only list mirrors without any #EXTINF.
                //so if it hasn't #EXTINF check if it was an internet stream, take the mirror and
                //copy the title of the original item
                if(line.startsWith("#EXTINF:")) {
                    add = true;
                    line = line.replace("#EXTINF:","");
                    QStringList durTitle = line.split(",");
                    if (durTitle.count() == 1) {
                        //No title
                        duration = 0;
                        title = durTitle.at(0);
                    } else {
                        duration = durTitle.at(0).toInt();
                        title = durTitle.at(1);
                    }
                    url = in.readLine().trimmed();
                } else if (originNotLocal) {
                    title = savedListMediaItem.title;
                    duration = -1;
                    add = true;
                    url = line;
                }
                if (add) {
                    MediaItem mediaItem;
                    KUrl itemUrl(url);
                    if (!url.isEmpty()) {
                        mediaItem = Utilities::mediaItemFromUrl(itemUrl);
                    } else {
                        continue;
                    }
                    if (mediaItem.title == itemUrl.fileName()) {
                        mediaItem.title = title;
                    }
                    if ((duration > 0) && (mediaItem.fields["duration"].toInt() <= 0)) {
                        mediaItem.duration = Utilities::durationString(duration);
                        mediaItem.fields["duration"] = duration;
                    } else if (duration == -1) {
                        mediaItem.duration = QString();
                        mediaItem.fields["audioType"] = "Audio Stream";
                    }
                    mediaList << mediaItem;
                }
            }
            if ((isPLS) && line.startsWith("File")) {
                QString url = line.mid(line.indexOf("=") + 1).trimmed();
                QString title;
                if (!in.atEnd()) {
                    line = in.readLine();
                    title = line.mid(line.indexOf("=") + 1).trimmed();
                }
                int duration = 0;
                if (!in.atEnd()) {
                    line = in.readLine();
                    duration = line.mid(line.indexOf("=") + 1).trimmed().toInt();
                }

                MediaItem mediaItem;
                KUrl itemUrl(url);
                if (!url.isEmpty()) {
                    mediaItem = Utilities::mediaItemFromUrl(itemUrl);
                } else {
                    continue;
                }
                if (mediaItem.title == itemUrl.fileName()) {
                    mediaItem.title = title;
                }
                if ((duration > 0) && (mediaItem.fields["duration"].toInt() <= 0)) {
                    mediaItem.duration = Utilities::durationString(duration);
                    mediaItem.fields["duration"] = duration;
                } else if (duration == -1) {
                    mediaItem.duration = QString();
                    mediaItem.fields["audioType"] = "Audio Stream";
                }
                mediaList << mediaItem;
            }
        }
    }

    return mediaList;
}

MediaItem Utilities::completeMediaItem(const MediaItem & sourceMediaItem)
{
    MediaItem mediaItem = sourceMediaItem;
    QString resourceUri = sourceMediaItem.fields["resourceUri"].toString();
    QString subType = mediaItem.fields["videoType"].toString();

    if (subType == "Movie" || subType == "TV Show") {
        MediaVocabulary mediaVocabulary;
        MediaQuery query;
        QStringList bindings;
        bindings.append(mediaVocabulary.mediaResourceBinding());
        bindings.append(mediaVocabulary.mediaResourceUrlBinding());
        bindings.append(mediaVocabulary.videoAudienceRatingBinding());
        bindings.append(mediaVocabulary.videoWriterBinding());
        bindings.append(mediaVocabulary.videoDirectorBinding());
        bindings.append(mediaVocabulary.videoProducerBinding());
        bindings.append(mediaVocabulary.videoActorBinding());
        if (subType == "TV Show") {
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoEpisodeNumberBinding());
        }
        query.select(bindings, MediaQuery::Distinct);

        query.startWhere();
        query.addCondition(mediaVocabulary.hasResource(resourceUri));
        query.addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
        if (subType == "TV Show") {
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
        }
        query.endWhere();

        Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
        Soprano::QueryResultIterator it = query.executeSelect(mainModel);

        while (it.next()) {
            Nepomuk::Resource res(it.binding(MediaVocabulary::mediaResourceBinding()).uri());
            QStringList writers;
            QList<Nepomuk::Resource> writerResources = res.property(mediaVocabulary.videoWriter()).toResourceList();
            for (int i = 0; i < writerResources.count(); i++) {
                QString name = writerResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    writers.append(name);
                }
            }
            writers = cleanStringList(writers);
            mediaItem.fields["writer"] = writers;

            QStringList directors;
            QList<Nepomuk::Resource> directorResources = res.property(mediaVocabulary.videoDirector()).toResourceList();
            for (int i = 0; i < directorResources.count(); i++) {
                QString name = directorResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    directors.append(name);
                }
            }
            directors = cleanStringList(directors);
            mediaItem.fields["director"] = directors;

            QStringList producers;
            QList<Nepomuk::Resource> producerResources = res.property(mediaVocabulary.videoProducer()).toResourceList();
            for (int i = 0; i < producerResources.count(); i++) {
                QString name = producerResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    producers.append(name);
                }
            }
            producers = cleanStringList(producers);
            mediaItem.fields["producer"] = producers;

            QStringList actors;
            QList<Nepomuk::Resource> actorResources = res.property(mediaVocabulary.videoActor()).toResourceList();
            for (int i = 0; i < actorResources.count(); i++) {
                QString name = actorResources.at(i).property(mediaVocabulary.ncoFullname()).toString();
                if (!name.isEmpty()) {
                    actors.append(name);
                }
            }
            actors = cleanStringList(actors);
            mediaItem.fields["actor"] = actors;
            if (subType == "TV Show") {
                mediaItem.artwork = KIcon("video-television");
                QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                }

                int season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                } else {
                    mediaItem.fields["season"] = QVariant(QVariant::Int);
                }

                int episodeNumber = it.binding(MediaVocabulary::videoEpisodeNumberBinding()).literal().toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                } else {
                    mediaItem.fields["episodeNumber"] = QVariant(QVariant::Int);
                }
            }
            break;
        }
    }
    mediaItem = makeSubtitle(mediaItem);
    return mediaItem;
}

QString Utilities::lriFilterFromMediaListField(const QList<MediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator)
{
    QString lriFilter;
    for (int i = 0; i < mediaList.count(); i++) {
        lriFilter = lriFilter + QString("||") + filterFieldName + lriFilterOperator + mediaList.at(i).fields[mediaItemField].toString();
    }
    return lriFilter;
}

QList<MediaItem> Utilities::mergeGenres(QList<MediaItem> genreList)
{
    for (int i = 0; i < genreList.count(); i++) {
        MediaItem genreItem = genreList.at(i);
        if (genreItem.type == "Category" && genreItem.fields["categoryType"] == "AudioGenre") {
            QString rawGenre = genreItem.title;
            QString convertedGenre = Utilities::genreFromRawTagGenre(rawGenre);
            if (convertedGenre != rawGenre) {
                bool matchFound = false;
                for (int j = 0; j < genreList.count(); j++) {
                    if (genreList.at(j).title == convertedGenre) {
                        matchFound = true;
                        MediaItem matchedGenre = genreList.at(j);
                        MediaListProperties matchedGenreProperties;
                        matchedGenreProperties.lri = matchedGenre.url;
                        QString newUrl = QString("%1%2?")
                                         .arg(matchedGenreProperties.engine())
                                         .arg(matchedGenreProperties.engineArg());
                        QString mergedFilter;
                        for (int k = 0; k < matchedGenreProperties.engineFilterList().count(); k++) {
                            QString filter = matchedGenreProperties.engineFilterList().at(k);
                            if (matchedGenreProperties.filterField(filter) == "genre") {
                                mergedFilter.append(QString("%1|OR|%2||")
                                                    .arg(filter)
                                                    .arg(rawGenre));
                            } else {
                                mergedFilter.append(filter);
                            }
                        }
                        newUrl.append(mergedFilter);
                        matchedGenre.url = newUrl;
                        genreList.replace(j, matchedGenre);
                        genreList.removeAt(i);
                        i = -1;
                    }
                }
                if (!matchFound) {
                    genreItem.title = convertedGenre;
                    genreList.replace(i, genreItem);
                }
            }
        }
    }
    return genreList;
}

QList<MediaItem> Utilities::sortMediaList(QList<MediaItem> mediaList)
{
    QList<MediaItem> sortedList;
    QMap<QString, int> sortedIndices;
    for (int i = 0; i < mediaList.count(); i++) {
        sortedIndices[mediaList.at(i).title] = i;
    }
    QMapIterator<QString, int> it(sortedIndices);
    while (it.hasNext()) {
        it.next();
        sortedList.append(mediaList.at(it.value()));
    }
    return sortedList;
}

MediaItem Utilities::makeSubtitle(const MediaItem & mediaItem)
{
    MediaItem updatedItem = mediaItem;
    QString subType = mediaItem.subType();

    if (subType == "Music") {
        updatedItem.subTitle.clear();
        QStringList artists = mediaItem.fields["artist"].toStringList();
        if (artists.count() == 1) {
            updatedItem.subTitle = artists.at(0);
        }

        QString album = mediaItem.fields["album"].toString();
        if (!album.isEmpty()) {
            if (!updatedItem.subTitle.isEmpty()) {
                updatedItem.subTitle += " - ";
            }
            updatedItem.subTitle += album;
        }
    } else if (subType == "Album") {
        updatedItem.subTitle = mediaItem.fields["artist"].toString();
    } else if (subType == "Movie") {
        updatedItem.subTitle.clear();
        int year = mediaItem.fields["year"].toInt();
        if (year > 0) {
            updatedItem.subTitle = QString("%1").arg(year);
        }
    } else if (subType == "TV Show") {
        updatedItem.subTitle.clear();
        QString seriesName = mediaItem.fields["seriesName"].toString();
        if (!seriesName.isEmpty()) {
            updatedItem.subTitle = seriesName;
        }

        int season = mediaItem.fields["season"].toInt();
        if (season > 0 ) {
            if (!updatedItem.subTitle.isEmpty()) {
                updatedItem.subTitle += " - ";
            }
            updatedItem.subTitle += i18n("Season %1", season);
        }

        int episodeNumber = mediaItem.fields["episodeNumber"].toInt();
        if (episodeNumber > 0) {
            if (!updatedItem.subTitle.isEmpty()) {
                updatedItem.subTitle += " - ";
            }
            updatedItem.subTitle += i18n("Episode %1", episodeNumber);
        }
    } else if (subType == "TV Season") {
        updatedItem.subTitle.clear();
        int season = mediaItem.fields["season"].toInt();
        if (season > 0 ) {
            updatedItem.subTitle = i18nc("%1=Number of the Season", "Season %1", season);
        }
    } else if (subType == "Audio Feed" || subType == "Video Feed") {
        QString description = mediaItem.fields["description"].toString();
        updatedItem.subTitle = QString("%1...").arg(description.left(50));
    } else if (subType == "DVD Title"){
        updatedItem.subTitle = i18n("DVD Video");
        QString discTitle = mediaItem.fields["discTitle"].toString();
        if (!discTitle.isEmpty()) {
            updatedItem.subTitle += " - ";
            updatedItem.subTitle += discTitle;
        }
    } else if (subType == "CD Track") {
        updatedItem.subTitle = i18n("Audio CD");
    }
    return updatedItem;
}

QStringList Utilities::getLinksForResource(Nepomuk::Resource &res)
{
    QStringList related;
    QUrl relatedProperty("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#relatedTo");
    QList<QUrl> relatedUrls = res.property(relatedProperty).toUrlList();
    for (int i = 0; i < relatedUrls.count(); i++) {
        related.append(KUrl(relatedUrls.at(i)).prettyUrl());
    }
    return related;
}

bool Utilities::isTemporaryAudioStream(const MediaItem& item)
{
    if (item.type != "Audio") {
        return false;
    }
    if (item.fields["audioType"] != "Audio Stream") {
        return false;
    }
    const QVariant &rscUri = item.fields["resourceUri"];
    if (rscUri.isValid() && !rscUri.isNull()) {
        return false;
    }
    return true;
}

KUrl Utilities::urlForFilex(KUrl url)
{
    Solid::StorageAccess *storage = 0;
    QString solidQuery = QString::fromLatin1( "[ StorageVolume.usage=='FileSystem' AND StorageVolume.uuid=='%1' ]" )
                         .arg(url.host().toLower());
    QList<Solid::Device> devices = Solid::Device::listFromQuery(solidQuery);
    if (!devices.isEmpty()) {
        storage = devices.first().as<Solid::StorageAccess>();
    }
    QString normalUrl;
    if (storage && storage->isAccessible()) {
        normalUrl = QString("%1/%2").arg(storage->filePath()).arg(url.path());
    } else {
        normalUrl = url.prettyUrl();
    }
    return KUrl(normalUrl);
}

#endif //UTILITIES_MEDIAITEMS_CPP

