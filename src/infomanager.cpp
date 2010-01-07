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
#include <KUrlRequester>
#include <KLineEdit>
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
    m_infoMediaItemsModel = new MediaItemModel(this);
    connect(ui->saveInfo, SIGNAL(clicked()), this, SLOT(saveInfoView()));
    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(showInfoView()));
    connect(ui->editInfo, SIGNAL(clicked()), this, SLOT(editInfoView()));
    connect(ui->mediaViewHolder, SIGNAL(currentChanged(int)), this, SLOT(mediaViewHolderChanged(int)));
    m_editToggle = false;
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        ui->editInfo->setEnabled(true);
    } else {
        ui->editInfo->setEnabled(false);
    }
    
}

InfoManager::~InfoManager()
{
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
    
void InfoManager::mediaViewHolderChanged(int index)
{
    if (index == 0) {
        m_editToggle = false;
        ui->editInfo->setText(i18n("Edit"));
        ui->editInfo->setIcon(KIcon("document-edit"));
    }
        
}

void InfoManager::showInfoView()
{
    loadInfoView();
}

void InfoManager::showInfoViewForMediaItem(const MediaItem &mediaItem)
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->mediaViewHolder->setCurrentIndex(1);
    ui->previous->setVisible(true);
    ui->previous->setText(ui->listTitle->text());
    
    m_rows.clear();
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    if (mediaList.count() == 0) {
        return;
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadMediaList(mediaList);
    showFields();
    m_editToggle = false;
    ui->editInfo->setText(i18n("Edit"));
    ui->editInfo->setIcon(KIcon("document-edit"));
    ui->playSelected->setVisible(false);
    ui->playAll->setVisible(false);
}

void InfoManager::editInfoView()
{
    m_editToggle = !m_editToggle;
    showFields(m_editToggle);
    if (m_editToggle) {
        ui->editInfo->setText(i18n("Cancel Edit"));
        ui->editInfo->setIcon(KIcon("dialog-cancel"));
    } else {
        ui->editInfo->setText(i18n("Edit"));
        ui->editInfo->setIcon(KIcon("document-edit"));
    }
}

void InfoManager::loadInfoView()
{
    ui->mediaViewHolder->setCurrentIndex(1);
    ui->previous->setVisible(true);
    ui->previous->setText(ui->listTitle->text());
    //ui->playSelected->setVisible(false);
    //ui->playAll->setVisible(false);
    
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
    m_infoMediaItemsModel->loadMediaList(mediaList);
    showFields();
}

void InfoManager::audioTypeChanged(int type)
{
    ui->infoView->clear();
    showCommonFields(true);
    if (!multipleAudioTypes()) {
        if (type == 0) {
            showAudioType(0, true);
            showAudioMusicFields(true);
        } else if (type == 1) {
            showAudioType(1, true);
            showAudioStreamFields(true);
        } else if (type == 2) {
            showAudioType(2, true);
            //No special audio clip fields
        }
    }
    QTreeWidgetItem * footer = new QTreeWidgetItem(ui->infoView);
    ui->infoView->addTopLevelItem(footer);
    ui->infoView->setFocus();
}

void InfoManager::videoTypeChanged(int type)
{
    ui->infoView->clear();
    showCommonFields(true);
    if (!multipleVideoTypes()) {
        if (type == 0) {
            showVideoType(0, true);
            showVideoMovieFields(true);
        } else if (type == 1) {
            showVideoType(1, true);
            showVideoTVShowFields(true);
        } else if (type == 2) {
            showVideoType(2, true);
            //No special video clip fields
        }
    }
    QTreeWidgetItem * footer = new QTreeWidgetItem(ui->infoView);
    ui->infoView->addTopLevelItem(footer);
    ui->infoView->setFocus();
}

void InfoManager::saveInfoView()
{
    ui->saveInfo->setText(i18n("Saving..."));
    ui->saveInfo->setEnabled(false);
    
    //Save info data to nepomuk store
    saveInfoToMediaModel();
    saveFileMetaData();
    m_parent->m_mediaItemModel->updateSourceInfo(m_infoMediaItemsModel->mediaList());
     
    //show non-editable fields
    m_editToggle = false;
    ui->editInfo->setText(i18n("Edit"));
    ui->editInfo->setIcon(KIcon("document-edit"));
    ui->saveInfo->setText(i18n("Save"));
    ui->saveInfo->setEnabled(true);
    ui->saveInfo->setVisible(false);
    showFields(false);
}

void InfoManager::showFields(bool edit) 
{
    ui->infoView->clear();
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    showCommonFields(edit);
    if (mediaList.at(0).type == "Audio") {
        if (!multipleAudioTypes()) {
            if (mediaList.at(0).fields["audioType"] == "Music") {
                showAudioType(0, edit);
                showAudioMusicFields(edit);
            } else if (mediaList.at(0).fields["audioType"] == "Audio Stream") {
                showAudioType(1, edit);
                showAudioStreamFields(edit);
            } else if (mediaList.at(0).fields["audioType"] == "Audio Clip") {
                showAudioType(2, edit);
                //No special audio clip fields
            }
        }
    } else if (mediaList.at(0).type == "Video") {
        if (!multipleVideoTypes()) {
            if (mediaList.at(0).fields["videoType"] == "Movie") {
                showVideoType(0, edit);
                showVideoMovieFields(edit);
            } else if (mediaList.at(0).fields["videoType"] == "TV Show") {
                showVideoType(1, edit);
                showVideoTVShowFields(edit);
            } else if (mediaList.at(0).fields["videoType"] == "Video Clip") {
                showVideoType(2, edit);
                //No special video clip fields
            }
        }
    }
    QTreeWidgetItem * footer = new QTreeWidgetItem(ui->infoView);
    ui->infoView->addTopLevelItem(footer);
    ui->infoView->setFocus();
    ui->saveInfo->setVisible(edit);
    if (mediaList.count() > 0 ) {
        if (!mediaList.at(0).fields["isTemplate"].toBool()) {
            ui->playSelected->setVisible(!edit);
        }
    }    
}
        
void InfoManager::showCommonFields(bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Title"), TitleFormat);
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 1, tr2i18n("Artwork"), TitleFormat);
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 2, tr2i18n("Description"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 3, tr2i18n("Location"));
    
    if (!edit) {
        setInfo(startRow, commonValue("title").toString(), TitleFormat);
        if (m_infoMediaItemsModel->rowCount() == 1) {
            MediaItem mediaItem = m_infoMediaItemsModel->mediaItemAt(0);
            QPixmap artwork = Utilities::getArtworkFromMediaItem(mediaItem);
            setInfo(startRow + 1, artwork);
        } else {
            setInfo(startRow + 1, QPixmap());
        }
        QString description;
        if (!commonValue("synopsis").toString().isEmpty()) {
            description = commonValue("synopsis").toString();
        } else {
            description = commonValue("description").toString();
        }
        setInfo(startRow + 2, description);
        setInfo(startRow + 3, commonValue("url").toString());
    } else {
        setEditWidget(startRow, new KLineEdit(), commonValue("title").toString());
        if (m_infoMediaItemsModel->rowCount() == 1) {
            MediaItem mediaItem = m_infoMediaItemsModel->mediaItemAt(0);
            QPixmap artwork = Utilities::getArtworkFromMediaItem(mediaItem);
            setEditWidget(startRow + 1, new ArtworkWidget(), artwork);
        } else {
            setEditWidget(startRow + 1, new ArtworkWidget(), QPixmap());
        }
        QString description;
        if (!commonValue("synopsis").toString().isEmpty()) {
            description = commonValue("synopsis").toString();
        } else {
            description = commonValue("description").toString();
        }
        setEditWidget(startRow + 2, new QTextEdit(), description);
        if (m_infoMediaItemsModel->rowCount() == 1) {
            MediaItem mediaItem = m_infoMediaItemsModel->mediaItemAt(0);
            if (mediaItem.type == "Audio" && mediaItem.fields["audioType"].toString() == "Audio Stream") {
                setEditWidget(startRow + 3, new KLineEdit(), commonValue("url").toString());
            } else {
                setInfo(startRow + 3, commonValue("url").toString());
            }
        } else {
            setInfo(startRow + 3, commonValue("url").toString());
        }
    }
}

