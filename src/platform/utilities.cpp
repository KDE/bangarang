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
#include "mediaquery.h"

#include <KUrl>
#include <KEncodingProber>
#include <KMimeType>
#include <KIcon>
#include <KIconEffect>
#include <KLocale>
#include <KDebug>
#include <kio/netaccess.h>
#include <Solid/Device>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/Model>
#include <Nepomuk/Resource>
#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>

#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QPainter>
#include <QImage>
#include <QTime>
#include <phonon/backendcapabilities.h>
#include <phonon/MediaObject>

#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
//#include "blur.cpp"

QPixmap Utilities::getArtworkFromTag(const QString &url, QSize size)
{
    /*TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit());
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
    }*/
    QImage attachedImage = getArtworkImageFromTag(url, size);    
    return QPixmap::fromImage(attachedImage);
}

QImage Utilities::getArtworkImageFromTag(const QString &url, QSize size)
{
    TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit().constData());
    TagLib::ID3v2::Tag *id3tag = mpegFile.ID3v2Tag(false);
    
    if (!id3tag) {
        return QImage();
    }

    TagLib::ID3v2::AttachedPictureFrame *selectedFrame = Utilities::attachedPictureFrame(id3tag);
    
    if (!selectedFrame) { // Could occur for encrypted picture frames.
        return QImage();
    }
    
    QByteArray pictureData = QByteArray(selectedFrame->picture().data(), selectedFrame->picture().size());
    QImage attachedImage = QImage::fromData(pictureData);
    
    if (size != attachedImage.size() && !attachedImage.isNull()) {
        attachedImage = attachedImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return attachedImage;
}

QPixmap Utilities::getArtworkFromMediaItem(const MediaItem &mediaItem)
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
        
        if (pixmap.isNull()) {
            artworkUrl = Utilities::getArtworkUrlFromExternalImage(mediaItem.url, mediaItem.fields["album"].toString());
            if (!artworkUrl.isEmpty()) {
                QPixmap rawPixmap= QPixmap(artworkUrl);
                if (!pixmap.isNull()) {
                    pixmap = rawPixmap.scaled(128,128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }
	}
    }
    return pixmap;
}

QImage Utilities::getArtworkImageFromMediaItem(const MediaItem &mediaItem)
{
    QImage image = QImage();
    if (Utilities::isMusic(mediaItem.url)) {
        image = Utilities::getArtworkImageFromTag(mediaItem.url);
    }
    if (image.isNull()) {
        QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            image = QImage(KUrl(artworkUrl).path()).scaled(128,128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            artworkUrl = Utilities::getArtworkUrlFromExternalImage(mediaItem.url, mediaItem.fields["album"].toString());
            if (!artworkUrl.isEmpty()) {
                QImage rawImage = QImage(artworkUrl);
                if (!rawImage.isNull()) {
                    image = rawImage.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }
	}
    }
    return image;
}

QString Utilities::getArtworkUrlFromExternalImage(const QString& url, const QString& album)
{
  if (url.isNull() || url.isEmpty())
    return QString();
  
  const QString title = url.split("/").last();
  QString path = url;
  path.remove(title); // string containg an 'url'
  path = KUrl(path).path();
  QDir dir(path);
  
  QStringList files = dir.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.gif" << "*.png");
	  
  if (files.count() == 1)
    return path + files[0];
  else if (files.count() >= 1)
  {
    for (int i = files.count() - 1; i >= 0; i--)
    {
      //TODO: find better match cases
      //since windows media player stores more then one file,
      //we are forced to choose the right one (e.g folder is better then 
      //albumartsmall)
      if (files[i].contains(i18n("folder")) || files[i].contains("album"))
	return path + files[i];
      
      if (!album.isEmpty() && files[i].contains(album, Qt::CaseInsensitive))
	return path + files[i];
    }
    
    //still here? take the first one
    return path + files[0];
  }
  return QString();
}

QString Utilities::getArtistFromTag(const QString &url)
{
    QString artist;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        artist  = TStringToQString(file.tag()->artist()).trimmed();
    }
    return artist;
}

QString Utilities::getAlbumFromTag(const QString &url)
{
    QString album;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        album = TStringToQString(file.tag()->album()).trimmed();
    }
    return album;
}

QString Utilities::getTitleFromTag(const QString &url)
{
    QString title;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        title = TStringToQString(file.tag()->title()).trimmed();
    }
    return title;
}

QString Utilities::getGenreFromTag(const QString &url)
{
    QString genre;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        genre   = TStringToQString(file.tag()->genre()).trimmed();
    }
    return genre;
}

int Utilities::getYearFromTag(const QString &url)
{
    int year = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        year = file.tag()->year();
    }
    return year;
}

int Utilities::getDurationFromTag(const QString &url)
{
    int duration = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        duration = file.audioProperties()->length();
    }
    return duration;
}

int Utilities::getTrackNumberFromTag(const QString &url)
{
    int track = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        track   = file.tag()->track();
    }
    return track;
}

bool Utilities::saveArtworkToTag(const QString &url, const QPixmap *pixmap)
{
    TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit().constData());
    TagLib::ID3v2::Tag *id3tag = mpegFile.ID3v2Tag(true);
    
    TagLib::ID3v2::AttachedPictureFrame *frame = Utilities::attachedPictureFrame(id3tag, true);
    
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pixmap->save(&buffer, "PNG");
    frame->setMimeType(TagLib::String("image/png"));
    frame->setPicture(TagLib::ByteVector(data.data(), data.size()));
    frame->setDescription("Cover Image");
    return mpegFile.save();
}

bool Utilities::saveArtworkToTag(const QString &url, const QString &imageurl)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    if (result->is("audio/mpeg")) {
        TagLib::MPEG::File mpegFile(KUrl(url).path().toUtf8().constData());
        if (mpegFile.isValid()) {
            TagLib::ID3v2::Tag *id3tag = mpegFile.ID3v2Tag(true);

            TagLib::ID3v2::AttachedPictureFrame *frame = Utilities::attachedPictureFrame(id3tag, true);
            
            QFile file(KUrl(imageurl).path());
            file.open(QIODevice::ReadOnly);
            QByteArray data = file.readAll();
            
            KMimeType::Ptr result = KMimeType::findByUrl(KUrl(imageurl), 0, true);
            if (result->is("image/png")) {
                frame->setMimeType("image/png");
            } else if (result->is("image/jpeg")) {
                frame->setMimeType("image/jpeg");
            }
            
            frame->setPicture(TagLib::ByteVector(data.data(), data.size()));
            frame->setDescription("Cover Image");
            return mpegFile.save();
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void Utilities::setArtistTag(const QString &url, const QString &artist)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        file.tag()->setArtist(tArtist);
        file.save();
    }
}

void Utilities::setAlbumTag(const QString &url, const QString &album)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        file.tag()->setAlbum(tAlbum);
        file.save();
    }
}

void Utilities::setTitleTag(const QString &url, const QString &title)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        file.tag()->setTitle(tTitle);
        file.save();
    }
}

void Utilities::setGenreTag(const QString &url, const QString &genre)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        file.tag()->setGenre(tGenre);
        file.save();
    }
}

void Utilities::setYearTag(const QString &url, int year)
{
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        file.tag()->setYear(year);
        file.save();
    }
}

void Utilities::setTrackNumberTag(const QString &url, int trackNumber)
{
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
        file.tag()->setTrack(trackNumber);
        file.save();
    }
}

bool Utilities::isMusic(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return isMusicMimeType(result);
}

bool Utilities::isMusicMimeType(KMimeType::Ptr type)
{
    return type->is("audio/mpeg") || type->is("application/ogg") || type->is("audio/x-flac") || type->is("audio/x-musepack") || type->is("audio/x-oma") || type->is("audio/x-m4a") || type->is("audio/mp4") || type->is("audio/x-monkeys-audio") || type->is("audio/x-wv") || type->is("audio/x-ms-wma") || type->is("audio/aac") || type->is("audio/3gpp")  || type->is("audio/3gpp2");
}

