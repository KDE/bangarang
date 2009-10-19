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
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"
#include "mediaitemdelegate.h"
#include "platform/mediaindexer.h"
#include <KUrlRequester>
#include <KLineEdit>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
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
    m_infoMediaItemsModel = new MediaItemModel(this);
    m_mediaIndexer = new MediaIndexer(this);
    connect(ui->saveInfo, SIGNAL(clicked()), this, SLOT(saveInfoView()));
    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(loadInfoView()));
    
}

InfoManager::~InfoManager()
{
}

void InfoManager::loadInfoView()
{
    ui->mediaViewHolder->setCurrentIndex(1);
    ui->previous->setVisible(true);
    ui->previous->setText(ui->listTitle->text());
    ui->playSelected->setVisible(false);
    ui->playAll->setVisible(false);
    ui->saveInfo->setVisible(true);
    
    
    m_rows.clear();
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        m_rows << selectedRows.at(i).row();
        mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadSources(mediaList);
    
    if (mediaList.at(0).type == "Audio") {
        if (multipleAudioTypes()) {
            //Only show fields applicable to all types (fields for Audio Clip)
            mediaTypeChanged(-1);
        } else {
            if (mediaList.at(0).fields["audioType"] == "Music") {
                mediaTypeChanged(0);
            } else if (mediaList.at(0).fields["audioType"] == "Audio Stream") {
                mediaTypeChanged(1);
            } else if (mediaList.at(0).fields["audioType"] == "Audio Clip") {
                mediaTypeChanged(2);
            }
        }
    } else if (mediaList.at(0).type == "Video") {
        if (multipleVideoTypes()) {
            //Only show fields applicable to all types (fields for Video Clip)
            mediaTypeChanged(-1);
        } else {
            if (mediaList.at(0).fields["videoType"] == "Movie") {
                mediaTypeChanged(0);
            } else if (mediaList.at(0).fields["videoType"] == "Series") {
                mediaTypeChanged(1);
            } else if (mediaList.at(0).fields["videoType"] == "Video Clip") {
                mediaTypeChanged(2);
            }
        }
    }
}

void InfoManager::mediaTypeChanged(int index)
{
    //index = -1 means that there are multiple audio/video types
    //so only fields for applicable to all types will be shown
    
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    if (mediaList.count() > 0) {
        ui->infoMediaItems->clear();
        if (mediaList.at(0).type == "Audio") {
            if (index != -1) {
                showAudioType(index);
            }
            showAudioFields();
            if (index != -1) {
                if (index == 0) {
                    showAudioMusicFields();
                } else if (index == 1) {
                    showAudioStreamFields();
                } else if (index == 2) {
                }
            }
        } else if (mediaList.at(0).type == "Video") {
            if (index != -1) {
                showVideoType(index);
            }
            showVideoFields();
            if (index != -1) {
                if (index == 0) {
                } else if (index == 1) {
                    showVideoSeriesFields();
                } else if (index == 2) {
                }
            }
        }
        
        QTreeWidgetItem * tags = new QTreeWidgetItem(ui->infoMediaItems);
        tags->setText(0, tr2i18n("Tags"));
        ui->infoMediaItems->addTopLevelItem(tags);
    }
}

void InfoManager::saveInfoView()
{

    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(0), 1));
    if (typeComboBox->currentText() == "Music") {
        saveMusicInfoToFiles();
    }
    saveInfoToMediaModel();
    m_mediaIndexer->indexMediaItems(m_infoMediaItemsModel->mediaList());
    
    //Go back to media view
    ui->mediaViewHolder->setCurrentIndex(0);
    ui->saveInfo->setVisible(false);
    if (ui->mediaView->selectionModel()->selectedRows().count() > 0) {
        ui->playSelected->setVisible(true);
        ui->playAll->setVisible(false);
        ui->showInfo->setVisible(true);
    } else {
        ui->playSelected->setVisible(false);
        ui->playAll->setVisible(true);
        ui->showInfo->setVisible(false);
    }
    if (m_parent->m_mediaListPropertiesHistory.count() > 0) {
        ui->previous->setVisible(true);
        ui->previous->setText(m_parent->m_mediaListPropertiesHistory.last().name);
    } else {
        ui->previous->setVisible(false);
    }
    
}
        

QVariant InfoManager::commonValue(QString field)
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

