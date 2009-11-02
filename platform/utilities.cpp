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

#include "utilities.h"
#include "mediaitemmodel.h"
#include "mediavocabulary.h"

#include <KUrl>
#include <KMimeType>
#include <KIcon>
#include <KIconEffect>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>

#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QPainter>
#include <QImage>
#include <QTime>

#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include "blur.cpp"

QPixmap Utilities::getArtworkFromTag(QString url, QSize size)
{
    TagLib::MPEG::File mpegFile(KUrl(url).path().toUtf8());
    TagLib::ID3v2::Tag *id3tag = mpegFile.ID3v2Tag(false);
    
    if (!id3tag) {
        return QPixmap();
    }

    TagLib::ID3v2::AttachedPictureFrame *selectedFrame = Utilities::attachedPictureFrame(id3tag);
    
    if (!selectedFrame) { // Could occur for encrypted picture frames.
        return QPixmap();
    }
    
    QByteArray pictureData = QByteArray(selectedFrame->picture().data(), selectedFrame->picture().size());
    QImage attachedImage = QImage::fromData(pictureData);
    
    if(size != attachedImage.size()) {
        attachedImage = attachedImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return QPixmap::fromImage(attachedImage);
}

QPixmap Utilities::getArtworkFromMediaItem(MediaItem mediaItem)
{
    QPixmap pixmap = QPixmap();
    if (Utilities::isMusic(mediaItem.url)) {
        pixmap = Utilities::getArtworkFromTag(mediaItem.url);
    }
    if (pixmap.isNull()) {
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            pixmap = QPixmap(KUrl(artworkUrl).path()).scaled(128,128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    return pixmap;
}

QString Utilities::getArtistFromTag(QString url)
{
    QString artist;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        artist  = TStringToQString(file.tag()->artist()).trimmed();
    }
    return artist;
}

QString Utilities::getAlbumFromTag(QString url)
{
    QString album;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        album = TStringToQString(file.tag()->album()).trimmed();
    }
    return album;
}

QString Utilities::getTitleFromTag(QString url)
{
    QString title;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        title = TStringToQString(file.tag()->title()).trimmed();
    }
    return title;
}

QString Utilities::getGenreFromTag(QString url)
{
    QString genre;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        genre   = TStringToQString(file.tag()->genre()).trimmed();
    }
    return genre;
}

int Utilities::getYearFromTag(QString url)
{
    int year = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        year = file.tag()->year();
    }
    return year;
}

int Utilities::getDurationFromTag(QString url)
{
    int duration = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        duration = file.audioProperties()->length();
    }
    return duration;
}

int Utilities::getTrackNumberFromTag(QString url)
{
    int track = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        track   = file.tag()->track();
    }
    return track;
}

bool Utilities::saveArtworkToTag(QString url, const QPixmap *pixmap)
{
    //FIXME:: HELP! Can't figure out why this doesn't work
    TagLib::MPEG::File mpegFile(KUrl(url).path().toUtf8());
    TagLib::ID3v2::Tag *id3tag = mpegFile.ID3v2Tag(true);
    
    TagLib::ID3v2::AttachedPictureFrame *frame = Utilities::attachedPictureFrame(id3tag, true);
    
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pixmap->save(&buffer, "PNG");
    //TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame();
    frame->setMimeType(TagLib::String("image/png"));
    frame->setPicture(TagLib::ByteVector(data.data(), data.size()));
    frame->setDescription("Cover Image");
    //id3tag->removeFrames("APIC");
    //id3tag->addFrame(frame);
    return mpegFile.save();
}

bool Utilities::saveArtworkToTag(QString url, QString imageurl)
{
    //QByteArray filePath = QFile::encodeName(KUrl(url).path());
    TagLib::MPEG::File mpegFile(KUrl(url).path().toUtf8());
    TagLib::ID3v2::Tag *id3tag = mpegFile.ID3v2Tag(true);
    
    TagLib::ID3v2::AttachedPictureFrame *frame = Utilities::attachedPictureFrame(id3tag, true);
    
    QFile file(KUrl(imageurl).path());
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    
    //TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame();
    frame->setMimeType(TagLib::String("image/png"));
    frame->setPicture(TagLib::ByteVector(data.data(), data.size()));
    frame->setDescription("Cover Image");
    //id3tag->removeFrames("APIC");
    //id3tag->addFrame(frame);
    return mpegFile.save();
}

void Utilities::setArtistTag(QString url, QString artist)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        file.tag()->setArtist(tArtist);
        file.save();
    }
}

void Utilities::setAlbumTag(QString url, QString album)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        file.tag()->setAlbum(tAlbum);
        file.save();
    }
}

