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
#include "../common/bangarangapplication.h"
#include "medialistsmanager.h"
#include "../../platform/utilities/utilities.h"
#include "../common/mainwindow.h"
#include "infomanager.h"
#include "../common/actionsmanager.h"
#include "ui_mainwindow.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/playlist.h"

#include <KStandardDirs>
#include <KMessageBox>
#include <KDebug>
#include <QFile>
#include <Nepomuk/ResourceManager>

SavedListsManager::SavedListsManager(MainWindow * parent) : QObject(parent)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_parent = parent;
    ui = m_parent->ui;
    
    m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(false);
    m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(false);
    loadSavedListsIndex();
    
    connect(m_parent->audioListsStack()->ui->addAudioList, SIGNAL(clicked()), this, SLOT(showAudioListSave()));
    connect(m_parent->videoListsStack()->ui->addVideoList, SIGNAL(clicked()), this, SLOT(showVideoListSave()));
    connect(m_parent->audioListsStack()->ui->aCancelSaveList, SIGNAL(clicked()), this, SLOT(returnToAudioList()));
    connect(m_parent->videoListsStack()->ui->vCancelSaveList, SIGNAL(clicked()), this, SLOT(returnToVideoList()));
    connect(m_parent->audioListsStack()->ui->saveAudioList, SIGNAL(clicked()), this, SLOT(saveAudioList()));
    connect(m_parent->videoListsStack()->ui->saveVideoList, SIGNAL(clicked()), this, SLOT(saveVideoList()));
    connect(m_parent->audioListsStack()->ui->aNewListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(m_parent->videoListsStack()->ui->vNewListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(m_parent->audioListsStack()->ui->aNewListName, SIGNAL(returnPressed()), this, SLOT(saveAudioList()));
    connect(m_parent->videoListsStack()->ui->vNewListName, SIGNAL(returnPressed()), this, SLOT(saveVideoList()));
    connect(m_parent->audioListsStack()->ui->removeAudioList, SIGNAL(clicked()), this, SLOT(removeAudioList()));
    connect(m_parent->videoListsStack()->ui->removeVideoList, SIGNAL(clicked()), this, SLOT(removeVideoList()));
    connect(m_parent->audioListsStack()->ui->aslsCancel, SIGNAL(clicked()), this, SLOT(returnToAudioList()));
    connect(m_parent->videoListsStack()->ui->vslsCancel, SIGNAL(clicked()), this, SLOT(returnToVideoList()));
    connect(m_parent->audioListsStack()->ui->aslsSave, SIGNAL(clicked()), this, SLOT(saveAudioListSettings()));
    connect(m_parent->videoListsStack()->ui->vslsSave, SIGNAL(clicked()), this, SLOT(saveVideoListSettings()));
    connect(m_parent->audioListsStack()->ui->aslsListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(m_parent->videoListsStack()->ui->vslsListName, SIGNAL(textChanged(QString)), this, SLOT(enableValidSave(QString)));
    connect(m_parent->audioListsStack()->ui->aslsListName, SIGNAL(returnPressed()), this, SLOT(saveAudioListSettings()));
    connect(m_parent->videoListsStack()->ui->vslsListName, SIGNAL(returnPressed()), this, SLOT(saveVideoListSettings()));
    
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(selectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_parent->audioListsStack()->ui->audioLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(audioListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_parent->videoListsStack()->ui->videoLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(videoListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_application->browsingModel(), SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_application->infoManager(), SIGNAL(infoBoxSelectionChanged(QList<MediaItem>)), this, SLOT(infoBoxSelectionChanged(QList<MediaItem>)));
    
    m_nepomukInited = Utilities::nepomukInited();
    
}

SavedListsManager::~SavedListsManager()
{
}

void SavedListsManager::showAudioListSave()
{
    m_parent->audioListsStack()->ui->audioListsStack->setCurrentIndex(1);
    m_parent->audioListsStack()->ui->aNewListName->setText(i18n("Untitled"));
    if (m_parent->audioListsStack()->ui->aListSourceSelection->isEnabled()) {
        m_parent->audioListsStack()->ui->aListSourceSelection->setChecked(true);
    } else if (m_parent->audioListsStack()->ui->aListSourceView->isEnabled()) {
        m_parent->audioListsStack()->ui->aListSourceView->setChecked(true);
    } else {
        m_parent->audioListsStack()->ui->aListSourcePlaylist->setChecked(true);
    }
    m_parent->audioListsStack()->ui->aNewListName->setFocus();
    enableValidSave();
    m_application->actionsManager()->setContextMenuSource(MainWindow::Default);
}

void SavedListsManager::showVideoListSave()
{
    m_parent->videoListsStack()->ui->videoListsStack->setCurrentIndex(1);
    m_parent->videoListsStack()->ui->vNewListName->setText(i18n("Untitled"));
    if (m_parent->videoListsStack()->ui->vListSourceSelection->isEnabled()) {
        m_parent->videoListsStack()->ui->vListSourceSelection->setChecked(true);
    } else if (m_parent->videoListsStack()->ui->vListSourceView->isEnabled()) {
        m_parent->videoListsStack()->ui->vListSourceView->setChecked(true);
    } else {
        m_parent->videoListsStack()->ui->vListSourcePlaylist->setChecked(true);
    }
    m_parent->videoListsStack()->ui->vNewListName->setFocus();
    enableValidSave();
    m_application->actionsManager()->setContextMenuSource(MainWindow::Default);
}

void SavedListsManager::returnToAudioList()
{
    m_parent->audioListsStack()->ui->aNewListName->clear();
    m_parent->audioListsStack()->ui->aslsListName->clear();
    m_parent->audioListsStack()->ui->audioListsStack->setCurrentIndex(0);
}

void SavedListsManager::returnToVideoList()
{
    m_parent->videoListsStack()->ui->vNewListName->clear();
    m_parent->videoListsStack()->ui->vslsListName->clear();
    m_parent->videoListsStack()->ui->videoListsStack->setCurrentIndex(0);
}

void SavedListsManager::saveAudioList()
{
    if (m_parent->audioListsStack()->ui->aListSourceSelection->isChecked()) {
        //Get selected media items and save
        QList<MediaItem> mediaList = m_application->actionsManager()->selectedMediaItems();
        saveMediaList(mediaList, m_parent->audioListsStack()->ui->aNewListName->text(), QString("Audio"));
    } else if (m_parent->audioListsStack()->ui->aListSourceView->isChecked()) {
        saveView(m_parent->audioListsStack()->ui->aNewListName->text(), QString("Audio"));
    } else if (m_parent->audioListsStack()->ui->aListSourcePlaylist->isChecked()) {
        QList<MediaItem> mediaList = m_application->playlist()->playlistModel()->mediaList();
        saveMediaList(mediaList, m_parent->audioListsStack()->ui->aNewListName->text(), QString("Audio"));
    }
    MediaListProperties audioListsProperties = m_application->mediaListsManager()->audioListsModel()->mediaListProperties();
    m_application->mediaListsManager()->audioListsModel()->clearMediaListData();
    m_application->mediaListsManager()->audioListsModel()->setMediaListProperties(audioListsProperties);
    m_application->mediaListsManager()->audioListsModel()->load();
    returnToAudioList();
}

void SavedListsManager::saveVideoList()
{
    if (m_parent->videoListsStack()->ui->vListSourceSelection->isChecked()) {
        //Get selected media items and save
        QList<MediaItem> mediaList = m_application->infoManager()->selectedInfoBoxMediaItems();
        if (mediaList.count() == 0) {
            QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
            for (int i = 0 ; i < selectedRows.count() ; ++i) {
                mediaList.append(m_application->browsingModel()->mediaItemAt(selectedRows.at(i).row()));
            }
        }
        saveMediaList(mediaList, m_parent->videoListsStack()->ui->vNewListName->text(), QString("Video"));
    } else if (m_parent->videoListsStack()->ui->vListSourceView->isChecked()) {
        saveView(m_parent->videoListsStack()->ui->vNewListName->text(), QString("Video"));
    } else if (m_parent->videoListsStack()->ui->vListSourcePlaylist->isChecked()) {
        QList<MediaItem> mediaList = m_application->playlist()->playlistModel()->mediaList();
        saveMediaList(mediaList, m_parent->videoListsStack()->ui->vNewListName->text(), QString("Video"));
    }
    MediaListProperties videoListsProperties = m_application->mediaListsManager()->videoListsModel()->mediaListProperties();
    m_application->mediaListsManager()->videoListsModel()->clearMediaListData();
    m_application->mediaListsManager()->videoListsModel()->setMediaListProperties(videoListsProperties);
    m_application->mediaListsManager()->videoListsModel()->load();
    returnToVideoList();
}

void SavedListsManager::removeAudioList()
{
    if (m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes().count() > 0){
        int selectedRow = m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        QString name = m_application->mediaListsManager()->audioListsModel()->mediaItemAt(selectedRow).title;
        
        KGuiItem removeSavedList;
        removeSavedList.setText(i18n("Remove"));
        removeSavedList.setIcon(KIcon("list-remove"));
        QString message = i18n("Are you sure you want to remove \"%1\"?", name);
        
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
            MediaListProperties audioListsProperties = m_application->mediaListsManager()->audioListsModel()->mediaListProperties();
            m_application->mediaListsManager()->audioListsModel()->clearMediaListData();
            m_application->mediaListsManager()->audioListsModel()->setMediaListProperties(audioListsProperties);
            m_application->mediaListsManager()->audioListsModel()->load();
            emit savedListsChanged();
        }
    }
}

void SavedListsManager::removeVideoList()
{
    if (m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes().count() > 0){
        int selectedRow = m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        QString name = m_application->mediaListsManager()->videoListsModel()->mediaItemAt(selectedRow).title;
        
        KGuiItem removeSavedList;
        removeSavedList.setText(i18n("Remove"));
        removeSavedList.setIcon(KIcon("list-remove"));
        QString message = i18n("Are you sure you want to remove \"%1\"?", name);
        
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
            MediaListProperties videoListsProperties = m_application->mediaListsManager()->videoListsModel()->mediaListProperties();
            m_application->mediaListsManager()->videoListsModel()->clearMediaListData();
            m_application->mediaListsManager()->videoListsModel()->setMediaListProperties(videoListsProperties);
            m_application->mediaListsManager()->videoListsModel()->load();
            emit savedListsChanged();
        }
    }
}

void SavedListsManager::enableValidSave(QString newText)
{
    m_parent->audioListsStack()->ui->saveAudioList->setEnabled(true);
    if (!m_parent->audioListsStack()->ui->aNewListName->text().isEmpty()) {
        m_parent->audioListsStack()->ui->saveAudioList->setEnabled(true);
    } else {
        m_parent->audioListsStack()->ui->saveAudioList->setEnabled(false);
    }
    if (!m_parent->videoListsStack()->ui->vNewListName->text().isEmpty()) {
        m_parent->videoListsStack()->ui->saveVideoList->setEnabled(true);
    } else {
        m_parent->videoListsStack()->ui->saveVideoList->setEnabled(false);
    } 
    if (!m_parent->audioListsStack()->ui->aslsListName->text().isEmpty()) {
        m_parent->audioListsStack()->ui->aslsSave->setEnabled(true);
    } else {
        m_parent->audioListsStack()->ui->aslsSave->setEnabled(false);
    } 
    if (!m_parent->videoListsStack()->ui->vslsListName->text().isEmpty()) {
        m_parent->videoListsStack()->ui->vslsSave->setEnabled(true);
    } else {
        m_parent->videoListsStack()->ui->vslsSave->setEnabled(false);
    } 
    Q_UNUSED(newText); //not used since method may be called directly
}

void SavedListsManager::audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if (selected.indexes().count() > 0) {
        bool isSavedList = selected.indexes().at(0).data(MediaItem::IsSavedListRole).toBool();
        if (isSavedList) {
            m_parent->audioListsStack()->ui->removeAudioList->setEnabled(true);
        } else {
            m_parent->audioListsStack()->ui->removeAudioList->setEnabled(false);
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
            m_parent->videoListsStack()->ui->removeVideoList->setEnabled(true);
        } else {
            m_parent->videoListsStack()->ui->removeVideoList->setEnabled(false);
        }
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void SavedListsManager::selectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    if (m_application->infoManager()->selectedInfoBoxMediaItems().count() > 0) {
        QString listItemType = m_application->infoManager()->selectedInfoBoxMediaItems().at(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(true);
            m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(true);
        }
    } else if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        QString listItemType = m_application->browsingModel()->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(true);
            m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(true);
        }
    } else {
        m_parent->audioListsStack()->ui->aListSourceSelection->setChecked(false);
        m_parent->videoListsStack()->ui->vListSourceSelection->setChecked(false);
        m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(false);
        m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(false);
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void SavedListsManager::infoBoxSelectionChanged(QList<MediaItem> selectedItems)
{
    if (selectedItems.count() > 0) {
        QString listItemType = selectedItems.at(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(true);
            m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(true);
        }
    } else if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        QString listItemType = m_application->browsingModel()->mediaItemAt(0).type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(true);
            m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(true);
        }
    } else {
        m_parent->audioListsStack()->ui->aListSourceSelection->setChecked(false);
        m_parent->videoListsStack()->ui->vListSourceSelection->setChecked(false);
        m_parent->audioListsStack()->ui->aListSourceSelection->setEnabled(false);
        m_parent->videoListsStack()->ui->vListSourceSelection->setEnabled(false);
    }
}