QStringList InfoManager::valueList(QString field)
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

void InfoManager::saveMusicInfoToFiles()
{
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(0), 1));
    if (typeComboBox->currentText() == "Music") {
        QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
        for (int i = 0; i < mediaList.count(); i++) {
            if (Utilities::isMusic(mediaList.at(i).url)) {
                TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toUtf8());
                
                KLineEdit * titleWidget = static_cast<KLineEdit*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(2), 1));
                QString title = titleWidget->text();
                if (!title.isEmpty()) {
                    TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setTitle(tTitle);
                }
                
                QComboBox *artistWidget = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(3), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setArtist(tArtist);
                }
                
                QComboBox *albumWidget = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(4), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setAlbum(tAlbum);
                }
                
                QSpinBox *yearWidget = static_cast<QSpinBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(5), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    file.tag()->setYear(year);
                }
                
                QSpinBox *trackNumberWidget = static_cast<QSpinBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(6), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    file.tag()->setTrack(trackNumber);
                }
                
                QComboBox *genreWidget = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(7), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setGenre(tGenre);
                }
                
                file.save();
            }
        }
    }
    
}

void InfoManager::saveInfoToMediaModel()
{
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    QList<MediaItem> updatedList;
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(0), 1));
    
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        
        //All media types have a title
        KLineEdit * titleWidget = static_cast<KLineEdit*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(2), 1));
        QString title = titleWidget->text();
        if (!title.isEmpty()) {
            mediaItem.title = title;
            mediaItem.fields["title"] = title;
        }
        
        if (mediaItem.type == "Audio" ) {
            if (typeComboBox->currentText() == "Music") {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Music";
                
                QComboBox *artistWidget = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(3), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    mediaItem.fields["artist"] = artist;
                }
                
                QComboBox *albumWidget = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(4), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    mediaItem.fields["album"] = album;
                }
                
                mediaItem.subTitle = mediaItem.fields["artist"].toString() + QString(" - ") + mediaItem.fields["album"].toString();
                
                QSpinBox *yearWidget = static_cast<QSpinBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(5), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    mediaItem.fields["year"] = year;
                }
                
                QSpinBox *trackNumberWidget = static_cast<QSpinBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(6), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    mediaItem.fields["trackNumber"] = trackNumber;
                }
                
                QComboBox *genreWidget = static_cast<QComboBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(7), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    mediaItem.fields["genre"] = genre;
                }
            } else if (typeComboBox->currentText() == "Audio Clip") {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Clip";
                
                KLineEdit * descriptionWidget = static_cast<KLineEdit*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(3), 1));
                QString description = descriptionWidget->text();
                if (!description.isEmpty()) {
                    mediaItem.fields["description"] = description;
                }
            } else if (typeComboBox->currentText() == "Audio Stream") {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Stream";
                
                KLineEdit * descriptionWidget = static_cast<KLineEdit*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(3), 1));
                QString description = descriptionWidget->text();
                if (!description.isEmpty()) {
                    mediaItem.fields["description"] = description;
                }
            }
        } else if (mediaItem.type == "Video") {            
            //All video types can have a description
            KLineEdit * descriptionWidget = static_cast<KLineEdit*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(3), 1));
            QString description = descriptionWidget->text();
            if (!description.isEmpty()) {
                mediaItem.fields["description"] = description;
            }
            
            if (typeComboBox->currentText() == "Video Clip") {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Video Clip";
            } else if (typeComboBox->currentText() == "Movie") {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Movie";
            } else if (typeComboBox->currentText() == "Series") {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Series";
                
                QSpinBox *seasonWidget = static_cast<QSpinBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(4), 1));
                int season = seasonWidget->value();
                if (season != 0) {
                    mediaItem.fields["season"] = season;
                    mediaItem.subTitle = QString("Season %1 ").arg(season);
                }
                QSpinBox *episodeWidget = static_cast<QSpinBox*>(ui->infoMediaItems->itemWidget(ui->infoMediaItems->topLevelItem(5), 1));
                int episode = episodeWidget->value();
                if (episode != 0) {
                    mediaItem.fields["episode"] = episode;
                    mediaItem.subTitle = mediaItem.subTitle + QString("Episode %1").arg(episode);
                }
            }
        }
            
        updatedList << mediaItem;
        m_parent->m_mediaItemModel->replaceMediaItemAt(m_rows.at(i), mediaItem);        
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadMediaList(updatedList);
}

