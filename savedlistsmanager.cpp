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
#include <KMessageBox>
#include <QFile>

SavedListsManager::SavedListsManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;
    
    ui->aListSourceSelection->setEnabled(false);
    ui->vListSourceSelection->setEnabled(false);
    loadSavedListsIndex();
    
    connect(ui->addAudioList, SIGNAL(clicked()), this, SLOT(showAudioListSave()));
    connect(ui->addVideoList, SIGNAL(clicked()), this, SLOT(showVideoListSave()));
    connect(ui->aCancelSaveList, SIGNAL(clicked()), this, SLOT(hideAudioListSave()));
    connect(ui->vCancelSaveList, SIGNAL(clicked()), this, SLOT(hideVideoListSave()));
    connect(ui->saveAudioList, SIGNAL(clicked()), this, SLOT(saveAudioList()));
    connect(ui->saveVideoList, SIGNAL(clicked()), this, SLOT(saveVideoList()));
    connect(ui->aNewListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(ui->vNewListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(ui->aNewListName, SIGNAL(returnPressed()), this, SLOT(saveAudioList()));
    connect(ui->vNewListName, SIGNAL(returnPressed()), this, SLOT(saveVideoList()));
    connect(ui->removeAudioList, SIGNAL(clicked()), this, SLOT(removeAudioList()));
    connect(ui->removeVideoList, SIGNAL(clicked()), this, SLOT(removeVideoList()));
    
    
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(selectionChanged(const QItemSelection, const QItemSelection)));
    connect(ui->audioLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(audioListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(ui->videoLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(videoListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_parent->m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
}

SavedListsManager::~SavedListsManager()
{
}

void SavedListsManager::showAudioListSave()
{
    ui->audioListsStack->setCurrentIndex(1);
    ui->aNewListName->setText("Untitled");
    if (ui->aListSourceSelection->isEnabled()) {
        ui->aListSourceSelection->setChecked(true);
    } else if (ui->aListSourceView->isEnabled()) {
        ui->aListSourceView->setChecked(true);
    } else {
        ui->aListSourcePlaylist->setChecked(true);
    }
    ui->aNewListName->setFocus();
    enableValidSave();
}

void SavedListsManager::showVideoListSave()
{
    ui->videoListsStack->setCurrentIndex(1);
    ui->vNewListName->setText("Untitled");
    if (ui->vListSourceSelection->isEnabled()) {
        ui->vListSourceSelection->setChecked(true);
    } else if (ui->vListSourceView->isEnabled()) {
        ui->vListSourceView->setChecked(true);
    } else {
        ui->vListSourcePlaylist->setChecked(true);
    }
    ui->vNewListName->setFocus();
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
    MediaListProperties audioListsProperties = m_parent->m_audioListsModel->mediaListProperties();
    m_parent->m_audioListsModel->clearMediaListData();
    m_parent->m_audioListsModel->setMediaListProperties(audioListsProperties);
    m_parent->m_audioListsModel->load();
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
    MediaListProperties videoListsProperties = m_parent->m_videoListsModel->mediaListProperties();
    m_parent->m_videoListsModel->clearMediaListData();
    m_parent->m_videoListsModel->setMediaListProperties(videoListsProperties);
    m_parent->m_videoListsModel->load();
    hideVideoListSave();
}

void SavedListsManager::removeAudioList()
{
    if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0){
        int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        QString name = m_parent->m_audioListsModel->mediaItemAt(selectedRow).title;
        
        KGuiItem removeSavedList;
        removeSavedList.setText(QString("Remove"));
        removeSavedList.setIcon(KIcon("list-remove"));
        QString message = QString("Are you sure you want to remove \"%1\"?").arg(name);
        
        if (KMessageBox::warningContinueCancel(m_parent, message, QString(), removeSavedList) == KMessageBox::Continue) {
            //Remove M3U file
            QString filename = name;
            filename = filename.replace(" ", "");
            QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/Audio-%1.m3u").arg(filename), false));
            
            QString savedListEntry = QString("Audio:::%1")
                                        .arg(name);
            QList<int> rowsToRemove;
            for (int i = 0; i < m_savedAudioLists.count(); i++) {
                if (m_savedAudioLists.at(i).startsWith(savedListEntry)) {
                    rowsToRemove << i;
                }
            }
            for (int i = 0; i < rowsToRemove.count(); i++) {
                m_savedAudioLists.removeAt(rowsToRemove.at(i));
            }
            updateSavedListsIndex();
            MediaListProperties audioListsProperties = m_parent->m_audioListsModel->mediaListProperties();
            m_parent->m_audioListsModel->clearMediaListData();
            m_parent->m_audioListsModel->setMediaListProperties(audioListsProperties);
            m_parent->m_audioListsModel->load();
        }
    }
}

void SavedListsManager::removeVideoList()
{
    if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0){
        int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        QString name = m_parent->m_videoListsModel->mediaItemAt(selectedRow).title;
        
        KGuiItem removeSavedList;
        removeSavedList.setText(QString("Remove"));
        removeSavedList.setIcon(KIcon("list-remove"));
        QString message = QString("Are you sure you want to remove \"%1\"?").arg(name);
        
        if (KMessageBox::warningContinueCancel(m_parent, message, QString(), removeSavedList) == KMessageBox::Continue) {
            //Remove M3U file
            QString filename = name;
            filename = filename.replace(" ", "");
            QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/Video-%1.m3u").arg(filename), false));
            
            QString savedListEntry = QString("Video:::%1")
                                        .arg(name);
            QList<int> rowsToRemove;
            for (int i = 0; i < m_savedVideoLists.count(); i++) {
                if (m_savedVideoLists.at(i).startsWith(savedListEntry)) {
                    rowsToRemove << i;
                }
            }
            for (int i = 0; i < rowsToRemove.count(); i++) {
                m_savedVideoLists.removeAt(rowsToRemove.at(i));
            }
            updateSavedListsIndex();
            MediaListProperties videoListsProperties = m_parent->m_videoListsModel->mediaListProperties();
            m_parent->m_videoListsModel->clearMediaListData();
            m_parent->m_videoListsModel->setMediaListProperties(videoListsProperties);
            m_parent->m_videoListsModel->load();
        }
    }
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

void SavedListsManager::audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if (selected.indexes().count() > 0) {
        bool isSavedList = selected.indexes().at(0).data(MediaItem::IsSavedListRole).toBool();
        if (isSavedList) {
            ui->removeAudioList->setEnabled(true);
        } else {
            ui->removeAudioList->setEnabled(false);
        }
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void SavedListsManager::videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if (selected.indexes().count() > 0) {
        bool isSavedList = selected.indexes().at(0).data(MediaItem::IsSavedListRole).toBool();
        if (isSavedList) {
            ui->removeVideoList->setEnabled(true);
        } else {
            ui->removeVideoList->setEnabled(false);
        }
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
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

void SavedListsManager::mediaListChanged()
{
    if (m_parent->m_mediaItemModel->rowCount() > 0) {
        QString listItemType = m_parent->m_mediaItemModel->mediaItemAt(0).type;
        if (listItemType == "Audio") {
            ui->aListSourceView->setEnabled(true);
        } else if (listItemType == "Video") {
            ui->vListSourceView->setEnabled(true);
        } else {
            ui->aListSourceView->setChecked(false);
            ui->vListSourceView->setChecked(false);
            ui->aListSourceView->setEnabled(false);
            ui->vListSourceView->setEnabled(false);
        }
    }
}

void SavedListsManager::saveMediaList(QList<MediaItem> mediaList, QString name, QString type)
{
    if (!name.isEmpty()) {
        
        //Create and populate M3U file
        QString filename = name;
        filename = filename.replace(" ", "");
        QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(filename), false));
        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(filename), true));
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
        
        //Save list to index file
        if (type == "Audio") {
            QString indexEntry = QString("%1:::%2:::%3")
            .arg(type)
            .arg(name)
            .arg(QString("savedlists://%1.m3u").arg(filename));
            QString savedListEntry = QString("Audio:::%1")
            .arg(name);
            QList<int> rowsToRemove;
            for (int i = 0; i < m_savedAudioLists.count(); i++) {
                if (m_savedAudioLists.at(i).startsWith(savedListEntry)) {
                    rowsToRemove << i;
                }
            }
            for (int i = 0; i < rowsToRemove.count(); i++) {
                m_savedAudioLists.removeAt(rowsToRemove.at(i));
            }
            m_savedAudioLists << indexEntry;
        } else if (type == "Video") {
            QString indexEntry = QString("%1:::%2:::%3")
            .arg(type)
            .arg(name)
            .arg(QString("savedlists://%1.m3u").arg(filename));
            QString savedListEntry = QString("Video:::%1")
            .arg(name);
            QList<int> rowsToRemove;
            for (int i = 0; i < m_savedVideoLists.count(); i++) {
                if (m_savedVideoLists.at(i).startsWith(savedListEntry)) {
                    rowsToRemove << i;
                }
            }
            for (int i = 0; i < rowsToRemove.count(); i++) {
                m_savedVideoLists.removeAt(rowsToRemove.at(i));
            }
            m_savedVideoLists << indexEntry;
        }
        updateSavedListsIndex();
    }
}

void SavedListsManager::saveView(QString name, QString type)
{
    if (!name.isEmpty()) {
        //Add to saved list index
        if (type == "Audio") {
            QString indexEntry = QString("%1:::%2:::%3")
            .arg(type)
            .arg(name)
            .arg(m_parent->m_mediaItemModel->mediaListProperties().lri);
            QString savedListEntry = QString("Audio:::%1")
            .arg(name);
            QList<int> rowsToRemove;
            for (int i = 0; i < m_savedAudioLists.count(); i++) {
                if (m_savedAudioLists.at(i).startsWith(savedListEntry)) {
                    rowsToRemove << i;
                }
            }
            for (int i = 0; i < rowsToRemove.count(); i++) {
                m_savedAudioLists.removeAt(rowsToRemove.at(i));
            }
            m_savedAudioLists << indexEntry;
        } else if (type == "Video") {
            QString indexEntry = QString("%1:::%2:::%3")
            .arg(type)
            .arg(name)
            .arg(m_parent->m_mediaItemModel->mediaListProperties().lri);
            QString savedListEntry = QString("Video:::%1")
            .arg(name);
            QList<int> rowsToRemove;
            for (int i = 0; i < m_savedVideoLists.count(); i++) {
                if (m_savedVideoLists.at(i).startsWith(savedListEntry)) {
                    rowsToRemove << i;
                }
            }
            for (int i = 0; i < rowsToRemove.count(); i++) {
                m_savedVideoLists.removeAt(rowsToRemove.at(i));
            }
            m_savedVideoLists << indexEntry;
        }
        updateSavedListsIndex();
    }
}


void SavedListsManager::loadSavedListsIndex()
{
    //Load lists from index
    m_savedAudioLists.clear();
    m_savedVideoLists.clear();
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
            if (type == "Audio") {
                m_savedAudioLists << line;
            } else if (type == "Video") {
                m_savedVideoLists << line;
            }
        }
    }
    indexFile.close();
}
void SavedListsManager::updateSavedListsIndex()
{
    QFile::remove(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
    QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", true));
    if (!indexFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream outIndex(&indexFile);
    for (int i = 0; i < m_savedAudioLists.count(); i++) {
        outIndex << m_savedAudioLists.at(i) << "\r\n";
    }
    for (int i = 0; i < m_savedVideoLists.count(); i++) {
        outIndex << m_savedVideoLists.at(i) << "\r\n";
    }
    indexFile.close();    
}



