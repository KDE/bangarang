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

#ifndef UTILITIES_ARTWORK_CPP
#define UTILITIES_ARTWORK_CPP

#include "general.h"
#include "artwork.h"
#include "typechecks.h"
#include "filetags.h"
#include "mediaitems.h"
#include "../mediaitemmodel.h"
#include "../mediaquery.h"
#include "../mediavocabulary.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KIconEffect>
#include <KStandardDirs>
#include <Soprano/QueryResultIterator>
#include <Nepomuk/ResourceManager>

#include <QtCore>
#include <QLinearGradient>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QMutexLocker>


QPixmap Utilities::getArtworkFromMediaItem(const MediaItem &mediaItem, bool ignoreCache)
{
    int artworkSize = 164;
    QPixmap pixmap = QPixmap();
    if (!ignoreCache && artworkIsInCache(mediaItem)) {
        QImage image = findArtworkInCache(mediaItem);
        pixmap = QPixmap::fromImage(image);
        return pixmap;
    }
    if (Utilities::isMusic(mediaItem.url)) {
        pixmap = Utilities::getArtworkFromTag(mediaItem.url);
    }
    if (mediaItem.subType() == "Album") {
        QImage image = getAlbumArtwork(mediaItem.fields["title"].toString());
        if (!image.isNull()) {
            pixmap = QPixmap::fromImage(image);
        }
    }
    if (pixmap.isNull()) {
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            QPixmap rawPixmap= QPixmap(KUrl(artworkUrl).path());
            if (!rawPixmap.isNull()) {
                //Always scale height to artwork size
                qreal widthScale = (qreal)artworkSize/rawPixmap.height();
                int width = rawPixmap.width()*widthScale;
                pixmap = rawPixmap.scaled(width, artworkSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }

        if (pixmap.isNull()) {
            artworkUrl = Utilities::getArtworkUrlFromExternalImage(mediaItem.url, mediaItem.fields["album"].toString());
            if (!artworkUrl.isEmpty()) {
                QPixmap rawPixmap= QPixmap(artworkUrl);
                if (!rawPixmap.isNull()) {
                    //Always scale height to artwork size
                    qreal widthScale = (qreal)artworkSize/rawPixmap.height();
                    int width = rawPixmap.width()*widthScale;
                    pixmap = rawPixmap.scaled(width, artworkSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }
        }
    }
    if (!ignoreCache) {
        updateImageCache(mediaItem, pixmap.toImage());
    }
    return pixmap;
}

QImage Utilities::getArtworkImageFromMediaItem(const MediaItem &mediaItem, bool ignoreCache)
{
    int artworkSize = 164;
    QImage image = QImage();
    if (!ignoreCache && artworkIsInCache(mediaItem)) {
        image = findArtworkInCache(mediaItem);
        return image;
    }
    if (Utilities::isMusic(mediaItem.url)) {
        image = Utilities::getArtworkImageFromTag(mediaItem.url);
    }
    if (mediaItem.subType() == "Album") {
        image = getAlbumArtwork(mediaItem.fields["title"].toString());
    }
    if (image.isNull()) {
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            QImage rawImage= QImage(KUrl(artworkUrl).path());
            if (!rawImage.isNull()) {
                //Always scale height to artwork size
                qreal widthScale = (qreal)artworkSize/rawImage.height();
                int width = rawImage.width()*widthScale;
                image = rawImage.scaled(width, artworkSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        } else {
            artworkUrl = Utilities::getArtworkUrlFromExternalImage(mediaItem.url, mediaItem.fields["album"].toString());
            if (!artworkUrl.isEmpty()) {
                QImage rawImage = QImage(artworkUrl);
                if (!rawImage.isNull()) {
                    //Always scale height to artwork size
                    qreal widthScale = (qreal)artworkSize/rawImage.height();
                    int width = rawImage.width()*widthScale;
                    image = rawImage.scaled(width, artworkSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }
        }
    }
    if (!ignoreCache) {
        updateImageCache(mediaItem, image);
    }
    return image;
}

QImage Utilities::getAlbumArtwork(const QString &album, bool ignoreCache)
{
    if (album.isEmpty()) {
        return QImage();
    }

    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    bindings.append(mediaVocabulary.artworkBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required, album, MediaQuery::Equal));
    query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
    query.endWhere();
    query.addLimit(5);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    while( it.next() ) {
        MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString("Music"));
        QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
        if (!artwork.isNull()) {
            return artwork;
        }
    }
    return QImage();
}

QList<QImage> Utilities::getGenreArtworks(const QString &genre, const QString &type, bool ignoreCache)
{
    QList<QImage> artworks;
    if (genre.isEmpty()) {
        return artworks;
    }

    if (type == "audio" || type == "music") {
        //Get album artworks associated with genre
        MediaVocabulary mediaVocabulary;
        MediaQuery query;
        QStringList bindings;
        bindings.append(mediaVocabulary.musicAlbumTitleBinding());
        query.select(bindings, MediaQuery::Distinct);
        query.startWhere();
        query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
        query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required));
        query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required,
                                                    Utilities::genreFilter(genre),
                                                    MediaQuery::Equal));
        query.endWhere();
        QStringList orderByBindings(mediaVocabulary.musicAlbumTitleBinding());
        query.orderBy(orderByBindings);
        query.addLimit(8);
        Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

        //Build media list from results
        while( it.next() ) {
            QString album = it.binding(mediaVocabulary.musicAlbumTitleBinding()).literal().toString().trimmed();
            if (!album.isEmpty()) {
                QImage artwork = getAlbumArtwork(album, ignoreCache);
                if (!artwork.isNull()) {
                    artworks.append(artwork);
                }
            }
        }
    } else if (type == "video") {
        MediaVocabulary mediaVocabulary;
        MediaQuery query;
        QStringList bindings;
        bindings.append(mediaVocabulary.mediaResourceBinding());
        bindings.append(mediaVocabulary.mediaResourceUrlBinding());
        bindings.append(mediaVocabulary.artworkBinding());
        query.select(bindings, MediaQuery::Distinct);
        query.startWhere();
        query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
        query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasGenre(MediaQuery::Required, genre, MediaQuery::Equal));
        query.endWhere();
        query.addLimit(5);
        Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

        while( it.next() ) {
            MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString("Video Clip"));
            QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
            if (!artwork.isNull()) {
                artworks.append(artwork);
            }
        }
    }
    return artworks;
}

