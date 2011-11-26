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
#include "../common/bangarangapplication.h"
#include "medialistsmanager.h"
#include "../common/ratingdelegate.h"
#include "../../platform/utilities/utilities.h"
#include "../common/mainwindow.h"
#include "ui_mainwindow.h"
#include "../../platform/mediaitemmodel.h"

#include <KConfig>
#include <KDateTime>
#include <KDebug>

MediaListSettings::MediaListSettings(MainWindow * parent) : QObject(parent)
{
    m_application = (BangarangApplication*)KApplication::kApplication();
    m_parent = parent;
    ui = m_parent->ui;
    m_parent->audioListsStack()->ui->semATimeHolder->setVisible(false);
    m_parent->audioListsStack()->ui->semARatingHolder->setVisible(false);
    m_parent->audioListsStack()->ui->semAFreqHolder->setVisible(false);
    m_parent->videoListsStack()->ui->semVTimeHolder->setVisible(false);
    m_parent->videoListsStack()->ui->semVRatingHolder->setVisible(false);
    m_parent->videoListsStack()->ui->semVFreqHolder->setVisible(false);
    for (int i = 10; i >= 0; i--) {
        m_parent->audioListsStack()->ui->semARating->addItem(QIcon(), QString(), i);
    }
    for (int i = 10; i >= 0; i--) {
        m_parent->videoListsStack()->ui->semVRating->addItem(QIcon(), QString(), i);
    }
    connect(m_parent->audioListsStack()->ui->aCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(m_parent->videoListsStack()->ui->vCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(m_parent->audioListsStack()->ui->semAConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(m_parent->videoListsStack()->ui->semVConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(m_parent->audioListsStack()->ui->semASelectMore, SIGNAL(toggled(bool)), this, SLOT(moreSelected(bool)));
    connect(m_parent->videoListsStack()->ui->semVSelectMore, SIGNAL(toggled(bool)), this, SLOT(moreSelected(bool)));
    m_parent->audioListsStack()->ui->semATime->setDateTime(QDateTime::currentDateTime());
    m_parent->videoListsStack()->ui->semVTime->setDateTime(QDateTime::currentDateTime());
}

MediaListSettings::~MediaListSettings()
{
}

void MediaListSettings::showMediaListSettings()
{
    moreSelected(false);
    if (m_application->mediaListsManager()->currentMediaListSelection() == MediaListsManager::AudioList) {
        if (m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_application->mediaListsManager()->audioListsModel()->mediaItemAt(selectedRow);
            m_parent->audioListsStack()->ui->aConfigureSemListTitle->setText(selectedItem.title);
            showMediaListSettingsForLri(selectedItem.url);
            readConfigEntryForLRI(selectedItem.url);
        }
    } else {
        if (m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_application->mediaListsManager()->videoListsModel()->mediaItemAt(selectedRow);
            m_parent->videoListsStack()->ui->vConfigureSemListTitle->setText(selectedItem.title);
            showMediaListSettingsForLri(selectedItem.url);
            readConfigEntryForLRI(selectedItem.url);
        }
    }
}

void MediaListSettings::showMediaListSettingsForLri(const QString &lri)
{
    if (m_application->mediaListsManager()->currentMediaListSelection() == MediaListsManager::AudioList) {
        if (lri.startsWith("semantics://")) {
            m_parent->audioListsStack()->ui->audioListsStack->setCurrentIndex(3);
            if (lri.startsWith("semantics://recent?")) {
                m_parent->audioListsStack()->ui->semATimeHolder->show();
                m_parent->audioListsStack()->ui->semARatingHolder->hide();
                m_parent->audioListsStack()->ui->semAFreqHolder->hide();
                m_parent->audioListsStack()->ui->semAMoreVerb->setText(i18n("played"));
                m_parent->audioListsStack()->ui->semASelectMore->show();
            } else if (lri.startsWith("semantics://highest?")) {
                m_parent->audioListsStack()->ui->semATimeHolder->hide();
                m_parent->audioListsStack()->ui->semARatingHolder->show();
                m_parent->audioListsStack()->ui->semAFreqHolder->hide();
                m_parent->audioListsStack()->ui->semAMoreVerb->setText(i18n("rated"));
                m_parent->audioListsStack()->ui->semASelectMore->show();
            } else if (lri.startsWith("semantics://frequent?")) {
                m_parent->audioListsStack()->ui->semATimeHolder->hide();
                m_parent->audioListsStack()->ui->semARatingHolder->hide();
                m_parent->audioListsStack()->ui->semAFreqHolder->show();
                m_parent->audioListsStack()->ui->semAMoreVerb->setText(i18n("played"));
                m_parent->audioListsStack()->ui->semASelectMore->show();
            } else if (lri.startsWith("semantics://recentlyadded?")) {
                m_parent->audioListsStack()->ui->semATimeHolder->hide();
                m_parent->audioListsStack()->ui->semARatingHolder->hide();
                m_parent->audioListsStack()->ui->semAFreqHolder->hide();
                m_parent->audioListsStack()->ui->semAMoreVerb->hide();
                m_parent->audioListsStack()->ui->semASelectMore->hide();
            }
        }
    } else {
        if (lri.startsWith("semantics://")) {
            m_parent->videoListsStack()->ui->videoListsStack->setCurrentIndex(3);
            if (lri.startsWith("semantics://recent?")) {
                m_parent->videoListsStack()->ui->semVTimeHolder->show();
                m_parent->videoListsStack()->ui->semVRatingHolder->hide();
                m_parent->videoListsStack()->ui->semVFreqHolder->hide();
                m_parent->videoListsStack()->ui->semVMoreVerb->setText(i18n("played"));
                m_parent->videoListsStack()->ui->semVSelectMore->show();
            } else if (lri.startsWith("semantics://highest?")) {
                m_parent->videoListsStack()->ui->semVTimeHolder->hide();
                m_parent->videoListsStack()->ui->semVRatingHolder->show();
                m_parent->videoListsStack()->ui->semVFreqHolder->hide();
                m_parent->videoListsStack()->ui->semVMoreVerb->setText(i18n("rated"));
                m_parent->videoListsStack()->ui->semVSelectMore->show();
            } else if (lri.startsWith("semantics://frequent?")) {
                m_parent->videoListsStack()->ui->semVTimeHolder->hide();
                m_parent->videoListsStack()->ui->semVRatingHolder->hide();
                m_parent->videoListsStack()->ui->semVFreqHolder->show();
                m_parent->videoListsStack()->ui->semVMoreVerb->setText(i18n("played"));
                m_parent->videoListsStack()->ui->semVSelectMore->show();
            } else if (lri.startsWith("semantics://recentlyadded?")) {
                m_parent->videoListsStack()->ui->semVTimeHolder->hide();
                m_parent->videoListsStack()->ui->semVRatingHolder->hide();
                m_parent->videoListsStack()->ui->semVFreqHolder->hide();
                m_parent->videoListsStack()->ui->semVMoreVerb->hide();
                m_parent->videoListsStack()->ui->semVSelectMore->hide();
            }
        }
    }
}

void MediaListSettings::hideMediaListSettings()
{
    if (m_application->mediaListsManager()->currentMediaListSelection() == MediaListsManager::AudioList) {
        m_parent->audioListsStack()->ui->audioListsStack->setCurrentIndex(0);
    } else {
        m_parent->videoListsStack()->ui->videoListsStack->setCurrentIndex(0);
    }
}

void MediaListSettings::saveMediaListSettings()
{
    if (m_application->mediaListsManager()->currentMediaListSelection() == MediaListsManager::AudioList) {
        int selectedRow = m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_application->mediaListsManager()->audioListsModel()->mediaItemAt(selectedRow);
        writeConfigEntryForLRI(selectedItem.url);
        m_application->mediaListsManager()->audioListsModel()->reload();
        m_parent->audioListsStack()->ui->audioListsStack->setCurrentIndex(0);
    } else {
        int selectedRow = m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_application->mediaListsManager()->videoListsModel()->mediaItemAt(selectedRow);
        writeConfigEntryForLRI(selectedItem.url);
        m_application->mediaListsManager()->videoListsModel()->reload();
        m_parent->videoListsStack()->ui->videoListsStack->setCurrentIndex(0);
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
    } else if (lri.startsWith("semantics://recentlyadded?audio")) {
        configEntry.append("RecentlyAddedAudioLimit");
        configEntry.append("RecentlyAddedAudioRated");
    } else if (lri.startsWith("semantics://recent?video")) {
        configEntry.append("RecentVideoLimit");
        configEntry.append("RecentVideoPlayed");
    } else if (lri.startsWith("semantics://frequent?video")) {
        configEntry.append("FrequentVideoLimit");
        configEntry.append("FrequentVideoPlayed");
    } else if (lri.startsWith("semantics://highest?video")) {
        configEntry.append("HighestVideoLimit");
        configEntry.append("HighestVideoRated");
    } else if (lri.startsWith("semantics://recentlyadded?video")) {
        configEntry.append("RecentlyAddedVideoLimit");
        configEntry.append("RecentlyAddedVideoRated");
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
        m_parent->audioListsStack()->ui->semALimit->setValue(limit);
        m_parent->audioListsStack()->ui->semASelectMore->setChecked(false);
        if (configEntry.count() > 1) {
            if (generalGroup.hasKey(configEntry.at(1))) {
                QStringList entry = generalGroup.readEntry(configEntry.at(1), QStringList());
                if (!entry.isEmpty() && configEntry.at(1) == "RecentAudioPlayed") {
                    if (entry.at(0) == "<") {
                        m_parent->audioListsStack()->ui->semATimeComp->setCurrentIndex(0);
                    } else {
                        m_parent->audioListsStack()->ui->semATimeComp->setCurrentIndex(1);
                    }
                    QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
                    m_parent->audioListsStack()->ui->semATime->setDateTime(recentDateTime);
                    m_parent->audioListsStack()->ui->semASelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "HighestAudioRated") {
                    if (entry.at(0) == ">=") {
                        m_parent->audioListsStack()->ui->semARatingComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->audioListsStack()->ui->semARatingComp->setCurrentIndex(1);
                    } else {
                        m_parent->audioListsStack()->ui->semARatingComp->setCurrentIndex(2);
                    }
                    int ratingIndex = 10 - (entry.at(1).toInt());
                    if (ratingIndex < 0 || ratingIndex >= m_parent->audioListsStack()->ui->semARating->count()) {
                        ratingIndex = 0;
                    }
                    m_parent->audioListsStack()->ui->semARating->setCurrentIndex(ratingIndex);
                    m_parent->audioListsStack()->ui->semASelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "FrequentAudioPlayed") {
                    if (entry.at(0) == ">=") {
                        m_parent->audioListsStack()->ui->semAFreqComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->audioListsStack()->ui->semAFreqComp->setCurrentIndex(1);
                    } else {
                        m_parent->audioListsStack()->ui->semAFreqComp->setCurrentIndex(2);
                    }
                    int frequentlyPlayedCount = qMax(0, entry.at(1).toInt());
                    m_parent->audioListsStack()->ui->semAFreq->setValue(frequentlyPlayedCount);
                    m_parent->audioListsStack()->ui->semASelectMore->setChecked(true);
                }
            }
        }
    } else if (!configEntry.isEmpty() && configEntry.at(0).contains("Video")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = generalGroup.readEntry(configEntry.at(0), 20);
        m_parent->videoListsStack()->ui->semVLimit->setValue(limit);
        m_parent->videoListsStack()->ui->semVSelectMore->setChecked(false);
        if (configEntry.count() > 1) {
            if (generalGroup.hasKey(configEntry.at(1))) {
                QStringList entry = generalGroup.readEntry(configEntry.at(1), QStringList());
                if (!entry.isEmpty() && configEntry.at(1) == "RecentVideoPlayed") {
                    if (entry.at(0) == "<") {
                        m_parent->videoListsStack()->ui->semVTimeComp->setCurrentIndex(0);
                    } else {
                        m_parent->videoListsStack()->ui->semVTimeComp->setCurrentIndex(1);
                    }
                    QDateTime recentDateTime = QDateTime::fromString(entry.at(1), "yyyyMMddHHmmss");
                    m_parent->videoListsStack()->ui->semVTime->setDateTime(recentDateTime);
                    m_parent->videoListsStack()->ui->semVSelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "HighestVideoRated") {
                    if (entry.at(0) == ">=") {
                        m_parent->videoListsStack()->ui->semVRatingComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->videoListsStack()->ui->semVRatingComp->setCurrentIndex(1);
                    } else {
                        m_parent->videoListsStack()->ui->semVRatingComp->setCurrentIndex(2);
                    }
                    int ratingIndex = 10 - (entry.at(1).toInt());
                    if (ratingIndex < 0 || ratingIndex >= m_parent->videoListsStack()->ui->semVRating->count()) {
                        ratingIndex = 0;
                    }
                    m_parent->videoListsStack()->ui->semVRating->setCurrentIndex(ratingIndex);
                    m_parent->videoListsStack()->ui->semVSelectMore->setChecked(true);
                } else if (!entry.isEmpty() && configEntry.at(1) == "FrequentVideoPlayed") {
                    if (entry.at(0) == ">=") {
                        m_parent->videoListsStack()->ui->semVFreqComp->setCurrentIndex(0);
                    } else if (entry.at(0) == "=") {
                        m_parent->videoListsStack()->ui->semVFreqComp->setCurrentIndex(1);
                    } else {
                        m_parent->videoListsStack()->ui->semVFreqComp->setCurrentIndex(2);
                    }
                    int frequentlyPlayedCount = qMax(0, entry.at(1).toInt());
                    m_parent->videoListsStack()->ui->semVFreq->setValue(frequentlyPlayedCount);
                    m_parent->videoListsStack()->ui->semVSelectMore->setChecked(true);
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
        int limit = m_parent->audioListsStack()->ui->semALimit->value();
        generalGroup.writeEntry(configEntry.at(0), limit);
        if (m_parent->audioListsStack()->ui->semASelectMore->isChecked() && configEntry.count() > 1) {
            if (configEntry.at(1) == "RecentAudioPlayed") {
                QStringList entry;
                if (m_parent->audioListsStack()->ui->semATimeComp->currentIndex() == 0) {
                    entry.append("<");
                } else {
                    entry.append(">");
                }
                entry.append(m_parent->audioListsStack()->ui->semATime->dateTime().toString("yyyyMMddHHmmss"));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "HighestAudioRated") {
                QStringList entry;
                if (m_parent->audioListsStack()->ui->semARatingComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->audioListsStack()->ui->semARatingComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(10 - m_parent->audioListsStack()->ui->semARating->currentIndex()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "FrequentAudioPlayed") {
                QStringList entry;
                if (m_parent->audioListsStack()->ui->semAFreqComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->audioListsStack()->ui->semAFreqComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(m_parent->audioListsStack()->ui->semAFreq->value()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            }
        } else if (configEntry.count() > 1) {
            generalGroup.deleteEntry(configEntry.at(1));
        }
        config.sync();
    } else if (!configEntry.isEmpty() && configEntry.at(0).contains("Video")) {
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        int limit = m_parent->videoListsStack()->ui->semVLimit->value();
        generalGroup.writeEntry(configEntry.at(0), limit);
        if (m_parent->videoListsStack()->ui->semVSelectMore->isChecked() && configEntry.count() > 1) {
            if (configEntry.at(1) == "RecentVideoPlayed") {
                QStringList entry;
                if (m_parent->videoListsStack()->ui->semVTimeComp->currentIndex() == 0) {
                    entry.append("<");
                } else {
                    entry.append(">");
                }
                entry.append(m_parent->videoListsStack()->ui->semVTime->dateTime().toString("yyyyMMddHHmmss"));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "HighestVideoRated") {
                QStringList entry;
                if (m_parent->videoListsStack()->ui->semVRatingComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->videoListsStack()->ui->semVRatingComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(10 - m_parent->videoListsStack()->ui->semVRating->currentIndex()));
                generalGroup.writeEntry(configEntry.at(1), entry);
            } else if (configEntry.at(1) == "FrequentVideoPlayed") {
                QStringList entry;
                if (m_parent->videoListsStack()->ui->semVFreqComp->currentIndex() == 0) {
                    entry.append(">=");
                } else if (m_parent->videoListsStack()->ui->semVFreqComp->currentIndex() == 1) {
                    entry.append("=");
                } else {
                    entry.append("<=");
                }
                entry.append(QString("%1").arg(m_parent->videoListsStack()->ui->semVFreq->value()));
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
    if (m_application->mediaListsManager()->currentMediaListSelection() == MediaListsManager::AudioList) {
        m_parent->audioListsStack()->ui->semATimeComp->setEnabled(checked);
        m_parent->audioListsStack()->ui->semATime->setEnabled(checked);
        m_parent->audioListsStack()->ui->semARatingComp->setEnabled(checked);
        m_parent->audioListsStack()->ui->semARating->setEnabled(checked);
        m_parent->audioListsStack()->ui->semAFreqComp->setEnabled(checked);
        m_parent->audioListsStack()->ui->semAFreq->setEnabled(checked);
    } else {
        m_parent->videoListsStack()->ui->semVTimeComp->setEnabled(checked);
        m_parent->videoListsStack()->ui->semVTime->setEnabled(checked);
        m_parent->videoListsStack()->ui->semVRatingComp->setEnabled(checked);
        m_parent->videoListsStack()->ui->semVRating->setEnabled(checked);
        m_parent->videoListsStack()->ui->semVFreqComp->setEnabled(checked);
        m_parent->videoListsStack()->ui->semVFreq->setEnabled(checked);
    }
}

