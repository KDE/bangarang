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

#ifndef UTILITIES_FILETAGS_CPP
#define UTILITIES_FILETAGS_CPP

#include "filetags.h"
#include "typechecks.h"

#include <KUrl>
#include <KEncodingProber>

#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QPainter>
#include <QImage>
#include <QTextCodec>
#include <QTime>

#include <taglib/mpegfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/speexfile.h>
#include <taglib/flacfile.h>
#include <taglib/mpcfile.h>
#include <taglib/trueaudiofile.h>
#include <taglib/wavpackfile.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/attachedpictureframe.h>

QPixmap Utilities::getArtworkFromTag(const QString &url, QSize size)
{
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

QString Utilities::tagType(const QString &url)
{
    if (isMusic(url)) {
        TagLib::Ogg::Vorbis::File vorbisFile(KUrl(url).path().toLocal8Bit().constData());
        TagLib::Ogg::XiphComment *oggTag = vorbisFile.tag();
        if (oggTag) {
            return "OGG";
        }
        TagLib::FLAC::File flacFile(KUrl(url).path().toLocal8Bit().constData());
        oggTag = flacFile.xiphComment(false);
        if (oggTag) {
            return "OGG";
        }
        TagLib::MPEG::File mpegFile(KUrl(url).path().toLocal8Bit().constData());
        TagLib::APE::Tag *apeTag = mpegFile.APETag(false);
        if (apeTag) {
            return "APE";
        }
        TagLib::ID3v2::Tag *id3v2Tag = flacFile.ID3v2Tag(false);
        if (id3v2Tag) {
            return "ID3V2";
        }
        TagLib::ID3v1::Tag *id3v1Tag = flacFile.ID3v1Tag(false);
        if (id3v1Tag) {
            return "ID3V1";
        }
        id3v2Tag = mpegFile.ID3v2Tag(false);
        if (id3v2Tag) {
            return "ID3V2";
        }
        id3v1Tag = mpegFile.ID3v1Tag(false);
        if (id3v1Tag) {
            return "ID3V1";
        }

    }
    return QString();
}

MediaItem Utilities::getAllInfoFromTag(const QString &url, MediaItem templateItem)
{
    MediaItem mediaItem = templateItem;
    mediaItem.url = url;
    mediaItem.fields["url"] = url;
    TagLib::FileRef file(KUrl(url).path().toLocal8Bit().constData());
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
    return mediaItem;
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

void Utilities::saveAllInfoToTag(const QList<MediaItem> &mediaList)
{
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        if ((mediaItem.type == "Audio") && (mediaItem.fields["audioType"] == "Music")) {
            if (Utilities::isMusic(mediaList.at(i).url)) {
                QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
                if (!artworkUrl.isEmpty()) {
                    Utilities::saveArtworkToTag(mediaList.at(i).url, artworkUrl);
                }
                TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toLocal8Bit().constData());
                if (!file.isNull()) {
                    QString title = mediaItem.fields["title"].toString();
                    if (!title.isEmpty()) {
                        TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
                        file.tag()->setTitle(tTitle);
                    }
                    QString artist = mediaItem.fields["artist"].toString();
                    if (!artist.isEmpty()) {
                        TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
                        file.tag()->setArtist(tArtist);
                    }
                    QString album = mediaItem.fields["album"].toString();
                    if (!album.isEmpty()) {
                        TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
                        file.tag()->setAlbum(tAlbum);
                    }
                    int year = mediaItem.fields["year"].toInt();
                    if (year != 0) {
                        file.tag()->setYear(year);
                    }
                    int trackNumber = mediaItem.fields["trackNumber"].toInt();
                    if (trackNumber != 0) {
                        file.tag()->setTrack(trackNumber);
                    }
                    QString genre = mediaItem.fields["genre"].toString();
                    if (!genre.isEmpty()) {
                        TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
                        file.tag()->setGenre(tGenre);
                    }
                    file.save();
                }
            }
        }
    }
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
QStringList Utilities::genresFromRawTagGenres(QStringList rawTagGenres)
{
    QStringList genres;
    for (int i = 0; i < rawTagGenres.count(); i++) {
        QString genre = genreFromRawTagGenre(rawTagGenres.at(i));
        if (genres.indexOf(genre) == -1 && !genre.isEmpty()) {
            genres.append(genre);
        }
    }
    return genres;
}

QStringList Utilities::rawTagGenresFromGenres(QStringList genres)
{
    QStringList rawTagGenres;
    for (int i = 0; i < genres.count(); i++) {
        QString rawTagGenre = rawTagGenreFromGenre(genres.at(i));
        if (rawTagGenres.indexOf(rawTagGenre) == -1 && !rawTagGenre.isEmpty()) {
            rawTagGenres.append(rawTagGenre);
        }
    }
    return rawTagGenres;
}

QString Utilities::genreFilter(QString genre)
{
    QHash<int, QString> genreDictionary = tagGenreDictionary();

    QString genreFilter = genre;
    int tagGenreNo = genreDictionary.key(genre, -1);
    if (tagGenreNo != -1) {
        genreFilter += QString("|OR|(%1)|OR|%1").arg(tagGenreNo);
    }
    return genreFilter;
}

#endif //UTILITIES_FILETAGS_CPP