QList<QImage> Utilities::getArtistArtworks(const QString &artist, bool ignoreCache)
{
    QList<QImage> artworks;
    if (artist.isEmpty()) {
        return artworks;
    }

    //Get album artworks associated with artist
    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.musicAlbumTitleBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasMusicArtistName(MediaQuery::Required, artist, MediaQuery::Equal));
    query.endWhere();
    QStringList orderByBindings(mediaVocabulary.musicAlbumTitleBinding());
    query.orderBy(orderByBindings);
    query.addLimit(8);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    //Build media list from results
    while( it.next() ) {
        QString album = it.binding(mediaVocabulary.musicAlbumTitleBinding()).literal().toString().trimmed();
        if (!album.isEmpty()) {
            QImage artwork = getAlbumArtwork(album, ignoreCache);
            if (!artwork.isNull()) {
                artworks.append(artwork);
            }
        }
    }
    return artworks;
}

QList<QImage> Utilities::getTagArtworks(const QString &tag, const QString &type, bool ignoreCache)
{
    QList<QImage> artworks;
    if (tag.isEmpty()) {
        return artworks;
    }

    if (type == "audio" || type == "music") {
        //Get album artworks associated with tag
        MediaVocabulary mediaVocabulary;
        MediaQuery query;
        QStringList bindings;
        bindings.append(mediaVocabulary.musicAlbumTitleBinding());
        query.select(bindings, MediaQuery::Distinct);
        query.startWhere();
        query.addCondition(mediaVocabulary.hasTypeAudioMusic(MediaQuery::Required));
        query.addCondition(mediaVocabulary.hasMusicAlbumTitle(MediaQuery::Required));
        query.addCondition(mediaVocabulary.hasTag(MediaQuery::Required, tag, MediaQuery::Equal));
        query.endWhere();
        QStringList orderByBindings(mediaVocabulary.musicAlbumTitleBinding());
        query.orderBy(orderByBindings);
        query.addLimit(8);
        Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

        //Build media list from results
        while( it.next() ) {
            QString album = it.binding(mediaVocabulary.musicAlbumTitleBinding()).literal().toString().trimmed();
            if (!album.isEmpty()) {
                QImage artwork = getAlbumArtwork(album, ignoreCache);
                if (!artwork.isNull()) {
                    artworks.append(artwork);
                }
            }
        }
    } else if (type == "video") {
        MediaVocabulary mediaVocabulary;
        MediaQuery query;
        QStringList bindings;
        bindings.append(mediaVocabulary.mediaResourceBinding());
        bindings.append(mediaVocabulary.mediaResourceUrlBinding());
        bindings.append(mediaVocabulary.artworkBinding());
        query.select(bindings, MediaQuery::Distinct);
        query.startWhere();
        query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
        query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasTag(MediaQuery::Required, tag, MediaQuery::Equal));
        query.endWhere();
        query.addLimit(8);
        Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

        while( it.next() ) {
            MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString("Video Clip"));
            QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
            if (!artwork.isNull()) {
                artworks.append(artwork);
            }
        }
    }
    return artworks;
}