void Utilities::setTitleTag(QString url, QString title)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        file.tag()->setTitle(tTitle);
        file.save();
    }
}

void Utilities::setGenreTag(QString url, QString genre)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        file.tag()->setGenre(tGenre);
        file.save();
    }
}

void Utilities::setYearTag(QString url, int year)
{
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        file.tag()->setYear(year);
        file.save();
    }
}

void Utilities::setTrackNumberTag(QString url, int trackNumber)
{
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toUtf8());
        file.tag()->setTrack(trackNumber);
        file.save();
    }
}

bool Utilities::isMusic(QString url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return result->is("audio/mpeg") || result->is("application/ogg") || result->is("audio/x-flac") || result->is("audio/x-musepack");
}

bool Utilities::isAudio(QString url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    return result->is("audio/mpeg") || result->is("audio/mp4") || result->is("audio/ogg") || result->is("audio/vorbis") || result->is("audio/aac") || result->is("audio/aiff") || result->is("audio/basic") || result->is("audio/flac") || result->is("audio/mp2") || result->is("audio/mp3") || result->is("audio/vnd.rn-realaudio") || result->is("audio/wav") || result->is("application/ogg") || result->is("audio/x-flac") || result->is("audio/x-musepack");
}

bool Utilities::isVideo(QString url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return result->is("video/mp4") || result->is("video/mpeg") || result->is("video/ogg") || result->is("video/quicktime") || result->is("video/msvideo") || result->is("video/x-theora")|| result->is("video/x-theora+ogg") || result->is("video/x-ogm")|| result->is("video/x-ogm+ogg") || result->is("video/divx") || result->is("video/x-msvideo") || result->is("video/x-wmv") || result->is("video/x-flv") || result->is("video/x-flv");
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
    transBlack.setAlpha(160);
    linearGrad.setColorAt(0, transBlack);
    linearGrad.setColorAt(0.55, Qt::transparent);
    QBrush brush(linearGrad);
    painter1.fillRect(0, 0, pixmap.width(), pixmap.height(), brush);
    painter1.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter1.drawPixmap(QPoint(0,0), reflection);
    painter1.end();
    return alphamask;
}

void Utilities::shadowBlur(QImage &image, int radius, const QColor &color)
{
    if (radius < 1) {
        return;
    }
    
    expblur<16, 7>(image, radius);
    
    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), color);
    p.end();
}

MediaItem Utilities::mediaItemFromUrl(KUrl url)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    MediaItem mediaItem;
    url.cleanPath();
    url = QUrl::fromPercentEncoding(url.url().toUtf8());
    mediaItem.url = url.url();
    mediaItem.title = url.fileName();
    
    mediaItem.artwork = KIcon("audio-x-generic"); //Assume audio unless we can tell otherwise
    mediaItem.type = "Audio"; 
    
    if (isMusic(mediaItem.url)) {
        mediaItem.artwork = KIcon("audio-mp4");
        mediaItem.type = "Audio";
        TagLib::FileRef file(KUrl(mediaItem.url).path().toUtf8());
        if (!file.isNull()) {
            QString title = TStringToQString(file.tag()->title()).trimmed();
            QString artist  = TStringToQString(file.tag()->artist()).trimmed();
            QString album = TStringToQString(file.tag()->album()).trimmed();
            QString genre   = TStringToQString(file.tag()->genre()).trimmed();
            int track   = file.tag()->track();
            int duration = file.audioProperties()->length();
            if (!title.isEmpty()) {
                mediaItem.title = title;
            }
            mediaItem.subTitle = artist + QString(" - ") + album;
            mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
            mediaItem.fields["title"] = title;
            mediaItem.fields["artist"] = artist;
            mediaItem.fields["album"] = album;
            mediaItem.fields["genre"] = genre;
            mediaItem.fields["trackNumber"] = track;
        }
        mediaItem.fields["audioType"] = "Music";
        Nepomuk::Resource res(mediaItem.url);
        if (res.exists()) {
            mediaItem.fields["rating"] = res.rating();
        }
    }
    if (isVideo(mediaItem.url)){
        mediaItem.artwork = KIcon("video-x-generic");
        mediaItem.type = "Video";
        mediaItem.fields["url"] = mediaItem.url;
        mediaItem.fields["title"] = mediaItem.title;
        Nepomuk::Resource res(mediaItem.url);
        if (res.exists()) {
            QString title = res.property(mediaVocabulary.title()).toString();
            if (!title.isEmpty()) {
                mediaItem.title = title;
                mediaItem.fields["title"] = title;
            }
            QString description = res.property(mediaVocabulary.description()).toString();
            if (!description.isEmpty()) {
                mediaItem.fields["description"] = description;
            }
            if (res.hasProperty(mediaVocabulary.videoIsMovie())) {
                if (res.property(mediaVocabulary.videoIsMovie()).toBool()) {
                    mediaItem.artwork = KIcon("tool-animator");
                    mediaItem.fields["videoType"] = "Movie";
                    QString seriesName = res.property(mediaVocabulary.videoSeriesName()).toString();
                    if (!seriesName.isEmpty()) {
                        mediaItem.fields["seriesName"] = seriesName;
                        mediaItem.subTitle = seriesName;
                    }
                }
            } else if (res.hasProperty(mediaVocabulary.videoIsTVShow())) {
                if (res.property(mediaVocabulary.videoIsTVShow()).toBool()) {
                    mediaItem.artwork = KIcon("video-television");
                    mediaItem.fields["videoType"] = "TV Show";
                }
                QString seriesName = res.property(mediaVocabulary.videoSeriesName()).toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                    mediaItem.subTitle = seriesName;
                }
                int season = res.property(mediaVocabulary.videoSeriesSeason()).toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle = QString("%1 - Season %2")
                        .arg(mediaItem.subTitle)
                        .arg(season);
                    } else {
                        mediaItem.subTitle = QString("Season %1").arg(season);
                    }
                }
                int episode = res.property(mediaVocabulary.videoSeriesEpisode()).toInt();
                if (episode !=0 ) {
                    mediaItem.fields["episode"] = episode;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle = QString("%1 - Episode %2")
                        .arg(mediaItem.subTitle)
                        .arg(episode);
                    } else {
                        mediaItem.subTitle = QString("Episode %2").arg(episode);
                    }
                }
            } else {
                mediaItem.fields["videoType"] = "Video Clip";
                mediaItem.artwork = KIcon("video-x-generic");
            }
            Nepomuk::Resource res(mediaItem.url);
            mediaItem.fields["rating"] = res.rating();
        }
    }
    return mediaItem;
}