bool Utilities::isAudio(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    return isAudioMimeType(result);
}

bool Utilities::isAudioMimeType(KMimeType::Ptr type)
{
    return type->is("audio/mpeg") || type->is("audio/mp4") || type->is("audio/ogg") || type->is("audio/vorbis") || type->is("audio/aac") || type->is("audio/aiff") || type->is("audio/basic") || type->is("audio/flac") || type->is("audio/mp2") || type->is("audio/mp3") || type->is("audio/vnd.rn-realaudio") || type->is("audio/wav") || type->is("application/ogg") || type->is("audio/x-flac") || type->is("audio/x-musepack") || type->is("audio/x-m4a") || type->is("audio/x-oma") || type->is("audio/x-monkeys-audio") || type->is("audio/x-wv") || type->is("audio/x-ms-wma") || type->is("audio/3gpp")  || type->is("audio/3gpp2");
}

bool Utilities::isVideo(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return isVideoMimeType(result);
}

bool Utilities::isVideoMimeType(KMimeType::Ptr type)
{
    return type->is("video/mp4") || type->is("video/mpeg") || type->is("video/ogg") || type->is("video/quicktime") || type->is("video/msvideo") || type->is("video/x-theora")|| type->is("video/x-theora+ogg") || type->is("video/x-ogm")|| type->is("video/x-ogm+ogg") || type->is("video/divx") || type->is("video/x-msvideo") || type->is("video/x-wmv") || type->is("video/x-flv") || type->is("video/x-matroska");
}

bool Utilities::isM3u(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return result->is("audio/m3u") || result->is("audio/x-mpegurl");
}

bool Utilities::isPls(const QString &url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return result->is("audio/x-scpls");
}

bool Utilities::isDvd(const KUrl& url)
{
    return (deviceTypeFromUrl(url) == "dvd");
}

bool Utilities::isCd(const KUrl& url)
{
    return (deviceTypeFromUrl(url) == "cd");
}

bool Utilities::isDisc(const KUrl& url)
{
    return (isDvd(url) || isCd(url));
}

bool Utilities::isMediaItem(const QModelIndex *index)
{
    QString type = index->data(MediaItem::TypeRole).toString();
    return Utilities::isMedia(type);

}

bool Utilities::isMedia(const QString& type)
{
   return (
            (type == "Audio") ||
            (type == "Video")
        );
}

bool Utilities::isFeed(const QString& categoryType)
{
   return (categoryType == "Audio Feed" || categoryType == "Video Feed");
}

bool Utilities::isAudioStream(const QString& audioType)
{
  return (audioType == "Audio Stream");
}

bool Utilities::isCategory(const QString& type)
{
   return (type == "Category");
}

bool Utilities::isMessage(const QString& type)
{
    return (type == "Message");
}

bool Utilities::isAction(const QString& type)
{
    return (type == "Action");
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

MediaItem Utilities::mediaItemFromUrl(const KUrl& url, bool preferFileMetaData)
{
    MediaItem mediaItem;
    if(isDisc(url)) {
        bool dvd = isDvd(url);
        QString album = dvd ? "DVD Video" : "Audio CD";
        QString discTitle = deviceNameFromUrl(url);
        int track = deviceTitleFromUrl(url);
        QString title;
        if (track != invalidTitle())
            title = i18n(dvd ? "Title %1" : "Track %1", track);
        else
            title = i18n("Full Disc");
        mediaItem.url = url.url();
        mediaItem.artwork = dvd ? KIcon("media-optical-dvd") : KIcon("media-optical-audio");
        mediaItem.title = title;
        if (discTitle.isEmpty() || !dvd)
            mediaItem.subTitle = album;
        else
            mediaItem.subTitle = i18nc("%1=Title of DVD", "DVD Video - %1", discTitle);
        mediaItem.fields["url"] = mediaItem.url;
        mediaItem.fields["title"] = mediaItem.title;
        if ( dvd )
            mediaItem.fields["videoType"] = "DVD Title";
        else
            mediaItem.fields["audioType"] = "CD Track";
        mediaItem.fields["album"] = album;
        mediaItem.type = dvd ? "Video" : "Audio";
        mediaItem.fields["trackNumber"] = track;
        return mediaItem;
    }
    
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    //url.cleanPath();
    //url = QUrl::fromPercentEncoding(url.url().toUtf8());

    if (url.isLocalFile() && (Utilities::isM3u(url.url()) || Utilities::isPls(url.url()))) {
        mediaItem.artwork = KIcon("view-list-text");
        mediaItem.url = QString("savedlists://%1").arg(url.url());
        mediaItem.title = url.fileName();
        mediaItem.type = "Category";
        return mediaItem;
    } 
    
    mediaItem.url = url.prettyUrl();
    mediaItem.title = url.fileName();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["title"] = mediaItem.title;
    
    //Determine type of file - nepomuk is primary source
    bool foundInNepomuk = false;
    if (nepomukInited() && !preferFileMetaData) {
        //Try to find the corresponding resource in Nepomuk
        Nepomuk::Resource res = mediaResourceFromUrl(url);
        if (res.exists() && (res.hasType(mediaVocabulary.typeAudio()) ||
            res.hasType(mediaVocabulary.typeAudioMusic()) ||
            res.hasType(mediaVocabulary.typeAudioStream()) || 
            res.hasType(mediaVocabulary.typeVideo()) ||
            res.hasType(mediaVocabulary.typeVideoMovie()) ||
            res.hasType(mediaVocabulary.typeVideoTVShow())) ) {
            mediaItem = mediaItemFromNepomuk(res);
            foundInNepomuk = true;
        }
    }
    
    if (!foundInNepomuk || mediaItem.type.isEmpty()) {
        if (isAudio(mediaItem.url)) {
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Audio Clip";
        }
        if (isMusic(mediaItem.url)) {
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Music";
        }
        if (isVideo(mediaItem.url)){
            mediaItem.type = "Video";
            mediaItem.fields["videoType"] = "Video Clip";
        }
        if (!url.isLocalFile()) {
            mediaItem.type = "Audio";
            mediaItem.fields["audioType"] = "Audio Stream";
        }
    }
    
    if (mediaItem.type == "Audio") {
        if (mediaItem.fields["audioType"] == "Audio Clip") {
            mediaItem.artwork = KIcon("audio-x-generic");
        } else if (mediaItem.fields["audioType"] == "Music") {
            mediaItem.artwork = KIcon("audio-mp4");
            //File metadata is always primary for music items.
            TagLib::FileRef file(KUrl(mediaItem.url).path().toLocal8Bit().constData());
            if (!file.isNull()) {
                QString title = TStringToQString(file.tag()->title()).trimmed();
                QString artist  = TStringToQString(file.tag()->artist()).trimmed();
                QString album   = TStringToQString(file.tag()->album()).trimmed();
                QString genre   = TStringToQString(file.tag()->genre()).trimmed();
                if (KUrl(mediaItem.url).path().endsWith(".mp3")) {
                    // detect encoding for mpeg id3v2
                    QString tmp = title + artist + album + genre;
                    KEncodingProber prober(KEncodingProber::Universal);
                    KEncodingProber::ProberState result = prober.feed(tmp.toAscii());
                    if (result != KEncodingProber::NotMe) {
                        QByteArray encodingname = prober.encoding().toLower();
                        if ( prober.confidence() > 0.47
                            && ( ( encodingname == "gb18030" )
                            || ( encodingname == "big5" )
                            || ( encodingname == "euc-kr" )
                            || ( encodingname == "euc-jp" )
                            || ( encodingname == "koi8-r" ) ) ) {
                            title = QTextCodec::codecForName(encodingname)->toUnicode(title.toAscii());
                            artist = QTextCodec::codecForName(encodingname)->toUnicode(artist.toAscii());
                            album = QTextCodec::codecForName(encodingname)->toUnicode(album.toAscii());
                            genre = QTextCodec::codecForName(encodingname)->toUnicode(genre.toAscii());
                        } else if ((prober.confidence() < 0.3 || encodingname != "utf-8")
                            && QTextCodec::codecForLocale()->name().toLower() != "utf-8") {
                            title = QTextCodec::codecForLocale()->toUnicode(title.toAscii());
                            artist = QTextCodec::codecForLocale()->toUnicode(artist.toAscii());
                            album = QTextCodec::codecForLocale()->toUnicode(album.toAscii());
                            genre = QTextCodec::codecForLocale()->toUnicode(genre.toAscii());
                        }
                    }
                }
                int track   = file.tag()->track();
                int duration = file.audioProperties()->length();
                int year = file.tag()->year();
                if (!title.isEmpty()) {
                    mediaItem.title = title;
                }
                mediaItem.subTitle = artist + QString(" - ") + album;
                mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                mediaItem.fields["duration"] = duration;
                mediaItem.fields["title"] = title;
                mediaItem.fields["artist"] = artist;
                mediaItem.fields["album"] = album;
                mediaItem.fields["genre"] = genreFromRawTagGenre(genre);
                mediaItem.fields["trackNumber"] = track;
                mediaItem.fields["year"] = year;
            }
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            mediaItem.artwork = KIcon("text-html");
            if (mediaItem.title.isEmpty()) {
                mediaItem.title = url.prettyUrl();
            }
        }
    } else if (mediaItem.type == "Video") {
        if (mediaItem.fields["videoType"] == "Video Clip") {
            mediaItem.artwork = KIcon("video-x-generic");
        }
    }

    //Lookup nepomuk metadata not stored with file
    if (nepomukInited() && preferFileMetaData) {
        Nepomuk::Resource res(KUrl(mediaItem.url));
        QUrl nieUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url");
        mediaItem.fields["rating"] = res.rating();
        QStringList tags;
        foreach (Nepomuk::Tag tag, res.tags()) {
            tags.append(tag.label());
        }
        mediaItem.fields["tags"] = tags.join(";");
        mediaItem.fields["playCount"] = res.property(mediaVocabulary.playCount()).toInt();
        if (res.property(mediaVocabulary.lastPlayed()).isValid()) {
            QDateTime lastPlayed = res.property(mediaVocabulary.lastPlayed()).toDateTime();
            if (lastPlayed.isValid()) {
                mediaItem.fields["lastPlayed"] = lastPlayed;
            }
        }
        mediaItem.fields["artworkUrl"] = res.property(mediaVocabulary.artwork()).toResource()
                                         .property(nieUrl).toString();

    }
    return mediaItem;
}

QStringList Utilities::mediaListUrls(const QList<MediaItem> &mediaList)
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

int Utilities::mediaListDuration(const QList<MediaItem> &mediaList) 
{
    int duration = 0;
    for (int i = 0; i < mediaList.count(); i++) {
        duration += mediaList.at(i).fields["duration"].toInt();
    }
    return duration;
}

QString Utilities::mediaListDurationText(const QList<MediaItem> &mediaList) 
{
    int duration = mediaListDuration(mediaList);
    int hours = duration/3600;
    int minutes = (duration - (hours*3600))/60;
    int seconds = duration - (hours*3600) - (minutes*60);
    QString min = minutes < 10 ? QString("0%1").arg(minutes): QString("%1").arg(minutes);
    QString sec = seconds < 10 ? QString("0%1").arg(seconds): QString("%1").arg(seconds);
    
    return QString("%1:%2:%3").arg(hours).arg(min).arg(sec);
}

QList<MediaItem> Utilities::mediaItemsDontExist(const QList<MediaItem> &mediaList)
{
    QList<MediaItem> items;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        bool dvdNotFound = false;
        QString url_string = mediaItem.url;
        if (isDisc(url_string)) {
            bool dvd = isDvd(url_string);
            continue;
            dvdNotFound = true;
            Q_UNUSED(dvd);
        }
        KUrl url = KUrl(url_string);
        if (dvdNotFound ||
            (url.isValid() && url.isLocalFile() && !QFile(url.path()).exists()) ||
            url_string.startsWith("trash:/")
           ) {
            mediaItem.exists = false;
            kDebug() << mediaItem.url << " missing";
            items << mediaItem;
        }
    }
    return items;
}

