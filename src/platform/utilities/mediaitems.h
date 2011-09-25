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

#ifndef UTILITIES_MEDIAITEMS_H
#define UTILITIES_MEDIAITEMS_H

#include <KUrl>
#include <Soprano/QueryResultIterator>
#include <Nepomuk/Resource>

class MediaItem;

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    MediaItem getArtistCategoryItem(const QString &artist);
    MediaItem mediaItemFromUrl(KUrl url, bool preferFileMetaData = false);
    QStringList mediaListUrls(const QList<MediaItem> &mediaList);
    int mediaListDuration(const QList<MediaItem> &mediaList);
    QString mediaListDurationText(const QList<MediaItem> &mediaList);
    QList<MediaItem> mediaItemsDontExist(const QList<MediaItem> &mediaList);
    MediaItem mediaItemFromNepomuk(Nepomuk::Resource res, const QString &sourceLri = QString());
    MediaItem mediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &sourceLri = QString());
    MediaItem categoryMediaItemFromNepomuk(Nepomuk::Resource &res, const QString &type, const QString &sourceLri = QString());
    MediaItem categoryMediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &lri = QString(), const QString &sourceLri = QString());
    Nepomuk::Resource mediaResourceFromUrl(KUrl Url);
    QString lriFilterFromMediaListField(const QList<MediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator);
    QList<MediaItem> mediaListFromSavedList(const MediaItem &savedListMediaItem);
    MediaItem completeMediaItem(const MediaItem & sourceMediaItem);
    QList<MediaItem> mergeGenres(QList<MediaItem> genreList);
    QList<MediaItem> sortMediaList(QList<MediaItem> mediaList);
    MediaItem makeSubtitle(const MediaItem & mediaItem);
    QStringList getLinksForResource(Nepomuk::Resource &res);
    bool isTemporaryAudioStream(const MediaItem &item);
    KUrl urlForFilex(KUrl url);
    KUrl decodedUrl(QUrl url);
}

#endif // UTILITIES_MEDIAITEMS_H
