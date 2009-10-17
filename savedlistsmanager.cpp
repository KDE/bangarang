/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)
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

#include "savedlistsmanager.h"
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"
#include "platform/playlist.h"

#include <KStandardDirs>
#include <QFile>

SavedListsManager::SavedListsManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
    
    ui->aListSourceSelection->setEnabled(false);
    ui->vListSourceSelection->setEnabled(false);
    
    connect(ui->addAudioList, SIGNAL(clicked()), this, SLOT(showAudioListSave()));
    connect(ui->addVideoList, SIGNAL(clicked()), this, SLOT(showVideoListSave()));
    connect(ui->aCancelSaveList, SIGNAL(clicked()), this, SLOT(hideAudioListSave()));
    connect(ui->vCancelSaveList, SIGNAL(clicked()), this, SLOT(hideVideoListSave()));
    connect(ui->saveAudioList, SIGNAL(clicked()), this, SLOT(saveAudioList()));
    connect(ui->addVideoList, SIGNAL(clicked()), this, SLOT(saveVideoList()));
    connect(ui->aNewListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(ui->vNewListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(selectionChanged(const QItemSelection, const QItemSelection)));
    
}

SavedListsManager::~SavedListsManager()
{
}

void SavedListsManager::showAudioListSave()
{
    ui->audioListsStack->setCurrentIndex(1);
    ui->aNewListName->setText("Untitled");
    enableValidSave();
}

void SavedListsManager::showVideoListSave()
{
    ui->videoListsStack->setCurrentIndex(1);
    ui->vNewListName->setText("Untitled");
    enableValidSave();
}

void SavedListsManager::hideAudioListSave()
{
    ui->aNewListName->clear();
    ui->audioListsStack->setCurrentIndex(0);
}

void SavedListsManager::hideVideoListSave()
{
    ui->vNewListName->clear();
    ui->videoListsStack->setCurrentIndex(0);
}

void SavedListsManager::saveAudioList()
{
    if (ui->aListSourceSelection->isChecked()) {
        //Get selected media items and save
        QList<MediaItem> mediaList;
        QList<MediaItem> viewMediaList = m_parent->m_mediaItemModel->mediaList();
        QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            mediaList.append(viewMediaList.at(selectedRows.at(i).row()));
        }
        saveMediaList(mediaList, ui->aNewListName->text(), QString("Audio"));
    } else if (ui->aListSourceView->isChecked()) {
        saveView(ui->aNewListName->text(), QString("Audio"));
    } else if (ui->aListSourcePlaylist->isChecked()) {
        QList<MediaItem> mediaList = m_parent->m_playlist->playlistModel()->mediaList();
        saveMediaList(mediaList, ui->aNewListName->text(), QString("Audio"));
    }
    hideAudioListSave();
}

void SavedListsManager::saveVideoList()
{
    if (ui->vListSourceSelection->isChecked()) {
        //Get selected media items and save
        QList<MediaItem> mediaList;
        QList<MediaItem> viewMediaList = m_parent->m_mediaItemModel->mediaList();
        QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            mediaList.append(viewMediaList.at(selectedRows.at(i).row()));
        }
        saveMediaList(mediaList, ui->vNewListName->text(), QString("Video"));
    } else if (ui->vListSourceView->isChecked()) {
        saveView(ui->vNewListName->text(), QString("Video"));
    } else if (ui->vListSourcePlaylist->isChecked()) {
        QList<MediaItem> mediaList = m_parent->m_playlist->playlistModel()->mediaList();
        saveMediaList(mediaList, ui->vNewListName->text(), QString("Video"));
    }
    hideVideoListSave();
}

void SavedListsManager::enableValidSave(QString newText)
{
    ui->saveAudioList->setEnabled(true);
    if (!ui->aNewListName->text().isEmpty()) {
        ui->saveAudioList->setEnabled(true);
    } else {
        ui->saveAudioList->setEnabled(false);
    }
    if (!ui->vNewListName->text().isEmpty()) {
        ui->saveVideoList->setEnabled(true);
    } else {
        ui->saveVideoList->setEnabled(false);
    } 
    Q_UNUSED(newText); //not used since method may be called directly
}

