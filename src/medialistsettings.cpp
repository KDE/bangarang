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

#include "medialistsettings.h"
#include "ratingdelegate.h"
#include "platform/utilities/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"

#include <KConfig>
#include <KDateTime>
#include <KDebug>

MediaListSettings::MediaListSettings(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
    ui->semATimeHolder->setVisible(false);
    ui->semARatingHolder->setVisible(false);
    ui->semAFreqHolder->setVisible(false);
    ui->semVTimeHolder->setVisible(false);
    ui->semVRatingHolder->setVisible(false);
    ui->semVFreqHolder->setVisible(false);
    for (int i = 10; i >= 0; i--) {
        ui->semARating->addItem(QIcon(), QString(), i);
    }
    for (int i = 10; i >= 0; i--) {
        ui->semVRating->addItem(QIcon(), QString(), i);
    }
    connect(ui->aCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(ui->vCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(ui->semAConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(ui->semVConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(ui->semASelectMore, SIGNAL(toggled(bool)), this, SLOT(moreSelected(bool)));
    connect(ui->semVSelectMore, SIGNAL(toggled(bool)), this, SLOT(moreSelected(bool)));
    ui->semATime->setDateTime(QDateTime::currentDateTime());
    ui->semVTime->setDateTime(QDateTime::currentDateTime());
}

MediaListSettings::~MediaListSettings()
{
}

void MediaListSettings::showMediaListSettings()
{
    moreSelected(false);
    if (ui->mediaLists->currentIndex() == 0) {
        if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_parent->m_audioListsModel->mediaItemAt(selectedRow);
            ui->aConfigureSemListTitle->setText(selectedItem.title);
            showMediaListSettingsForLri(selectedItem.url);
            readConfigEntryForLRI(selectedItem.url);
        }
    } else {
        if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_parent->m_videoListsModel->mediaItemAt(selectedRow);
            ui->vConfigureSemListTitle->setText(selectedItem.title);
            showMediaListSettingsForLri(selectedItem.url);
            readConfigEntryForLRI(selectedItem.url);
        }
    }
}

void MediaListSettings::showMediaListSettingsForLri(const QString &lri)
{
    if (ui->mediaLists->currentIndex() == 0) {
        if (lri.startsWith("semantics://")) {
            ui->audioListsStack->setCurrentIndex(3);
            if (lri.startsWith("semantics://recent")) {
                ui->semATimeHolder->show();
                ui->semARatingHolder->hide();
                ui->semAFreqHolder->hide();
                ui->semAMoreVerb->setText(i18n("played"));
            } else if (lri.startsWith("semantics://highest")) {
                ui->semATimeHolder->hide();
                ui->semARatingHolder->show();
                ui->semAFreqHolder->hide();
                ui->semAMoreVerb->setText(i18n("rated"));
            } else if (lri.startsWith("semantics://frequent")) {
                ui->semATimeHolder->hide();
                ui->semARatingHolder->hide();
                ui->semAFreqHolder->show();
                ui->semAMoreVerb->setText(i18n("played"));
            }
        }
    } else {
        if (lri.startsWith("semantics://")) {
            ui->videoListsStack->setCurrentIndex(3);
            if (lri.startsWith("semantics://recent")) {
                ui->semVTimeHolder->show();
                ui->semVRatingHolder->hide();
                ui->semVFreqHolder->hide();
                ui->semVMoreVerb->setText(i18n("played"));
            } else if (lri.startsWith("semantics://highest")) {
                ui->semVTimeHolder->hide();
                ui->semVRatingHolder->show();
                ui->semVFreqHolder->hide();
                ui->semVMoreVerb->setText(i18n("rated"));
            } else if (lri.startsWith("semantics://frequent")) {
                ui->semVTimeHolder->hide();
                ui->semVRatingHolder->hide();
                ui->semVFreqHolder->show();
                ui->semVMoreVerb->setText(i18n("played"));
            }
        }
    }
}

void MediaListSettings::hideMediaListSettings()
{
    if (ui->mediaLists->currentIndex() == 0) {
        ui->audioListsStack->setCurrentIndex(0);
    } else {
        ui->videoListsStack->setCurrentIndex(0);
    }
}

void MediaListSettings::saveMediaListSettings()
{
    if (ui->mediaLists->currentIndex() == 0) {
        int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_parent->m_audioListsModel->mediaItemAt(selectedRow);
        writeConfigEntryForLRI(selectedItem.url);
        m_parent->m_audioListsModel->reload();
        ui->audioListsStack->setCurrentIndex(0);
    } else {
        int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_parent->m_videoListsModel->mediaItemAt(selectedRow);
        writeConfigEntryForLRI(selectedItem.url);
        m_parent->m_videoListsModel->reload();
        ui->videoListsStack->setCurrentIndex(0);
    }
}

QStringList MediaListSettings::configEntryForLRI(const QString &lri)
{
    QStringList configEntry;
    if (lri.startsWith("semantics://recent?audio")) {
        configEntry.append("RecentAudioLimit");
        configEntry.append("RecentAudioPlayed");
    } else if (lri.startsWith("semantics://frequent?audio")) {
        configEntry.append("FrequentAudioLimit");
        configEntry.append("FrequentAudioPlayed");
    } else if (lri.startsWith("semantics://highest?audio")) {
        configEntry.append("HighestAudioLimit");
        configEntry.append("HighestAudioRated");
    } else if (lri.startsWith("semantics://recent?video")) {
        configEntry.append("RecentVideoLimit");
        configEntry.append("RecentVideoPlayed");
    } else if (lri.startsWith("semantics://frequent?video")) {
        configEntry.append("FrequentVideoLimit");
        configEntry.append("FrequentVideoPlayed");
    } else if (lri.startsWith("semantics://highest?video")) {
        configEntry.append("HighestVideoLimit");
        configEntry.append("HighestVideoRated");
    }
    return configEntry;
}

void MediaListSettings::readConfigEntryForLRI(const QString &lri)
{
    QStringList configEntry = configEntryForLRI(lri);
    if (!configEntry.isEmpty() && configEntry.at(0).contains("Audio")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = generalGroup.readEntry(configEntry.at(0), 20);
        ui->semALimit->setValue(limit);
        ui->semASelectMore->setChecked(false);
        if (configEntry.count() > 1) {
            if (generalGroup.hasKey(configEntry.at(1))) {
                QStringList entry = generalGroup.readEntry(configEntry.at(1), QStringList());
                if (!entry.isEmpty() && configEntry.at(1) == "RecentAudioPlayed") {
                    if (entry.at(0) == "<") {
                        ui->semATimeComp->setCurrentIndex(0);
                    } else {
                        ui->semATimeComp->setCurrentIndex(1);
                    }
                    QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
                    ui->semATime->setDateTime(recentDateTime);
                    ui->semASelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "HighestAudioRated") {
                    if (entry.at(0) == ">=") {
                        ui->semARatingComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        ui->semARatingComp->setCurrentIndex(1);
                    } else {
                        ui->semARatingComp->setCurrentIndex(2);
                    }
                    int ratingIndex = 10 - (entry.at(1).toInt());
                    if (ratingIndex < 0 || ratingIndex >= ui->semARating->count()) {
                        ratingIndex = 0;
                    }
                    ui->semARating->setCurrentIndex(ratingIndex);
                    ui->semASelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "FrequentAudioPlayed") {
                    if (entry.at(0) == ">=") {
                        ui->semAFreqComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        ui->semAFreqComp->setCurrentIndex(1);
                    } else {
                        ui->semAFreqComp->setCurrentIndex(2);
                    }
                    int frequentlyPlayedCount = qMax(0, entry.at(1).toInt());
                    ui->semAFreq->setValue(frequentlyPlayedCount);
                    ui->semASelectMore->setChecked(true);
                }
            }
        }
    } else if (!configEntry.isEmpty() && configEntry.at(0).contains("Video")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = generalGroup.readEntry(configEntry.at(0), 20);
        ui->semVLimit->setValue(limit);
        ui->semVSelectMore->setChecked(false);
        if (configEntry.count() > 1) {
            if (generalGroup.hasKey(configEntry.at(1))) {
                QStringList entry = generalGroup.readEntry(configEntry.at(1), QStringList());
                if (!entry.isEmpty() && configEntry.at(1) == "RecentVideoPlayed") {
                    if (entry.at(0) == "<") {
                        ui->semVTimeComp->setCurrentIndex(0);
                    } else {
                        ui->semVTimeComp->setCurrentIndex(1);
                    }
                    QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
                    ui->semVTime->setDateTime(recentDateTime);
                    ui->semVSelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "HighestVideoRated") {
                    if (entry.at(0) == ">=") {
                        ui->semVRatingComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        ui->semVRatingComp->setCurrentIndex(1);
                    } else {
                        ui->semVRatingComp->setCurrentIndex(2);
                    }
                    int ratingIndex = 10 - (entry.at(1).toInt());
                    if (ratingIndex < 0 || ratingIndex >= ui->semVRating->count()) {
                        ratingIndex = 0;
                    }
                    ui->semVRating->setCurrentIndex(ratingIndex);
                    ui->semVSelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "FrequentVideoPlayed") {
                    if (entry.at(0) == ">=") {
                        ui->semVFreqComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        ui->semVFreqComp->setCurrentIndex(1);
                    } else {
                        ui->semVFreqComp->setCurrentIndex(2);
                    }
                    int frequentlyPlayedCount = qMax(0, entry.at(1).toInt());
                    ui->semVFreq->setValue(frequentlyPlayedCount);
                    ui->semVSelectMore->setChecked(true);
                }
            }
        }
    }
}

void MediaListSettings::writeConfigEntryForLRI(const QString &lri)
{
    QStringList configEntry = configEntryForLRI(lri);
    if (!configEntry.isEmpty() && configEntry.at(0).contains("Audio")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = ui->semALimit->value();
        generalGroup.writeEntry(configEntry.at(0), limit);
        if (ui->semASelectMore->isChecked() && configEntry.count() > 1) {
            if (configEntry.at(1) == "RecentAudioPlayed") {
                QStringList entry;
                if (ui->semATimeComp->currentIndex() == 0) {
                    entry.append("<");
                } else {
                    entry.append(">");
                }
                entry.append(ui->semATime->dateTime().toString("yyyyMMddHHmmss"));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "HighestAudioRated") {
                QStringList entry;
                if (ui->semARatingComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (ui->semARatingComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(10 - ui->semARating->currentIndex()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "FrequentAudioPlayed") {
                QStringList entry;
                if (ui->semAFreqComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (ui->semAFreqComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(ui->semAFreq->value()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            }
        } else if (configEntry.count() > 1) {
            generalGroup.deleteEntry(configEntry.at(1));
        }
        config.sync();
    } else if (!configEntry.isEmpty() && configEntry.at(0).contains("Video")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = ui->semVLimit->value();
        generalGroup.writeEntry(configEntry.at(0), limit);
        if (ui->semVSelectMore->isChecked() && configEntry.count() > 1) {
            if (configEntry.at(1) == "RecentVideoPlayed") {
                QStringList entry;
                if (ui->semVTimeComp->currentIndex() == 0) {
                    entry.append("<");
                } else {
                    entry.append(">");
                }
                entry.append(ui->semVTime->dateTime().toString("yyyyMMddHHmmss"));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "HighestVideoRated") {
                QStringList entry;
                if (ui->semVRatingComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (ui->semVRatingComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(10 - ui->semVRating->currentIndex()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "FrequentVideoPlayed") {
                QStringList entry;
                if (ui->semVFreqComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (ui->semVFreqComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(ui->semVFreq->value()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            }
        } else if (configEntry.count() > 1) {
            generalGroup.deleteEntry(configEntry.at(1));
        }
        config.sync();
    }
}

void MediaListSettings::moreSelected(bool checked)
{
    if (ui->mediaLists->currentIndex() == 0) {
        ui->semATimeComp->setEnabled(checked);
        ui->semATime->setEnabled(checked);
        ui->semARatingComp->setEnabled(checked);
        ui->semARating->setEnabled(checked);
        ui->semAFreqComp->setEnabled(checked);
        ui->semAFreq->setEnabled(checked);
    } else {
        ui->semVTimeComp->setEnabled(checked);
        ui->semVTime->setEnabled(checked);
        ui->semVRatingComp->setEnabled(checked);
        ui->semVRating->setEnabled(checked);
        ui->semVFreqComp->setEnabled(checked);
        ui->semVFreq->setEnabled(checked);
    }
}