QList<QImage> Utilities::getTVSeriesArtworks(const QString &seriesTitle, bool ignoreCache)
{
    QList<QImage> artworks;
    if (seriesTitle.isEmpty()) {
        return artworks;
    }

    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    bindings.append(mediaVocabulary.artworkBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required, seriesTitle, MediaQuery::Equal));
    query.endWhere();
    query.addLimit(8);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    while( it.next() ) {
        MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString("TV Show"));
        QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
        if (!artwork.isNull()) {
            artworks.append(artwork);
        }
    }
    return artworks;
}

QList<QImage> Utilities::getTVSeasonArtworks(const QString &seriesTitle, int season, bool ignoreCache)
{
    QList<QImage> artworks;
    if (seriesTitle.isEmpty() || season < 1) {
        return artworks;
    }

    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    bindings.append(mediaVocabulary.artworkBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeVideoTVShow(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Required, seriesTitle, MediaQuery::Equal));
    query.addCondition((mediaVocabulary.hasVideoSeason(MediaQuery::Required, season, MediaQuery::Equal)));
    query.endWhere();
    query.addLimit(8);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    while( it.next() ) {
        MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString("TV Show"));
        QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
        if (!artwork.isNull()) {
            artworks.append(artwork);
        }
    }
    return artworks;
}

QList<QImage> Utilities::getActorArtworks(const QString &actor, bool ignoreCache)
{
    QList<QImage> artworks;
    if (actor.isEmpty()) {
        return artworks;
    }

    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    bindings.append(mediaVocabulary.artworkBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Required, actor, MediaQuery::Equal));
    query.endWhere();
    query.addLimit(8);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    while( it.next() ) {
        MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString());
        QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
        if (!artwork.isNull()) {
            artworks.append(artwork);
        }
    }
    return artworks;
}

QList<QImage> Utilities::getDirectorArtworks(const QString &director, bool ignoreCache)
{
    QList<QImage> artworks;
    if (director.isEmpty()) {
        return artworks;
    }
    MediaVocabulary mediaVocabulary;
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    bindings.append(mediaVocabulary.artworkBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasTypeAnyVideo(MediaQuery::Required));
    query.addCondition(mediaVocabulary.hasTitle(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasArtwork(MediaQuery::Optional));
    query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Required, director, MediaQuery::Equal));
    query.endWhere();
    query.addLimit(8);
    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    while( it.next() ) {
        MediaItem artworkMediaItem = Utilities::mediaItemFromIterator(it, QString());
        QImage artwork = Utilities::getArtworkImageFromMediaItem(artworkMediaItem, ignoreCache);
        if (!artwork.isNull()) {
            artworks.append(artwork);
        }
    }
    return artworks;
}


