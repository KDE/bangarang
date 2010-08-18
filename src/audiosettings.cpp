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

#include "audiosettings.h"
#include "bangarangapplication.h"
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "actionsmanager.h"
#include "platform/playlist.h"

#include <KStandardDirs>
#include <KMessageBox>
#include <KHelpMenu>
#include <KDebug>
#include <QFile>
#include <phonon/backendcapabilities.h>

AudioSettings::AudioSettings(MainWindow * parent) : QObject(parent), m_eqCount(11)
{
    /*Set up basics */
    BangarangApplication * application = (BangarangApplication *)KApplication::kApplication();
    m_mainWindow = parent;
    ui = m_mainWindow->ui;

    //Setup and connect ui widgets
    connect(ui->restoreDefaultAudioSettings, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui->hideAudioSettings, SIGNAL(clicked()), application->actionsManager()->action("show_audio_settings"), SLOT(trigger()));
    ui->eq1Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq2Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq3Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq4Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq5Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq6Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq7Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq8Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq9Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq10Label->setFont(KGlobalSettings::smallestReadableFont());
    ui->eq11Label->setFont(KGlobalSettings::smallestReadableFont());
    
    m_uiEqs << ui->eq1 << ui->eq2 << ui->eq3 << ui->eq4 << ui->eq5 << ui->eq6 << ui->eq7 << ui->eq8 << ui->eq9 << ui->eq10 << ui->eq11;
    for (int i = 0; i < m_eqCount; i++) {
        m_uiEqs.at(i)->setProperty("EQ_NO", i);
    }
    

    //connect the eq only if the audio path was set and the m_audioEq was initialized!
}

AudioSettings::~AudioSettings()
{
}

void AudioSettings::setAudioPath(Phonon::Path *audioPath)
{
    //Determine if equalizer capability is present
    bool eqCapable = false;
    QList<Phonon::EffectDescription> effects = Phonon::BackendCapabilities::availableAudioEffects();
    foreach (Phonon::EffectDescription effect, effects) {
        if(effect.name()=="KEqualizer") {
            m_audioEq = new Phonon::Effect(effect, this);
            if (m_audioEq == NULL)
                continue;
            audioPath->insertEffect(m_audioEq);
            eqCapable = true;
            connectEq();
            break;
        }
    }
    
    if (!eqCapable) {
        ui->eqHolder->setEnabled(false);
    }
    //Load presets
    QList<int> preset;
    preset <<0<<0<<0<<0<<0<<0<<0<<0<<0<<0<<0;
    m_eqPresetNames << i18n("No effect");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<6<<5<<3<<0<<-2<<-2<<0<<3<<4<<6;
    m_eqPresetNames << i18n("Rock");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<6<<5<<0<<-2<<-1<<0<<0<<2<<4<<6;
    m_eqPresetNames << i18n("Reggae");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<6<<6<<4<<0<<-1<<-1<<0<<0<<2<<4;
    m_eqPresetNames << i18n("Dance");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<6<<2<<-1<<2<<4<<4<<4<<4<<2<<2;
    m_eqPresetNames << i18n("Live");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<-2<<-2<<1<<0<<1<<2<<2<<1<<0<<0;
    m_eqPresetNames << i18n("Classical");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<8<<6<<0<<0<<1<<2<<1<<0<<3<<5;
    m_eqPresetNames << i18n("Blockbuster");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<0<<0<<1<<2<<3<<4<<3<<2<<1<<0;
    m_eqPresetNames << i18n("Documentary");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<6<<3<<1<<2<<4<<4<<3<<0<<2<<4;
    m_eqPresetNames << i18n("Drama");
    m_eqPresets << preset;
    preset.clear();
    preset <<0<<0<<0<<0<<0<<0<<0<<0<<0<<0<<0;
    m_eqPresetNames << i18n("Manual");
    m_eqPresets << preset;
    
    ui->eqPresets->addItems(m_eqPresetNames);
    ui->eqPresets->setCurrentIndex(0);
    connect(ui->eqPresets, SIGNAL(currentIndexChanged(const QString)), this, SLOT(loadPreset(const QString)));
}

