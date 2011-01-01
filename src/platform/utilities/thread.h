/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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
#ifndef UTILITIES_THREAD_H
#define UTILITIES_THREAD_H

#include "../mediaitemmodel.h"
#include <QThread>
#include <QtCore>
#include <QImage>

namespace Utilities {
    class Thread : public QThread
    {
        Q_OBJECT
    public:
        explicit Thread(QObject *parent = 0);
        ~Thread();
        void run();
        void getArtworksFromMediaItem(const MediaItem &mediaItem, bool ignoreCache = false);
        void getArtworkFromMediaItem(const MediaItem &mediaItem, bool ignoreCache = false);

    private:
        MediaItem m_mediaItem;
        QString m_action;
        bool m_ignoreCache;

    signals:
        void gotArtworks(QList<QImage> artworks, MediaItem mediaItem);
        void gotArtwork(const QImage &artwork, const MediaItem &mediaItem);
    };
}

#endif // UTLITIES_THREAD_H