QString Utilities::audioMimeFilter()
{
    QStringList supportedList = Phonon::BackendCapabilities::availableMimeTypes().filter("audio");
    QStringList appList = Phonon::BackendCapabilities::availableMimeTypes().filter("application");
    QStringList ambiguousList;
    for (int i = 0; i < appList.count(); i++) {
        if (!appList.at(i).contains("video") && !appList.at(i).contains("audio")) {
            ambiguousList.append(appList.at(i));
        }
    }
    supportedList << ambiguousList;
    supportedList << "audio/m3u" << "audio/x-mpegurl" << "audio/x-scpls"; //add playlist mimetypes
    return supportedList.join(" ");
    /* This section might be useful if Phonon doesn't report 
     * supported mimetypes correctly. For now I'll assume it 
     * does so it is disabled. */
    /*QString mimeFilter = QString("audio/mpeg audio/mp4 audio/ogg audio/vorbis audio/aac audio/aiff audio/basic audio/flac audio/mp2 audio/mp3 audio/vnd.rn-realaudio audio/wav application/ogg audio/x-flac audio/x-musepack ");
    mimeFilter += supportedList.join(" ");
    return mimeFilter;*/
}

QString Utilities::videoMimeFilter()
{
    QStringList supportedList = Phonon::BackendCapabilities::availableMimeTypes().filter("video");
    QStringList appList = Phonon::BackendCapabilities::availableMimeTypes().filter("application");
    QStringList ambiguousList;
    for (int i = 0; i < appList.count(); i++) {
        if (!appList.at(i).contains("video") && !appList.at(i).contains("audio")) {
            ambiguousList.append(appList.at(i));
        }
    }
    supportedList << ambiguousList;
    return supportedList.join(" ");
    
    /* This section might be useful if Phonon doesn't report 
    * supported mimetypes correctly. For now I'll assume it 
    * does so it is disabled. */
    /*QString mimeFilter =  QString("video/mp4 video/mpeg video/ogg video/quicktime video/msvideo video/x-theora video/x-theora+ogg video/x-ogm video/x-ogm+ogg video/divx video/x-msvideo video/x-wmv video/x-flv video/flv");
    mimeFilter += supportedList.join(" ");
    return mimeFilter;*/
}

