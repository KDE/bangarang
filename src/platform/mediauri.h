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

#ifndef MEDIAURI_H
#define MEDIAURI_H

#include <QObject>
#include <QMap>

class MediaUri : public QObject
{
    Q_OBJECT

public:
    MediaUri(const QString &uri);
    MediaUri();
    MediaUri& operator=(const MediaUri& mediaUri);

    const QString& engine() const;
    const QString& path() const;
    const QMap<QString, QString>& parameters() const;
    const QString& uri() const;

    bool isNull() const;

private:
    QString m_engine;
    QString m_path;
    QMap<QString, QString> m_parameters;
    QString m_uri;
};

#endif // MEDIAURI_H
