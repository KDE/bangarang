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
    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(showInfoView()));
    connect(ui->editInfo, SIGNAL(clicked()), this, SLOT(editInfoView()));
    connect(ui->mediaViewHolder, SIGNAL(currentChanged(int)), this, SLOT(mediaViewHolderChanged(int)));
    m_editToggle = false;
}

InfoManager::~InfoManager()
{
}

void InfoManager::mediaViewHolderChanged(int index)
{
    if (index == 0) {
        m_editToggle = false;
        ui->editInfo->setText("Edit");
        ui->editInfo->setIcon(KIcon("document-edit"));
    }
        
}

void InfoManager::showInfoView()
{
    loadInfoView();
}

void InfoManager::editInfoView()
{
    m_editToggle = !m_editToggle;
    showFields(m_editToggle);
    if (m_editToggle) {
        ui->editInfo->setText("Cancel Edit");
        ui->editInfo->setIcon(KIcon("dialog-cancel"));
    } else {
        ui->editInfo->setText("Edit");
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
    ui->saveInfo->setText("Saving...");
    ui->saveInfo->setEnabled(false);
    
    //Save info data to nepomuk store
    saveInfoToMediaModel();
    m_mediaIndexer->indexMediaItems(m_infoMediaItemsModel->mediaList());
    connect(m_mediaIndexer, SIGNAL(indexingComplete()), m_parent->m_mediaItemModel, SLOT(reload()));

    //Save metadata to files
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
    if ((m_infoMediaItemsModel->mediaItemAt(0).type == "Audio") && (typeComboBox->currentIndex() == 0)) {
        saveMusicInfoToFiles();
    }
     
    //show non-editable fields
    m_editToggle = false;
    ui->editInfo->setText("Edit");
    ui->editInfo->setIcon(KIcon("document-edit"));
    ui->saveInfo->setText("Save");
    ui->saveInfo->setEnabled(true);
    ui->saveInfo->setVisible(false);
    showFields(false);
    m_parent->m_mediaItemModel->reload();
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
        setInfo(startRow + 2, commonValue("description").toString());
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
        setEditWidget(startRow + 2, new QTextEdit(), commonValue("description").toString());
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
    typeList << "Music" << "Audio Stream" << "Audio Clip";
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
    /*ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 4, tr2i18n("Genre"));*/
    
    if (!edit) {
        setInfo(startRow, commonValue("artist").toString());
        setInfo(startRow + 1, commonValue("album").toString());
        setInfo(startRow + 2, QString("%1").arg(commonValue("year").toInt()));
        setInfo(startRow + 3, QString("%1").arg(commonValue("trackNumber").toInt()));
        //setInfo(startRow + 4, commonValue("genre").toString());
    } else {
        setEditWidget(startRow, new QComboBox(), commonValue("artist").toString(), valueList("artist"), true);
        setEditWidget(startRow + 1, new QComboBox(), commonValue("album").toString(), valueList("album"), true);
        setEditWidget(startRow + 2, new QSpinBox(), commonValue("year").toInt());
        setEditWidget(startRow + 3, new QSpinBox(), commonValue("trackNumber").toInt());
        //setEditWidget(startRow + 4, new QComboBox(), commonValue("genre").toString(), valueList("genre"), true);
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
    typeList << "Movie" << "TV Show" << "Video Clip";
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
    setLabel(startRow, tr2i18n("Collection/Series Name"));
    if (!edit) {
        setInfo(startRow, QString("%1").arg(commonValue("seriesName").toString()));
    } else {
        setEditWidget(startRow, new KLineEdit(), commonValue("seriesName").toString());
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
    setLabel(startRow + 2, tr2i18n("Episode"));
    
    if (!edit) {
        setInfo(startRow, QString("%1").arg(commonValue("seriesName").toString()));
        setInfo(startRow + 1, QString("%1").arg(commonValue("season").toInt()));
        setInfo(startRow + 2, QString("%1").arg(commonValue("episode").toInt()));
    } else {
        setEditWidget(startRow, new KLineEdit(), commonValue("seriesName").toString());
        setEditWidget(startRow + 1, new QSpinBox(), commonValue("season").toInt());
        setEditWidget(startRow + 2, new QSpinBox(), commonValue("episode").toInt());
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
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
    if ((mediaList.at(0).type == "Audio") && (typeComboBox->currentIndex() == 0)) {
        for (int i = 0; i < mediaList.count(); i++) {
            if (Utilities::isMusic(mediaList.at(i).url)) {
                TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toUtf8());
                
                KLineEdit * titleWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(0), 1));
                QString title = titleWidget->text();
                if (!title.isEmpty()) {
                    TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setTitle(tTitle);
                }
                
                ArtworkWidget * artworkWidget = static_cast<ArtworkWidget*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(1), 1));
                QUrl url = artworkWidget->url();
                const QPixmap *pixmap = artworkWidget->artwork();
                if (!url.isEmpty() && !pixmap->isNull()) {
                    //FIXME: Can't understand why this doesn't work.
                    Utilities::saveArtworkToTag(mediaList.at(i).url, pixmap);
                    //(Utilities::saveArtworkToTag(mediaList.at(i).url, url.toString()));
                }
                
                QComboBox *artistWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setArtist(tArtist);
                }
                
                QComboBox *albumWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setAlbum(tAlbum);
                }
                
                QSpinBox *yearWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    file.tag()->setYear(year);
                }
                
                QSpinBox *trackNumberWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    file.tag()->setTrack(trackNumber);
                }
                
                /*QComboBox *genreWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    TagLib::String tGenre(genre.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setGenre(tGenre);
                }*/
                
                file.save();
            }
        }
    }
    
}

