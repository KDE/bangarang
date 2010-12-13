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

#ifndef UTILITIES_ARTWORK_H
#define UTILITIES_ARTWORK_H

#include <KUrl>
#include <KIcon>
#include <KMimeType>
#include <QPixmap>
#include <QImage>

class MediaItem;

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {

    /**
    * Getting the album cover from an image stored within the same location as the media item (url).
    * Returns the path (without url.scheme) to the image or an empty string.
    */
    QString getArtworkUrlFromExternalImage(const QString& url, const QString& album = QString());

    QPixmap getArtworkFromMediaItem(const MediaItem &mediaItem);
    QImage getArtworkImageFromMediaItem(const MediaItem &mediaItem);
    QImage getAlbumArtwork(const QString &album);
    QList<QImage> getGenreArtworks(const QString &genre, const QString &type);
    QList<QImage> getArtistArtworks(const QString &artist);
    QList<QImage> getTagArtworks(const QString &tag, const QString &type);
    QList<QImage> getTVSeriesArtworks(const QString &seriesTitle);
    QList<QImage> getActorArtworks(const QString &actor);
    QList<QImage> getDirectorArtworks(const QString &director);
    bool compareImage(const QImage &image1, const QImage image2, int strength = 20);
    QString getGenreArtworkUrl(const QString &genre);
    QIcon defaultArtworkForMediaItem(const MediaItem &mediaItem);
    QPixmap reflection(QPixmap &pixmap);
    KIcon turnIconOff(KIcon icon, QSize size);
}
#endif // UTILITIES_ARTWORK_H