bool Utilities::compareImage(const QImage &image1, const QImage image2, int strength)
{
    //Do the gross comparisons first
    if (image1.size() != image2.size()) {
        return false;
    }

    int width = image1.width();
    int height = image1.height();
    int xCheckIncrement = strength*((1-width/2)/100) + width/2;
    if (xCheckIncrement == 0) {
        xCheckIncrement = 1;
    }
    int yCheckIncrement = strength*((1-height/2)/100) + height/2;
    if (yCheckIncrement == 0) {
        yCheckIncrement = 1;
    }
    bool same = true;
    for (int x = xCheckIncrement; x < width; x = x + xCheckIncrement) {
        for (int y = yCheckIncrement; y < height; y = y + yCheckIncrement) {
            //QColor pixel1(image1.color(image1.pixelIndex(x, y)));
            //QColor pixel2(image2.color(image2.pixelIndex(x, y)));
            QColor pixel1(image1.pixel(x, y));
            QColor pixel2(image2.pixel(x, y));
            if (!((pixel1.red() > pixel2.red() - 10) &&
                (pixel1.red() < pixel2.red() + 10) &&
                (pixel1.blue() > pixel2.blue() - 10) &&
                (pixel1.blue() < pixel2.blue() + 10) &&
                (pixel1.green() > pixel2.green() - 10) &&
                (pixel1.green() < pixel2.green() + 10) &&
                (pixel1.alpha() > pixel2.alpha() - 10) &&
                (pixel1.alpha() < pixel2.alpha() + 10))) {
                return false;
            }
        }
    }
    return same;
}

QString Utilities::getArtworkUrlFromExternalImage(const QString& url, const QString& album)
{
    if (url.isNull() || url.isEmpty()) {
        return QString();
    }

    QMutexLocker locker(&mutex);

    //kDebug() << "Url submitted:" << url;
    KUrl pathUrl(url);
    if (!pathUrl.isValid() ||
        !pathUrl.isLocalFile()) {
        return QString();
    }
    QString path = pathUrl.directory(KUrl::AppendTrailingSlash);
    if (path.isEmpty()) {
        return QString();
    }

    //kDebug() << "Directory determined:" << path;

    QDir dir(path);
    QStringList files = dir.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.gif" << "*.png");

    if (files.count() == 1) {
        //kDebug() << "Found 1 file:" << path + files[0];
        return path + files[0];
    } else if (files.count() >= 1) {
        for (int i = files.count() - 1; i >= 0; i--) {
            //TODO: find better match cases
            //since windows media player stores more then one file,
            //we are forced to choose the right one (e.g folder is better then
            //albumartsmall)
            if (files[i].contains(i18n("folder")) || files[i].contains("album")) {
                kDebug() << "Found multiple files, picking name [folder/album]:" << path + files[i];
                return path + files[i];
            }

            if (!album.isEmpty() && files[i].contains(album, Qt::CaseInsensitive))
                kDebug() << "Found multiple files, picking name [album name]:" << path + files[i];
                return path + files[i];
        }

        //still here? take the first one
        //kDebug() << "Found multiple files, picking first one:" << path + files[0];
        return path + files[0];
    }
    return QString();
}

QString Utilities::getGenreArtworkUrl(const QString &genre)
{
    if (genre.isEmpty()) {
        return QString();
    }
    QString artworkUrl;
    QString localGenreFile = KGlobal::dirs()->locateLocal("data","bangarang/genrerc", false);
    if (!localGenreFile.isEmpty()) {
        if (QFile::exists(localGenreFile)) {
            KConfig genreConfig(localGenreFile);
            KConfigGroup genreGroup(&genreConfig, genre);
            artworkUrl = KGlobal::dirs()->locate("data", genreGroup.readEntry("artworkUrl", "|||").trimmed());
            if (artworkUrl.isEmpty()) {
                KUrl testUrl(genreGroup.readEntry("artworkUrl", QString()));
                if (!testUrl.isEmpty() && testUrl.isLocalFile()) {
                    artworkUrl = testUrl.path();
                    if (!QFile::exists(artworkUrl)) {
                        artworkUrl = QString();
                    }
                }
            }
        }
    }
    if (artworkUrl.isEmpty()) {
        QString defaultGenreFile = KGlobal::dirs()->locate("data","bangarang/genrerc");
        if (!defaultGenreFile.isEmpty()) {
            KConfig genreConfig(defaultGenreFile);
            KConfigGroup genreGroup(&genreConfig, genre);
            artworkUrl = KGlobal::dirs()->locate("data", genreGroup.readEntry("artworkUrl", "|||").trimmed());
        }
    }
    return artworkUrl;
}