void InfoManager::saveInfoToMediaModel()
{
    QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
    QList<MediaItem> updatedList;
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
    
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        
        //All media types have a title
        KLineEdit * titleWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(0), 1));
        QString title = titleWidget->text();
        if (!title.isEmpty()) {
            mediaItem.title = title;
            mediaItem.fields["title"] = title;
        }
        
        ArtworkWidget * artworkWidget = static_cast<ArtworkWidget*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(1), 1));
        QUrl url = artworkWidget->url();
        if (!url.isEmpty()) {
            mediaItem.fields["artworkUrl"] = url.toString();
        }

        QTextEdit * descriptionWidget = static_cast<QTextEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(2), 1));
        QString description = descriptionWidget->toPlainText();
        if (!description.isEmpty()) {
            mediaItem.fields["description"] = description;
        }

        if (mediaItem.type == "Audio" ) {
            if (typeComboBox->currentIndex() == 0) {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Music";
                
                QComboBox *artistWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    mediaItem.fields["artist"] = artist;
                }
                
                QComboBox *albumWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    mediaItem.fields["album"] = album;
                }
                
                mediaItem.subTitle = mediaItem.fields["artist"].toString() + QString(" - ") + mediaItem.fields["album"].toString();
                
                QSpinBox *yearWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    mediaItem.fields["year"] = year;
                }
                
                QSpinBox *trackNumberWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    mediaItem.fields["trackNumber"] = trackNumber;
                }
                
                /*QComboBox *genreWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    mediaItem.fields["genre"] = genre;
                }*/
            } else if (typeComboBox->currentIndex() == 1) {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Stream";
                
                KLineEdit * urlWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(3), 1));
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
                
                KLineEdit * seriesNameWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString seriesName = seriesNameWidget->text();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName.trimmed();
                }
            } else if (typeComboBox->currentIndex() == 1) {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "TV Show";
                
                KLineEdit * seriesNameWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString seriesName = seriesNameWidget->text();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                }
                QSpinBox *seasonWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                int season = seasonWidget->value();
                mediaItem.fields["season"] = season;
                mediaItem.subTitle = QString("Season %1 ").arg(season);
                QSpinBox *episodeWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int episode = episodeWidget->value();
                mediaItem.fields["episode"] = episode;
                mediaItem.subTitle = mediaItem.subTitle + QString("Episode %1").arg(episode);
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

void InfoManager::setLabel(int row, QString label, int format)
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

void InfoManager::setInfo(int row, QString info, int format)
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

void InfoManager::setInfo(int row, QPixmap pixmap)
{
    QLabel * infoLabel= new QLabel();
    infoLabel->setPixmap(pixmap);
    infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    infoLabel->setMargin(8);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, infoLabel);
}

void InfoManager::setEditWidget(int row, KLineEdit *lineEdit, QString value)
{
    lineEdit->setText(value);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row ), 1, lineEdit);
}

void InfoManager::setEditWidget(int row, QTextEdit *textEdit, QString value)
{
    textEdit->setText(value);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, textEdit);
}

void InfoManager::setEditWidget(int row, QComboBox *comboBox, QString value, QStringList list, bool editable)
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

void InfoManager::setEditWidget(int row, KUrlRequester *urlRequester, QString value)
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

void InfoManager::setEditWidget(int row, ArtworkWidget *artworkWidget, QPixmap pixmap)
{
    artworkWidget->setPixmap(pixmap);
    ui->infoView->setItemWidget(ui->infoView->topLevelItem(row), 1, artworkWidget);
}
