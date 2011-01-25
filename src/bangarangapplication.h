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

#ifndef BANGARANGAPPLICATION_H
#define BANGARANGAPPLICATION_H

#include <KApplication>
#include <KAboutData>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <QtCore>

class VideoSettings;
class MediaListProperties;
class MediaItemModel;
class MediaListCache;
class Playlist;
class InfoManager;
class SavedListsManager;
class ActionsManager;
class BookmarksManager;
class AudioSettings;
class MainWindow;
class BangarangNotifierItem;
class MprisRootObject;

class BangarangApplication : public KApplication
{
  Q_OBJECT
  
    public:
        ~BangarangApplication();
        void setup();
        MainWindow * mainWindow();
        Phonon::MediaObject * mediaObject();
        Phonon::MediaObject * newMediaObject();
        Phonon::AudioOutput * audioOutput();
        qreal volume();
        Playlist * playlist();
        MediaItemModel * browsingModel();
        MediaListCache * sharedMediaListCache();
        InfoManager * infoManager();
        SavedListsManager * savedListsManager();
        ActionsManager * actionsManager();
        BookmarksManager * bookmarksManager();
        BangarangNotifierItem * statusNotifierItem();
        AudioSettings * audioSettings();
        VideoSettings * videoSettings() { return m_videoSettings; }
        MprisRootObject * mprisRootObject();
        KLocale * locale() { return m_locale; }
        const KAboutData * aboutData();
        void processCommandLineArgs();
        
    private:
        MainWindow * m_mainWindow;
        KLocale * m_locale;
        Phonon::MediaObject * m_mediaObject;
        Phonon::AudioOutput * m_audioOutput;
        Phonon::Path m_audioPath;
        Phonon::Path m_videoPath;
        qreal m_volume;
        Playlist * m_playlist;
        MediaItemModel * m_browsingModel;
        MediaListCache * m_sharedMediaListCache;
        InfoManager * m_infoManager;
        SavedListsManager * m_savedListsManager;
        ActionsManager * m_actionsManager;
        BookmarksManager * m_bookmarksManager;
        BangarangNotifierItem * m_statusNotifierItem;
        AudioSettings * m_audioSettings;
        VideoSettings * m_videoSettings;
        MprisRootObject * m_mprisRootObject;
        bool m_nepomukInited;

    private slots:
        void handleNotifierStateRequest(Phonon::State state);
        void nowPlayingChanged();
        void volumeChanged(qreal newVolume);
        void volumeChanged(int delta);
        
};
#endif