QStringList Utilities::mediaListUrls(QList<MediaItem> mediaList)
{
    QStringList urls;
    for (int i = 0; i < mediaList.count(); i++) {
        urls << mediaList.at(i).url;
    }
    return urls;
}

KIcon Utilities::turnIconOff(KIcon icon, QSize size)
{
    QImage image = KIcon(icon).pixmap(size).toImage();
    KIconEffect::toGray(image, 0.8);
    return KIcon(QPixmap::fromImage(image));
}

TagLib::ID3v2::AttachedPictureFrame *Utilities::attachedPictureFrame(TagLib::ID3v2::Tag *id3tag, bool create)
{
    // Look for attached picture frames.
    TagLib::ID3v2::FrameList frames = id3tag->frameListMap()["APIC"];
    
    if (frames.isEmpty()) {
        if (create) {
            TagLib::ID3v2::AttachedPictureFrame *selectedFrame = new TagLib::ID3v2::AttachedPictureFrame();
            id3tag->addFrame(selectedFrame);
            return selectedFrame;
        } else {
            return 0;
        }
    }
    
    // According to the spec attached picture frames have different types.
    // So we should look for the corresponding picture depending on what
    // type of image (i.e. front cover, file info) we want.  If only 1
    // frame, just return that (scaled if necessary).
    
    TagLib::ID3v2::AttachedPictureFrame *selectedFrame = 0;
    
    if (frames.size() != 1) {
        TagLib::ID3v2::FrameList::Iterator it = frames.begin();
        for (; it != frames.end(); ++it) {
            
            // This must be dynamic_cast<>, TagLib will return UnknownFrame in APIC for
            // encrypted frames.
            TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);
            
            // Both thumbnail and full size should use FrontCover, as
            // FileIcon may be too small even for thumbnail.
            if (frame && frame->type() != TagLib::ID3v2::AttachedPictureFrame::FrontCover) {
                continue;
            }
            
            selectedFrame = frame;
            break;
        }
    }
    
    // If we get here we failed to pick a picture, or there was only one,
    // so just use the first picture.
    
    if (!selectedFrame) {
        selectedFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
    }
    
    if (!selectedFrame) { // Could occur for encrypted picture frames.
        if (create) {
            TagLib::ID3v2::AttachedPictureFrame *selectedFrame = new TagLib::ID3v2::AttachedPictureFrame();
            id3tag->addFrame(selectedFrame);
            return selectedFrame;
        } else {
            return 0;
        }
    }
    
    return selectedFrame;
}
