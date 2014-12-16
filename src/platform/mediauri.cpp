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

#include "mediauri.h"
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

MediaUri::MediaUri(const QString& uri): QObject()
{
    m_uri = QString(uri);
    QUrl url(uri);
    m_engine = url.scheme();
    m_path = url.path();
    QPair<QString, QString> pair;
    foreach (pair, QUrlQuery(url).queryItems())
    {
        m_parameters[pair.first] = pair.second;
    }
}

MediaUri::MediaUri(): QObject()
{
}

MediaUri& MediaUri::MediaUri::operator=(const MediaUri& mediaUri)
{
    m_engine = mediaUri.engine();
    m_path = mediaUri.path();
    m_parameters = mediaUri.parameters();
    m_uri = mediaUri.uri();
    return *this;
}

const QString& MediaUri::engine() const
{
    return m_engine;
}

const QMap< QString, QString >& MediaUri::parameters() const
{
    return m_parameters;
}

const QString& MediaUri::uri() const
{
    return m_uri;
}

const QString& MediaUri::path() const
{
    return m_path;
}

#include "mediauri.moc"
