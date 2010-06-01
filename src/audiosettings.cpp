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

AudioSettings::AudioSettings(MainWindow * parent) : QObject(parent)
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
    connect(ui->eq1, SIGNAL(valueChanged(int)), this, SLOT(eq1Changed(int)));
    connect(ui->eq2, SIGNAL(valueChanged(int)), this, SLOT(eq2Changed(int)));
    connect(ui->eq3, SIGNAL(valueChanged(int)), this, SLOT(eq3Changed(int)));
    connect(ui->eq4, SIGNAL(valueChanged(int)), this, SLOT(eq4Changed(int)));
    connect(ui->eq5, SIGNAL(valueChanged(int)), this, SLOT(eq5Changed(int)));
    connect(ui->eq6, SIGNAL(valueChanged(int)), this, SLOT(eq6Changed(int)));
    connect(ui->eq7, SIGNAL(valueChanged(int)), this, SLOT(eq7Changed(int)));
    connect(ui->eq8, SIGNAL(valueChanged(int)), this, SLOT(eq8Changed(int)));
    connect(ui->eq9, SIGNAL(valueChanged(int)), this, SLOT(eq9Changed(int)));
    connect(ui->eq10, SIGNAL(valueChanged(int)), this, SLOT(eq10Changed(int)));
    connect(ui->eq11, SIGNAL(valueChanged(int)), this, SLOT(eq11Changed(int)));
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
            audioPath->insertEffect(m_audioEq);
            eqCapable = true;
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
        preset << ui->eq1->value();
        preset << ui->eq2->value();
        preset << ui->eq3->value();
        preset << ui->eq4->value();
        preset << ui->eq5->value();
        preset << ui->eq6->value();
        preset << ui->eq7->value();
        preset << ui->eq8->value();
        preset << ui->eq9->value();
        preset << ui->eq10->value();
        preset << ui->eq11->value();
        m_eqPresets.replace(manualIndex, preset);
        disconnect(ui->eqPresets, SIGNAL(currentIndexChanged(const QString)), this, SLOT(loadPreset(const QString)));
        ui->eqPresets->setCurrentIndex(manualIndex);
        connect(ui->eqPresets, SIGNAL(currentIndexChanged(const QString)), this, SLOT(loadPreset(const QString)));
    }
}

void AudioSettings::setEq(const QList<int> &preset)
{
    if (preset.count() == 11) {
        disconnect(ui->eq1, SIGNAL(valueChanged(int)), this, SLOT(eq1Changed(int)));
        disconnect(ui->eq2, SIGNAL(valueChanged(int)), this, SLOT(eq2Changed(int)));
        disconnect(ui->eq3, SIGNAL(valueChanged(int)), this, SLOT(eq3Changed(int)));
        disconnect(ui->eq4, SIGNAL(valueChanged(int)), this, SLOT(eq4Changed(int)));
        disconnect(ui->eq5, SIGNAL(valueChanged(int)), this, SLOT(eq5Changed(int)));
        disconnect(ui->eq6, SIGNAL(valueChanged(int)), this, SLOT(eq6Changed(int)));
        disconnect(ui->eq7, SIGNAL(valueChanged(int)), this, SLOT(eq7Changed(int)));
        disconnect(ui->eq8, SIGNAL(valueChanged(int)), this, SLOT(eq8Changed(int)));
        disconnect(ui->eq9, SIGNAL(valueChanged(int)), this, SLOT(eq9Changed(int)));
        disconnect(ui->eq10, SIGNAL(valueChanged(int)), this, SLOT(eq10Changed(int)));
        disconnect(ui->eq11, SIGNAL(valueChanged(int)), this, SLOT(eq11Changed(int)));
        m_audioEq->setParameterValue(m_audioEq->parameters()[0], preset.at(0));
        ui->eq1->setValue(preset.at(0));
        m_audioEq->setParameterValue(m_audioEq->parameters()[1], preset.at(1));
        ui->eq2->setValue(preset.at(1));
        m_audioEq->setParameterValue(m_audioEq->parameters()[2], preset.at(2));
        ui->eq3->setValue(preset.at(2));
        m_audioEq->setParameterValue(m_audioEq->parameters()[3], preset.at(3));
        ui->eq4->setValue(preset.at(3));
        m_audioEq->setParameterValue(m_audioEq->parameters()[4], preset.at(4));
        ui->eq5->setValue(preset.at(4));
        m_audioEq->setParameterValue(m_audioEq->parameters()[5], preset.at(5));
        ui->eq6->setValue(preset.at(5));
        m_audioEq->setParameterValue(m_audioEq->parameters()[6], preset.at(6));
        ui->eq7->setValue(preset.at(6));
        m_audioEq->setParameterValue(m_audioEq->parameters()[7], preset.at(7));
        ui->eq8->setValue(preset.at(7));
        m_audioEq->setParameterValue(m_audioEq->parameters()[8], preset.at(8));
        ui->eq9->setValue(preset.at(8));
        m_audioEq->setParameterValue(m_audioEq->parameters()[9], preset.at(9));
        ui->eq10->setValue(preset.at(9));
        m_audioEq->setParameterValue(m_audioEq->parameters()[10], preset.at(10));
        ui->eq11->setValue(preset.at(10));
        connect(ui->eq1, SIGNAL(valueChanged(int)), this, SLOT(eq1Changed(int)));
        connect(ui->eq2, SIGNAL(valueChanged(int)), this, SLOT(eq2Changed(int)));
        connect(ui->eq3, SIGNAL(valueChanged(int)), this, SLOT(eq3Changed(int)));
        connect(ui->eq4, SIGNAL(valueChanged(int)), this, SLOT(eq4Changed(int)));
        connect(ui->eq5, SIGNAL(valueChanged(int)), this, SLOT(eq5Changed(int)));
        connect(ui->eq6, SIGNAL(valueChanged(int)), this, SLOT(eq6Changed(int)));
        connect(ui->eq7, SIGNAL(valueChanged(int)), this, SLOT(eq7Changed(int)));
        connect(ui->eq8, SIGNAL(valueChanged(int)), this, SLOT(eq8Changed(int)));
        connect(ui->eq9, SIGNAL(valueChanged(int)), this, SLOT(eq9Changed(int)));
        connect(ui->eq10, SIGNAL(valueChanged(int)), this, SLOT(eq10Changed(int)));
        connect(ui->eq11, SIGNAL(valueChanged(int)), this, SLOT(eq11Changed(int)));
    }
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

void AudioSettings::eq1Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[0], v); 
    updateManualEqPresets();
}

void AudioSettings::eq2Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[1], v); 
    updateManualEqPresets();
}

void AudioSettings::eq3Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[2], v); 
    updateManualEqPresets();
}

void AudioSettings::eq4Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[3], v); 
    updateManualEqPresets();
}

void AudioSettings::eq5Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[4], v); 
    updateManualEqPresets();
}

void AudioSettings::eq6Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[5], v); 
    updateManualEqPresets();
}

void AudioSettings::eq7Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[6], v); 
    updateManualEqPresets();
}

void AudioSettings::eq8Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[7], v); 
    updateManualEqPresets();
}

void AudioSettings::eq9Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[8], v); 
    updateManualEqPresets();
}

void AudioSettings::eq10Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[9], v); 
    updateManualEqPresets();
}

void AudioSettings::eq11Changed(int v)
{
    m_audioEq->setParameterValue(m_audioEq->parameters()[10], v); 
    updateManualEqPresets();
}