void InfoManager::showAudioType(int index)
{
    int startRow = ui->infoMediaItems->topLevelItemCount();
    QTreeWidgetItem * type = new QTreeWidgetItem(ui->infoMediaItems);
    type->setText(0, tr2i18n("Type"));
    ui->infoMediaItems->addTopLevelItem(type);
    QComboBox *typeComboBox = new QComboBox();
    QStringList typeList;
    typeList << "Music" << "Audio Stream" << "Audio Clip";
    typeComboBox->addItems(typeList);
    typeComboBox->setCurrentIndex(index);
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow), 1, typeComboBox);
    connect(typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(mediaTypeChanged(int)));
}

void InfoManager::showAudioFields()
{
    int startRow = ui->infoMediaItems->topLevelItemCount();
    QTreeWidgetItem * location = new QTreeWidgetItem(ui->infoMediaItems);
    location->setText(0, tr2i18n("Location"));
    ui->infoMediaItems->addTopLevelItem(location);
    KUrlRequester * locationEdit = new KUrlRequester();
    locationEdit->setText(commonValue("url").toString());
    locationEdit->setEnabled(false);
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow), 1, locationEdit);
    
    QTreeWidgetItem * title = new QTreeWidgetItem(ui->infoMediaItems);
    title->setText(0, tr2i18n("Title"));
    ui->infoMediaItems->addTopLevelItem(title);
    KLineEdit * titleLineEdit = new KLineEdit();
    titleLineEdit->setText(commonValue("title").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 1), 1, titleLineEdit);
    
    QTreeWidgetItem * description = new QTreeWidgetItem(ui->infoMediaItems);
    description->setText(0, tr2i18n("Description"));
    ui->infoMediaItems->addTopLevelItem(description);
    KLineEdit * descriptionEdit = new KLineEdit();
    descriptionEdit->setText(commonValue("description").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 2), 1, descriptionEdit);
}

void InfoManager::showAudioMusicFields()
{
    int startRow = ui->infoMediaItems->topLevelItemCount();
    QTreeWidgetItem * artist = new QTreeWidgetItem(ui->infoMediaItems);
    artist->setText(0, tr2i18n("Artist"));
    ui->infoMediaItems->addTopLevelItem(artist);
    QComboBox *artistComboBox = new QComboBox();
    artistComboBox->setEditable(true);
    artistComboBox->addItems(valueList("artist"));
    artistComboBox->setEditText(commonValue("artist").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow), 1, artistComboBox);
    
    QTreeWidgetItem * album = new QTreeWidgetItem(ui->infoMediaItems);
    album->setText(0, tr2i18n("Album"));
    ui->infoMediaItems->addTopLevelItem(album);
    QComboBox *albumComboBox = new QComboBox();
    albumComboBox->setEditable(true);
    albumComboBox->addItems(valueList("album"));
    albumComboBox->setEditText(commonValue("album").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 1), 1, albumComboBox);
    
    QTreeWidgetItem * year = new QTreeWidgetItem(ui->infoMediaItems);
    year->setText(0, tr2i18n("Year"));
    ui->infoMediaItems->addTopLevelItem(year);
    QSpinBox *yearSpinBox = new QSpinBox();
    yearSpinBox->setRange(0,9999);
    yearSpinBox->setSpecialValueText("-");
    yearSpinBox->setValue(commonValue("year").toInt());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 2), 1, yearSpinBox);
    
    QTreeWidgetItem * trackNumber = new QTreeWidgetItem(ui->infoMediaItems);
    trackNumber->setText(0, tr2i18n("Track Number"));
    ui->infoMediaItems->addTopLevelItem(trackNumber);
    QSpinBox *trackNumberSpinBox = new QSpinBox();
    trackNumberSpinBox->setRange(0,999);
    trackNumberSpinBox->setSpecialValueText("-");
    trackNumberSpinBox->setValue(commonValue("trackNumber").toInt());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 3), 1, trackNumberSpinBox);
    
    QTreeWidgetItem * genre = new QTreeWidgetItem(ui->infoMediaItems);
    genre->setText(0, tr2i18n("Genre"));
    ui->infoMediaItems->addTopLevelItem(genre);
    QComboBox *genreComboBox = new QComboBox();
    genreComboBox->setEditable(true);
    genreComboBox->addItems(valueList("genre"));
    genreComboBox->setEditText(commonValue("genre").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 4), 1, genreComboBox);
}

