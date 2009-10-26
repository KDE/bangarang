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
    loadInfoView(m_editToggle);
    if (m_editToggle) {
        ui->editInfo->setText("Cancel Edit");
        ui->editInfo->setIcon(KIcon("dialog-cancel"));
    } else {
        ui->editInfo->setText("Edit");
        ui->editInfo->setIcon(KIcon("document-edit"));
    }
}

void InfoManager::loadInfoView(bool edit)
{
    ui->mediaViewHolder->setCurrentIndex(1);
    ui->previous->setVisible(true);
    ui->previous->setText(ui->listTitle->text());
    ui->playSelected->setVisible(false);
    ui->playAll->setVisible(false);
    ui->saveInfo->setVisible(edit);
    ui->infoView->clear();
    
    m_rows.clear();
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        m_rows << selectedRows.at(i).row();
        mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
    }
    m_infoMediaItemsModel->clearMediaListData();
    m_infoMediaItemsModel->loadSources(mediaList);
    
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
            } else if (mediaList.at(0).fields["videoType"] == "Series") {
                showVideoType(1, edit);
                showVideoSeriesFields(edit);
            } else if (mediaList.at(0).fields["videoType"] == "Video Clip") {
                showVideoType(2, edit);
                //No special video clip fields
            }
        }
    }
    QTreeWidgetItem * footer = new QTreeWidgetItem(ui->infoView);
    ui->infoView->addTopLevelItem(footer);
    ui->infoView->setFocus();
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
        } else if (type == 1) {
            showVideoType(1, true);
            showVideoSeriesFields(true);
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

    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(3), 1));
    if (typeComboBox->currentText() == "Music") {
        saveMusicInfoToFiles();
    }
    saveInfoToMediaModel();
    m_mediaIndexer->indexMediaItems(m_infoMediaItemsModel->mediaList());
    
    //Go back to media view
    m_editToggle = false;
    ui->editInfo->setText("Edit");
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
        
