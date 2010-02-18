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

//TODO:This module could use a good deal of simplification. :-)

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
    
    m_infoMediaItemsModel = new MediaItemModel(this);
    connect(ui->saveInfo, SIGNAL(clicked()), this, SLOT(saveInfoView()));
    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(showInfoView()));
    connect(ui->hideInfo, SIGNAL(clicked()), this, SLOT(hideInfoView()));
    
    m_infoItemModel = new QStandardItemModel(this);
    ui->infoItemView->setModel(m_infoItemModel);
    InfoItemDelegate * infoItemDelegate = new InfoItemDelegate(m_parent);
    infoItemDelegate->setView(ui->infoItemView);
    ui->infoItemView->setItemDelegate(infoItemDelegate);
    connect(m_infoMediaItemsModel, SIGNAL(mediaListChanged()), this, SLOT(updateInfoItemModel()));
    connect(m_infoItemModel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoItemChanged(QStandardItem *)));
    connect(ui->infoItemCancelEdit, SIGNAL(clicked()), this, SLOT(updateInfoItemModel()));
    connect(ui->infoItemSave, SIGNAL(clicked()), this, SLOT(saveInfoView()));
    ui->infoSaveHolder->setVisible(false);
    //ui->infoView->setVisible(false);
}

InfoManager::~InfoManager()
{
}