void InfoManager::showAudioType(int index, bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    QStringList typeList;
    typeList << i18n("Music") << i18n("Audio Stream") << i18n("Audio Clip");
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Type"));
    
    if (!edit) {
        setInfo(startRow, typeList.at(index));
    } else {
        QComboBox *typeComboBox = new QComboBox();
        setEditWidget(startRow, typeComboBox, typeList.at(index) , typeList);
        connect(typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(audioTypeChanged(int)));
    }
}

void InfoManager::showAudioFields()
{
}

void InfoManager::showAudioMusicFields(bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Artist"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 1, tr2i18n("Album"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 2, tr2i18n("Year"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 3, tr2i18n("Track Number"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 4, tr2i18n("Genre"));
    
    if (!edit) {
        setInfo(startRow, commonValue("artist").toString());
        setInfo(startRow + 1, commonValue("album").toString());
        setInfo(startRow + 2, QString("%1").arg(commonValue("year").toInt()));
        setInfo(startRow + 3, QString("%1").arg(commonValue("trackNumber").toInt()));
        setInfo(startRow + 4, commonValue("genre").toString());
    } else {
        setEditWidget(startRow, new QComboBox(), commonValue("artist").toString(), valueList("artist"), true);
        setEditWidget(startRow + 1, new QComboBox(), commonValue("album").toString(), valueList("album"), true);
        setEditWidget(startRow + 2, new QSpinBox());
        QSpinBox * yw = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(startRow + 2), 1));
        yw->setRange(0, 9999);
        yw->setValue(commonValue("year").toInt());
        setEditWidget(startRow + 3, new QSpinBox(), commonValue("trackNumber").toInt());
        setEditWidget(startRow + 4, new QComboBox(), commonValue("genre").toString(), valueList("genre"), true);
    }
}