MediaItem Utilities::mediaItemFromNepomuk(Nepomuk::Resource res, const QString &sourceLri)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    QString type;
    //Check types beyond the current vocabulary to detect basic Audio type indexed by Strigi
    if (res.hasType(Soprano::Vocabulary::Xesam::Audio()) || 
        res.hasType(QUrl("http://www.semanticdesktop.org/ontologies/nfo#Audio"))) {
        type = "Audio Clip";
    }
    if (res.hasType(mediaVocabulary.typeAudioMusic())) {
        type = "Music";
    }
    if (res.hasType(mediaVocabulary.typeAudioStream())) {
        type = "Audio Stream";
    } 
    //Check types beyond the current vocabulary to detect basic Video type indexed by Strigi
    if (res.hasType(mediaVocabulary.typeVideo()) ||
        res.hasType(QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Video")) ||
        res.hasType(Soprano::Vocabulary::Xesam::Video())) { 
        type = "Video Clip";
    } 
    if (res.hasType(mediaVocabulary.typeVideoMovie())) {
        type = "Movie";
    } 
    if (res.hasType(mediaVocabulary.typeVideoTVShow())) {
        type = "TV Show";
    }

    //If nepomuk resource type is not recognized try recognition by mimetype
    QUrl nieUrl = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url");
    QString url = KUrl(res.property(nieUrl).toUrl()).prettyUrl();
    if (isAudio(url)) {
        type = "Audio Clip";
    }
    if (isMusic(url)) {
        type = "Music";
    }
    if (isVideo(url)){
        type = "Video Clip";
    }

    MediaItem mediaItem;

    mediaItem.url = KUrl(res.property(nieUrl).toUrl()).prettyUrl();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["resourceUri"] = res.resourceUri().toString();
    mediaItem.fields["sourceLri"] = sourceLri;
    mediaItem.title = res.property(mediaVocabulary.title()).toString();
    mediaItem.fields["title"] = mediaItem.title;
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
            mediaItem.fields["title"] = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
            mediaItem.fields["title"] = mediaItem.url;
        }
    }
    mediaItem.fields["description"] = res.property(mediaVocabulary.description()).toString();
    int duration = res.property(mediaVocabulary.duration()).toInt();
    if (duration != 0) {
        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
        mediaItem.fields["duration"] = duration;
    }
    QString genre = res.property(mediaVocabulary.genre()).toString();
    mediaItem.fields["genre"] = genreFromRawTagGenre(genre);
    mediaItem.fields["rating"] = res.rating();

    QStringList tags;
    foreach (Nepomuk::Tag tag, res.tags()) {
        tags.append(tag.label());
    }
    mediaItem.fields["tags"] = tags.join(";");
    mediaItem.fields["playCount"] = res.property(mediaVocabulary.playCount()).toInt();
    if (res.property(mediaVocabulary.lastPlayed()).isValid()) {
        QDateTime lastPlayed = res.property(mediaVocabulary.lastPlayed()).toDateTime();
        if (lastPlayed.isValid()) {
            mediaItem.fields["lastPlayed"] = lastPlayed;
        }
    }
    mediaItem.fields["artworkUrl"] = res.property(mediaVocabulary.artwork()).toResource()
                                     .property(nieUrl).toString();
    if (type == "Audio Clip" || type == "Audio Stream" || type == "Music") {
        mediaItem.type = "Audio";
        mediaItem.fields["audioType"] = type;
        mediaItem.artwork = KIcon("audio-x-wav");
        if (type == "Audio Stream") {
            mediaItem.artwork = KIcon("text-html");
        } else if (type == "Music") {
            mediaItem.artwork = KIcon("audio-mpeg");
            //TODO:: Multiple artists per resource
            QString artist = res.property(mediaVocabulary.musicArtist()).toResource()
                             .property(mediaVocabulary.musicArtistName()).toString();
            if (!artist.isEmpty()) {
                mediaItem.fields["artist"] = artist;
                mediaItem.subTitle = artist;
            }

            QString album = res.property(mediaVocabulary.musicAlbum()).toResource()
                            .property(mediaVocabulary.musicAlbumName()).toString();
            if (!album.isEmpty()) {
                mediaItem.fields["album"] = album;
                if (!artist.isEmpty()) {
                    mediaItem.subTitle += QString(" - %1").arg(album);
                } else {
                    mediaItem.subTitle = album;
                }
            }
            if (res.property(mediaVocabulary.musicAlbumYear()).isValid()) {
                QDate yearDate = res.property(mediaVocabulary.musicAlbumYear()).toDate();
                if (yearDate.isValid()) {
                    mediaItem.fields["year"] = yearDate.year();
                }
            }

            int trackNumber = res.property(mediaVocabulary.musicTrackNumber()).toInt();
            if (trackNumber != 0) {
                mediaItem.fields["trackNumber"] = trackNumber;
            }
        }
    } else if (type == "Video Clip" || type == "Movie" || type == "TV Show") {
        mediaItem.type = "Video";
        mediaItem.fields["videoType"] = type;
        mediaItem.artwork = KIcon("video-x-generic");
        if (type == "Movie" || type == "TV Show") {
            mediaItem.artwork = KIcon("tool-animator");
            if (res.property(mediaVocabulary.releaseDate()).isValid()) {
                QDate releaseDate = res.property(mediaVocabulary.releaseDate()).toDate();
                if (releaseDate.isValid()) {
                    mediaItem.fields["releaseDate"] = releaseDate;
                    mediaItem.fields["year"] = releaseDate.year();
                }
            }
            //TODO: Multiple writers/directors/producers/actors
            mediaItem.fields["writer"] = res.property(mediaVocabulary.videoWriter()).toResource()
                                         .property(mediaVocabulary.ncoFullname()).toString();
            mediaItem.fields["director"] = res.property(mediaVocabulary.videoDirector()).toResource()
                                           .property(mediaVocabulary.ncoFullname()).toString();
            mediaItem.fields["producer"] = res.property(mediaVocabulary.videoProducer()).toResource()
                                           .property(mediaVocabulary.ncoFullname()).toString();
            mediaItem.fields["actor"] = res.property(mediaVocabulary.videoActor()).toResource()
                                        .property(mediaVocabulary.ncoFullname()).toString();
            if (type == "TV Show") {
                mediaItem.artwork = KIcon("video-television");
                QString seriesName = res.property(mediaVocabulary.videoSeries()).toResource()
                                     .property(mediaVocabulary.videoSeriesTitle()).toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                    mediaItem.subTitle = seriesName;
                }

                int season = res.property(mediaVocabulary.videoSeason()).toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Season %1").arg(season);
                }

                int episodeNumber = res.property(mediaVocabulary.videoEpisodeNumber()).toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                }
            }
        }
    }

    return mediaItem;
}

