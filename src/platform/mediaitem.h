/*
 * BANGARANG MEDIA PLAYER
 * Copyright (C) 2014  Stefan Burnicki (stefan.burnicki@burnicki.net)
 * <https://projects.kde.org/projects/playground/multimedia/bangarang>
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

#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include <QObject>
#include <QUrl>
#include <QImage>
#include "mediauri.h"

enum MediaItemType
{
    Music,
    Category
};

class MediaItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QUrl playbackUrl READ playbackUrl WRITE setPlaybackUrl NOTIFY playbackUrlChanged)
    Q_PROPERTY(QImage artwork READ artwork WRITE setArtwork NOTIFY artworkChanged)
    Q_PROPERTY(MediaItemType type READ type)
    Q_PROPERTY(MediaUri uri READ uri)


public:
    MediaItem();
    MediaItem(const MediaItem& other);
    MediaItem(const QString& uri, MediaItemType type, const QImage& artwork = QImage(),
              const QString& title = QString(), const QUrl& playbackUrl = QUrl());

    MediaItem& operator =(const MediaItem& other);

    const QString& title() const;
    const QUrl& playbackUrl() const;
    MediaItemType type() const;
    const MediaUri& uri() const;
    const QImage& artwork() const;

    bool isNull() const;

public slots:
    void setTitle(const QString& title);
    void setPlaybackUrl(const QUrl& playbackUrl);
    void setArtwork(const QImage& artwork);

signals:
    void titleChanged();
    void playbackUrlChanged();
    void artworkChanged();

private:
    void init(const MediaItem &other);

    QImage m_artwork;
    QString m_title;
    QUrl m_playbackUrl;
    MediaItemType m_type;
    MediaUri m_uri;
};

#endif // MEDIAITEM_H
