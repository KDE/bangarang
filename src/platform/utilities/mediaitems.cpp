/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://projects.kde.org/projects/playground/multimedia/bangarang>
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
#include "../downloader.h"


#include <QUrl>
#include <QIcon>
#include <KLocalizedString>
#include <QDebug>
#include <QStandardPaths>
#include <KConfig>
#include <KConfigGroup>
#include <Solid/Device>
#include <Solid/StorageAccess>


#include <QFile>
#include <QTemporaryFile>
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

OldMediaItem Utilities::getArtistCategoryItem(const QString &artist)
{
    OldMediaItem mediaItem;

    /*
     * TODO: plug in metadata storage
     */
    return mediaItem;

}


OldMediaItem Utilities::mediaItemFromUrl(QUrl url, bool preferFileMetaData)
{
    OldMediaItem mediaItem;
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
        mediaItem.artwork = dvd ? QIcon::fromTheme("media-optical-dvd") : QIcon::fromTheme("media-optical-audio");
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

    if (url.toDisplayString().startsWith(QLatin1String("filex:/"))) {
        url = urlForFilex(url);
    }


    if (url.isLocalFile() && (Utilities::isM3u(url.url()) || Utilities::isPls(url.url()))) {
        mediaItem.artwork = QIcon::fromTheme("audio-x-scpls");
        mediaItem.url = QString("savedlists://%1").arg(url.url());
        mediaItem.title = url.fileName();
        mediaItem.fields["title"] = url.fileName();
        mediaItem.subTitle = i18n("Playlist");
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = "Basic+Artwork";
        return mediaItem;
    }

    if (url.isLocalFile() && Utilities::isFSDirectory(url.url())) {
        mediaItem.artwork = QIcon::fromTheme("folder");
        mediaItem.url = QString("files://media?browseFolder||%1").arg(url.toDisplayString());
        mediaItem.title = url.path();
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = "Basic+Artwork";
        return mediaItem;
    }

    mediaItem.url = url.toDisplayString();
    mediaItem.title = url.fileName();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["title"] = mediaItem.title;

    //Determine type of file - file metadata preferred
    bool foundInNepomuk = false;

    if (!foundInNepomuk || mediaItem.type.isEmpty()) {
        mediaItem.type = "Audio"; // default to Audio;
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
            mediaItem.title = url.host() + " - " + url.fileName();
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Audio Stream";
            mediaItem.fields["sourceLri"] = "audiostreams://"; //Set sourceLri so that any MediaItemModel will know how to save info
        }
    }

    if (mediaItem.type == "Audio") {
        if (mediaItem.fields["audioType"] == "Audio Clip") {
            mediaItem.artwork = QIcon::fromTheme("audio-x-generic");
        } else if (mediaItem.fields["audioType"] == "Music") {
            mediaItem.artwork = QIcon::fromTheme("audio-mp4");
            if (!foundInNepomuk || preferFileMetaData) {
                mediaItem = Utilities::getAllInfoFromTag(mediaItem.url, mediaItem);
            }
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            mediaItem.artwork = QIcon::fromTheme("text-html");
            if (mediaItem.title.isEmpty()) {
                mediaItem.title = url.toDisplayString();
            }
        }
    } else if (mediaItem.type == "Video") {
        if (mediaItem.fields["videoType"] == "Video Clip") {
            mediaItem.artwork = QIcon::fromTheme("video-x-generic");
        }
    }

    //Lookup metadata not stored with file, TODO: plug in metadata storage

    return mediaItem;
}

QStringList Utilities::mediaListUrls(const QList<OldMediaItem> &mediaList)
{
    QStringList urls;
    for (int i = 0; i < mediaList.count(); i++) {
        urls << mediaList.at(i).url;
    }
    return urls;
}

int Utilities::mediaListDuration(const QList<OldMediaItem> &mediaList)
{
    int duration = 0;
    for (int i = 0; i < mediaList.count(); i++) {
        duration += mediaList.at(i).fields["duration"].toInt();
    }
    return duration;
}

QString Utilities::mediaListDurationText(const QList<OldMediaItem> &mediaList)
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