void SavedListsManager::showAudioSavedListSettings()
{
    m_parent->audioListsStack()->ui->audioListsStack->setCurrentIndex(2);
    QModelIndexList selectedIndexes = m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes();
    for(int i = 0; i < selectedIndexes.count(); i++) {
        int row = selectedIndexes.at(i).row();
        m_parent->audioListsStack()->ui->aslsListName->setText(m_application->mediaListsManager()->audioListsModel()->mediaItemAt(row).title);
    }
    m_parent->audioListsStack()->ui->aslsListName->setFocus();
}

void SavedListsManager::showVideoSavedListSettings()
{
    m_parent->videoListsStack()->ui->videoListsStack->setCurrentIndex(2);
    QModelIndexList selectedIndexes = m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes();
    for(int i = 0; i < selectedIndexes.count(); i++) {
        int row = selectedIndexes.at(i).row();
        m_parent->videoListsStack()->ui->vslsListName->setText(m_application->mediaListsManager()->videoListsModel()->mediaItemAt(row).title);
    }
    m_parent->videoListsStack()->ui->vslsListName->setFocus();
}


void SavedListsManager::mediaListChanged()
{
    if (m_application->browsingModel()->rowCount() > 0) {
        QString listItemType = m_application->browsingModel()->mediaItemAt(0).type;
        if (listItemType == "Audio" && m_nepomukInited && m_application->browsingModel()->lriIsLoadable()) {
            m_parent->audioListsStack()->ui->aListSourceView->setEnabled(true);
        } else if (listItemType == "Video" && m_nepomukInited && m_application->browsingModel()->lriIsLoadable()) {
            m_parent->videoListsStack()->ui->vListSourceView->setEnabled(true);
        } else {
            m_parent->audioListsStack()->ui->aListSourceView->setChecked(false);
            m_parent->videoListsStack()->ui->vListSourceView->setChecked(false);
            m_parent->audioListsStack()->ui->aListSourceView->setEnabled(false);
            m_parent->videoListsStack()->ui->vListSourceView->setEnabled(false);
        }
    }
}