MediaItem Utilities::mediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &sourceLri)
{
    MediaItem mediaItem;
    
    KUrl url = it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri().isEmpty() ? 
    it.binding(MediaVocabulary::mediaResourceBinding()).uri() :
    it.binding(MediaVocabulary::mediaResourceUrlBinding()).uri();
    mediaItem.url = url.prettyUrl();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["resourceUri"] = it.binding(MediaVocabulary::mediaResourceBinding()).uri().toString();
    mediaItem.fields["sourceLri"] = sourceLri;
    mediaItem.title = it.binding(MediaVocabulary::titleBinding()).literal().toString();
    mediaItem.fields["title"] = mediaItem.title;
    if (mediaItem.title.isEmpty()) {
        if (KUrl(mediaItem.url).isLocalFile()) {
            mediaItem.title = KUrl(mediaItem.url).fileName();
            mediaItem.fields["title"] = KUrl(mediaItem.url).fileName();
        } else {
            mediaItem.title = mediaItem.url;
            mediaItem.fields["title"] = mediaItem.url;
        }
    }
    mediaItem.fields["description"] = it.binding(MediaVocabulary::descriptionBinding()).literal().toString();
    int duration = it.binding(MediaVocabulary::durationBinding()).literal().toInt();
    if (duration != 0) {
        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
        mediaItem.fields["duration"] = duration;
    }
    QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
    mediaItem.fields["genre"] = genreFromRawTagGenre(genre);
    mediaItem.fields["rating"] = it.binding(MediaVocabulary::ratingBinding()).literal().toInt();
    Nepomuk::Resource res(it.binding(MediaVocabulary::mediaResourceBinding()).uri());
    QStringList tags;
    foreach (Nepomuk::Tag tag, res.tags()) {
        tags.append(tag.label());
    }
    mediaItem.fields["tags"] = tags.join(";");
    mediaItem.fields["playCount"] = it.binding(MediaVocabulary::playCountBinding()).literal().toInt();
    if (it.binding(MediaVocabulary::lastPlayedBinding()).isValid()) {
        QDateTime lastPlayed = it.binding(MediaVocabulary::lastPlayedBinding()).literal().toDateTime();
        if (lastPlayed.isValid()) {
            mediaItem.fields["lastPlayed"] = lastPlayed;
        }
    }
    mediaItem.fields["artworkUrl"] = it.binding(MediaVocabulary::artworkBinding()).uri().toString();
    if (type == "Audio Clip" || type == "Audio Stream" || type == "Music") {
        mediaItem.type = "Audio";
        mediaItem.fields["audioType"] = type;
        mediaItem.artwork = KIcon("audio-x-wav");
        if (type == "Audio Stream") {
            mediaItem.artwork = KIcon("text-html");
        } else if (type == "Music") {
            mediaItem.artwork = KIcon("audio-mpeg");
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            if (!artist.isEmpty()) {
                mediaItem.fields["artist"] = artist;
                mediaItem.subTitle = artist;
            }
            
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            if (!album.isEmpty()) {
                mediaItem.fields["album"] = album;
                if (!artist.isEmpty()) {
                    mediaItem.subTitle += QString(" - %1").arg(album);
                } else {
                    mediaItem.subTitle = album;
                }
            }
            if (it.binding(MediaVocabulary::musicAlbumYearBinding()).isValid()) {
                QDate yearDate = it.binding(MediaVocabulary::musicAlbumYearBinding()).literal().toDate();
                if (yearDate.isValid()) {
                    mediaItem.fields["year"] = yearDate.year();
                }
            }
            
            int trackNumber = it.binding(MediaVocabulary::musicTrackNumberBinding()).literal().toInt();
            if (trackNumber != 0) {
                mediaItem.fields["trackNumber"] = trackNumber;
            }
        }
    } else if (type == "Video Clip" || type == "Movie" || type == "TV Show") {
        mediaItem.type = "Video";
        mediaItem.fields["videoType"] = type;
        mediaItem.artwork = KIcon("video-x-generic");
        if (type == "Movie" || type == "TV Show") {
            mediaItem.artwork = KIcon("tool-animator");
            if (it.binding(MediaVocabulary::releaseDateBinding()).isValid()) {
                QDate releaseDate = it.binding(MediaVocabulary::releaseDateBinding()).literal().toDate();
                if (releaseDate.isValid()) {
                    mediaItem.fields["releaseDate"] = releaseDate;
                    mediaItem.fields["year"] = releaseDate.year();
                }
            }
            mediaItem.fields["writer"] = it.binding(MediaVocabulary::videoWriterBinding()).literal().toString();
            mediaItem.fields["director"] = it.binding(MediaVocabulary::videoDirectorBinding()).literal().toString();
            mediaItem.fields["producer"] = it.binding(MediaVocabulary::videoProducerBinding()).literal().toString();
            mediaItem.fields["actor"] = it.binding(MediaVocabulary::videoActorBinding()).literal().toString();
            if (type == "TV Show") {
                mediaItem.artwork = KIcon("video-television");
                QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                    mediaItem.subTitle = seriesName;
                }
                
                int season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Season %1").arg(season);
                }
                
                int episodeNumber = it.binding(MediaVocabulary::videoEpisodeNumberBinding()).literal().toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                }
            }
        }
    }
        
    return mediaItem;
}

MediaItem Utilities::categoryMediaItemFromIterator(Soprano::QueryResultIterator &it, const QString &type, const QString &lri, const QString &sourceLri)
{
    MediaItem mediaItem;
    
    if (type == "Artist" ||
         type == "Album" ||
         type == "AudioGenre" ||
         type == "AudioTag" ||
         type == "TV Series" ||
         type == "VideoGenre" ||
         type == "Actor" ||
         type == "Director"||
         type == "VideoTag") {
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = type;
        mediaItem.nowPlaying = false;
        
        if (type =="Artist") {
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            mediaItem.url = QString("music://albums?%1||%2||%3")
                            .arg(artistFilter)
                            .arg(albumFilter)
                            .arg(genreFilter);
            mediaItem.title = artist;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("system-users");
            //Provide context info for artist
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
        } else if (type == "Album") {
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            mediaItem.url = QString("music://songs?%1||%2||%3")
                            .arg(artistFilter)
                            .arg(albumFilter)
                            .arg(genreFilter);
            mediaItem.title = album;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.subTitle = artist;
            mediaItem.artwork = KIcon("media-optical-audio");
            //Provide context info for album
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
        } else if (type == "AudioGenre") {
            QString artist = it.binding(MediaVocabulary::musicArtistNameBinding()).literal().toString();
            QString album = it.binding(MediaVocabulary::musicAlbumTitleBinding()).literal().toString();
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString artistFilter = artist.isEmpty() ? QString(): QString("artist=%1").arg(artist);
            QString albumFilter = album.isEmpty() ? QString(): QString("album=%1").arg(album);
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            mediaItem.url = QString("music://artists?%1||%2||%3")
                            .arg(artistFilter)
                            .arg(albumFilter)
                            .arg(genreFilter);
            mediaItem.title = genre;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("flag-blue");
            //Provide context info for genre
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||artist=%1||album=%2||genre=%3").arg(artist).arg(album).arg(genre));
        } else if (type == "AudioTag") {
            QString tag = it.binding(MediaVocabulary::tagBinding()).literal().toString();
            QString tagFilter = tag.isEmpty() ? QString(): QString("tag=%1").arg(tag);
            mediaItem.url = QString("tag://audio?%1")
                            .arg(tagFilter);
            mediaItem.title = tag;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-pim-notes");;
            //Provide context info for tag
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?audio||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?audio||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?audio||limit=4||tag=%1").arg(tag));
        } else if (type == "TV Series") {
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
            QString season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toString();
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            QString seriesNameFilter = seriesName.isEmpty() ? QString(): QString("seriesName=%1").arg(seriesName);
            QString seasonFilter = season.isEmpty() ? QString(): QString("season=%1").arg(season);
            mediaItem.url = QString("video://seasons?||%1||%2||%3")
                            .arg(genreFilter)
                            .arg(seriesNameFilter)
                            .arg(seasonFilter);
            mediaItem.title = seriesName;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("video-television");
            //Provide context info for TV series
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||%1||seriesName=%2").arg(genreFilter).arg(seriesName));
        } else if (type == "TV Season") {
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
            QString season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toString();
            QString genreFilter = genre.isEmpty() ? QString(): QString("genre=%1").arg(genre);
            QString seriesNameFilter = seriesName.isEmpty() ? QString(): QString("seriesName=%1").arg(seriesName);
            QString seasonFilter = season.isEmpty() ? QString(): QString("season=%1").arg(season);
            mediaItem.url = QString("video://seasons?||%1||%2||%3")
                            .arg(genreFilter)
                            .arg(seriesNameFilter)
                            .arg(seasonFilter);
            mediaItem.title = seriesName;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.subTitle = i18nc("%1=Number of the Season", "Season %1", season);
            mediaItem.artwork = KIcon("video-television");
            //Provide context info for genre
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||%1||%2||season=%3").arg(genreFilter).arg(seriesNameFilter).arg(season));
        } else if (type == "VideoGenre") {
            QString genre = it.binding(MediaVocabulary::genreBinding()).literal().toString();
            mediaItem.url = QString("video://sources?||genre=%1").arg(genre);
            mediaItem.title = genre;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("flag-green");
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||genre=%1").arg(genre));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||genre=%1").arg(genre));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||genre=%1").arg(genre));
        } else if (type == "Actor") {
            QString actor = it.binding(MediaVocabulary::videoActorBinding()).literal().toString();
            mediaItem.url = QString("video://sources?||actor=%1").arg(actor);
            mediaItem.title = actor;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||actor=%1").arg(actor));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||actor=%1").arg(actor));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||actor=%1").arg(actor));
        } else if (type == "Director") {
            QString director = it.binding(MediaVocabulary::videoDirectorBinding()).literal().toString();
            mediaItem.url = QString("video://sources?||director=%1").arg(director);
            mediaItem.title = director;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-media-artist");
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||director=%1").arg(director));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||director=%1").arg(director));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||director=%1").arg(director));
        } else if (type == "VideoTag") {
            QString tag = it.binding(MediaVocabulary::tagBinding()).literal().toString();
            QString tagFilter = tag.isEmpty() ? QString(): QString("tag=%1").arg(tag);
            mediaItem.url = QString("tag://video?%1")
                            .arg(tagFilter);
            mediaItem.title = tag;
            mediaItem.fields["title"] = mediaItem.title;
            mediaItem.artwork = KIcon("view-pim-notes");;
            //Provide context info for tag
            QStringList contextTitles;
            contextTitles << i18n("Recently Played") << i18n("Highest Rated") << i18n("Frequently Played");
            QStringList contextLRIs;
            contextLRIs << QString("semantics://recent?video||limit=4||tag=%1").arg(tag);
            contextLRIs << QString("semantics://highest?video||limit=4||tag=%1").arg(tag);
            contextLRIs << QString("semantics://frequent?video||limit=4||tag=%1").arg(tag);
            mediaItem.fields["contextTitles"] = contextTitles;
            mediaItem.fields["contextLRIs"] = contextLRIs;
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||tag=%1").arg(tag));
            mediaItem.addContext(i18n("Frequently Played"),QString("semantics://frequent?video||limit=4||tag=%1").arg(tag));
        }
        /*if (!lri.isEmpty()) {
            mediaItem.url = lri;
        }*/
        Q_UNUSED(lri);
        mediaItem.fields["sourceLri"] = sourceLri;
    }
    return mediaItem;
}

