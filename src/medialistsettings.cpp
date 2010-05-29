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
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"

#include <KConfig>
#include <KDebug>

MediaListSettings::MediaListSettings(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
    connect(ui->aCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(ui->vCancelSemConfigure, SIGNAL(clicked()), this, SLOT(hideMediaListSettings()));
    connect(ui->semAConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
    connect(ui->semVConfigSave, SIGNAL(clicked()), this, SLOT(saveMediaListSettings()));
}

MediaListSettings::~MediaListSettings()
{
}

void MediaListSettings::showMediaListSettings()
{
    if (ui->mediaLists->currentIndex() == 0) {
        ui->audioListsStack->setCurrentIndex(3);
        if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_parent->m_audioListsModel->mediaItemAt(selectedRow);
            ui->aConfigureSemListTitle->setText(selectedItem.title);
            KConfig config;
            KConfigGroup generalGroup( &config, "General" );
            int limit = generalGroup.readEntry(configEntryForLRI(selectedItem.url), 20);
            ui->semALimit->setValue(limit);
        }
    } else {
        ui->videoListsStack->setCurrentIndex(3);
        if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0) {
            int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
            MediaItem selectedItem = m_parent->m_videoListsModel->mediaItemAt(selectedRow);
            ui->vConfigureSemListTitle->setText(selectedItem.title);
            KConfig config;
            KConfigGroup generalGroup( &config, "General" );
            int limit = generalGroup.readEntry(configEntryForLRI(selectedItem.url), 20);
            ui->semVLimit->setValue(limit);
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
        int limit;
        limit = ui->semALimit->value();
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        generalGroup.writeEntry(configEntryForLRI(selectedItem.url), limit);
        config.sync();
        m_parent->m_audioListsModel->reload();
        ui->audioListsStack->setCurrentIndex(0);
    } else {
        int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_parent->m_videoListsModel->mediaItemAt(selectedRow);
        int limit;
        limit = ui->semVLimit->value();
        KConfig config;
        KConfigGroup generalGroup( &config, "General" );
        generalGroup.writeEntry(configEntryForLRI(selectedItem.url), limit);
        config.sync();
        m_parent->m_videoListsModel->reload();
        ui->videoListsStack->setCurrentIndex(0);
    }
}

QString MediaListSettings::configEntryForLRI(const QString &lri)
{
    QString configEntry;
    if (lri.startsWith("semantics://recent?audio")) {
        configEntry = "RecentAudioLimit";
    } else if (lri.startsWith("semantics://frequent?audio")) {
        configEntry = "FrequentAudioLimit";
    } else if (lri.startsWith("semantics://highest?audio")) {
        configEntry = "HighestAudioLimit";
    } else if (lri.startsWith("semantics://recent?video")) {
        configEntry = "RecentVideoLimit";
    } else if (lri.startsWith("semantics://frequent?video")) {
        configEntry = "FrequentVideoLimit";
    } else if (lri.startsWith("semantics://highest?video")) {
        configEntry = "HighestVideoLimit";
    }
    return configEntry;
}
