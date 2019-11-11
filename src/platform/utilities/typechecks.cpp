/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://commits.kde.org/bangarang>
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

#ifndef UTILITIES_TYPECHECKS_CPP
#define UTILITIES_TYPECHECKS_CPP

#include "typechecks.h"
#include "../mediaitemmodel.h"
#include <phonon/backendcapabilities.h>


bool Utilities::isMusic(const QString &url)
{
    QMimeDatabase db;
    if (!url.isEmpty()) {
        QMimeType result = db.mimeTypeForUrl((QUrl::fromLocalFile(url)));
        return isMusicMimeType(result);
    } else {
        return false;
    }
}

bool Utilities::isMusicMimeType(QMimeType type)
{
    return (type.inherits("audio/mpeg") ||
            type.inherits("audio/mp4") ||
            type.inherits("audio/mp3") ||
            type.inherits("audio/ogg") ||
            type.inherits("audio/flac") ||
            type.inherits("application/ogg") ||
            type.inherits("audio/x-flac") ||
            type.inherits("audio/x-musepack") ||
            type.inherits("audio/x-oma") ||
            type.inherits("audio/x-m4a") ||
            type.inherits("audio/x-monkeys-audio") ||
            type.inherits("audio/x-wv") ||
            type.inherits("audio/x-ms-wma") ||
            type.inherits("audio/aac") ||
            type.inherits("audio/3gpp")  ||
            type.inherits("audio/3gpp2"));
}

bool Utilities::isAudio(const QString &url)
{
    QMimeDatabase db;
    if (!url.isEmpty()) {
          QMimeType result = db.mimeTypeForUrl((QUrl::fromLocalFile(url)));
        return isAudioMimeType(result);
    } else {
        return false;
    }
}

bool Utilities::isAudioMimeType(QMimeType type)
{
    return (type.inherits("audio/mpeg") ||
            type.inherits("audio/mp4") ||
            type.inherits("audio/ogg") ||
            type.inherits("audio/vorbis") ||
            type.inherits("audio/aac") ||
            type.inherits("audio/ac3") ||
            type.inherits("audio/aiff") ||
            type.inherits("audio/basic") ||
            type.inherits("audio/flac") ||
            type.inherits("audio/mp2") ||
            type.inherits("audio/mp3") ||
            type.inherits("audio/vnd.rn-realaudio") ||
            type.inherits("audio/wav") ||
            type.inherits("application/ogg") ||
            type.inherits("audio/x-ac3") ||
            type.inherits("audio/x-flac") ||
            type.inherits("audio/x-musepack") ||
            type.inherits("audio/x-m4a") ||
            type.inherits("audio/x-oma") ||
            type.inherits("audio/x-monkeys-audio") ||
            type.inherits("audio/x-wv") ||
            type.inherits("audio/x-ms-asf") ||
            type.inherits("audio/x-ms-wma") ||
            type.inherits("audio/3gpp")  ||
            type.inherits("audio/3gpp2"));
}

bool Utilities::isVideo(const QString &url)
{
    QMimeDatabase db;
    if (!url.isEmpty()) {
         QMimeType result = db.mimeTypeForUrl((QUrl::fromLocalFile(url)));
        //NOTE: Special handling for .wma extensions
        //It turns out that wma files may pass the "video/x-ms-asf" mimetype test.
        //Per Microsoft KB284094, the only way to distinguish between audio-only
        //and audio+video content is to look at the file extenstion.
        if (result.inherits("video/x-ms-asf") && (QUrl::fromLocalFile(url)).fileName().endsWith(QLatin1String(".wma"))) {
            return false;
        }
        return isVideoMimeType(result);
    } else {
        return false;
    }
}

bool Utilities::isVideoMimeType(QMimeType type)
{
    return (type.inherits("video/mp4") ||
            type.inherits("video/mpeg") ||
            type.inherits("video/ogg") ||
            type.inherits("video/quicktime") ||
            type.inherits("video/msvideo") ||
            type.inherits("video/x-theora") ||
            type.inherits("video/x-theora+ogg") ||
            type.inherits("video/x-ogm")||
            type.inherits("video/x-ogm+ogg") ||
            type.inherits("video/divx") ||
            type.inherits("video/x-msvideo") ||
            type.inherits("video/x-ms-asf") ||
            type.inherits("video/x-ms-wmv") ||
            type.inherits("video/x-wmv") ||
            type.inherits("video/x-flv") ||
            type.inherits("video/x-matroska")) ||
            type.inherits("video/3gpp") ||
            type.inherits("video/3gpp2");
}