void InfoManager::showAudioStreamFields()
{
}

void InfoManager::showVideoType(int index)
{
    int startRow = ui->infoMediaItems->topLevelItemCount();
    QTreeWidgetItem * type = new QTreeWidgetItem(ui->infoMediaItems);
    type->setText(0, tr2i18n("Type"));
    ui->infoMediaItems->addTopLevelItem(type);
    QComboBox *typeComboBox = new QComboBox();
    QStringList typeList;
    typeList << "Movie" << "Series" << "Video Clip";
    typeComboBox->addItems(typeList);
    typeComboBox->setCurrentIndex(index);
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow), 1, typeComboBox);
    connect(typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(mediaTypeChanged(int)));
}

void InfoManager::showVideoFields()
{
    int startRow = ui->infoMediaItems->topLevelItemCount();
    QTreeWidgetItem * location = new QTreeWidgetItem(ui->infoMediaItems);
    location->setText(0, tr2i18n("Location"));
    ui->infoMediaItems->addTopLevelItem(location);
    KUrlRequester * locationEdit = new KUrlRequester();
    locationEdit->setText(commonValue("url").toString());
    locationEdit->setEnabled(false);
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow), 1, locationEdit);
    
    QTreeWidgetItem * title = new QTreeWidgetItem(ui->infoMediaItems);
    title->setText(0, tr2i18n("Title"));
    ui->infoMediaItems->addTopLevelItem(title);
    KLineEdit * titleLineEdit = new KLineEdit();
    titleLineEdit->setText(commonValue("title").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 1), 1, titleLineEdit);
    
    QTreeWidgetItem * description = new QTreeWidgetItem(ui->infoMediaItems);
    description->setText(0, tr2i18n("Description"));
    ui->infoMediaItems->addTopLevelItem(description);
    KLineEdit * descriptionLineEdit = new KLineEdit();
    descriptionLineEdit->setText(commonValue("description").toString());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 2), 1, descriptionLineEdit);
}

void InfoManager::showVideoSeriesFields()
{
    int startRow = ui->infoMediaItems->topLevelItemCount();
    QTreeWidgetItem * season = new QTreeWidgetItem(ui->infoMediaItems);
    season->setText(0, tr2i18n("Season"));
    ui->infoMediaItems->addTopLevelItem(season);
    QSpinBox *seasonSpinBox = new QSpinBox();
    seasonSpinBox->setRange(0,999);
    seasonSpinBox->setSpecialValueText("-");
    seasonSpinBox->setValue(commonValue("season").toInt());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow), 1, seasonSpinBox);
    
    QTreeWidgetItem * episode = new QTreeWidgetItem(ui->infoMediaItems);
    episode->setText(0, tr2i18n("Episode"));
    ui->infoMediaItems->addTopLevelItem(episode);
    QSpinBox *episodeSpinBox = new QSpinBox();
    episodeSpinBox->setRange(0,999);
    episodeSpinBox->setSpecialValueText("-");
    episodeSpinBox->setValue(commonValue("episode").toInt());
    ui->infoMediaItems->setItemWidget(ui->infoMediaItems->topLevelItem(startRow + 1), 1, episodeSpinBox);               
}

bool InfoManager::multipleVideoTypes()
{
    int videoTypeCount = 0;
    QString lastType;
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    for (int i = 0; i < mediaList.count(); i++) {
        QString currentType = mediaList.at(i).fields["videoType"].toString();
        if (!currentType.isEmpty()) {
            if (currentType != lastType) {
                videoTypeCount++;
                if (videoTypeCount > 1) {
                    return true;
                }
                lastType = currentType;
            }
        }
    }
    return false;
}

bool InfoManager::multipleAudioTypes()
{
    int audioTypeCount = 0;
    QString lastType;
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    for (int i = 0; i < mediaList.count(); i++) {
        QString currentType = mediaList.at(i).fields["videoType"].toString();
        if (!currentType.isEmpty()) {
            if (currentType != lastType) {
                audioTypeCount++;
                if (audioTypeCount > 1) {
                    return true;
                }
                lastType = currentType;
            }
        }
    }
    return false;
}
