/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef UTILITIES_TYPECHECKS_H
#define UTILITIES_TYPECHECKS_H

#include <KUrl>
#include <KMimeType>

#include <QModelIndex>

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    bool isMusic(const QString &url);
    bool isMusicMimeType(KMimeType::Ptr type);
    bool isAudio(const QString &url);
    bool isAudioMimeType(KMimeType::Ptr type);
    bool isVideo(const QString &url);
    bool isVideoMimeType(KMimeType::Ptr type);
    bool isM3u(const QString &url);
    bool isPls(const QString &url);
    bool isDvd(const KUrl& url);
    bool isCd(const KUrl& url);
    bool isDisc(const KUrl& url);
    bool isMediaItem(const QModelIndex *index);
    bool isMedia(const QString& type);
    bool isCategory(const QString& type);
    bool isMessage(const QString& type);
    bool isAction(const QString& type);
    bool isFeed(const QString& categoryType);
    bool isAudioStream(const QString& audioType);
    bool isFSDirectory(const QString& url);
    QString audioMimeFilter();
    QString videoMimeFilter();
    QString deviceTypeFromUrl(const KUrl &url);
}

#endif // UTILITIES_TYPECHECKS_H
