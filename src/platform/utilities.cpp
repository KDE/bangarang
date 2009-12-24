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
#include <KEncodingProber>
#include <KMimeType>
#include <KIcon>
#include <KIconEffect>
#include <KDebug>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <Nepomuk/ResourceManager>

#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QPainter>
#include <QImage>
#include <QTime>
#include <Phonon/BackendCapabilities>

#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
//#include "blur.cpp"

QPixmap Utilities::getArtworkFromTag(QString url, QSize size)
{
    TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit());
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
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        artist  = TStringToQString(file.tag()->artist()).trimmed();
    }
    return artist;
}

QString Utilities::getAlbumFromTag(QString url)
{
    QString album;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        album = TStringToQString(file.tag()->album()).trimmed();
    }
    return album;
}

QString Utilities::getTitleFromTag(QString url)
{
    QString title;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        title = TStringToQString(file.tag()->title()).trimmed();
    }
    return title;
}

QString Utilities::getGenreFromTag(QString url)
{
    QString genre;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        genre   = TStringToQString(file.tag()->genre()).trimmed();
    }
    return genre;
}

int Utilities::getYearFromTag(QString url)
{
    int year = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        year = file.tag()->year();
    }
    return year;
}

int Utilities::getDurationFromTag(QString url)
{
    int duration = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        duration = file.audioProperties()->length();
    }
    return duration;
}

int Utilities::getTrackNumberFromTag(QString url)
{
    int track = 0;
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        track   = file.tag()->track();
    }
    return track;
}

bool Utilities::saveArtworkToTag(QString url, const QPixmap *pixmap)
{
    //FIXME:: HELP! Can't figure out why this doesn't work
    TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit());
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
    TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit());
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
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        file.tag()->setArtist(tArtist);
        file.save();
    }
}

void Utilities::setAlbumTag(QString url, QString album)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        file.tag()->setAlbum(tAlbum);
        file.save();
    }
}

void Utilities::setTitleTag(QString url, QString title)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        file.tag()->setTitle(tTitle);
        file.save();
    }
}

void Utilities::setGenreTag(QString url, QString genre)
{
    if (Utilities::isMusic(url)) {
        TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        file.tag()->setGenre(tGenre);
        file.save();
    }
}

void Utilities::setYearTag(QString url, int year)
{
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
        file.tag()->setYear(year);
        file.save();
    }
}

