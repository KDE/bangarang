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

#include "mediaitem.h"

MediaItem::MediaItem(const MediaItem& other) : QObject()
{
    m_uri = other.uri();
    m_type = other.type();
    m_title = other.title();
    m_playbackUrl = other.playbackUrl();
}

MediaItem::MediaItem(const QString& uri, MediaItemType type,
                     const QString& title, const QUrl& playbackUrl): QObject()
{
    m_uri = MediaUri(uri);
    m_type = type;
    m_title = title;
    m_playbackUrl = playbackUrl;
}

const QString &MediaItem::title() const
{
    return m_title;
}

void MediaItem::setTitle(const QString& title)
{
    if (title != m_title)
    {
        m_title = title;
        emit titleChanged();
    }
}

const QUrl &MediaItem::playbackUrl() const
{
    return m_playbackUrl;
}

void MediaItem::setPlaybackUrl(const QUrl& playbackUrl)
{
    if (playbackUrl != m_playbackUrl)
    {
        m_playbackUrl = playbackUrl;
        emit playbackUrlChanged();
    }
}

MediaItemType MediaItem::type() const
{
    return m_type;
}

const MediaUri &MediaItem::uri() const
{
    return m_uri;
}

#include "mediaitem.moc"
