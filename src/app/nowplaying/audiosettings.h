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

#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

#include <KConfigGroup>
#include <QtCore>
#include <QTreeView>
#include <QAction>
#include <phonon/path.h>
#include <phonon/effect.h>
#include <phonon/effectparameter.h>
#include <phonon/mediacontroller.h>

class BangarangApplication;
namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItemModel;
class MediaItemDelegate;

/*
 * This class is provide audio settings functions.
 */
class AudioSettings : public QObject
{
    Q_OBJECT
    public:
        AudioSettings(MainWindow * parent = 0);
        ~AudioSettings();
        void setAudioPath(Phonon::Path *audioPath);
        void reconnectAudioPath(Phonon::Path *audioPath);
        void setMediaController(Phonon::MediaController *mediaController);
        void saveAudioSettings(KConfigGroup *configGroup);
        void restoreAudioSettings(KConfigGroup *configGroup);
        void connectEq();
        void disconnectEq();
        void enableTouch();
        
    public slots:
        void loadPreset(const QString &presetName);
        void setEq(const QList<int> &set);
        void restoreDefaults();
        void setAudioChannel(int idx);
        void updateAudioChannelCombo();
        
    private:
        bool insertAudioEffects(Phonon::Path *audioPath);
        void connectAudioChannelCombo();
        void disconnectAudioChannelCombo();
        
        BangarangApplication * m_application;
        MainWindow * m_mainWindow;
        Ui::MainWindowClass *ui;
        Phonon::Effect * m_audioEq;
        Phonon::MediaController * m_mediaController;
        QStringList m_eqPresetNames;
        QList<QList<int> > m_eqPresets;
        QList<QSlider *> m_uiEqs;
        void updateManualEqPresets();
        
    private slots:
        void eqChanged(int v);
        void updateAudioChannels();


};
#endif // AUDIOSETTINGS_H