void Utilities::setTrackNumberTag(QString url, int trackNumber)
{
    if (Utilities::isMusic(url)) {
        TagLib::FileRef file(KUrl(url).path().toLocal8Bit());
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

bool Utilities::isM3u(QString url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return result->is("audio/m3u") || result->is("audio/x-mpegurl");
}

bool Utilities::isPls(QString url)
{
    KMimeType::Ptr result = KMimeType::findByUrl(KUrl(url), 0, true);
    
    return result->is("audio/x-scpls");
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

MediaItem Utilities::mediaItemFromUrl(KUrl url)
{
    //Initialize Nepomuk
    bool nepomukInited = false;
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        nepomukInited = true; //resource manager inited successfully
    } else {
        nepomukInited = false; //no resource manager
    };
    
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    
    MediaItem mediaItem;
    url.cleanPath();
    url = QUrl::fromPercentEncoding(url.url().toUtf8());

    if (Utilities::isM3u(url.url()) || Utilities::isPls(url.url())) {
        mediaItem.artwork = KIcon("view-list-text");
        mediaItem.url = QString("savedlists://%1").arg(url.url());
        mediaItem.title = url.fileName();
        mediaItem.type = "Category";
        return mediaItem;
    } 
    
    mediaItem.url = url.url();
    mediaItem.title = url.fileName();
    mediaItem.fields["url"] = mediaItem.url;
    mediaItem.fields["title"] = mediaItem.title;
    
    //Determine type of file - nepomuk is primary source
    bool foundInNepomuk = false;
    if (nepomukInited) {
        Nepomuk::Resource res(mediaItem.url);
        if (res.exists()) {
            if (res.hasType(mediaVocabulary.typeAudio()) ||
                res.hasType(mediaVocabulary.typeAudioMusic()) ||
                res.hasType(mediaVocabulary.typeAudioStream())) {
                mediaItem.type = "Audio";
                if (res.hasType(mediaVocabulary.typeAudioMusic())) {
                    mediaItem.fields["audioType"] = "Music";
                } else if (res.hasType(mediaVocabulary.typeAudioStream())) {
                    mediaItem.fields["audioType"] = "Audio Stream";
                } else {
                    mediaItem.fields["audioType"] = "Audio Clip";
                }
                foundInNepomuk = true;
            } 
            if (res.hasType(mediaVocabulary.typeVideo()) ||
                res.hasType(mediaVocabulary.typeVideoMovie()) ||
                res.hasType(mediaVocabulary.typeVideoTVShow())) {
                mediaItem.type = "Video";
                if (res.hasType(mediaVocabulary.typeVideoMovie())) {
                    mediaItem.fields["videoType"] = "Movie";
                } else if (res.hasType(mediaVocabulary.typeVideoTVShow())) {
                    mediaItem.fields["videoType"] = "TV Show";
                } else {
                    mediaItem.fields["videoType"] = "Video Clip";
                }
                foundInNepomuk = true;
            }
            if (foundInNepomuk) {
                if (mediaItem.type == "Video") {
                    mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
                }
                // Get metadata common to all media types
                QString title = res.property(mediaVocabulary.title()).toString();
                if (!title.isEmpty()) {
                    mediaItem.title = title;
                    mediaItem.fields["title"] = title;
                }
                QString description = res.property(mediaVocabulary.description()).toString();
                if (!description.isEmpty()) {
                    mediaItem.fields["description"] = description;
                }
                int duration = res.property(mediaVocabulary.duration()).toInt();
                if (duration != 0) {
                    mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                    mediaItem.fields["duration"] = duration;
                }
                int playCount = res.property(mediaVocabulary.playCount()).toInt();
                mediaItem.fields["playCount"] = playCount;
                QDateTime lastPlayed = res.property(mediaVocabulary.lastPlayed()).toDateTime();
                mediaItem.fields["lastPlayed"] = lastPlayed;
                mediaItem.fields["rating"] = res.rating();
                Nepomuk::Resource artworkRes = res.property(mediaVocabulary.artwork()).toResource();
                if (artworkRes.isValid()) {
                    mediaItem.fields["artworkUrl"] = artworkRes.resourceUri().toString();
                }
            }
        }
    }
    
    if (!foundInNepomuk) {
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
            mediaItem.title = url.prettyUrl();
        }
    }
    
    if (mediaItem.type == "Audio") {
        if (mediaItem.fields["audioType"] == "Audio Clip") {
            mediaItem.artwork = KIcon("audio-x-generic");
        } else if (mediaItem.fields["audioType"] == "Music") {
            mediaItem.artwork = KIcon("audio-mp4");
            //File metadata is always primary for music items.
            TagLib::FileRef file(KUrl(mediaItem.url).path().toLocal8Bit());
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
                        QByteArray encodingname = prober.encoding();
                        QString track_encoding(encodingname);
                        if ( ( track_encoding.toLatin1() == "gb18030" ) 
                            || ( track_encoding.toLatin1() == "big5" )
                            || ( track_encoding.toLatin1() == "euc-kr" ) 
                            || ( track_encoding.toLatin1() == "euc-jp" )
                            || ( track_encoding.toLatin1() == "koi8-r" ) ) {
                            title = QTextCodec::codecForName(encodingname)->toUnicode(title.toAscii());
                            artist = QTextCodec::codecForName(encodingname)->toUnicode(artist.toAscii());
                            album = QTextCodec::codecForName(encodingname)->toUnicode(album.toAscii());
                            genre = QTextCodec::codecForName(encodingname)->toUnicode(genre.toAscii());
                        } else if (QTextCodec::codecForLocale()->name().toLower() != "utf-8") {
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
                mediaItem.fields["genre"] = genre;
                mediaItem.fields["trackNumber"] = track;
                mediaItem.fields["year"] = year;
            }
        } else if (mediaItem.fields["audioType"] == "Audio Stream") {
            mediaItem.artwork = KIcon("x-media-podcast");
        }
    } else if (mediaItem.type == "Video") {
        if (mediaItem.fields["videoType"] == "Video Clip") {
            mediaItem.artwork = KIcon("video-x-generic");
        } else if (mediaItem.fields["videoType"] == "Movie") {
            mediaItem.artwork = KIcon("tool-animator");
            Nepomuk::Resource res(mediaItem.url);
            QString synopsis = res.property(mediaVocabulary.videoSynopsis()).toString();
            if (!synopsis.isEmpty()) {
                mediaItem.fields["synopsis"] = synopsis;
            }
            QString genre = res.property(mediaVocabulary.videoGenre()).toString();
            if (!genre.isEmpty()) {
                mediaItem.fields["genre"] = genre;
            }
            QDate releaseDate = res.property(mediaVocabulary.releaseDate()).toDate();
            if (releaseDate.isValid()) {
                mediaItem.fields["releaseDate"] = releaseDate;
            }
            QString writer = res.property(mediaVocabulary.videoWriter()).toString();
            if (!writer.isEmpty()) {
                mediaItem.fields["writer"] = writer;
            }
            QString director = res.property(mediaVocabulary.videoDirector()).toString();
            if (!director.isEmpty()) {
                mediaItem.fields["director"] = director;
            }
            QString assistantDirector = res.property(mediaVocabulary.videoAssistantDirector()).toString();
            if (!assistantDirector.isEmpty()) {
                mediaItem.fields["assistantDirector"] = assistantDirector;
            }
            QString producer = res.property(mediaVocabulary.videoProducer()).toString();
            if (!producer.isEmpty()) {
                mediaItem.fields["producer"] = producer;
            }
            QString actor = res.property(mediaVocabulary.videoActor()).toString();
            if (!actor.isEmpty()) {
                mediaItem.fields["actor"] = actor;
            }
            QString cinematographer = res.property(mediaVocabulary.videoGenre()).toString();
            if (!cinematographer.isEmpty()) {
                mediaItem.fields["cinematographer"] = cinematographer;
            }
        } else if (mediaItem.fields["videoType"] == "TV Show") {
            mediaItem.artwork = KIcon("video-television");
            Nepomuk::Resource res(mediaItem.url);
            Nepomuk::Resource series = res.property(mediaVocabulary.videoSeries()).toResource();
            QString seriesName = series.property(mediaVocabulary.title()).toString();
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
            if (episodeNumber !=0 ) {
                mediaItem.fields["episodeNumber"] = episodeNumber;
                if (!mediaItem.subTitle.isEmpty()) {
                    mediaItem.subTitle += " - ";
                }
                mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
            }
            QString synopsis = res.property(mediaVocabulary.videoSynopsis()).toString();
            if (!synopsis.isEmpty()) {
                mediaItem.fields["synopsis"] = synopsis;
            }
            QString genre = res.property(mediaVocabulary.videoGenre()).toString();
            if (!genre.isEmpty()) {
                mediaItem.fields["genre"] = genre;
            }
            QDate releaseDate = res.property(mediaVocabulary.releaseDate()).toDate();
            if (releaseDate.isValid()) {
                mediaItem.fields["releaseDate"] = releaseDate;
            }
            QString writer = res.property(mediaVocabulary.videoWriter()).toString();
            if (!writer.isEmpty()) {
                mediaItem.fields["writer"] = writer;
            }
            QString director = res.property(mediaVocabulary.videoDirector()).toString();
            if (!director.isEmpty()) {
                mediaItem.fields["director"] = director;
            }
            QString assistantDirector = res.property(mediaVocabulary.videoAssistantDirector()).toString();
            if (!assistantDirector.isEmpty()) {
                mediaItem.fields["assistantDirector"] = assistantDirector;
            }
            QString producer = res.property(mediaVocabulary.videoProducer()).toString();
            if (!producer.isEmpty()) {
                mediaItem.fields["producer"] = producer;
            }
            QString actor = res.property(mediaVocabulary.videoActor()).toString();
            if (!actor.isEmpty()) {
                mediaItem.fields["actor"] = actor;
            }
            QString cinematographer = res.property(mediaVocabulary.videoGenre()).toString();
            if (!cinematographer.isEmpty()) {
                mediaItem.fields["cinematographer"] = cinematographer;
            }
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

int Utilities::mediaListDuration(QList<MediaItem> mediaList) 
{
    int duration = 0;
    for (int i = 0; i < mediaList.count(); i++) {
        duration += mediaList.at(i).fields["duration"].toInt();
    }
    return duration;
}

QString Utilities::mediaListDurationText(QList<MediaItem> mediaList) 
{
    int duration = mediaListDuration(mediaList);
    int hours = duration/3600;
    int minutes = (duration - (hours*3600))/60;
    int seconds = duration - (hours*3600) - (minutes*60);
    QString min = minutes < 10 ? QString("0%1").arg(minutes): QString("%1").arg(minutes);
    QString sec = seconds < 10 ? QString("0%1").arg(seconds): QString("%1").arg(seconds);
    
    return QString("%1:%2:%3").arg(hours).arg(min).arg(sec);
}

QList<MediaItem> Utilities::mediaItemsDontExist(QList<MediaItem> mediaList)
{
    QList<MediaItem> items;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        KUrl url = KUrl(mediaItem.url);
        if (url.isValid()) {
            if (url.isLocalFile()) {
                if (!QFile(url.path()).exists()) {
                    mediaItem.exists = false;
                    kDebug() << mediaItem.url << " missing";
                    items << mediaItem;
                }
            } else if (mediaItem.url.startsWith("trash:/")) {
                mediaItem.exists = false;
                kDebug() << mediaItem.url << " missing";
                items << mediaItem;
            }
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
