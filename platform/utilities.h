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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <KUrl>
#include <KIcon>
#include <QPixmap>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>


class MediaItem;
namespace Utilities {
    QPixmap getArtworkFromTag(QString url, QSize size = QSize(128,128));
    QPixmap getArtworkFromMediaItem(MediaItem mediaItem);
    QString getArtistFromTag(QString url);
    QString getAlbumFromTag(QString url);
    QString getTitleFromTag(QString url);
    QString getGenreFromTag(QString genre);
    int getYearFromTag(QString url);
    int getDurationFromTag(QString url);
    int getTrackNumberFromTag(QString url);
    bool saveArtworkToTag(QString url, const QPixmap *pixmap);
    bool saveArtworkToTag(QString url, QString imageUrl);
    void setArtistTag(QString url, QString artist);
    void setAlbumTag(QString url, QString album);
    void setTitleTag(QString url, QString title);
    void setGenreTag(QString url, QString genre);
    void setYearTag(QString url, int year);
    void setDurationTag(QString url, int duration);
    void setTrackNumberTag(QString url, int trackNumber);
    bool isMusic(QString url);
    bool isAudio(QString url);
    bool isVideo(QString url);
    QPixmap reflection(QPixmap &pixmap);
    void shadowBlur(QImage &image, int radius, const QColor &color);
    MediaItem mediaItemFromUrl(KUrl url);
    QStringList mediaListUrls(QList<MediaItem> mediaList);
    KIcon turnIconOff(KIcon icon, QSize size);
    TagLib::ID3v2::AttachedPictureFrame *attachedPictureFrame(TagLib::ID3v2::Tag *id3Tag, bool create = false);
}
#endif //UTILITIES_H    