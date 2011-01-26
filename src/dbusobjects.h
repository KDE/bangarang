/* BANGARANG MEDIA PLAYER
 * Copyright (C) 2010 Ni Hui (shuizhuyuanluo@126.com)
 * <http://gitorious.org/bangarang>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBUSOBJECTS_H
#define DBUSOBJECTS_H

#include <QVariantMap>
#include <phonon/mediaobject.h>

class BangarangApplication;

struct MprisStatusStruct;
struct MprisVersionStruct;

typedef QList< QPair<QString, QString> > MprisMetaDataFieldList;

class MprisRootObject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.MediaPlayer")
    public:
        explicit MprisRootObject(QObject *parent);
        ~MprisRootObject();

        const MprisMetaDataFieldList &metaDataFieldList() { return m_metaDataFields; }

    public Q_SLOTS:
        QString Identity();
        void Quit();
        MprisVersionStruct MprisVersion();

    private:
        MprisMetaDataFieldList m_metaDataFields;
};

class MprisPlayerObject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.MediaPlayer")
    public:
        MprisPlayerObject(BangarangApplication *app_);
        ~MprisPlayerObject();

    public Q_SLOTS:
        void Next();
        void Prev();
        void Pause();
        void Stop();
        void Play();
        void Repeat(bool repeat);
        MprisStatusStruct GetStatus();
        QVariantMap GetMetadata();
        int GetCaps();
        void VolumeSet(int volume);
        int VolumeGet();
        void PositionSet(int position);
        int PositionGet();

    Q_SIGNALS:
        void TrackChange(const QVariantMap &metadata);
        void StatusChange(const MprisStatusStruct &status);
        void CapsChange(int capabilities);

    private:
        BangarangApplication *m_app;

    private Q_SLOTS:
        void slotTrackChange();
        void slotStatusChange();
        void slotMediaStateChange(Phonon::State newstate, Phonon::State oldstate);
};

class MprisTrackListObject : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.MediaPlayer")
    public:
        MprisTrackListObject(BangarangApplication *app_);
        ~MprisTrackListObject();

    public Q_SLOTS:
        QVariantMap GetMetadata(int index);
        int GetCurrentTrack();
        int GetLength();
        int AddTrack(const QString &url, bool playImmediately);
        void DelTrack(int index);
        void SetLoop(bool loop);
        void SetRandom(bool random);

    Q_SIGNALS:
        void TrackListChange(int size);

    private:
        BangarangApplication *m_app;

    private Q_SLOTS:
        void slotTrackListChange();
};

struct MprisStatusStruct
{
    int state;
    int random;
    int repeatTrack;
    int repeatPlaylist;
};

Q_DECLARE_METATYPE(MprisStatusStruct)

struct MprisVersionStruct
{
    quint16 major;
    quint16 minor;
};

Q_DECLARE_METATYPE(MprisVersionStruct)

#endif // DBUSOBJECTS_H
