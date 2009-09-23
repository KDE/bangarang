#ifndef UTILITIES_H
#define UTILITIES_H

#include <KUrl>
#include <QPixmap>

class MediaItem;
namespace Utilities {
    QPixmap getArtworkFromTag(QString url, QSize size = QSize(128,128));
    QString getArtistFromTag(QString url);
    QString getAlbumFromTag(QString url);
    QString getTitleFromTag(QString url);
    QString getGenreFromTag(QString genre);
    int getYearFromTag(QString url);
    int getDurationFromTag(QString url);
    int getTrackNumberFromTag(QString url);
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
}
#endif //UTILITIES_H    