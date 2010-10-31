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

#include "filetags.h"
#include "typechecks.h"
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
    query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required, artist, MediaQuery::Equal));
    query.addCondition(mediaVocabulary.hasMusicArtistDescription(MediaQuery::Optional));
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


MediaItem Utilities::mediaItemFromUrl(const KUrl& url, bool preferFileMetaData)
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
        if (discTitle.isEmpty() || !dvd)
            mediaItem.subTitle = album;
        else
            mediaItem.subTitle = i18nc("%1=Title of DVD", "DVD Video - %1", discTitle);
        mediaItem.fields["url"] = mediaItem.url;
        mediaItem.fields["title"] = mediaItem.title;
        if ( dvd )
            mediaItem.fields["videoType"] = "DVD Title";
        else
            mediaItem.fields["audioType"] = "CD Track";
        mediaItem.fields["album"] = album;
        mediaItem.type = dvd ? "Video" : "Audio";
        mediaItem.fields["trackNumber"] = track;
        return mediaItem;
    }

    MediaVocabulary mediaVocabulary = MediaVocabulary();

    //url.cleanPath();
    //url = QUrl::fromPercentEncoding(url.url().toUtf8());

    if (url.isLocalFile() && (Utilities::isM3u(url.url()) || Utilities::isPls(url.url()))) {
        mediaItem.artwork = KIcon("view-list-text");
        mediaItem.url = QString("savedlists://%1").arg(url.url());
        mediaItem.title = url.fileName();
        mediaItem.type = "Category";
        return mediaItem;
    }

    mediaItem.url = url.prettyUrl();
    mediaItem.title = url.fileName();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["title"] = mediaItem.title;

    //Determine type of file - nepomuk is primary source
    bool foundInNepomuk = false;
    if (nepomukInited() && !preferFileMetaData) {
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
    }

    if (!foundInNepomuk || mediaItem.type.isEmpty()) {
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
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Audio Stream";
        }
    }

    if (mediaItem.type == "Audio") {
        if (mediaItem.fields["audioType"] == "Audio Clip") {
            mediaItem.artwork = KIcon("audio-x-generic");
        } else if (mediaItem.fields["audioType"] == "Music") {
            mediaItem.artwork = KIcon("audio-mp4");
            //File metadata is always primary for music items.
            TagLib::FileRef file(KUrl(mediaItem.url).path().toLocal8Bit().constData());
            if (!file.isNull()) {
                QString title = TStringToQString(file.tag()->title()).trimmed();
                QString artist  = TStringToQString(file.tag()->artist()).trimmed();
                QString album   = TStringToQString(file.tag()->album()).trimmed();
                QString genre   = TStringToQString(file.tag()->genre()).trimmed();
                if (KUrl(mediaItem.url).path().endsWith(".mp3")) {
                    // detect encoding for mpeg id3v2
                    QString tmp = title + artist + album + genre;
                    KEncodingProber prober(KEncodingProber::Universal);
                    KEncodingProber::ProberState result = prober.feed(tmp.toAscii());
                    if (result != KEncodingProber::NotMe) {
                        QByteArray encodingname = prober.encoding().toLower();
                        if ( prober.confidence() > 0.47
                            && ( ( encodingname == "gb18030" )
                            || ( encodingname == "big5" )
                            || ( encodingname == "euc-kr" )
                            || ( encodingname == "euc-jp" )
                            || ( encodingname == "koi8-r" ) ) ) {
                            title = QTextCodec::codecForName(encodingname)->toUnicode(title.toAscii());
                            artist = QTextCodec::codecForName(encodingname)->toUnicode(artist.toAscii());
                            album = QTextCodec::codecForName(encodingname)->toUnicode(album.toAscii());
                            genre = QTextCodec::codecForName(encodingname)->toUnicode(genre.toAscii());
                        } else if ((prober.confidence() < 0.3 || encodingname != "utf-8")
                            && QTextCodec::codecForLocale()->name().toLower() != "utf-8") {
                            title = QTextCodec::codecForLocale()->toUnicode(title.toAscii());
                            artist = QTextCodec::codecForLocale()->toUnicode(artist.toAscii());
                            album = QTextCodec::codecForLocale()->toUnicode(album.toAscii());
                            genre = QTextCodec::codecForLocale()->toUnicode(genre.toAscii());
                        }
                    }
                }
                int track   = file.tag()->track();
                int duration = file.audioProperties()->length();
                int year = file.tag()->year();
                if (!title.isEmpty()) {
                    mediaItem.title = title;
                }
                mediaItem.subTitle = artist + QString(" - ") + album;
                mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                mediaItem.fields["duration"] = duration;
                mediaItem.fields["title"] = title;
                mediaItem.fields["artist"] = artist;
                mediaItem.fields["album"] = album;
                mediaItem.fields["genre"] = genreFromRawTagGenre(genre);
                mediaItem.fields["trackNumber"] = track;
                mediaItem.fields["year"] = year;
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

    //If nepomuk resource type is not recognized try recognition by mimetype
    if (type.isEmpty()) {
        QString url = KUrl(res.property(nieUrl).toUrl()).prettyUrl();
        if (isAudio(url)) {
            type = "Audio Clip";
        }
        if (isMusic(url)) {
            type = "Music";
        }
        if (isVideo(url)){
            type = "Video Clip";
        }
    }

    MediaItem mediaItem;

    mediaItem.url = KUrl(res.property(nieUrl).toUrl()).prettyUrl();
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
        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
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
            artistResources = res.property(mediaVocabulary.musicComposer()).toResourceList();
            for (int i = 0; i < artistResources.count(); i++) {
                artists.append(artistResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            artists = cleanStringList(artists);
            mediaItem.fields["artist"] = artists;
            if (artists.count() == 1) {
                mediaItem.subTitle = artists.at(0);
            }

            QString album = res.property(mediaVocabulary.musicAlbum()).toResource()
                            .property(mediaVocabulary.musicAlbumName()).toString();
            if (!album.isEmpty()) {
                mediaItem.fields["album"] = album;
                if (artists.count() == 1) {
                    mediaItem.subTitle += QString(" - %1").arg(album);
                } else {
                    mediaItem.subTitle = album;
                }
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
                    mediaItem.subTitle = seriesName;
                }

                int season = res.property(mediaVocabulary.videoSeason()).toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Season %1").arg(season);
                } else {
                    mediaItem.fields["season"] = QVariant(QVariant::Int);
                }

                int episodeNumber = res.property(mediaVocabulary.videoEpisodeNumber()).toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                } else {
                    mediaItem.fields["episodeNumber"] = QVariant(QVariant::Int);
                }
            }
        }
    }

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
    mediaItem.url = url.prettyUrl();
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
        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
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
            artistResources = res.property(mediaVocabulary.musicComposer()).toResourceList();
            for (int i = 0; i < artistResources.count(); i++) {
                artists.append(artistResources.at(i).property(mediaVocabulary.musicArtistName()).toString());
            }
            artists = cleanStringList(artists);
            mediaItem.fields["artist"] = artists;
            if (artists.count() == 1) {
                mediaItem.subTitle = artists.at(0);
            }

            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            if (!album.isEmpty()) {
                mediaItem.fields["album"] = album;
                if (artists.count() == 1) {
                    mediaItem.subTitle += QString(" - %1").arg(album);
                } else {
                    mediaItem.subTitle = album;
                }
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
                    mediaItem.subTitle = seriesName;
                }

                int season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Season %1").arg(season);
                } else {
                    mediaItem.fields["season"] = QVariant(QVariant::Int);
                }

                int episodeNumber = it.binding(MediaVocabulary::videoEpisodeNumberBinding()).literal().toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                } else {
                    mediaItem.fields["episodeNumber"] = QVariant(QVariant::Int);
                }
            }
        }
    }

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

        if (type =="Artist") {
            kDebug() << mediaVocabulary.musicArtistName();
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
            mediaItem.subTitle = artist;
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
            mediaItem.subTitle = i18nc("%1=Number of the Season", "Season %1", season);
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

QList<MediaItem> Utilities::mediaListFromSavedList(const QString &savedListLocation)
{
    QList<MediaItem> mediaList;

    //Download playlist if it is remote
    KUrl location = KUrl(savedListLocation);
    if (!location.isLocalFile() &&
        (Utilities::isPls(savedListLocation) || Utilities::isM3u(savedListLocation))) {
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
            if ((isM3U) && line.startsWith("#EXTINF:")) {
                line = line.replace("#EXTINF:","");
                QStringList durTitle = line.split(",");
                QString title;
                int duration;
                if (durTitle.count() == 1) {
                    //No title
                    duration = 0;
                    title = durTitle.at(0);
                } else {
                    duration = durTitle.at(0).toInt();
                    title = durTitle.at(1);
                }
                QString url = in.readLine().trimmed();
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
                    mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                    mediaItem.fields["duration"] = duration;
                } else if (duration == -1) {
                    mediaItem.duration = QString();
                    mediaItem.fields["audioType"] = "Audio Stream";
                }
                mediaList << mediaItem;
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
                    mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
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
                    mediaItem.subTitle = seriesName;
                }

                int season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Season %1").arg(season);
                } else {
                    mediaItem.fields["season"] = QVariant(QVariant::Int);
                }

                int episodeNumber = it.binding(MediaVocabulary::videoEpisodeNumberBinding()).literal().toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                } else {
                    mediaItem.fields["episodeNumber"] = QVariant(QVariant::Int);
                }
            }
            break;
        }
    }
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
#endif //UTILITIES_MEDIAITEMS_CPP

