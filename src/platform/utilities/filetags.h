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

#ifndef UTILITIES_FILETAGS_H
#define UTILITIES_FILETAGS_H

#include "../mediaitemmodel.h"
#include <KUrl>
#include <KIcon>
#include <KMimeType>

#include <QPixmap>
#include <QImage>

#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/attachedpictureframe.h>

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    QPixmap getArtworkFromTag(const QString &url, QSize size = QSize(128,128));
    QImage getArtworkImageFromTag(const QString &url, QSize size = QSize(128,128));
    QString tagType(const QString &url);
    MediaItem getAllInfoFromTag(const QString &url, MediaItem templateItem = MediaItem());
    QString getArtistFromTag(const QString &url);
    QString getAlbumFromTag(const QString &url);
    QString getTitleFromTag(const QString &url);
    QString getGenreFromTag(const QString &genre);
    int getYearFromTag(const QString &url);
    int getDurationFromTag(const QString &url);
    int getTrackNumberFromTag(const QString &url);
    QStringList getID3V2TextFrameFields(TagLib::ID3v2::Tag *id3v2, const TagLib::ByteVector &type);
    QStringList getXiphTextFields(TagLib::Ogg::XiphComment *xiph, const TagLib::ByteVector &type);
    void saveAllInfoToTag(const QList<MediaItem> &mediaList);
    bool saveArtworkToTag(const QString &url, const QPixmap *pixmap);
    bool saveArtworkToTag(const QString &url, const QString &imageUrl);
    void setArtistTag(const QString &url, const QString &artist);
    void setAlbumTag(const QString &url, const QString &album);
    void setTitleTag(const QString &url, const QString &title);
    void setGenreTag(const QString &url, const QString &genre);
    void setYearTag(const QString &url, int year);
    void setDurationTag(const QString &url, int duration);
    void setTrackNumberTag(const QString &url, int trackNumber);
    void setID3V2TextFrameFields(TagLib::ID3v2::Tag *id3v2, const TagLib::ByteVector &type, const QStringList &values);
    void setXiphTextFields(TagLib::Ogg::XiphComment *xiph, const TagLib::ByteVector &type, const QStringList &values);
    TagLib::ID3v2::AttachedPictureFrame *attachedPictureFrame(TagLib::ID3v2::Tag *id3Tag, bool create = false);
    QHash<int, QString> tagGenreDictionary();
    QString genreFromRawTagGenre(QString rawTagGenre);
    QString rawTagGenreFromGenre(QString genre);
    QStringList genresFromRawTagGenres(QStringList rawTagGenres);
    QStringList rawTagGenresFromGenres(QStringList genres);
    QString genreFilter(QString genre);
}

#endif //UTILITIES_FILETAGS_H
