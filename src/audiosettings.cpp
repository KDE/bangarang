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
#include "platform/utilities/utilities.h"
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

using namespace Phonon;

AudioSettings::AudioSettings(MainWindow * parent) : QObject(parent)
{
    /*Set up basics */
    m_application = (BangarangApplication *) KApplication::kApplication();
    m_mainWindow = parent;
    ui = m_mainWindow->ui;
    m_mediaController = NULL;

    //Setup and connect ui widgets
    connect(ui->restoreDefaultAudioSettings, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(ui->hideAudioSettings, SIGNAL(clicked()), m_application->actionsManager()->action("show_audio_settings"), SLOT(trigger()));
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
    for (int i = 0; i < 11; i++) {
        m_uiEqs.at(i)->setProperty("EQ_NO", i);
    }

    ui->audioChannelSelectionHolder->setEnabled(false);
}

AudioSettings::~AudioSettings()
{
}

void AudioSettings::setMediaController(MediaController* mediaController)
{
    m_mediaController = mediaController;
    updateAudioChannels();
    connect(m_mediaController, SIGNAL(availableAudioChannelsChanged()), this, SLOT(updateAudioChannels()));
    connectAudioChannelCombo();
}

void AudioSettings::setAudioPath(Path *audioPath)
{
    //Determine if equalizer capability is present
    if (!insertAudioEffects(audioPath)) {
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

void AudioSettings::reconnectAudioPath(Path* audioPath)
{
    if (!insertAudioEffects(audioPath)) {
       ui->eqHolder->setEnabled(false);
       return;
    }

    //be sure the values are still set
    for (int i = 0; i < m_audioEq->parameters().count(); i++ ) {
        m_audioEq->setParameterValue(m_audioEq->parameters()[i], m_uiEqs.at(i)->value());
    }
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
    if (m_audioEq == NULL) {
        return;
    }
    disconnectEq();
    for (int i = 0; i < m_audioEq->parameters().count(); i++ ) {
        m_audioEq->setParameterValue(m_audioEq->parameters().at(i), preset.at(i));
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
    if (!var.isValid() || var.toInt() >= m_audioEq->parameters().count()) {
        return;
    }
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

void AudioSettings::setAudioChannel(int idx)
{
    if ( idx < 0 )
        return;
    int sidx = ui->audioChannelSelection->itemData(idx).toInt();
    if ( m_mediaController->currentAudioChannel().index() == sidx )
        return;
    AudioChannelDescription aud = AudioChannelDescription::fromIndex(sidx);
    m_mediaController->setCurrentAudioChannel(aud);
}

void AudioSettings::updateAudioChannels()
{
    disconnectAudioChannelCombo();
    QComboBox *cb = ui->audioChannelSelection;
    QList<AudioChannelDescription> auds = m_mediaController->availableAudioChannels();
    int no = auds.count();
    ui->audioChannelSelectionHolder->setEnabled( no > 1 ); //has at least one audio channel
    cb->clear();
    foreach (AudioChannelDescription aud, auds) {
        QString descr = aud.description().trimmed();
        QString more = descr.isEmpty() ? QString() : QString(" (%1)").arg(descr);
        QString name = aud.name().trimmed();
        QString trans_name = m_application->locale()->languageCodeToName( name );
        QString display_name = trans_name.isEmpty() ? name : trans_name;
        cb->addItem( display_name + more, QVariant( aud.index() ));
    }
    updateAudioChannelCombo(); //will also reconnect
}

void AudioSettings::updateAudioChannelCombo()
{
    disconnectAudioChannelCombo();
    int curIdx = m_mediaController->currentAudioChannel().index();
    QComboBox *cb = ui->audioChannelSelection;
    for (int i = 0; i < cb->count(); i++) {
        if (cb->itemData(i).toInt() == curIdx) {
            cb->setCurrentIndex(i);
            break;
        }
    }
    connectAudioChannelCombo();
}

bool AudioSettings::insertAudioEffects(Path* audioPath)
{
    QList<EffectDescription> effects = BackendCapabilities::availableAudioEffects();
    foreach (EffectDescription effect, effects) {
        if(effect.name() != "KEqualizer") {
            continue;
        }
        m_audioEq = new Effect(effect, this);
        if (m_audioEq == NULL)
            continue;
        audioPath->insertEffect(m_audioEq);
        connectEq();
        return true;
    }
    m_audioEq = NULL;
    return false;
}


void AudioSettings::connectAudioChannelCombo()
{
    connect(ui->audioChannelSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setAudioChannel(int)));
}

void AudioSettings::disconnectAudioChannelCombo()
{
    disconnect(ui->audioChannelSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setAudioChannel(int)));
}
