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
    m_parent->m_audioListsStack->ui->semATimeHolder->setVisible(false);
    m_parent->m_audioListsStack->ui->semARatingHolder->setVisible(false);
    m_parent->m_audioListsStack->ui->semAFreqHolder->setVisible(false);
    m_parent->m_videoListsStack->ui->semVTimeHolder->setVisible(false);
    m_parent->m_videoListsStack->ui->semVRatingHolder->setVisible(false);
    m_parent->m_videoListsStack->ui->semVFreqHolder->setVisible(false);
    for (int i = 10; i >= 0; i--) {
        m_parent->m_audioListsStack->ui->semARating->addItem(QIcon(), QString(), i);
    }
    for (int i = 10; i >= 0; i--) {
        m_parent->m_videoListsStack->ui->semVRating->addItem(QIcon(), QString(), i);
    }
    connect(m_parent->m_audioListsStack->ui->aCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(m_parent->m_videoListsStack->ui->vCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(m_parent->m_audioListsStack->ui->semAConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(m_parent->m_videoListsStack->ui->semVConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(m_parent->m_audioListsStack->ui->semASelectMore, SIGNAL(toggled(bool)), this, SLOT(moreSelected(bool)));
    connect(m_parent->m_videoListsStack->ui->semVSelectMore, SIGNAL(toggled(bool)), this, SLOT(moreSelected(bool)));
    m_parent->m_audioListsStack->ui->semATime->setDateTime(QDateTime::currentDateTime());
    m_parent->m_videoListsStack->ui->semVTime->setDateTime(QDateTime::currentDateTime());
}

MediaListSettings::~MediaListSettings()
{
}

void MediaListSettings::showMediaListSettings()
{
    moreSelected(false);
    if (m_parent->currentMediaListSelection() == MainWindow::AudioList) {
        if (m_parent->m_audioListsStack->ui->audioLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = m_parent->m_audioListsStack->ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_parent->m_audioListsModel->mediaItemAt(selectedRow);
            m_parent->m_audioListsStack->ui->aConfigureSemListTitle->setText(selectedItem.title);
            showMediaListSettingsForLri(selectedItem.url);
            readConfigEntryForLRI(selectedItem.url);
        }
    } else {
        if (m_parent->m_videoListsStack->ui->videoLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = m_parent->m_videoListsStack->ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_parent->m_videoListsModel->mediaItemAt(selectedRow);
            m_parent->m_videoListsStack->ui->vConfigureSemListTitle->setText(selectedItem.title);
            showMediaListSettingsForLri(selectedItem.url);
            readConfigEntryForLRI(selectedItem.url);
        }
    }
}

void MediaListSettings::showMediaListSettingsForLri(const QString &lri)
{
    if (m_parent->currentMediaListSelection() == MainWindow::AudioList) {
        if (lri.startsWith("semantics://")) {
            m_parent->m_audioListsStack->ui->audioListsStack->setCurrentIndex(3);
            if (lri.startsWith("semantics://recent")) {
                m_parent->m_audioListsStack->ui->semATimeHolder->show();
                m_parent->m_audioListsStack->ui->semARatingHolder->hide();
                m_parent->m_audioListsStack->ui->semAFreqHolder->hide();
                m_parent->m_audioListsStack->ui->semAMoreVerb->setText(i18n("played"));
            } else if (lri.startsWith("semantics://highest")) {
                m_parent->m_audioListsStack->ui->semATimeHolder->hide();
                m_parent->m_audioListsStack->ui->semARatingHolder->show();
                m_parent->m_audioListsStack->ui->semAFreqHolder->hide();
                m_parent->m_audioListsStack->ui->semAMoreVerb->setText(i18n("rated"));
            } else if (lri.startsWith("semantics://frequent")) {
                m_parent->m_audioListsStack->ui->semATimeHolder->hide();
                m_parent->m_audioListsStack->ui->semARatingHolder->hide();
                m_parent->m_audioListsStack->ui->semAFreqHolder->show();
                m_parent->m_audioListsStack->ui->semAMoreVerb->setText(i18n("played"));
            }
        }
    } else {
        if (lri.startsWith("semantics://")) {
            m_parent->m_videoListsStack->ui->videoListsStack->setCurrentIndex(3);
            if (lri.startsWith("semantics://recent")) {
                m_parent->m_videoListsStack->ui->semVTimeHolder->show();
                m_parent->m_videoListsStack->ui->semVRatingHolder->hide();
                m_parent->m_videoListsStack->ui->semVFreqHolder->hide();
                m_parent->m_videoListsStack->ui->semVMoreVerb->setText(i18n("played"));
            } else if (lri.startsWith("semantics://highest")) {
                m_parent->m_videoListsStack->ui->semVTimeHolder->hide();
                m_parent->m_videoListsStack->ui->semVRatingHolder->show();
                m_parent->m_videoListsStack->ui->semVFreqHolder->hide();
                m_parent->m_videoListsStack->ui->semVMoreVerb->setText(i18n("rated"));
            } else if (lri.startsWith("semantics://frequent")) {
                m_parent->m_videoListsStack->ui->semVTimeHolder->hide();
                m_parent->m_videoListsStack->ui->semVRatingHolder->hide();
                m_parent->m_videoListsStack->ui->semVFreqHolder->show();
                m_parent->m_videoListsStack->ui->semVMoreVerb->setText(i18n("played"));
            }
        }
    }
}

void MediaListSettings::hideMediaListSettings()
{
    if (m_parent->currentMediaListSelection() == MainWindow::AudioList) {
        m_parent->m_audioListsStack->ui->audioListsStack->setCurrentIndex(0);
    } else {
        m_parent->m_videoListsStack->ui->videoListsStack->setCurrentIndex(0);
    }
}

void MediaListSettings::saveMediaListSettings()
{
    if (m_parent->currentMediaListSelection() == MainWindow::AudioList) {
        int selectedRow = m_parent->m_audioListsStack->ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_parent->m_audioListsModel->mediaItemAt(selectedRow);
        writeConfigEntryForLRI(selectedItem.url);
        m_parent->m_audioListsModel->reload();
        m_parent->m_audioListsStack->ui->audioListsStack->setCurrentIndex(0);
    } else {
        int selectedRow = m_parent->m_videoListsStack->ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_parent->m_videoListsModel->mediaItemAt(selectedRow);
        writeConfigEntryForLRI(selectedItem.url);
        m_parent->m_videoListsModel->reload();
        m_parent->m_videoListsStack->ui->videoListsStack->setCurrentIndex(0);
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
        m_parent->m_audioListsStack->ui->semALimit->setValue(limit);
        m_parent->m_audioListsStack->ui->semASelectMore->setChecked(false);
        if (configEntry.count() > 1) {
            if (generalGroup.hasKey(configEntry.at(1))) {
                QStringList entry = generalGroup.readEntry(configEntry.at(1), QStringList());
                if (!entry.isEmpty() && configEntry.at(1) == "RecentAudioPlayed") {
                    if (entry.at(0) == "<") {
                        m_parent->m_audioListsStack->ui->semATimeComp->setCurrentIndex(0);
                    } else {
                        m_parent->m_audioListsStack->ui->semATimeComp->setCurrentIndex(1);
                    }
                    QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
                    m_parent->m_audioListsStack->ui->semATime->setDateTime(recentDateTime);
                    m_parent->m_audioListsStack->ui->semASelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "HighestAudioRated") {
                    if (entry.at(0) == ">=") {
                        m_parent->m_audioListsStack->ui->semARatingComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->m_audioListsStack->ui->semARatingComp->setCurrentIndex(1);
                    } else {
                        m_parent->m_audioListsStack->ui->semARatingComp->setCurrentIndex(2);
                    }
                    int ratingIndex = 10 - (entry.at(1).toInt());
                    if (ratingIndex < 0 || ratingIndex >= m_parent->m_audioListsStack->ui->semARating->count()) {
                        ratingIndex = 0;
                    }
                    m_parent->m_audioListsStack->ui->semARating->setCurrentIndex(ratingIndex);
                    m_parent->m_audioListsStack->ui->semASelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "FrequentAudioPlayed") {
                    if (entry.at(0) == ">=") {
                        m_parent->m_audioListsStack->ui->semAFreqComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->m_audioListsStack->ui->semAFreqComp->setCurrentIndex(1);
                    } else {
                        m_parent->m_audioListsStack->ui->semAFreqComp->setCurrentIndex(2);
                    }
                    int frequentlyPlayedCount = qMax(0, entry.at(1).toInt());
                    m_parent->m_audioListsStack->ui->semAFreq->setValue(frequentlyPlayedCount);
                    m_parent->m_audioListsStack->ui->semASelectMore->setChecked(true);
                }
            }
        }
    } else if (!configEntry.isEmpty() && configEntry.at(0).contains("Video")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = generalGroup.readEntry(configEntry.at(0), 20);
        m_parent->m_videoListsStack->ui->semVLimit->setValue(limit);
        m_parent->m_videoListsStack->ui->semVSelectMore->setChecked(false);
        if (configEntry.count() > 1) {
            if (generalGroup.hasKey(configEntry.at(1))) {
                QStringList entry = generalGroup.readEntry(configEntry.at(1), QStringList());
                if (!entry.isEmpty() && configEntry.at(1) == "RecentVideoPlayed") {
                    if (entry.at(0) == "<") {
                        m_parent->m_videoListsStack->ui->semVTimeComp->setCurrentIndex(0);
                    } else {
                        m_parent->m_videoListsStack->ui->semVTimeComp->setCurrentIndex(1);
                    }
                    QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
                    m_parent->m_videoListsStack->ui->semVTime->setDateTime(recentDateTime);
                    m_parent->m_videoListsStack->ui->semVSelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "HighestVideoRated") {
                    if (entry.at(0) == ">=") {
                        m_parent->m_videoListsStack->ui->semVRatingComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->m_videoListsStack->ui->semVRatingComp->setCurrentIndex(1);
                    } else {
                        m_parent->m_videoListsStack->ui->semVRatingComp->setCurrentIndex(2);
                    }
                    int ratingIndex = 10 - (entry.at(1).toInt());
                    if (ratingIndex < 0 || ratingIndex >= m_parent->m_videoListsStack->ui->semVRating->count()) {
                        ratingIndex = 0;
                    }
                    m_parent->m_videoListsStack->ui->semVRating->setCurrentIndex(ratingIndex);
                    m_parent->m_videoListsStack->ui->semVSelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "FrequentVideoPlayed") {
                    if (entry.at(0) == ">=") {
                        m_parent->m_videoListsStack->ui->semVFreqComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->m_videoListsStack->ui->semVFreqComp->setCurrentIndex(1);
                    } else {
                        m_parent->m_videoListsStack->ui->semVFreqComp->setCurrentIndex(2);
                    }
                    int frequentlyPlayedCount = qMax(0, entry.at(1).toInt());
                    m_parent->m_videoListsStack->ui->semVFreq->setValue(frequentlyPlayedCount);
                    m_parent->m_videoListsStack->ui->semVSelectMore->setChecked(true);
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
        int limit = m_parent->m_audioListsStack->ui->semALimit->value();
        generalGroup.writeEntry(configEntry.at(0), limit);
        if (m_parent->m_audioListsStack->ui->semASelectMore->isChecked() && configEntry.count() > 1) {
            if (configEntry.at(1) == "RecentAudioPlayed") {
                QStringList entry;
                if (m_parent->m_audioListsStack->ui->semATimeComp->currentIndex() == 0) {
                    entry.append("<");
                } else {
                    entry.append(">");
                }
                entry.append(m_parent->m_audioListsStack->ui->semATime->dateTime().toString("yyyyMMddHHmmss"));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "HighestAudioRated") {
                QStringList entry;
                if (m_parent->m_audioListsStack->ui->semARatingComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->m_audioListsStack->ui->semARatingComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(10 - m_parent->m_audioListsStack->ui->semARating->currentIndex()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "FrequentAudioPlayed") {
                QStringList entry;
                if (m_parent->m_audioListsStack->ui->semAFreqComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->m_audioListsStack->ui->semAFreqComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(m_parent->m_audioListsStack->ui->semAFreq->value()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            }
        } else if (configEntry.count() > 1) {
            generalGroup.deleteEntry(configEntry.at(1));
        }
        config.sync();
    } else if (!configEntry.isEmpty() && configEntry.at(0).contains("Video")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = m_parent->m_videoListsStack->ui->semVLimit->value();
        generalGroup.writeEntry(configEntry.at(0), limit);
        if (m_parent->m_videoListsStack->ui->semVSelectMore->isChecked() && configEntry.count() > 1) {
            if (configEntry.at(1) == "RecentVideoPlayed") {
                QStringList entry;
                if (m_parent->m_videoListsStack->ui->semVTimeComp->currentIndex() == 0) {
                    entry.append("<");
                } else {
                    entry.append(">");
                }
                entry.append(m_parent->m_videoListsStack->ui->semVTime->dateTime().toString("yyyyMMddHHmmss"));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "HighestVideoRated") {
                QStringList entry;
                if (m_parent->m_videoListsStack->ui->semVRatingComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->m_videoListsStack->ui->semVRatingComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(10 - m_parent->m_videoListsStack->ui->semVRating->currentIndex()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "FrequentVideoPlayed") {
                QStringList entry;
                if (m_parent->m_videoListsStack->ui->semVFreqComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->m_videoListsStack->ui->semVFreqComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(m_parent->m_videoListsStack->ui->semVFreq->value()));
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
    if (m_parent->currentMediaListSelection() == MainWindow::AudioList) {
        m_parent->m_audioListsStack->ui->semATimeComp->setEnabled(checked);
        m_parent->m_audioListsStack->ui->semATime->setEnabled(checked);
        m_parent->m_audioListsStack->ui->semARatingComp->setEnabled(checked);
        m_parent->m_audioListsStack->ui->semARating->setEnabled(checked);
        m_parent->m_audioListsStack->ui->semAFreqComp->setEnabled(checked);
        m_parent->m_audioListsStack->ui->semAFreq->setEnabled(checked);
    } else {
        m_parent->m_videoListsStack->ui->semVTimeComp->setEnabled(checked);
        m_parent->m_videoListsStack->ui->semVTime->setEnabled(checked);
        m_parent->m_videoListsStack->ui->semVRatingComp->setEnabled(checked);
        m_parent->m_videoListsStack->ui->semVRating->setEnabled(checked);
        m_parent->m_videoListsStack->ui->semVFreqComp->setEnabled(checked);
        m_parent->m_videoListsStack->ui->semVFreq->setEnabled(checked);
    }
}

