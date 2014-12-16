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

class OldMediaItem;

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    OldMediaItem getArtistCategoryItem(const QString &artist);
    OldMediaItem mediaItemFromUrl(QUrl url, bool preferFileMetaData = false);
    QStringList mediaListUrls(const QList<OldMediaItem> &mediaList);
    int mediaListDuration(const QList<OldMediaItem> &mediaList);
    QString mediaListDurationText(const QList<OldMediaItem> &mediaList);
    QList<OldMediaItem> mediaItemsDontExist(const QList<OldMediaItem> &mediaList);
    QString lriFilterFromMediaListField(const QList<OldMediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator);
    QList<OldMediaItem> mediaListFromSavedList(const OldMediaItem &savedListMediaItem);
    QList<OldMediaItem> mergeGenres(QList<OldMediaItem> genreList);
    QList<OldMediaItem> sortMediaList(QList<OldMediaItem> mediaList);
    OldMediaItem makeSubtitle(const OldMediaItem & mediaItem);
    bool isTemporaryAudioStream(const OldMediaItem &item);
    QUrl urlForFilex(QUrl url);
    QUrl decodedUrl(QUrl url);    

}

#endif // UTILITIES_MEDIAITEMS_H
