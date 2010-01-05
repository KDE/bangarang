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
#include <Soprano/QueryResultIterator>
#include <Nepomuk/Resource>


class MediaItem;

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    QPixmap getArtworkFromTag(const QString &url, QSize size = QSize(128,128));
    QPixmap getArtworkFromMediaItem(const MediaItem &mediaItem);
    QString getArtistFromTag(const QString &url);
    QString getAlbumFromTag(const QString &url);
    QString getTitleFromTag(const QString &url);
    QString getGenreFromTag(const QString &genre);
    int getYearFromTag(const QString &url);
    int getDurationFromTag(const QString &url);
    int getTrackNumberFromTag(const QString &url);
    bool saveArtworkToTag(const QString &url, const QPixmap *pixmap);
    bool saveArtworkToTag(const QString &url, const QString &imageUrl);
    void setArtistTag(const QString &url, const QString &artist);
    void setAlbumTag(const QString &url, const QString &album);
    void setTitleTag(const QString &url, const QString &title);
    void setGenreTag(const QString &url, const QString &genre);
    void setYearTag(const QString &url, int year);
    void setDurationTag(const QString &url, int duration);
    void setTrackNumberTag(const QString &url, int trackNumber);
    bool isMusic(const QString &url);
    bool isAudio(const QString &url);
    bool isVideo(const QString &url);
    bool isM3u(const QString &url);
    bool isPls(const QString &url);
    QPixmap reflection(QPixmap &pixmap);
    MediaItem mediaItemFromUrl(KUrl url);
    QStringList mediaListUrls(const QList<MediaItem> &mediaList);
    KIcon turnIconOff(KIcon icon, QSize size);
    TagLib::ID3v2::AttachedPictureFrame *attachedPictureFrame(TagLib::ID3v2::Tag *id3Tag, bool create = false);
    int mediaListDuration(const QList<MediaItem> &mediaList);
    QString mediaListDurationText(const QList<MediaItem> &mediaList);
    QList<MediaItem> mediaItemsDontExist(const QList<MediaItem> &mediaList);
    QString audioMimeFilter();
    QString videoMimeFilter();
    MediaItem mediaItemFromNepomuk(Nepomuk::Resource res);
    MediaItem mediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type);
}
#endif //UTILITIES_H    