void SavedListsManager::saveMediaList(QList<MediaItem> mediaList, const QString &name, const QString &type, bool append)
{
    if (!name.isEmpty()) {
        
        //Create and populate M3U file
        QString filename = name;
        filename = filename.replace(" ", "");
        QIODevice::OpenMode openMode;
        if (append) {
            openMode = QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append;
        } else {
            QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(filename), false));
            openMode = QIODevice::WriteOnly | QIODevice::Text;
        }
        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(filename), true));
        if (!file.open(openMode)) {
            return;
        }
        QTextStream out(&file);
        if (!append) {
            out << "#EXTM3U" << "\r\n";
        }
        for (int i = 0; i < mediaList.count(); i++) {
            if (mediaList.at(i).type == "Audio" || mediaList.at(i).type == "Video") {
                out << "#EXTINF:" << mediaList.at(i).fields["duration"].toInt() << "," << mediaList.at(i).title << "\r\n";
                out << mediaList.at(i).url << "\r\n";
            }
        }
        file.close();
        
        //Save list to index file
        if (type == "Audio") {
            QString indexEntry = QString("%1:::%2:::%3")
            .arg(type)
            .arg(name)
            .arg(QString("savedlists://%1-%2.m3u").arg(type).arg(filename));
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
            .arg(QString("savedlists://%1-%2.m3u").arg(type).arg(filename));
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
        emit savedListsChanged();
    }
}