Nepomuk::Resource Utilities::mediaResourceFromUrl(KUrl url)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append(mediaVocabulary.mediaResourceBinding());
    bindings.append(mediaVocabulary.mediaResourceUrlBinding());
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(mediaVocabulary.hasUrl(MediaQuery::Required, url.url()));
    query.endWhere();
    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);
    
    Nepomuk::Resource res = Nepomuk::Resource();
    while (it.next()) {
        res = Nepomuk::Resource(it.binding(mediaVocabulary.mediaResourceBinding()).uri());
        if (res.exists() && (res.hasType(mediaVocabulary.typeAudio()) ||
            res.hasType(mediaVocabulary.typeAudioMusic()) ||
            res.hasType(mediaVocabulary.typeAudioStream()) || 
            res.hasType(mediaVocabulary.typeVideo()) ||
            res.hasType(mediaVocabulary.typeVideoMovie()) ||
            res.hasType(mediaVocabulary.typeVideoTVShow())) ) {
            break;//returns first media resource found
        }
    }
    return res;   
}

QString Utilities::lriFilterFromMediaListField(const QList<MediaItem> &mediaList, const QString &mediaItemField, const QString &filterFieldName, const QString &lriFilterOperator)
{
    QString lriFilter;
    for (int i = 0; i < mediaList.count(); i++) {
        lriFilter = lriFilter + QString("||") + filterFieldName + lriFilterOperator + mediaList.at(i).fields[mediaItemField].toString();
    }
    return lriFilter;
}

QString Utilities::mergeLRIs(const QString &lri, const QString &lriToMerge)
{
    QString mergedLRI;
    MediaListProperties targetProperties(lri);
    MediaListProperties sourceProperties(lriToMerge);
    if (targetProperties.engine() == sourceProperties.engine() && targetProperties.engineArg() == sourceProperties.engineArg()) {
        mergedLRI = targetProperties.engine() + targetProperties.engineArg() + QString("?");
        QStringList targetFilterList = targetProperties.engineFilterList();
        QStringList sourceFilterList = sourceProperties.engineFilterList();
        QString mergedFilter;
        for (int i = 0; i < targetFilterList.count(); i++) {
            QString targetFilter = targetFilterList.at(i);
            QString field = targetProperties.filterField(targetFilter);
            QString sourceFilter = sourceProperties.filterForField(field);
            if (sourceFilter.isEmpty() || sourceFilter == targetFilter) {
                mergedFilter = targetFilter;
            } else if (!sourceFilter.isEmpty()) {
                mergedFilter = field + targetProperties.filterOperator(targetFilter) + targetProperties.filterValue(targetFilter) + QString("|OR|") + sourceProperties.filterValue(sourceFilter);
            }
            if (!mergedFilter.isEmpty()) {
                mergedLRI += QString("%1||").arg(mergedFilter);
            }
        }
        MediaListProperties mergedProperties(mergedLRI);
        mergedFilter = QString();
        for (int i = 0; i < sourceFilterList.count(); i++) {
            QString sourceFilter = sourceFilterList.at(i);
            QString field = sourceProperties.filterField(sourceFilter);
            if (mergedProperties.filterForField(field).isEmpty() && mergedProperties.engineFilterList().indexOf(sourceFilter) == -1) {
                mergedFilter = sourceFilter;
            }
            if (!mergedFilter.isEmpty()) {
                mergedLRI += QString("%1||").arg(mergedFilter);
            }
        }
    }
    return mergedLRI;
}

QUrl Utilities::artistResource(const QString &artistName)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append("r");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(QString("?r rdf:type %1 . ").arg(mediaVocabulary.typeMusicArtist().toString()));
    query.addCondition(QString("?r nco:fullname ?name ."));
    query.startFilter();
    query.addFilterConstraint("r", artistName, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();
    
    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);
    
    QUrl resource;
    while (it.next()) {
        resource = it.binding("r").uri();
    }
    return resource;
}

QList<MediaItem> Utilities::mediaListFromSavedList(const QString &savedListLocation)
{
    QList<MediaItem> mediaList;
    
    //Download playlist if it is remote
    KUrl location = KUrl(savedListLocation);
    if (!location.isLocalFile() && 
        (Utilities::isPls(savedListLocation) || Utilities::isM3u(savedListLocation))) {
        QString tmpFile;
        if( KIO::NetAccess::download(location, tmpFile, 0)) {
            location = KUrl(tmpFile);
        } else {
            return mediaList;
        }
    }
    QFile file(location.path());
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return mediaList;
        }
    }
        
    //Make sure it's a valid M3U or PLSfileref
    QTextStream in(&file);
    bool valid = false;
    bool isM3U = false;
    bool isPLS = false;
    if (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed() == "#EXTM3U") {
            valid = true;
            isM3U = true;
        } else if (line.trimmed() == "[playlist]") {
            valid = true;
            isPLS = true;
        }
    }
    
    //Create a MediaItem for each entry
    if (valid) {
        while (!in.atEnd()) {
            QString line = in.readLine();
            if ((isM3U) && line.startsWith("#EXTINF:")) {
                line = line.replace("#EXTINF:","");
                QStringList durTitle = line.split(",");
                QString title;
                int duration;
                if (durTitle.count() == 1) {
                    //No title
                    duration = 0;
                    title = durTitle.at(0);
                } else {
                    duration = durTitle.at(0).toInt();
                    title = durTitle.at(1);
                }
                QString url = in.readLine().trimmed();
                MediaItem mediaItem;
                KUrl itemUrl(url);
                if (!url.isEmpty()) {
                    mediaItem = Utilities::mediaItemFromUrl(itemUrl);
                } else {
                    continue;
                }
                if (mediaItem.title == itemUrl.fileName()) {
                    mediaItem.title = title;
                }
                if ((duration > 0) && (mediaItem.fields["duration"].toInt() <= 0)) {
                    mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                    mediaItem.fields["duration"] = duration;
                } else if (duration == -1) {
                    mediaItem.duration = QString();
                    mediaItem.fields["audioType"] = "Audio Stream";
                }
                mediaList << mediaItem;
            }
            if ((isPLS) && line.startsWith("File")) {
                QString url = line.mid(line.indexOf("=") + 1).trimmed();
                QString title;
                if (!in.atEnd()) {
                    line = in.readLine();
                    title = line.mid(line.indexOf("=") + 1).trimmed();
                }
                int duration = 0;
                if (!in.atEnd()) {
                    line = in.readLine();
                    duration = line.mid(line.indexOf("=") + 1).trimmed().toInt();
                }
                
                MediaItem mediaItem;
                KUrl itemUrl(url);
                if (!url.isEmpty()) {
                    mediaItem = Utilities::mediaItemFromUrl(itemUrl);
                } else {
                    continue;
                }
                if (mediaItem.title == itemUrl.fileName()) {
                    mediaItem.title = title;
                }
                if ((duration > 0) && (mediaItem.fields["duration"].toInt() <= 0)) {
                    mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                    mediaItem.fields["duration"] = duration;
                } else if (duration == -1) {
                    mediaItem.duration = QString();
                    mediaItem.fields["audioType"] = "Audio Stream";
                }
                mediaList << mediaItem;
            }
        }
    }
    
    return mediaList;
}

