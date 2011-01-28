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

#ifndef UTILITIES_TYPECHECKS_CPP
#define UTILITIES_TYPECHECKS_CPP

#include "typechecks.h"
#include "../mediaitemmodel.h"

#include <phonon/backendcapabilities.h>

bool Utilities::isMusic(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);

    return isMusicMimeType(result);
}

bool Utilities::isMusicMimeType(KMimeType::Ptr type)
{
    return (type->is("audio/mpeg") ||
            type->is("audio/mp4") ||
            type->is("audio/mp3") ||
            type->is("audio/ogg") ||
            type->is("audio/flac") ||
            type->is("application/ogg") ||
            type->is("audio/x-flac") ||
            type->is("audio/x-musepack") ||
            type->is("audio/x-oma") ||
            type->is("audio/x-m4a") ||
            type->is("audio/x-monkeys-audio") ||
            type->is("audio/x-wv") ||
            type->is("audio/x-ms-wma") ||
            type->is("audio/aac") ||
            type->is("audio/3gpp")  ||
            type->is("audio/3gpp2"));
}

bool Utilities::isAudio(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    return isAudioMimeType(result);
}

bool Utilities::isAudioMimeType(KMimeType::Ptr type)
{
    return (type->is("audio/mpeg") ||
            type->is("audio/mp4") ||
            type->is("audio/ogg") ||
            type->is("audio/vorbis") ||
            type->is("audio/aac") ||
            type->is("audio/ac3") ||
            type->is("audio/aiff") ||
            type->is("audio/basic") ||
            type->is("audio/flac") ||
            type->is("audio/mp2") ||
            type->is("audio/mp3") ||
            type->is("audio/vnd.rn-realaudio") ||
            type->is("audio/wav") ||
            type->is("application/ogg") ||
            type->is("audio/x-ac3") ||
            type->is("audio/x-flac") ||
            type->is("audio/x-musepack") ||
            type->is("audio/x-m4a") ||
            type->is("audio/x-oma") ||
            type->is("audio/x-monkeys-audio") ||
            type->is("audio/x-wv") ||
            type->is("audio/x-ms-asf") ||
            type->is("audio/x-ms-wma") ||
            type->is("audio/3gpp")  ||
            type->is("audio/3gpp2"));
}

bool Utilities::isVideo(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);

    //NOTE: Special handling for .wma extensions
    //It turns out that wma files may pass the "video/x-ms-asf" mimetype test.
    //Per Microsoft KB284094, the only way to distinguish between audio-only
    //and audio+video content is to look at the file extenstion.
    if (result->is("video/x-ms-asf") && KUrl(url).fileName().endsWith(".wma")) {
        return false;
    }

    return isVideoMimeType(result);
}

bool Utilities::isVideoMimeType(KMimeType::Ptr type)
{
    return (type->is("video/mp4") ||
            type->is("video/mpeg") ||
            type->is("video/ogg") ||
            type->is("video/quicktime") ||
            type->is("video/msvideo") ||
            type->is("video/x-theora") ||
            type->is("video/x-theora+ogg") ||
            type->is("video/x-ogm")||
            type->is("video/x-ogm+ogg") ||
            type->is("video/divx") ||
            type->is("video/x-msvideo") ||
            type->is("video/x-ms-asf") ||
            type->is("video/x-ms-wmv") ||
            type->is("video/x-wmv") ||
            type->is("video/x-flv") ||
            type->is("video/x-matroska"));
}

bool Utilities::isM3u(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    return (result->is("audio/m3u") ||
            result->is("application/vnd.apple.mpegurl") ||
            result->is("audio/x-mpegurl"));
}

bool Utilities::isPls(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);

    return result->is("audio/x-scpls");
}

bool Utilities::isDvd(const KUrl& url)
{
    return (deviceTypeFromUrl(url) == "dvd");
}

bool Utilities::isCd(const KUrl& url)
{
    return (deviceTypeFromUrl(url) == "cd");
}

bool Utilities::isDisc(const KUrl& url)
{
    return (isDvd(url) || isCd(url));
}

bool Utilities::isMediaItem(const QModelIndex *index)
{
    QString type = index->data(MediaItem::TypeRole).toString();
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

QString Utilities::deviceTypeFromUrl(const KUrl& url)
{
    return url.authority();
}

#endif //UTILITIES_TYPECHECKS_CPP