void SavedListsManager::saveView(const QString &name, const QString &type)
{
    if (!name.isEmpty()) {
        //Add to saved list index
        if (type == "Audio") {
            QString indexEntry = QString("%1:::%2:::%3")
            .arg(type)
            .arg(name)
            .arg(m_application->browsingModel()->mediaListProperties().lri);
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
            .arg(m_application->browsingModel()->mediaListProperties().lri);
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
        emit savedListsChanged();
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
    emit savedListsChanged();
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

QStringList SavedListsManager::savedListNames(const QString &type)
{
    QList<QString> savedLists ;
    if (type == "Audio") {
        savedLists = m_savedAudioLists;
    } else if (type == "Video") {
        savedLists = m_savedVideoLists;
    }
    QStringList savedListNames;
    for (int i = 0; i < savedLists.count(); i++) {
        QString name = savedLists.at(i).split(":::").at(1);
        QString lri = savedLists.at(i).split(":::").at(2);
        if (lri.startsWith("savedlists://")) {
            savedListNames.append(name);
        }
    }
    return savedListNames;
}

void SavedListsManager::removeSelected()
{
    if (m_application->browsingModel()->mediaListProperties().lri.startsWith("savedlists://")) {
        QList<MediaItem> mediaList;
        
        QList<int> rowsToRemove;
        //Rebuild mediaList without selected items
        for (int i = 0; i < m_application->browsingModel()->rowCount(); i++) {
            QModelIndex index = m_application->browsingModel()->index(i, 0);
            if (!ui->mediaView->selectionModel()->isSelected(index)) {
                mediaList.append(m_application->browsingModel()->mediaItemAt(i));
            } else {
                rowsToRemove.append(i);
            }
        }
        
        //Save new medialist        
        QString lri = m_application->browsingModel()->mediaListProperties().lri;
        QString name = savedListLriName(lri);
        saveMediaList(mediaList, name, m_application->browsingModel()->mediaItemAt(0).type);
        
        //Remove items from model
        m_application->browsingModel()->reload();
    }
    
}

void SavedListsManager::saveAudioListSettings()
{
    //Get old list name
    QString oldName;
    QModelIndexList selectedIndexes = m_parent->audioListsStack()->ui->audioLists->selectionModel()->selectedIndexes();
    int audioListsRow = selectedIndexes.at(0).row();
    MediaItem mediaItem;
    if (selectedIndexes.count() > 0) {
        oldName = m_application->mediaListsManager()->audioListsModel()->mediaItemAt(audioListsRow).title;
        mediaItem = m_application->mediaListsManager()->audioListsModel()->mediaItemAt(audioListsRow);
    }
    
    //Read index file to locate and rename saved list name
    QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    m_savedAudioLists.clear();
    QTextStream in(&indexFile);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList nameUrl = line.split(":::");
        if (nameUrl.count() >= 3) {
            QString type = nameUrl.at(0).trimmed();
            QString name = nameUrl.at(1).trimmed();
            QString lri = nameUrl.at(2).trimmed();
            if (type == "Audio") {
                QString indexEntry = line;
                if (name == oldName) {
                    QString newName = m_parent->audioListsStack()->ui->aslsListName->text();
                    if (lri.startsWith("savedlists://")) {
                        //rename file
                        QString filename = name.replace(" ", "");
                        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(filename), true));
                        QString newFilename = QString(newName).replace(" ", "");
                        QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(newFilename), true));
                        QString renamedFileName = file.fileName();
                        renamedFileName.replace(QString("%1.m3u").arg(filename), QString("%1.m3u").arg(newFilename));
                        file.rename(renamedFileName);
                        lri.replace(QString("%1.m3u").arg(filename), QString("%1.m3u").arg(newFilename));
                    }
                    
                    //Update Audio ListView
                    mediaItem.title = newName;
                    mediaItem.url = lri;
                    m_application->mediaListsManager()->audioListsModel()->replaceMediaItemAt(audioListsRow, mediaItem);
                    ui->listTitle->setText(newName);
                    
                    //create new index entry for index file
                    indexEntry = QString("%1:::%2:::%3")
                        .arg(type)
                        .arg(newName)
                        .arg(lri);
                }
                m_savedAudioLists.append(indexEntry);
            }
        }
    }
    indexFile.close();
    
    //Update index file
    updateSavedListsIndex();
    
    emit savedListsChanged();
    if (selectedIndexes.count() > 0) {
        m_parent->audioListsStack()->ui->audioLists->selectionModel()->select(selectedIndexes.at(0), QItemSelectionModel::Select);
    }
    returnToAudioList();
}