void InfoManager::showAudioStreamFields(bool edit)
{
    Q_UNUSED(edit);
}

void InfoManager::showVideoType(int index, bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    QStringList typeList;
    typeList << i18n("Movie") << i18n("TV Show") << i18n("Video Clip");
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Type"));
    
    if (!edit) {
        setInfo(startRow, typeList.at(index));
    } else {
        QComboBox *typeComboBox = new QComboBox();
        setEditWidget(startRow, typeComboBox, typeList.at(index), typeList);
        connect(typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(videoTypeChanged(int)));
    }
}

void InfoManager::showVideoFields()
{
}

void InfoManager::showVideoMovieFields(bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Year"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 1, tr2i18n("Genre"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 2, tr2i18n("Writer"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 3, tr2i18n("Director"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 4, tr2i18n("Producer"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 5, tr2i18n("Actor"));
    
    if (!edit) {
        QDate releaseDate = commonValue("releaseDate").toDate();
        QString year;
        if (releaseDate.isValid()) {
            year = QString("%1").arg(commonValue("releaseDate").toDate().year());
        }
        setInfo(startRow, year);
        setInfo(startRow + 1, commonValue("genre").toString());
        setInfo(startRow + 2, commonValue("writer").toString());
        setInfo(startRow + 3, commonValue("director").toString());
        setInfo(startRow + 4, commonValue("producer").toString());
        setInfo(startRow + 5, commonValue("actor").toString());
        
    } else {
        setEditWidget(startRow, new QSpinBox());
        QSpinBox * yw = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(startRow), 1));
        yw->setRange(0, 9999);
        yw->setValue(commonValue("releaseDate").toDate().year());
        setEditWidget(startRow + 1, new QComboBox(), commonValue("genre").toString(), valueList("genre"), true);
        setEditWidget(startRow + 2, new QComboBox(), commonValue("writer").toString(), valueList("writer"), true);
        setEditWidget(startRow + 3, new QComboBox(), commonValue("director").toString(), valueList("director"), true);
        setEditWidget(startRow + 4, new QComboBox(), commonValue("producer").toString(), valueList("producer"), true);
        setEditWidget(startRow + 5, new QComboBox(), commonValue("actor").toString(), valueList("actor"), true);
    }
    
}

void InfoManager::showVideoTVShowFields(bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Series Name"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 1, tr2i18n("Season"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 2, tr2i18n("Episode"));    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 3, tr2i18n("Year"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 4, tr2i18n("Genre"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 5, tr2i18n("Writer"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 6, tr2i18n("Director"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 7, tr2i18n("Producer"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 8, tr2i18n("Actor"));
    
    if (!edit) {
        setInfo(startRow, commonValue("seriesName").toString());
        setInfo(startRow + 1, QString("%1").arg(commonValue("season").toInt()));
        setInfo(startRow + 2, QString("%1").arg(commonValue("episodeNumber").toInt()));
        QDate releaseDate = commonValue("releaseDate").toDate();
        QString year;
        if (releaseDate.isValid()) {
            year = QString("%1").arg(commonValue("releaseDate").toDate().year());
        }
        setInfo(startRow + 3, year);
        setInfo(startRow + 4, commonValue("genre").toString());
        setInfo(startRow + 5, commonValue("writer").toString());
        setInfo(startRow + 6, commonValue("director").toString());
        setInfo(startRow + 7, commonValue("producer").toString());
        setInfo(startRow + 8, commonValue("actor").toString());
    } else {
        setEditWidget(startRow, new KLineEdit(), commonValue("seriesName").toString());
        setEditWidget(startRow + 1, new QSpinBox(), commonValue("season").toInt());
        setEditWidget(startRow + 2, new QSpinBox(), commonValue("episodeNumber").toInt());
        setEditWidget(startRow + 3, new QSpinBox());
        QSpinBox * yw = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(startRow + 3), 1));
        yw->setRange(0, 9999);
        setEditWidget(startRow + 4, new QComboBox(), commonValue("genre").toString(), valueList("genre"), true);
        setEditWidget(startRow + 5, new QComboBox(), commonValue("writer").toString(), valueList("writer"), true);
        setEditWidget(startRow + 6, new QComboBox(), commonValue("director").toString(), valueList("director"), true);
        setEditWidget(startRow + 7, new QComboBox(), commonValue("producer").toString(), valueList("producer"), true);
        setEditWidget(startRow + 8, new QComboBox(), commonValue("actor").toString(), valueList("actor"), true);
    }
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