MediaItem Utilities::completeMediaItem(const MediaItem & sourceMediaItem)
{
    MediaItem mediaItem = sourceMediaItem;
    QString resourceUri = sourceMediaItem.fields["resourceUri"].toString();
    QString subType = mediaItem.fields["videoType"].toString();
    
    if (subType == "Movie" || subType == "TV Show") {
        MediaVocabulary mediaVocabulary;
        MediaQuery query;
        QStringList bindings;
        bindings.append(mediaVocabulary.mediaResourceBinding());
        bindings.append(mediaVocabulary.mediaResourceUrlBinding());
        bindings.append(mediaVocabulary.videoAudienceRatingBinding());
        bindings.append(mediaVocabulary.videoWriterBinding());
        bindings.append(mediaVocabulary.videoDirectorBinding());
        bindings.append(mediaVocabulary.videoProducerBinding());
        bindings.append(mediaVocabulary.videoActorBinding());
        if (subType == "TV Show") {
            bindings.append(mediaVocabulary.videoSeriesTitleBinding());
            bindings.append(mediaVocabulary.videoSeasonBinding());
            bindings.append(mediaVocabulary.videoEpisodeNumberBinding());
        }
        query.select(bindings, MediaQuery::Distinct);
        
        query.startWhere();
        query.addCondition(mediaVocabulary.hasResource(resourceUri));
        query.addCondition(mediaVocabulary.hasVideoAudienceRating(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoWriter(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoDirector(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoProducer(MediaQuery::Optional));
        query.addCondition(mediaVocabulary.hasVideoActor(MediaQuery::Optional));
        if (subType == "TV Show") {
            query.addCondition(mediaVocabulary.hasVideoSeriesTitle(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoSeason(MediaQuery::Optional));
            query.addCondition(mediaVocabulary.hasVideoEpisodeNumber(MediaQuery::Optional));
        }
        query.endWhere();
        
        //kDebug() << query.query();
        
        Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
        Soprano::QueryResultIterator it = query.executeSelect(mainModel);
        
        while (it.next()) {
            mediaItem.fields["writer"] = it.binding(MediaVocabulary::videoWriterBinding()).literal().toString();
            mediaItem.fields["director"] = it.binding(MediaVocabulary::videoDirectorBinding()).literal().toString();
            mediaItem.fields["producer"] = it.binding(MediaVocabulary::videoProducerBinding()).literal().toString();
            mediaItem.fields["actor"] = it.binding(MediaVocabulary::videoActorBinding()).literal().toString();
            if (subType == "TV Show") {
                mediaItem.artwork = KIcon("video-television");
                QString seriesName = it.binding(MediaVocabulary::videoSeriesTitleBinding()).literal().toString();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                    mediaItem.subTitle = seriesName;
                }
                
                int season = it.binding(MediaVocabulary::videoSeasonBinding()).literal().toInt();
                if (season !=0 ) {
                    mediaItem.fields["season"] = season;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Season %1").arg(season);
                }
                
                int episodeNumber = it.binding(MediaVocabulary::videoEpisodeNumberBinding()).literal().toInt();
                if (episodeNumber != 0) {
                    mediaItem.fields["episodeNumber"] = episodeNumber;
                    if (!mediaItem.subTitle.isEmpty()) {
                        mediaItem.subTitle += " - ";
                    }
                    mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                }
            }
            break;
        }
    }
    return mediaItem;
}

KUrl Utilities::deviceUrl(const QString &type, const QString& udi, const QString& name, QString content, int title )
{
    KUrl url = QString("device://%1%2").arg(type, udi);
    QString query;
    if (!name.isEmpty())
        query += QString("?name=%1").arg(name);
    if (!content.isEmpty()) {
        if ( query.isEmpty() )
            query = "?";
        else
            query += "&";
        query += QString("content=%1").arg(content);
    }
    if (!query.isEmpty())
        url.setQuery(query);
    if (title != invalidTitle())
        url.setFragment(QString("%1").arg(title));
    return url;
}

QString Utilities::deviceNameFromUrl(const KUrl& url)
{
    return url.queryItemValue("name");
}

int Utilities::deviceTitleFromUrl(const KUrl& url)
{
    if (!url.hasFragment())
        return invalidTitle();
    bool ok = false;
    int title = url.fragment().toInt(&ok, 0);
    return ok ? title : invalidTitle();
}

QString Utilities::deviceUdiFromUrl(const KUrl& url)
{
    return url.path();
}

QString Utilities::deviceTypeFromUrl(const KUrl& url)
{
    return url.authority();
}

int Utilities::invalidTitle()
{
    return -1;
}

QString Utilities::deviceName(QString udi, Phonon::MediaObject *mobj)
{
    QString name;
    const Solid::OpticalDisc *disc = Solid::Device( udi ).as<const Solid::OpticalDisc>();
    if ( disc != NULL )
        name = disc->label();
    if ( !name.isEmpty() || mobj == NULL)
        return name;
    else if (!mobj->metaData("TITLE").isEmpty())
        return mobj->metaData("TITLE").join("");
    else
        return QString();
}

QStringList Utilities::availableDiscUdis(Solid::OpticalDisc::ContentType type)
{
    QStringList udis;
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString()))
    {
        const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc>();
        if (disc != NULL && disc->availableContent() & type)
            udis << device.udi();
    }
    return udis;
}

QHash<int, QString> Utilities::tagGenreDictionary()
{
    QHash<int, QString> genreDictionary;
    genreDictionary[0] = "Blues";
    genreDictionary[1] = "Classic Rock";
    genreDictionary[2] = "Country";
    genreDictionary[3] = "Dance";
    genreDictionary[4] = "Disco";
    genreDictionary[5] = "Funk";
    genreDictionary[6] = "Grunge";
    genreDictionary[7] = "Hip-Hop";
    genreDictionary[8] = "Jazz";
    genreDictionary[9] = "Metal";
    genreDictionary[10] = "New Age";
    genreDictionary[11] = "Oldies";
    genreDictionary[12] = "Other";
    genreDictionary[13] = "Pop";
    genreDictionary[14] = "R&B";
    genreDictionary[15] = "Rap";
    genreDictionary[16] = "Reggae";
    genreDictionary[17] = "Rock";
    genreDictionary[18] = "Techno";
    genreDictionary[19] = "Industrial";
    genreDictionary[20] = "Alternative";
    genreDictionary[21] = "Ska";
    genreDictionary[22] = "Death Metal";
    genreDictionary[23] = "Pranks";
    genreDictionary[24] = "Soundtrack";
    genreDictionary[25] = "Euro-Techno";
    genreDictionary[26] = "Ambient";
    genreDictionary[27] = "Trip-Hop";
    genreDictionary[28] = "Vocal";
    genreDictionary[29] = "Jazz+Funk";
    genreDictionary[30] = "Fusion";
    genreDictionary[31] = "Trance";
    genreDictionary[32] = "Classical";
    genreDictionary[33] = "Instrumental";
    genreDictionary[34] = "Acid";
    genreDictionary[35] = "House";
    genreDictionary[36] = "Game";
    genreDictionary[37] = "Sound Clip";
    genreDictionary[38] = "Gospel";
    genreDictionary[39] = "Noise";
    genreDictionary[40] = "Alternative Rock";
    genreDictionary[41] = "Bass";
    genreDictionary[42] = "Soul";
    genreDictionary[43] = "Punk";
    genreDictionary[44] = "Space";
    genreDictionary[45] = "Meditative";
    genreDictionary[46] = "Instrumental Pop";
    genreDictionary[47] = "Instrumental Rock";
    genreDictionary[48] = "Ethnic";
    genreDictionary[49] = "Gothic";
    genreDictionary[50] = "Darkwave";
    genreDictionary[51] = "Techno-Industrial";
    genreDictionary[52] = "Electronic";
    genreDictionary[53] = "Pop-Folk";
    genreDictionary[54] = "Eurodance";
    genreDictionary[55] = "Dream";
    genreDictionary[56] = "Southern Rock";
    genreDictionary[57] = "Comedy";
    genreDictionary[58] = "Cult";
    genreDictionary[59] = "Gangsta";
    genreDictionary[60] = "Top40";
    genreDictionary[61] = "Christian Rap";
    genreDictionary[62] = "Pop/Funk";
    genreDictionary[63] = "Jungle";
    genreDictionary[64] = "Native American";
    genreDictionary[65] = "Cabaret";
    genreDictionary[66] = "New Wave";
    genreDictionary[67] = "Psychadelic";
    genreDictionary[68] = "Rave";
    genreDictionary[69] = "Showtunes";
    genreDictionary[70] = "Trailer";
    genreDictionary[71] = "Lo-Fi";
    genreDictionary[72] = "Tribal";
    genreDictionary[73] = "Acid Punk";
    genreDictionary[74] = "Acid Jazz";
    genreDictionary[75] = "Polka";
    genreDictionary[76] = "Retro";
    genreDictionary[77] = "Musical";
    genreDictionary[78] = "Rock & Roll";
    genreDictionary[79] = "Hard Rock";
    genreDictionary[80] = "Folk";
    genreDictionary[81] = "Folk-Rock";
    genreDictionary[82] = "National Folk";
    genreDictionary[83] = "Swing";
    genreDictionary[84] = "Fast Fusion";
    genreDictionary[85] = "Bebob";
    genreDictionary[86] = "Latin";
    genreDictionary[87] = "Revival";
    genreDictionary[88] = "Celtic";
    genreDictionary[89] = "Bluegrass";
    genreDictionary[90] = "Avantgarde";
    genreDictionary[91] = "Gothic Rock";
    genreDictionary[92] = "Progressive Rock";
    genreDictionary[93] = "Psychedelic Rock";
    genreDictionary[94] = "Symphonic Rock";
    genreDictionary[95] = "Slow Rock";
    genreDictionary[96] = "Big Band";
    genreDictionary[97] = "Chorus";
    genreDictionary[98] = "Easy Listening";
    genreDictionary[99] = "Acoustic";
    genreDictionary[100] = "Humour";
    genreDictionary[101] = "Speech";
    genreDictionary[102] = "Chanson";
    genreDictionary[103] = "Opera";
    genreDictionary[104] = "Chamber Music";
    genreDictionary[105] = "Sonata";
    genreDictionary[106] = "Symphony";
    genreDictionary[107] = "Booty Bass";
    genreDictionary[108] = "Primus";
    genreDictionary[109] = "Porn Groove";
    genreDictionary[110] = "Satire";
    genreDictionary[111] = "Slow Jam";
    genreDictionary[112] = "Club";
    genreDictionary[113] = "Tango";
    genreDictionary[114] = "Samba";
    genreDictionary[115] = "Folklore";
    genreDictionary[116] = "Ballad";
    genreDictionary[117] = "Power Ballad";
    genreDictionary[118] = "Rhythmic Soul";
    genreDictionary[119] = "Freestyle";
    genreDictionary[120] = "Duet";
    genreDictionary[121] = "Punk Rock";
    genreDictionary[122] = "Drum Solo";
    genreDictionary[123] = "A capella";
    genreDictionary[124] = "Euro-House";
    genreDictionary[125] = "Dance Hall";
    return genreDictionary;
}

QString Utilities::genreFromRawTagGenre(QString rawTagGenre)
{
    QHash<int, QString> genreDictionary = tagGenreDictionary();
    QString genre = rawTagGenre;
    //if (rawTagGenre.startsWith("(") && rawTagGenre.endsWith(")")) {
        QString tagGenreNoParenth = rawTagGenre.remove("(").remove(")").trimmed();
        bool ok;
        int tagGenreNo = tagGenreNoParenth.toInt(&ok);
        if (ok) {
            genre = genreDictionary[tagGenreNo];
        }
    //}
    return genre;
}

QString Utilities::rawTagGenreFromGenre(QString genre)
{
    QHash<int, QString> genreDictionary = tagGenreDictionary();

    QString tagGenre = genre;
    int tagGenreNo = genreDictionary.key(genre, -1);
    if (tagGenreNo != -1) {
        tagGenre = QString("(%1)").arg(tagGenreNo);
    }
    return tagGenre;
}

QList<MediaItem> Utilities::mergeGenres(QList<MediaItem> genreList)
{
    for (int i = 0; i < genreList.count(); i++) {
        MediaItem genreItem = genreList.at(i);
        kDebug() << genreItem.title;
        if (genreItem.type == "Category" && genreItem.fields["categoryType"] == "AudioGenre") {
            QString rawGenre = genreItem.title;
            QString convertedGenre = Utilities::genreFromRawTagGenre(rawGenre);
            if (convertedGenre != rawGenre) {
                bool matchFound = false;
                for (int j = 0; j < genreList.count(); j++) {
                    if (genreList.at(j).title == convertedGenre) {
                        matchFound = true;
                        MediaItem matchedGenre = genreList.at(j);
                        MediaListProperties matchedGenreProperties;
                        matchedGenreProperties.lri = matchedGenre.url;
                        QString newUrl = QString("%1%2?")
                                         .arg(matchedGenreProperties.engine())
                                         .arg(matchedGenreProperties.engineArg());
                        QString mergedFilter;
                        for (int k = 0; k < matchedGenreProperties.engineFilterList().count(); k++) {
                            QString filter = matchedGenreProperties.engineFilterList().at(k);
                            if (matchedGenreProperties.filterField(filter) == "genre") {
                                mergedFilter.append(QString("%1|OR|%2||")
                                                    .arg(filter)
                                                    .arg(rawGenre));
                            } else {
                                mergedFilter.append(filter);
                            }
                        }
                        newUrl.append(mergedFilter);
                        kDebug() << newUrl;
                        matchedGenre.url = newUrl;
                        genreList.replace(j, matchedGenre);
                        genreList.removeAt(i);
                        i = -1;
                    }
                }
                if (!matchFound) {
                    genreItem.title = convertedGenre;
                    kDebug() << convertedGenre;
                    genreList.replace(i, genreItem);
                }
            }
        }
    }
    return genreList;
}

QList<MediaItem> Utilities::sortMediaList(QList<MediaItem> mediaList)
{
    QList<MediaItem> sortedList;
    QMap<QString, int> sortedIndices;
    for (int i = 0; i < mediaList.count(); i++) {
        sortedIndices[mediaList.at(i).title] = i;
    }
    QMapIterator<QString, int> it(sortedIndices);
    while (it.hasNext()) {
        it.next();
        sortedList.append(mediaList.at(it.value()));
    }
    return sortedList;
}

bool Utilities::nepomukInited()
{
    bool nepomukInited = Nepomuk::ResourceManager::instance()->initialized();
    if (!nepomukInited) {
        Nepomuk::ResourceManager::instance()->init();
        nepomukInited = Nepomuk::ResourceManager::instance()->initialized();
    }
    return nepomukInited;
}