bool Utilities::isM3u(const QString &url)
{
    QMimeDatabase db;
    if (!url.isEmpty()) {
          QMimeType result = db.mimeTypeForUrl((QUrl::fromLocalFile(url)));
        return (result.inherits("audio/m3u") ||
                result.inherits("application/vnd.apple.mpegurl") ||
                result.inherits("audio/x-mpegurl"));
    } else {
        return false;
    }
}

bool Utilities::isPls(const QString &url)
{
    QMimeDatabase db;
    if (!url.isEmpty()) {
          QMimeType result = db.mimeTypeForUrl((QUrl::fromLocalFile(url)));
        return result.inherits("audio/x-scpls");
    } else {
        return false;
    }
}

bool Utilities::isDvd(const QUrl &url)
{
    return (deviceTypeFromUrl(url) == "dvd");
}

bool Utilities::isCd(const QUrl &url)
{
    return (deviceTypeFromUrl(url) == "cd");
}

bool Utilities::isDisc(const QUrl& url)
{
    return (isDvd(url) || isCd(url));
}

bool Utilities::isFSDirectory(const QString& url)
{
    QMimeDatabase db;
    if (!url.isEmpty()) {
          QMimeType result = db.mimeTypeForUrl((QUrl::fromLocalFile(url)));
        return result.inherits("inode/directory");
    } else {
        return false;
    }
}

bool Utilities::isMediaItem(const QModelIndex *index)
{
    QString type = index->data(OldMediaItem::TypeRole).toString();
    return Utilities::isMedia(type);

}

bool Utilities::isMedia(const QString& type)
{
   return (
            (type == "Audio") ||
            (type == "Video")
        );
}

bool Utilities::isFeed(const QString& categoryType)
{
   return (categoryType == "Audio Feed" || categoryType == "Video Feed");
}

bool Utilities::isAudioStream(const QString& audioType)
{
  return (audioType == "Audio Stream");
}

bool Utilities::isCategory(const QString& type)
{
   return (type == "Category");
}

bool Utilities::isMessage(const QString& type)
{
    return (type == "Message");
}

bool Utilities::isAction(const QString& type)
{
    return (type == "Action");
}

QString Utilities::audioMimeFilter()
{
    QStringList supportedList = Phonon::BackendCapabilities::availableMimeTypes().filter("audio");
    QStringList appList = Phonon::BackendCapabilities::availableMimeTypes().filter("application");
    QStringList ambiguousList;
    for (int i = 0; i < appList.count(); i++) {
        if (!appList.at(i).contains("video") && !appList.at(i).contains("audio")) {
            ambiguousList.append(appList.at(i));
        }
    }
    supportedList << ambiguousList;
    supportedList << "audio/m3u" << "audio/x-mpegurl" << "audio/x-scpls" << "application/vnd.apple.url"; //add playlist mimetypes
    return supportedList.join(" ");
    /* This section might be useful if Phonon doesn't report
     * supported mimetypes correctly. For now I'll assume it
     * does so it is disabled. */
    /*QString mimeFilter = QString("audio/mpeg audio/mp4 audio/ogg audio/vorbis audio/aac audio/aiff audio/basic audio/flac audio/mp2 audio/mp3 audio/vnd.rn-realaudio audio/wav application/ogg audio/x-flac audio/x-musepack ");
    mimeFilter += supportedList.join(" ");
    return mimeFilter;*/
}

QString Utilities::videoMimeFilter()
{
    QStringList supportedList = Phonon::BackendCapabilities::availableMimeTypes().filter("video");
    QStringList appList = Phonon::BackendCapabilities::availableMimeTypes().filter("application");
    QStringList ambiguousList;
    for (int i = 0; i < appList.count(); i++) {
        if (!appList.at(i).contains("video") && !appList.at(i).contains("audio")) {
            ambiguousList.append(appList.at(i));
        }
    }
    supportedList << ambiguousList;
    return supportedList.join(" ");

    /* This section might be useful if Phonon doesn't report
    * supported mimetypes correctly. For now I'll assume it
    * does so it is disabled. */
    /*QString mimeFilter =  QString("video/mp4 video/mpeg video/ogg video/quicktime video/msvideo video/x-theora video/x-theora+ogg video/x-ogm video/x-ogm+ogg video/divx video/x-msvideo video/x-wmv video/x-flv video/flv");
    mimeFilter += supportedList.join(" ");
    return mimeFilter;*/
}

QString Utilities::deviceTypeFromUrl(const QUrl &url)
{
    return url.authority();
}

#endif //UTILITIES_TYPECHECKS_CPP
