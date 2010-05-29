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
        void saveAudioSettings(KConfigGroup *configGroup);
        void restoreAudioSettings(KConfigGroup *configGroup);

    public slots:
        void loadPreset(const QString &presetName);
        void setEq(const QList<int> &set);
        void restoreDefaults();
        
    private:
        MainWindow * m_mainWindow;
        Ui::MainWindowClass *ui;
        Phonon::Effect * m_audioEq;
        QStringList m_eqPresetNames;
        QList<QList<int> > m_eqPresets;
        void updateManualEqPresets();
        
    private slots:
        void eq1Changed(int v);
        void eq2Changed(int v);
        void eq3Changed(int v);
        void eq4Changed(int v);
        void eq5Changed(int v);
        void eq6Changed(int v);
        void eq7Changed(int v);
        void eq8Changed(int v);
        void eq9Changed(int v);
        void eq10Changed(int v);
        void eq11Changed(int v);

};
#endif // AUDIOSETTINGS_H
