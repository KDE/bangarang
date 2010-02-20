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

#include "infomanager.h"
#include "infoitemdelegate.h"
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"
#include "platform/infoitemmodel.h"
#include "platform/playlist.h"
#include "mediaitemdelegate.h"
#include <KUrlRequester>
#include <KLineEdit>
#include <KGlobalSettings>
#include <KDebug>
#include <QDateEdit>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <Nepomuk/ResourceManager>
#include <nepomuk/variant.h>
#include <QComboBox>
#include <QSpinBox>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

InfoManager::InfoManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;

    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true;
    } else {
        m_nepomukInited = false;
    }
    
    m_infoItemModel = new InfoItemModel(this);
    ui->infoItemView->setModel(m_infoItemModel);
    InfoItemDelegate * infoItemDelegate = new InfoItemDelegate(m_parent);
    infoItemDelegate->setView(ui->infoItemView);
    ui->infoItemView->setItemDelegate(infoItemDelegate);
    ui->infoSaveHolder->setVisible(false);
    
    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(showInfoView()));
    connect(ui->hideInfo, SIGNAL(clicked()), this, SLOT(hideInfoView()));
    connect(m_infoItemModel, SIGNAL(infoChanged(bool)), ui->infoSaveHolder, SLOT(setVisible(bool)));
    connect(ui->infoItemCancelEdit, SIGNAL(clicked()), this, SLOT(cancelItemEdit()));
    connect(ui->infoItemSave, SIGNAL(clicked()), this, SLOT(saveItemInfo()));
}

InfoManager::~InfoManager()
{
}


//---------------------
//-- UI Widget Slots --
//---------------------
void InfoManager::showInfoView()
{
    loadSelectedInfo();
    ui->showInfo->setVisible(false);
    ui->showMediaViewMenu->setVisible(false);
}

void InfoManager::hideInfoView()
{
    ui->semanticsHolder->setVisible(false);
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        ui->showInfo->setVisible(true);
    }
        ui->showMediaViewMenu->setVisible(true);
}

void InfoManager::saveItemInfo()
{
    //Save changed info in model
    m_infoItemModel->saveChanges();
    updateItemViewLayout();

    //Update file metadata
    saveFileMetaData(m_infoItemModel->mediaList());
    
    //Update source information
    m_parent->m_mediaItemModel->updateSourceInfo(m_infoItemModel->mediaList());
    
    //Update Now Playing and Playlist views
    m_parent->playlist()->nowPlayingModel()->updateMediaItems(m_infoItemModel->mediaList());
    m_parent->playlist()->playlistModel()->updateMediaItems(m_infoItemModel->mediaList());
    
    //Now that data is saved hide Save/Cancel controls
    ui->infoSaveHolder->setVisible(false);
}

void InfoManager::cancelItemEdit()
{
    m_infoItemModel->cancelChanges();
    ui->infoSaveHolder->setVisible(false);
    updateItemViewLayout();
}

void InfoManager::removeSelectedItemsInfo()
{
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        MediaItem mediaItem = m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row());
        if (mediaItem.type == "Audio" || mediaItem.type == "Video" || mediaItem.type == "Image") {
            mediaList.append(mediaItem);
        }
    }
    if (mediaList.count() > 0) {
        m_parent->m_mediaItemModel->removeSourceInfo(mediaList);
    }
            
}

void InfoManager::showInfoViewForMediaItem(const MediaItem &mediaItem)
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->semanticsHolder->setVisible(true);
    ui->semanticsStack->setCurrentIndex(1);
    
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    if (mediaList.count() == 0) {
        return;
    }
    m_infoItemModel->loadInfo(mediaList);
    updateItemViewLayout();
}



//----------------------
//-- Helper functions --
//----------------------
void InfoManager::loadSelectedInfo()
{
    //Show the Info Item page
    ui->semanticsHolder->setVisible(true);
    ui->semanticsStack->setCurrentIndex(1);
    
    //Get selected items
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
    }
    if (mediaList.count() == 0) {
        return;
    }
    
    //Load selected items into info model
    m_infoItemModel->loadInfo(mediaList);
    updateItemViewLayout();
}

void InfoManager::updateItemViewLayout()
{
    for (int row = 0; row < m_infoItemModel->rowCount(); row++) {
        QString field = m_infoItemModel->item(row)->data(InfoItemModel::FieldRole).toString();
        if (field == "artwork" || field == "title") {
            ui->infoItemView->setSpan(row,0,1,2);
        }
    }
    ui->infoItemView->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->infoItemView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void InfoManager::saveFileMetaData(QList<MediaItem> mediaList)
{
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        if ((mediaItem.type == "Audio") && (mediaItem.fields["audioType"] == "Music")) {
            if (Utilities::isMusic(mediaList.at(i).url)) {
                QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
                if (!artworkUrl.isEmpty()) {
                    Utilities::saveArtworkToTag(mediaList.at(i).url, artworkUrl);
                }
                TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toLocal8Bit());
                if (!file.isNull()) {
                    QString title = mediaItem.title;
                    if (!title.isEmpty()) {
                       TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
                       file.tag()->setTitle(tTitle);
                    }
                    QString artist = mediaItem.fields["artist"].toString();
                    if (!artist.isEmpty()) {
                      TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
                      file.tag()->setArtist(tArtist);
                    }
                    QString album = mediaItem.fields["album"].toString();
                    if (!album.isEmpty()) {
                     TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
                     file.tag()->setAlbum(tAlbum);
                    }
                    int year = mediaItem.fields["year"].toInt();
                    if (year != 0) {
                        file.tag()->setYear(year);
                    }
                    int trackNumber = mediaItem.fields["trackNumber"].toInt();
                    if (trackNumber != 0) {
                       file.tag()->setTrack(trackNumber);
                    }
                    QString genre = mediaItem.fields["genre"].toString();
                    if (!genre.isEmpty()) {
                      TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
                      file.tag()->setGenre(tGenre);
                    }
                    file.save();
                }
            }
        }
    }
}