void InfoManager::showCommonFields(bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Title"), TitleFormat);
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 1, tr2i18n("Description"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 2, tr2i18n("Location"));
    
    if (!edit) {
        setInfo(startRow, commonValue("title").toString(), TitleFormat);
        setInfo(startRow + 1, commonValue("description").toString());
        setInfo(startRow + 2, commonValue("url").toString());
    } else {
        setEditWidget(startRow, new KLineEdit(), commonValue("title").toString());
        setEditWidget(startRow + 1, new QTextEdit(), commonValue("description").toString());
        setInfo(startRow + 2, commonValue("url").toString());
        //setEditWidget(startRow + 2, new KUrlRequester(), commonValue("url").toString());
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
    typeList << "Movie" << "Series" << "Video Clip";
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

void InfoManager::showVideoSeriesFields(bool edit)
{
    int startRow = ui->infoView->topLevelItemCount();
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow, tr2i18n("Season"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 1, tr2i18n("Episode"));
    ui->infoView->addTopLevelItem(new QTreeWidgetItem());
    setLabel(startRow + 2, tr2i18n("Series Name"));
    
    if (!edit) {
        setInfo(startRow, QString("%1").arg(commonValue("season").toInt()));
        setInfo(startRow + 1, QString("%1").arg(commonValue("episode").toInt()));
        setInfo(startRow + 2, QString("%1").arg(commonValue("seriesName").toString()));
    } else {
        setEditWidget(startRow, new QSpinBox(), commonValue("season").toInt());
        setEditWidget(startRow + 1, new QSpinBox(), commonValue("episode").toInt());
        setEditWidget(startRow + 2, new KLineEdit(), commonValue("seriesName").toString());
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
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(3), 1));
    if (typeComboBox->currentText() == "Music") {
        QList<MediaItem> mediaList = m_infoMediaItemsModel->mediaList();
        for (int i = 0; i < mediaList.count(); i++) {
            if (Utilities::isMusic(mediaList.at(i).url)) {
                TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toUtf8());
                
                KLineEdit * titleWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(0), 1));
                QString title = titleWidget->text();
                if (!title.isEmpty()) {
                    TagLib::String tTitle(title.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setTitle(tTitle);
                }
                
                QComboBox *artistWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    TagLib::String tArtist(artist.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setArtist(tArtist);
                }
                
                QComboBox *albumWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    TagLib::String tAlbum(album.trimmed().toUtf8().data(), TagLib::String::UTF8);
                    file.tag()->setAlbum(tAlbum);
                }
                
                QSpinBox *yearWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    file.tag()->setYear(year);
                }
                
                QSpinBox *trackNumberWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    file.tag()->setTrack(trackNumber);
                }
                
                QComboBox *genreWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
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
    QComboBox *typeComboBox = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(3), 1));
    
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        
        //All media types have a title
        KLineEdit * titleWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(0), 1));
        QString title = titleWidget->text();
        if (!title.isEmpty()) {
            mediaItem.title = title;
            mediaItem.fields["title"] = title;
        }

        QTextEdit * descriptionWidget = static_cast<QTextEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(1), 1));
        QString description = descriptionWidget->toPlainText();
        if (!description.isEmpty()) {
            mediaItem.fields["description"] = description;
        }

        if (mediaItem.type == "Audio" ) {
            if (typeComboBox->currentText() == "Music") {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Music";
                
                QComboBox *artistWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
                QString artist = artistWidget->currentText();
                if (!artist.isEmpty()) {
                    mediaItem.fields["artist"] = artist;
                }
                
                QComboBox *albumWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                QString album = albumWidget->currentText();
                if (!album.isEmpty()) {
                    mediaItem.fields["album"] = album;
                }
                
                mediaItem.subTitle = mediaItem.fields["artist"].toString() + QString(" - ") + mediaItem.fields["album"].toString();
                
                QSpinBox *yearWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                int year = yearWidget->value();
                if (year != 0) {
                    mediaItem.fields["year"] = year;
                }
                
                QSpinBox *trackNumberWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(7), 1));
                int trackNumber = trackNumberWidget->value();
                if (trackNumber != 0) {
                    mediaItem.fields["trackNumber"] = trackNumber;
                }
                
                QComboBox *genreWidget = static_cast<QComboBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(8), 1));
                QString genre = genreWidget->currentText();
                if (!genre.isEmpty()) {
                    mediaItem.fields["genre"] = genre;
                }
            } else if (typeComboBox->currentText() == "Audio Clip") {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Clip";
            } else if (typeComboBox->currentText() == "Audio Stream") {
                mediaItem.type = "Audio";
                mediaItem.fields["audioType"] = "Audio Stream";
            }
        } else if (mediaItem.type == "Video") {            
            if (typeComboBox->currentText() == "Video Clip") {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Video Clip";
            } else if (typeComboBox->currentText() == "Movie") {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Movie";
            } else if (typeComboBox->currentText() == "Series") {
                mediaItem.type = "Video";
                mediaItem.fields["videoType"] = "Series";
                
                QSpinBox *seasonWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(4), 1));
                int season = seasonWidget->value();
                if (season != 0) {
                    mediaItem.fields["season"] = season;
                    mediaItem.subTitle = QString("Season %1 ").arg(season);
                }
                QSpinBox *episodeWidget = static_cast<QSpinBox*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(5), 1));
                int episode = episodeWidget->value();
                if (episode != 0) {
                    mediaItem.fields["episode"] = episode;
                    mediaItem.subTitle = mediaItem.subTitle + QString("Episode %1").arg(episode);
                }
                KLineEdit * seriesNameWidget = static_cast<KLineEdit*>(ui->infoView->itemWidget(ui->infoView->topLevelItem(6), 1));
                QString seriesName = seriesNameWidget->text();
                if (!seriesName.isEmpty()) {
                    mediaItem.fields["seriesName"] = seriesName;
                }
            }
        }
            
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
