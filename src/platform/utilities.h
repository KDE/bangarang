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
#include <KMimeType>
#include <QPixmap>
#include <QImage>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <Soprano/QueryResultIterator>
#include <Nepomuk/Resource>
#include <QModelIndex>
#include <phonon/Global>

namespace Phonon {
class MediaObject;
}


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
  
    QPixmap getArtworkFromTag(const QString &url, QSize size = QSize(128,128));
    QImage getArtworkImageFromTag(const QString &url, QSize size = QSize(128,128));
    QPixmap getArtworkFromMediaItem(const MediaItem &mediaItem);
    QImage getArtworkImageFromMediaItem(const MediaItem &mediaItem);
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
    bool isMusicMimeType(KMimeType::Ptr type);
    bool isAudio(const QString &url);
    bool isAudioMimeType(KMimeType::Ptr type);
    bool isVideo(const QString &url);
    bool isVideoMimeType(KMimeType::Ptr type);
    bool isM3u(const QString &url);
    bool isPls(const QString &url);
    bool isDvd(const QString &url);
    bool isCd(const QString &url);
    bool isDisc(const QString &url);
    bool isMediaItem(const QModelIndex *index);
    bool isMedia(const QString type);
    bool isCategory(const QString type);
    bool isFeed(const QString categoryType);
    bool isAudioStream(const QString audioType);
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
    MediaItem mediaItemFromNepomuk(Nepomuk::Resource res, const QString &sourceLri = QString());
    MediaItem mediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &sourceLri = QString());
    MediaItem categoryMediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &lri = QString(), const QString &sourceLri = QString());
    Nepomuk::Resource mediaResourceFromUrl(KUrl Url);
    QString lriFilterFromMediaListField(const QList<MediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator);
    QString mergeLRIs(const QString &lri, const QString &lriToMerge);
    QUrl artistResource(const QString &artistName);
    QList<MediaItem> mediaListFromSavedList(const QString &savedListLocation);
    MediaItem completeMediaItem(const MediaItem & sourceMediaItem);
    QString discUrl(Phonon::DiscType type, int title = -1, const QString& name = QString());
    int discTitleFromUrl(const QString &url);
    QString discNameFromUrl(const QString &url);
    Phonon::DiscType discTypeFromUrl(const QString &url);
    int discTitleFullDisc();
    QString discName( Phonon::MediaObject *mobj );
}
#endif //UTILITIES_H    