QList<OldMediaItem> Utilities::mediaItemsDontExist(const QList<OldMediaItem> &mediaList)
{
    QList<OldMediaItem> items;
    for (int i = 0; i < mediaList.count(); i++) {
        OldMediaItem mediaItem = mediaList.at(i);
        bool dvdNotFound = false;
        QString url_string = mediaItem.url;
        if (isDisc(url_string)) {
            bool dvd = isDvd(url_string);
            continue;
            dvdNotFound = true;
            Q_UNUSED(dvd);
        }
        QUrl url = QUrl(QUrl::toPercentEncoding(url_string).data());
        if (dvdNotFound ||
            (url.isValid() && url.isLocalFile() && !QFile(url.path()).exists()) ||
            url_string.startsWith(QLatin1String("trash:/"))
           ) {
            mediaItem.exists = false;
            qDebug() << mediaItem.url << " missing";
            items << mediaItem;
        }
    }
    return items;
}


QList<OldMediaItem> Utilities::mediaListFromSavedList(const OldMediaItem &savedListMediaItem)
{
    QList<OldMediaItem> mediaList;
    bool originNotLocal = false;
    Downloader download;

    //Download playlist if it is remote
    QUrl location = QUrl::fromUserInput(savedListMediaItem.url);
    if (!location.isLocalFile() &&
        (Utilities::isPls(savedListMediaItem.url) || Utilities::isM3u(savedListMediaItem.url))) {
        originNotLocal = true;

     QTemporaryFile tmp;
     if(tmp.open()){
     download.download(location, (QUrl::fromLocalFile(tmp.fileName())));
     location = QUrl::fromLocalFile(tmp.fileName());
     }else{
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
                if(line.startsWith(QLatin1String("#EXTINF:"))) {
                    add = true;
                    line = line.remove("#EXTINF:");
                    QStringList durTitle = line.split(',');
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
                    OldMediaItem mediaItem;
                    QUrl itemUrl = QUrl::fromUserInput(url);
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
                        mediaItem.duration.clear();
                        mediaItem.fields["audioType"] = "Audio Stream";
                    }
                    mediaList << mediaItem;
                }
            }
            if ((isPLS) && line.startsWith(QLatin1String("File"))) {
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

                OldMediaItem mediaItem;
                QUrl itemUrl = QUrl::fromUserInput(url);
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
                    mediaItem.duration.clear();
                    mediaItem.fields["audioType"] = "Audio Stream";
                }
                mediaList << mediaItem;
            }
        }
    }

    return mediaList;
}


QString Utilities::lriFilterFromMediaListField(const QList<OldMediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator)
{
    QString lriFilter;
    for (int i = 0; i < mediaList.count(); i++) {
        lriFilter = lriFilter + QString("||") + filterFieldName + lriFilterOperator + mediaList.at(i).fields[mediaItemField].toString();
    }
    return lriFilter;
}

QList<OldMediaItem> Utilities::mergeGenres(QList<OldMediaItem> genreList)
{
    for (int i = 0; i < genreList.count(); i++) {
        OldMediaItem genreItem = genreList.at(i);
        if (genreItem.type == "Category" && genreItem.fields["categoryType"] == "AudioGenre") {
            QString rawGenre = genreItem.title;
            QString convertedGenre = Utilities::genreFromRawTagGenre(rawGenre);
            if (convertedGenre != rawGenre) {
                bool matchFound = false;
                for (int j = 0; j < genreList.count(); j++) {
                    if (genreList.at(j).title == convertedGenre) {
                        matchFound = true;
                        OldMediaItem matchedGenre = genreList.at(j);
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

QList<OldMediaItem> Utilities::sortMediaList(QList<OldMediaItem> mediaList)
{
    QList<OldMediaItem> sortedList;
    QMap<QString, int> sortedIndices;
    for (int i = 0; i < mediaList.count(); i++) {
        sortedIndices[mediaList.at(i).title.toLower()] = i;
    }
    QMapIterator<QString, int> it(sortedIndices);
    while (it.hasNext()) {
        it.next();
        sortedList.append(mediaList.at(it.value()));
    }
    return sortedList;
}

OldMediaItem Utilities::makeSubtitle(const OldMediaItem & mediaItem)
{
    OldMediaItem updatedItem = mediaItem;
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


bool Utilities::isTemporaryAudioStream(const OldMediaItem& item)
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

QUrl Utilities::urlForFilex(QUrl url)
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
        normalUrl = url.toDisplayString();
    }
    return QUrl::fromLocalFile(normalUrl);
}

QUrl Utilities::decodedUrl(QUrl rawUrl)
{
    //NOTE: This undoes two layers of percent encoding.  This means that locations that
    //      actually have "%[0-9]" in the path may be incorrectly changed.
    QString rawUrlString = QUrl::fromPercentEncoding(rawUrl.toString().toUtf8());
    return QUrl::fromUserInput(rawUrlString);
}

#endif //UTILITIES_MEDIAITEMS_CPP

