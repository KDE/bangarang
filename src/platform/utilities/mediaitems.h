/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@gmail.com)
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

#ifndef UTILITIES_MEDIAITEMS_H
#define UTILITIES_MEDIAITEMS_H

#include <QUrl>

class MediaItem;

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    MediaItem getArtistCategoryItem(const QString &artist);
    MediaItem mediaItemFromUrl(QUrl url, bool preferFileMetaData = false);
    QStringList mediaListUrls(const QList<MediaItem> &mediaList);
    int mediaListDuration(const QList<MediaItem> &mediaList);
    QString mediaListDurationText(const QList<MediaItem> &mediaList);
    QList<MediaItem> mediaItemsDontExist(const QList<MediaItem> &mediaList);
    QString lriFilterFromMediaListField(const QList<MediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator);
    QList<MediaItem> mediaListFromSavedList(const MediaItem &savedListMediaItem);
    QList<MediaItem> mergeGenres(QList<MediaItem> genreList);
    QList<MediaItem> sortMediaList(QList<MediaItem> mediaList);
    MediaItem makeSubtitle(const MediaItem & mediaItem);
    bool isTemporaryAudioStream(const MediaItem &item);
    QUrl urlForFilex(QUrl url);
    QUrl decodedUrl(QUrl url);    

}

#endif // UTILITIES_MEDIAITEMS_H