void InfoManager::saveInfoToMediaModel()
{
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    QList<MediaItem> updatedList;
    QComboBox *typeComboBox = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
    
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        
        //All media types have a title
        KLineEdit * titleWidget = qobject_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(0), 1));
        QString title = titleWidget->text();
        if (!title.isEmpty()) {
            mediaItem.title = title;
            mediaItem.fields["title"] = title;
        }
        
        ArtworkWidget * artworkWidget = qobject_cast<ArtworkWidget*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(1), 1));
        QUrl url = artworkWidget->url();
        if (!url.isEmpty()) {
            mediaItem.fields["artworkUrl"] = url.toString();
        }

        QTextEdit * descriptionWidget = qobject_cast<QTextEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(2), 1));
        QString description = descriptionWidget->toPlainText();
        if (!description.isEmpty()) {
            if (mediaItem.type == "Video") {
                mediaItem.fields["synopsis"] = description;
            } else {
                mediaItem.fields["description"] = description;
            }
        }

        if (mediaItem.type == "Audio" ) {
            if (typeComboBox->currentIndex() == 0) {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Music";
                
                QComboBox *artistWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    mediaItem.fields["artist"] = artist;
                }
                
                QComboBox *albumWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    mediaItem.fields["album"] = album;
                }
                
                mediaItem.subTitle = mediaItem.fields["artist"].toString() + QString(" - ") + mediaItem.fields["album"].toString();
                
                QSpinBox *yearWidget = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    mediaItem.fields["year"] = year;
                }
                
                QSpinBox *trackNumberWidget = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    mediaItem.fields["trackNumber"] = trackNumber;
                }
                
                QComboBox *genreWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(9), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    mediaItem.fields["genre"] = genre;
                }
                
            } else if (typeComboBox->currentIndex() == 1) {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Stream";
                
                KLineEdit * urlWidget = qobject_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(3), 1));
                QString url = urlWidget->text();
                if (!url.isEmpty()) {
                    mediaItem.url = url;
                    mediaItem.fields["url"] = url;
                }
                
            } else if (typeComboBox->currentIndex() == 2) {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Clip";
            }
        } else if (mediaItem.type == "Video") {            
            if (typeComboBox->currentIndex() == 0) {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Movie";
                
                QSpinBox *yearWidget = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                if (yearWidget->value() != 0) {
                    QDate releaseDate = QDate(yearWidget->value(), 1, 1);
                    mediaItem.fields["releaseDate"] = releaseDate;
                }

                QComboBox *genreWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    mediaItem.fields["genre"] = genre;
                }
                
                QComboBox *writerWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                QString writer = writerWidget->currentText();
                if (!writer.isEmpty()) {
                    mediaItem.fields["writer"] = writer;
                }
                
                QComboBox *directorWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                QString director = directorWidget->currentText();
                if (!director.isEmpty()) {
                    mediaItem.fields["director"] = director;
                }
                
                QComboBox *producerWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(9), 1));
                QString producer = producerWidget->currentText();
                if (!producer.isEmpty()) {
                    mediaItem.fields["producer"] = producer;
                }
                
                QComboBox *actorWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(10), 1));
                QString actor = actorWidget->currentText();
                if (!actor.isEmpty()) {
                    mediaItem.fields["actor"] = actor;
                }
                
            } else if (typeComboBox->currentIndex() == 1) {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "TV Show";
                
                KLineEdit * seriesNameWidget = qobject_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString seriesName = seriesNameWidget->text();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                    mediaItem.subTitle = seriesName;
                }
                
                QSpinBox *seasonWidget = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                int season = seasonWidget->value();
                mediaItem.fields["season"] = season;
                if (!mediaItem.subTitle.isEmpty()) {
                    mediaItem.subTitle += " - ";
                }
                mediaItem.subTitle += QString("Season %1").arg(season);
                
                QSpinBox *episodeWidget = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int episodeNumber = episodeWidget->value();
                mediaItem.fields["episodeNumber"] = episodeNumber;
                if (!mediaItem.subTitle.isEmpty()) {
                    mediaItem.subTitle += " - ";
                }
                mediaItem.subTitle += QString("Episode %1").arg(episodeNumber);
                
                QSpinBox *yearWidget = qobject_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                if (yearWidget->value() != 0) {
                    QDate releaseDate = QDate(yearWidget->value(), 1, 1);
                    mediaItem.fields["releaseDate"] = releaseDate;
                }
                
                QComboBox *genreWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(9), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    mediaItem.fields["genre"] = genre;
                }
                
                
                QComboBox *writerWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(10), 1));
                QString writer = writerWidget->currentText();
                if (!writer.isEmpty()) {
                    mediaItem.fields["writer"] = genre;
                }
                
                QComboBox *directorWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(11), 1));
                QString director = directorWidget->currentText();
                if (!director.isEmpty()) {
                    mediaItem.fields["director"] = director;
                }
                
                QComboBox *producerWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(12), 1));
                QString producer = producerWidget->currentText();
                if (!producer.isEmpty()) {
                    mediaItem.fields["producer"] = producer;
                }
                
                QComboBox *actorWidget = qobject_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(13), 1));
                QString actor = actorWidget->currentText();
                if (!actor.isEmpty()) {
                    mediaItem.fields["actor"] = actor;
                }
                
            } else if (typeComboBox->currentIndex() == 2) {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Video Clip";
            }
        }
        mediaItem.fields["isTemplate"] = false;
            
        updatedList << mediaItem;
        m_parent->m_mediaItemModel->replaceMediaItemAt(m_rows.at(i), mediaItem);        
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadMediaList(updatedList);
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