void SavedListsManager::saveVideoListSettings()
{
    //Get old list name
    QString oldName;
    QModelIndexList selectedIndexes = m_parent->videoListsStack()->ui->videoLists->selectionModel()->selectedIndexes();
    int videoListsRow = selectedIndexes.at(0).row();
    MediaItem mediaItem;
    if (selectedIndexes.count() > 0) {
        oldName = m_application->mediaListsManager()->videoListsModel()->mediaItemAt(videoListsRow).title;
        mediaItem = m_application->mediaListsManager()->videoListsModel()->mediaItemAt(videoListsRow);
    }
    
    //Read index file to locate and rename saved list name
    QFile indexFile(KStandardDirs::locateLocal("data", "bangarang/savedlists", false));
    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    m_savedVideoLists.clear();
    QTextStream in(&indexFile);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList nameUrl = line.split(":::");
        if (nameUrl.count() >= 3) {
            QString type = nameUrl.at(0).trimmed();
            QString name = nameUrl.at(1).trimmed();
            QString lri = nameUrl.at(2).trimmed();
            if (type == "Video") {
                QString indexEntry = line;
                if (name == oldName) {
                    QString newName = m_parent->videoListsStack()->ui->vslsListName->text();
                    if (lri.startsWith("savedlists://")) {
                        //rename file
                        QString filename = name.replace(" ", "");
                        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(filename), true));
                        QString newFilename = QString(newName).replace(" ", "");
                        QFile::remove(KStandardDirs::locateLocal("data", QString("bangarang/%1-%2.m3u").arg(type).arg(newFilename), true));
                        QString renamedFileName = file.fileName();
                        renamedFileName.replace(QString("%1.m3u").arg(filename), QString("%1.m3u").arg(newFilename));
                        file.rename(renamedFileName);
                        lri.replace(QString("%1.m3u").arg(filename), QString("%1.m3u").arg(newFilename));
                    }
                    
                    //Update Video ListView
                    mediaItem.title = newName;
                    mediaItem.url = lri;
                    m_application->mediaListsManager()->videoListsModel()->replaceMediaItemAt(videoListsRow, mediaItem);
                    ui->listTitle->setText(newName);
                    
                    //create new index entry for index file
                    indexEntry = QString("%1:::%2:::%3")
                    .arg(type)
                    .arg(newName)
                    .arg(lri);
                }
                m_savedVideoLists.append(indexEntry);
            }
        }
    }
    indexFile.close();
    
    //Update index file
    updateSavedListsIndex();
    
    emit savedListsChanged();
    if (selectedIndexes.count() > 0) {
        m_parent->videoListsStack()->ui->videoLists->selectionModel()->select(selectedIndexes.at(0), QItemSelectionModel::Select);
    }
    returnToVideoList();
}

QString SavedListsManager::savedListLriName(const QString &lri)
{
    QString name;
    for (int i = 0; i < m_savedAudioLists.count(); i++) {
        if (m_savedAudioLists.at(i).endsWith(lri)) {
            QString indexEntry = m_savedAudioLists.at(i);
            name = indexEntry.split(":::").at(1);
        }
    }
        
    for (int i = 0; i < m_savedVideoLists.count(); i++) {
        if (m_savedVideoLists.at(i).endsWith(lri)) {
            QString indexEntry = m_savedVideoLists.at(i);
            name = indexEntry.split(":::").at(1);
        }
    }
    
    return name;
}

void SavedListsManager::savePlaylist()
{
    saveMediaList(m_application->playlist()->playlistModel()->mediaList(), "current", "Playlist");
}

void SavedListsManager::loadPlaylist()
{
    MediaItem mediaItem;
    mediaItem.type = "Category";
    mediaItem.title = i18n("Playlist");
    mediaItem.url = "savedlists://Playlist-current.m3u";
    m_application->playlist()->addMediaItem(mediaItem);
}