void SavedListsManager::selectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        QString listItemType = m_parent->m_mediaItemModel->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            ui->aListSourceSelection->setEnabled(true);
            ui->vListSourceSelection->setEnabled(true);
        }
    } else {
        ui->aListSourceSelection->setChecked(false);
        ui->vListSourceSelection->setChecked(false);
        ui->aListSourceSelection->setEnabled(false);
        ui->vListSourceSelection->setEnabled(false);
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}


void SavedListsManager::saveMediaList(QList<MediaItem> mediaList, QString name, QString type)
{
    if (!name.isEmpty()) {
        
        //Create and populate M3U file
        QString filename = name;
        filename = filename.replace(" ", "");
        QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/%1.m3u").arg(filename), false));
        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1.m3u").arg(filename), true));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }
        QTextStream out(&file);
        out << "#EXTM3U" << "\r\n";
        for (int i = 0; i < mediaList.count(); i++) {
            out << "#EXTINF:" << mediaList.at(i).fields["duration"].toInt() << "," << mediaList.at(i).title << "\r\n";
            out << mediaList.at(i).url << "\r\n";
        }
        file.close();
        
        //Add to saved list index
        QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", true));
        if (!indexFile.open(QIODevice::Append | QIODevice::Text)) {
            return;
        }
        QTextStream outIndex(&indexFile);
        outIndex << type << ":::" << name << ":::" << QString("savedlists://%1.m3u").arg(filename) << "\r\n";
        indexFile.close();
        
        //Add saved item to Audio/Video lists
        MediaListProperties listItemProperties;
        listItemProperties.name = name;
        listItemProperties.lri = QString("savedlists://%1").arg(QString("%1.m3u").arg(filename));
        if (type == "Audio") {
            int row = ui->audioLists->count();
            ui->audioLists->addItem(name);
            m_parent->setListItemProperties(ui->audioLists->item(row), listItemProperties);
        } else if (type == "Video") {
            int row = ui->videoLists->count();
            ui->videoLists->addItem(name);
            m_parent->setListItemProperties(ui->videoLists->item(row), listItemProperties);
        }
    }
}

void SavedListsManager::saveView(QString name, QString type)
{
    if (!name.isEmpty()) {
        //Add to saved list index
        QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", true));
        if (!indexFile.open(QIODevice::Append | QIODevice::Text)) {
            return;
        }
        QTextStream outIndex(&indexFile);
        outIndex << type << ":::" << name << ":::" << m_parent->m_mediaItemModel->mediaListProperties().lri << "\r\n";
        indexFile.close();
        
        //Add saved item to Audio/Video lists
        MediaListProperties listItemProperties;
        listItemProperties.name = name;
        listItemProperties.lri = m_parent->m_mediaItemModel->mediaListProperties().lri;
        if (type == "Audio") {
            int row = ui->audioLists->count();
            ui->audioLists->addItem(name);
            m_parent->setListItemProperties(ui->audioLists->item(row), listItemProperties);
        } else if (type == "Video") {
            int row = ui->videoLists->count();
            ui->videoLists->addItem(name);
            m_parent->setListItemProperties(ui->videoLists->item(row), listItemProperties);
        }
    }
}

void SavedListsManager::showSavedLists()
{
    //Load lists from index
    QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream in(&indexFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList nameUrl = line.split(":::");
        if (nameUrl.count() >= 3) {
            QString type = nameUrl.at(0).trimmed();
            QString name = nameUrl.at(1).trimmed();
            QString lri = nameUrl.at(2).trimmed();
            MediaListProperties listItemProperties;
            listItemProperties.name = name;
            listItemProperties.lri = lri;
            if (type == "Audio") {
                int row = ui->audioLists->count();
                ui->audioLists->addItem(name);
                m_parent->setListItemProperties(ui->audioLists->item(row), listItemProperties);
            } else if (type == "Video") {
                int row = ui->videoLists->count();
                ui->videoLists->addItem(name);
                m_parent->setListItemProperties(ui->videoLists->item(row), listItemProperties);
            }
        }
    }
    indexFile.close();
}