void AudioSettings::saveAudioSettings(KConfigGroup *configGroup)
{
    configGroup->writeEntry("EqualizerPresetSelection", ui->eqPresets->currentIndex());
    QList<int> manualPreset = m_eqPresets.at(m_eqPresetNames.indexOf(i18n("Manual")));
    QStringList manualPresetStrL;
    for (int i = 0; i < manualPreset.count(); i++) {
        manualPresetStrL << QString("%1").arg(manualPreset.at(i));
    }
    configGroup->writeEntry("EqualizerManualPreset", manualPresetStrL.join(","));
}

void AudioSettings::restoreAudioSettings(KConfigGroup *configGroup)
{
    QString manualPresetStr = configGroup->readEntry("EqualizerManualPreset", QString());
    QStringList manualPresetStrL = manualPresetStr.split(",");
    if (manualPresetStrL.count() == 11) {
        QList<int> preset;
        for (int i = 0; i < manualPresetStrL.count(); i++) {
            preset << manualPresetStrL.at(i).toInt();
        }
        int indexOfManual = m_eqPresetNames.indexOf(i18n("Manual"));
        if (indexOfManual != -1) {
            m_eqPresets.replace(indexOfManual, preset);
        }
    }
    ui->eqPresets->setCurrentIndex(configGroup->readEntry("EqualizerPresetSelection", 0));
}

void AudioSettings::loadPreset(const QString &presetName)
{
    int indexOfPreset = m_eqPresetNames.indexOf(presetName);
    if (indexOfPreset != -1) {
        QList<int> preset = m_eqPresets.at(indexOfPreset);
        setEq(preset);
    }
}

void AudioSettings::updateManualEqPresets()
{
    int manualIndex = m_eqPresetNames.indexOf(i18n("Manual"));
    if (manualIndex != -1) {
        QList<int> preset;
        foreach (QSlider *eq, m_uiEqs) {
            preset << eq->value();
        }
        m_eqPresets.replace(manualIndex, preset);
        disconnect(ui->eqPresets, SIGNAL(currentIndexChanged(const QString)), this, SLOT(loadPreset(const QString)));
        ui->eqPresets->setCurrentIndex(manualIndex);
        connect(ui->eqPresets, SIGNAL(currentIndexChanged(const QString)), this, SLOT(loadPreset(const QString)));
    }
}

void AudioSettings::setEq(const QList<int> &preset)
{
    if (m_audioEq == NULL)
        return;
    if (preset.count() != m_eqCount)
        return;
    disconnectEq();
    QList<EffectParameter> params = m_audioEq->parameters();
    for (int i = 0; i < m_eqCount; i++ ) {
        m_audioEq->setParameterValue(params[i], preset.at(i));
        m_uiEqs.at(i)->setValue(preset.at(i));
    }

    connectEq();
}

void AudioSettings::restoreDefaults()
{
    int manualIndex = m_eqPresetNames.indexOf(i18n("Manual"));
    if (manualIndex != -1) {
        QList<int> preset;
        preset <<0<<0<<0<<0<<0<<0<<0<<0<<0<<0<<0;
        m_eqPresets.replace(manualIndex, preset);
    }
    ui->eqPresets->setCurrentIndex(0);
}

void AudioSettings::eqChanged(int v)
{
    QVariant var = sender()->property("EQ_NO");
    if (!var.isValid())
        return;
    m_audioEq->setParameterValue(m_audioEq->parameters()[var.toInt()], v); 
    updateManualEqPresets();
}


void AudioSettings::connectEq()
{
    foreach (QSlider *eq, m_uiEqs) {
        connect(eq, SIGNAL(valueChanged(int)), this, SLOT(eqChanged(int)));
    }
}

void AudioSettings::disconnectEq()
{
    foreach (QSlider *eq, m_uiEqs) {
        disconnect(eq, SIGNAL(valueChanged(int)), this, SLOT(eqChanged(int)));
    }
}