void InfoManager::setLabel(int row, const QString &label, int format)
{
    QLabel * labelLabel= new QLabel();
    labelLabel->setWordWrap(true);
    labelLabel->setText(label);
    labelLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    labelLabel->setMargin(4);
    QFont font = QFont();
    font.setItalic(true);
    if (format == TitleFormat) {
        labelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    labelLabel->setFont(font);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 0, labelLabel);    
}

void InfoManager::setInfo(int row, const QString &info, int format)
{
    QLabel * infoLabel= new QLabel();
    infoLabel->setWordWrap(true);
    infoLabel->setText(info);
    infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    infoLabel->setMargin(4);
    QFont font = QFont();
    if (format == TitleFormat) {
        font.setPointSize(1.5*font.pointSize());
        infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    infoLabel->setFont(font);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, infoLabel);
}

void InfoManager::setInfo(int row, const QPixmap &pixmap)
{
    QLabel * infoLabel= new QLabel();
    infoLabel->setPixmap(pixmap);
    infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    infoLabel->setMargin(8);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, infoLabel);
}

void InfoManager::setEditWidget(int row, KLineEdit *lineEdit, const QString &value)
{
    lineEdit->setText(value);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row ), 1, lineEdit);
}

void InfoManager::setEditWidget(int row, QTextEdit *textEdit, const QString &value)
{
    textEdit->setText(value);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, textEdit);
}

void InfoManager::setEditWidget(int row, QComboBox *comboBox, const QString &value, const QStringList &list, bool editable)
{
    comboBox->addItems(list);
    if (editable) {
        comboBox->setEditable(true);
        comboBox->setEditText(value);
    } else {
        comboBox->setEditable(false);
        int indexOfValue = list.indexOf(value);
        if (indexOfValue != -1) {
            comboBox->setCurrentIndex(indexOfValue);
        } else {
            comboBox->setCurrentIndex(0);
        }
    }
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, comboBox);    
}

void InfoManager::setEditWidget(int row, KUrlRequester *urlRequester, const QString &value)
{
    urlRequester->setText(value);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, urlRequester);
}

void InfoManager::setEditWidget(int row, QSpinBox *spinBox, int value)
{
    spinBox->setRange(0,999);
    spinBox->setSpecialValueText("-");
    spinBox->setValue(value);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, spinBox);
}

void InfoManager::setEditWidget(int row, ArtworkWidget *artworkWidget, const QPixmap &pixmap)
{
    artworkWidget->setPixmap(pixmap);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, artworkWidget);
}

void InfoManager::setEditWidget(int row, QDateEdit *dateEdit, const QDate &date)
{
    dateEdit->setMinimumDate(QDate(-4000,1,1));
    dateEdit->setDisplayFormat("MMMM dd yyyy");
    dateEdit->setDate(date);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row ), 1, dateEdit);
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