QIcon Utilities::defaultArtworkForMediaItem(const MediaItem &mediaItem)
{
    QIcon artwork;
    if (mediaItem.type == "Audio") {
        if (mediaItem.subType() == "Audio Clip") {
            artwork = KIcon("audio-x-generic");
        } else if (mediaItem.subType() == "Music") {
            artwork = KIcon("audio-mp4");
        } else if (mediaItem.subType() == "Audio Stream") {
            artwork = KIcon("text-html");
        }
    } else if (mediaItem.type == "Video") {
        if (mediaItem.subType() == "Video Clip") {
            artwork = KIcon("video-x-generic");
        } else if (mediaItem.subType() == "Movie") {
            artwork = KIcon("tool-animator");
        } else if (mediaItem.subType() == "TV Show") {
            artwork = KIcon("video-television");
        }
    } else if (mediaItem.type == "Category") {
        if (mediaItem.subType() == "Artist") {
            artwork = KIcon("system-users");
        } else if (mediaItem.subType() == "Album") {
            artwork = KIcon("media-optical-audio");
        } else if (mediaItem.subType().endsWith(" Feed")) {
            artwork = KIcon("application-rss+xml");
        } else if (mediaItem.subType() == "AudioGenre") {
            artwork = KIcon("flag-blue");
        } else if (mediaItem.subType() == "VideoGenre") {
            artwork = KIcon("flag-green");
        } else if (mediaItem.subType() == "Actor" ||
                   mediaItem.subType() == "Director" ||
                   mediaItem.subType() == "Writer" ||
                   mediaItem.subType() == "Producer") {
            artwork = KIcon("view-media-artist");
        }
    }

    return artwork;
}

QPixmap Utilities::reflection(QPixmap &pixmap)
{
    QMatrix flipMatrix;
    QPixmap reflection = pixmap.transformed(flipMatrix.scale(1, -1));
    QPixmap alphamask(pixmap.size());
    alphamask.fill(Qt::transparent);
    QPainter painter1(&alphamask);
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, pixmap.height()));
    QColor transBlack = Qt::black;
    transBlack.setAlpha(100);
    linearGrad.setColorAt(0, transBlack);
    transBlack.setAlpha(30);
    linearGrad.setColorAt(0.2, transBlack);
    linearGrad.setColorAt(0.32, Qt::transparent);
    QBrush brush(linearGrad);
    painter1.fillRect(0, 0, pixmap.width(), pixmap.height(), brush);
    painter1.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter1.drawPixmap(QPoint(0,0), reflection);
    painter1.end();
    return alphamask;
}

KIcon Utilities::turnIconOff(KIcon icon, QSize size)
{
    QImage image = KIcon(icon).pixmap(size).toImage();
    KIconEffect::toGray(image, 0.8);
    return KIcon(QPixmap::fromImage(image));
}

QImage Utilities::findArtworkInCache(const MediaItem & mediaItem)
{
    QString key = QString("%1:%2").arg(mediaItem.subType(), mediaItem.url);
    return imageCache.value(key, QImage());
}

bool Utilities::artworkIsInCache(const MediaItem &mediaItem)
{
    QString key = QString("%1:%2").arg(mediaItem.subType(), mediaItem.url);
    return imageCache.contains(key);
}

void Utilities::updateImageCache(const MediaItem &mediaItem, const QImage &image)
{
    QString key = QString("%1:%2").arg(mediaItem.subType(), mediaItem.url);
    imageCache.insert(key, image);
}

void Utilities::removeFromImageCache(const MediaItem &mediaItem)
{
    QString key = QString("%1:%2").arg(mediaItem.subType(), mediaItem.url);
    imageCache.remove(key);
}

void Utilities::clearSubTypesFromImageCache(const QString &subType)
{
    QStringList keys = imageCache.keys();
    for (int i = 0; i < keys.count(); i++ ) {
        if (keys.at(i).startsWith(QString("%1:").arg(subType))) {
            imageCache.remove(keys.at(i));
        }
    }
}

#endif //UTILITIES_ARTWORK_CPP