//---------------------
//-- UI Widget Slots --
//---------------------
void InfoManager::showInfoView()
{
    loadInfoView();
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

void InfoManager::saveInfoView()
{
    //Save info data to nepomuk store
    saveInfoItemsToMediaModel();
    saveFileMetaData();
    m_parent->m_mediaItemModel->updateSourceInfo(m_infoMediaItemsModel->mediaList());
    
    //Update Now Playing and Playlist views
    m_parent->playlist()->nowPlayingModel()->updateMediaItems(m_infoMediaItemsModel->mediaList());
    m_parent->playlist()->playlistModel()->updateMediaItems(m_infoMediaItemsModel->mediaList());
    
    //Update the Info Item model to reflect that the data is saved
    updateInfoItemModel();
    
    //Now that data is saved hide Save/Cancel controls
    ui->infoSaveHolder->setVisible(false);
}

void InfoManager::removeSelectedItemsInfo()
{
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        m_rows << selectedRows.at(i).row();
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
    
    m_rows.clear();
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    if (mediaList.count() == 0) {
        return;
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadMediaList(mediaList, true);
}



//----------------------
//-- Helper functions --
//----------------------
void InfoManager::loadInfoView()
{
    //Show the Info Item page
    ui->semanticsHolder->setVisible(true);
    ui->semanticsStack->setCurrentIndex(1);
    
    //Load selected media items in to infoMediaItemsModel
    m_rows.clear();
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        m_rows << selectedRows.at(i).row();
        mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
    }
    if (mediaList.count() == 0) {
        return;
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadMediaList(mediaList, true);
}

bool InfoManager::hasMultipleValues(const QString &field)
{
    QVariant value;
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    
    if (field == "artwork") {
        if (mediaList.count() == 1) {
            return false;
        } else {
            return true;
        }
    }
            
    for (int i = 0; i < mediaList.count(); i++) {
        if (mediaList.at(i).fields.contains(field)) {
            if (value.isNull()) {
                value = mediaList.at(i).fields.value(field);
            } else if (mediaList.at(i).fields.value(field) != value) {
                return true;
            }
        }
    }
    return false;
}

QVariant InfoManager::commonValue(const QString &field)
{
    QVariant value;
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    for (int i = 0; i < mediaList.count(); i++) {
        if (mediaList.at(i).fields.contains(field)) {
            if (value.isNull()) {
                value = mediaList.at(i).fields.value(field);
            } else if (mediaList.at(i).fields.value(field) != value) {
                value = QVariant();
                break;
            }
        }
    }
    return value;
}

QStringList InfoManager::valueList(const QString &field)
{
    QStringList value;
    value << QString();
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    for (int i = 0; i < mediaList.count(); i++) {
        if (mediaList.at(i).fields.contains(field)) {
            if (value.indexOf(mediaList.at(i).fields.value(field).toString()) == -1) {
                value << mediaList.at(i).fields.value(field).toString();
            }
        }
    }
        return value;   
}

void InfoManager::saveFileMetaData()
{
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
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

//-------------------------
//-- Model functions --
//-------------------------
void InfoManager::updateInfoItemModel()
{
    //Update commonValuesModel and uncommonValuesModel
    m_infoItemModel->clear();
    ui->infoItemView->clearSpans();
    
    if (m_infoMediaItemsModel->rowCount() > 0) {
        // Get fields shared by all media types
        QString type = m_infoMediaItemsModel->mediaItemAt(0).type;
        
        if (type == "Audio") {
            addFieldToValuesModel(i18n("Type"), "audioType");
            addFieldToValuesModel(i18n("Artwork"), "artwork");
            addFieldToValuesModel(i18n("Title"), "title");
            QString subType = m_infoMediaItemsModel->mediaItemAt(0).fields["audioType"].toString();
            if (subType == "Music") {
                addFieldToValuesModel(i18n("Artist"), "artist");
                addFieldToValuesModel(i18n("Album"), "album");
                addFieldToValuesModel(i18n("Track"), "trackNumber");
                addFieldToValuesModel(i18n("Year"), "year");
                addFieldToValuesModel(i18n("Genre"), "genre");
            }
            if (subType == "Audio Stream") {
                bool forceEditable = true;
                addFieldToValuesModel(i18n("Location"), "url", forceEditable);
            } else {
                addFieldToValuesModel(i18n("Location"), "url");
            }
        } else {
            addFieldToValuesModel(i18n("Type"), "videoType");
            addFieldToValuesModel(i18n("Artwork"), "artwork");
            addFieldToValuesModel(i18n("Title"), "title");
            QString subType = m_infoMediaItemsModel->mediaItemAt(0).fields["videoType"].toString();
            if (subType == "Movie" || subType == "TV Show") {
                addFieldToValuesModel(i18n("Year"), "year");
                addFieldToValuesModel(i18n("Genre"), "genre");
                if (subType == "TV Show") {
                    addFieldToValuesModel(i18n("Series"), "series");
                    addFieldToValuesModel(i18n("Season"), "season");
                    addFieldToValuesModel(i18n("Episode"), "episodeNumber");
                }
                addFieldToValuesModel(i18n("Writer"), "writer");
                addFieldToValuesModel(i18n("Producer"), "producer");
                addFieldToValuesModel(i18n("Director"), "director");
                addFieldToValuesModel(i18n("Actor"), "actor");
            }
            addFieldToValuesModel(i18n("Location"), "url");
        }
        addFieldToValuesModel(i18n("Description"), "description");
        if (m_infoMediaItemsModel->rowCount() == 1) {
            addFieldToValuesModel(i18n("Play Count"), "playCount");
            addFieldToValuesModel(i18n("Last Played"), "lastPlayed");
        }
    }
            
    ui->infoItemView->resizeRowsToContents();
    ui->infoItemView->resizeColumnsToContents();
    ui->infoSaveHolder->setVisible(false);
}

void InfoManager::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable)
{
    QList<QStandardItem *> rowData;
    
    // Set field label (artwork and title fields span both columns)
    if (field != "artwork" && field != "title") {
        QStandardItem *fieldTitleItem = new QStandardItem(fieldTitle);
        fieldTitleItem->setData(field, Qt::UserRole);
        fieldTitleItem->setEditable(false);
        rowData.append(fieldTitleItem);
    }
        
    QStandardItem *fieldItem = new QStandardItem();
    fieldItem->setData(field, Qt::UserRole);
    bool hasMultiple = hasMultipleValues(field);
    fieldItem->setData(hasMultiple, Qt::UserRole + 1);
    bool isEditable = true;
    if ((field == "playCount" || field == "lastPlayed" || field == "url") && !forceEditable) {
        isEditable = false;
    }
    fieldItem->setEditable(isEditable);
    if (isEditable) {
        fieldItem->setData(i18n("Double-click to edit"), Qt::ToolTipRole);
    }
    
    if (field == "artwork") {
        if (m_infoMediaItemsModel->rowCount() == 1) {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(m_infoMediaItemsModel->mediaItemAt(0));
            if (!artwork.isNull()) {
                fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
            } else {
                fieldItem->setData(KIcon("image-x-generic"), Qt::DecorationRole);
            }
        } else {
            //We should eventually check for common artwork and set it here.
            fieldItem->setData(KIcon("image-x-generic"), Qt::DecorationRole);
        }
        rowData.append(fieldItem);
        m_infoItemModel->appendRow(rowData);
        ui->infoItemView->setSpan(fieldItem->index().row(),0,1,2);
        return;
    }

    if (!hasMultiple) {
        //Set field value
        QVariant value = commonValue(field);
        if (field == "audioType" || field == "videoType") {
            if (value.toString() == "Music" || value.toString() == "Movie") {
                value = QVariant(0);
            } else if (value.toString() == "Audio Stream" || value.toString() == "TV Show") {
                value = QVariant(1);
            } else if (value.toString() == "Audio Clip" || value.toString() == "Video Clip") {
                value = QVariant(2);
            }
        }
        fieldItem->setData(value, Qt::DisplayRole);
        fieldItem->setData(value, Qt::EditRole);
        fieldItem->setData(value, Qt::UserRole + 2); //stores copy of original data
    } else {
        //Set default field value
        QVariant value = m_infoMediaItemsModel->mediaItemAt(0).fields[field];
        if (value.type() == QVariant::String) {
            fieldItem->setData(QString(), Qt::EditRole);
        } else if (value.type() == QVariant::Int) {
            fieldItem->setData(0, Qt::EditRole);
        }
    }
    rowData.append(fieldItem);
    m_infoItemModel->appendRow(rowData);
    if (field == "title") {
        ui->infoItemView->setSpan(fieldItem->index().row(),0,1,2);
    }
}

void InfoManager::checkInfoItemChanged(QStandardItem *item)
{
    if (item->data(Qt::DisplayRole) != item->data(Qt::UserRole + 2)) {
        ui->infoSaveHolder->setVisible(true);
    } else {
        bool changed = false;
        for (int row = 0; row < m_infoItemModel->rowCount(); row++) {
            QStandardItem *otherItem = m_infoItemModel->item(row, 0);
            if (otherItem->data(Qt::UserRole).toString() != "title" && 
                otherItem->data(Qt::UserRole).toString() != "artwork") {
                otherItem = m_infoItemModel->item(row, 1);
            }
            if (otherItem->data(Qt::UserRole).toString() != "artwork") {
                if (otherItem->data(Qt::DisplayRole) != otherItem->data(Qt::UserRole + 2)) {
                    changed = true;
                    break;
                }
            }
        }
        ui->infoSaveHolder->setVisible(changed);
    }
}

void InfoManager::saveInfoItemsToMediaModel()
{
    QList<MediaItem> updatedList;
    for (int i = 0; i < m_infoMediaItemsModel->rowCount(); i++) {
        MediaItem mediaItem = m_infoMediaItemsModel->mediaItemAt(i);
        for (int row = 0; row < m_infoItemModel->rowCount(); row++) {
            QStandardItem *item = m_infoItemModel->item(row, 0);
            QString field = item->data(Qt::UserRole).toString();
            if (field != "title" && field != "artwork") {
                item = m_infoItemModel->item(row, 1); //These fields don't span both columns
            }
            bool multipleValues = item->data(Qt::UserRole +1).toBool();
            if (!multipleValues) { 
                if (field == "audioType") {
                    int value = item->data(Qt::EditRole).toInt();
                    if (value == 0) {
                        mediaItem.fields["audioType"] = "Music";
                    } else if (value == 1) {
                        mediaItem.fields["audioType"] = "Audio Stream";
                    } else if (value == 2) {
                        mediaItem.fields["audioType"] = "Audio Clip";
                    }
                } else if (field == "videoType") {
                    int value = item->data(Qt::EditRole).toInt();
                    if (value == 0) {
                        mediaItem.fields["videoType"] = "Movie";
                    } else if (value == 1) {
                        mediaItem.fields["videoType"] = "TV Show";
                    } else if (value == 2) {
                        mediaItem.fields["videoType"] = "Video Clip";
                    }
                } else if (field == "title") {
                    mediaItem.fields["title"] = item->data(Qt::EditRole);
                    mediaItem.title = item->data(Qt::EditRole).toString();
                } else if (field == "url") {
                    if (mediaItem.fields["audioType"].toString() == "Audio Stream") {
                        mediaItem.fields["url"] = item->data(Qt::EditRole);
                        mediaItem.url = item->data(Qt::EditRole).toString();
                    }
                } else if (field != "artwork") {              
                    mediaItem.fields[field] = item->data(Qt::EditRole);
                }
            }
        }
        updatedList << mediaItem;
        m_parent->m_mediaItemModel->replaceMediaItemAt(m_rows.at(i), mediaItem);        
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadMediaList(updatedList);
}
