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

#include "infoitemmodel.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include <KLocale>
#include <KDebug>

InfoItemModel::InfoItemModel(QObject *parent) : QStandardItemModel(parent)
{
    connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoModified(QStandardItem *)));
}

InfoItemModel::~InfoItemModel()
{
}

void InfoItemModel::loadInfo(const QList<MediaItem> & mediaList) 
{
    m_mediaList = mediaList;
    clear();
    
    if (m_mediaList.count() > 0) {
        // Get fields shared by all media types
        QString type = m_mediaList.at(0).type;
        
        if (type == "Audio") {
            addFieldToValuesModel(i18n("Type"), "audioType");
            addFieldToValuesModel(i18n("Artwork"), "artwork");
            addFieldToValuesModel(i18n("Title"), "title");
            QString subType = m_mediaList.at(0).fields["audioType"].toString();
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
            QString subType = m_mediaList.at(0).fields["videoType"].toString();
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
        if (m_mediaList.count() == 1) {
            addFieldToValuesModel(i18n("Play Count"), "playCount");
            addFieldToValuesModel(i18n("Last Played"), "lastPlayed");
        }
    }
}

void InfoItemModel::saveChanges()
{
    QList<MediaItem> updatedList;
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *currentItem = item(row, 0);
            QString field = currentItem->data(InfoItemModel::FieldRole).toString();
            if (field != "title" && field != "artwork") {
                currentItem = item(row, 1); //These fields don't span both columns
            }
            bool multipleValues = currentItem->data(InfoItemModel::MultipleValuesRole).toBool();
            if (!multipleValues) { 
                if (field == "audioType") {
                    int value = currentItem->data(Qt::EditRole).toInt();
                    if (value == 0) {
                        mediaItem.fields["audioType"] = "Music";
                    } else if (value == 1) {
                        mediaItem.fields["audioType"] = "Audio Stream";
                    } else if (value == 2) {
                        mediaItem.fields["audioType"] = "Audio Clip";
                    }
                } else if (field == "videoType") {
                    int value = currentItem->data(Qt::EditRole).toInt();
                    if (value == 0) {
                        mediaItem.fields["videoType"] = "Movie";
                    } else if (value == 1) {
                        mediaItem.fields["videoType"] = "TV Show";
                    } else if (value == 2) {
                        mediaItem.fields["videoType"] = "Video Clip";
                    }
                } else if (field == "title") {
                    mediaItem.fields["title"] = currentItem->data(Qt::EditRole);
                    mediaItem.title = currentItem->data(Qt::EditRole).toString();
                } else if (field == "url") {
                    if (mediaItem.fields["audioType"].toString() == "Audio Stream") {
                        mediaItem.fields["url"] = currentItem->data(Qt::EditRole);
                        mediaItem.url = currentItem->data(Qt::EditRole).toString();
                    }
                } else if (field != "artwork") {              
                    mediaItem.fields[field] = currentItem->data(Qt::EditRole);
                }
            }
        }
        updatedList << mediaItem;
    }
    m_mediaList = updatedList;
    loadInfo(m_mediaList); //This to ensure that original values in the model are updated
}

void InfoItemModel::cancelChanges()
{
    loadInfo(m_mediaList);
}

QList<MediaItem> InfoItemModel::mediaList()
{
    return m_mediaList;
}

void InfoItemModel::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable)
{
    QList<QStandardItem *> rowData;
    
    // Set field label (artwork and title fields span both columns)
    if (field != "artwork" && field != "title") {
        QStandardItem *fieldTitleItem = new QStandardItem(fieldTitle);
        fieldTitleItem->setData(field, InfoItemModel::FieldRole);
        fieldTitleItem->setEditable(false);
        rowData.append(fieldTitleItem);
    }

    QStandardItem *fieldItem = new QStandardItem();
    fieldItem->setData(field, InfoItemModel::FieldRole);
    bool hasMultiple = hasMultipleValues(field);
    fieldItem->setData(hasMultiple, InfoItemModel::MultipleValuesRole);
    bool isEditable = true;
    if ((field == "playCount" || field == "lastPlayed" || field == "url") && !forceEditable) {
        isEditable = false;
    }
    fieldItem->setEditable(isEditable);
    if (isEditable) {
        fieldItem->setData(i18n("Double-click to edit"), Qt::ToolTipRole);
    }

    if (field == "artwork") {
        if (m_mediaList.count() == 1) {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(m_mediaList.at(0));
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
        appendRow(rowData);
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
        fieldItem->setData(value, InfoItemModel::OriginalValueRole); //stores copy of original data
    } else {
        //Set default field value
        QVariant value = m_mediaList.at(0).fields[field];
        if (value.type() == QVariant::String) {
            fieldItem->setData(QString(), Qt::EditRole);
        } else if (value.type() == QVariant::Int) {
            fieldItem->setData(0, Qt::EditRole);
        }
    }
    rowData.append(fieldItem);
    appendRow(rowData);
}

bool InfoItemModel::hasMultipleValues(const QString &field)
{
    QVariant value;
    
    if (field == "artwork") {
        if (m_mediaList.count() == 1) {
            return false;
        } else {
            return true;
        }
    }
                
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).fields.contains(field)) {
            if (value.isNull()) {
                value = m_mediaList.at(i).fields.value(field);
            } else if (m_mediaList.at(i).fields.value(field) != value) {
                return true;
            }
        }
    }
    return false;
}

QVariant InfoItemModel::commonValue(const QString &field)
{
    QVariant value;
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).fields.contains(field)) {
            if (value.isNull()) {
                value = m_mediaList.at(i).fields.value(field);
            } else if (m_mediaList.at(i).fields.value(field) != value) {
                value = QVariant();
                break;
            }
        }
    }
    return value;
}

QStringList InfoItemModel::valueList(const QString &field)
{
    QStringList value;
    value << QString();
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).fields.contains(field)) {
            if (value.indexOf(m_mediaList.at(i).fields.value(field).toString()) == -1) {
                value << m_mediaList.at(i).fields.value(field).toString();
            }
        }
    }
    return value;   
}

void InfoItemModel::checkInfoModified(QStandardItem *changedItem)
{
    bool modified;
    if (changedItem->data(Qt::DisplayRole) != changedItem->data(InfoItemModel::OriginalValueRole)) {
        modified = true;
    } else {
        modified = false;
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *otherItem = item(row, 0);
            if (otherItem->data(InfoItemModel::FieldRole).toString() != "title" && 
                otherItem->data(InfoItemModel::FieldRole).toString() != "artwork") {
                otherItem = item(row, 1);
            }
            if (otherItem->data(Qt::UserRole).toString() != "artwork") {
                if (otherItem->data(Qt::DisplayRole) != otherItem->data(InfoItemModel::OriginalValueRole)) {
                    modified = true;
                    break;
                }
            }
        }
    }
    emit infoChanged(modified);
    
}

        