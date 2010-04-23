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
#include <KUrl>
#include <QPainter>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

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
            addFieldToValuesModel(i18n("Description"), "description");
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
            addFieldToValuesModel(i18n("Description"), "description");
            QString subType = m_mediaList.at(0).fields["videoType"].toString();
            if (subType == "Movie" || subType == "TV Show") {
                if (subType == "TV Show") {
                    addFieldToValuesModel(i18n("Series"), "seriesName");
                    addFieldToValuesModel(i18n("Season"), "season");
                    addFieldToValuesModel(i18n("Episode"), "episodeNumber");
                }
                addFieldToValuesModel(i18n("Genre"), "genre");
                addFieldToValuesModel(i18n("Year"), "year");
                addFieldToValuesModel(i18n("Actor"), "actor");
                addFieldToValuesModel(i18n("Director"), "director");
                addFieldToValuesModel(i18n("Writer"), "writer");
                addFieldToValuesModel(i18n("Producer"), "producer");
            }
            addFieldToValuesModel(i18n("Location"), "url");
        }
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
            if (field != "title" && field != "artwork" && field != "description") {
                currentItem = item(row, 1); //These fields don't span both columns
            }
            //Save any field that does not have multiple values.
            //If multiple items are selected and a field is edited
            //then the edited field won't have multiple values
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
                } else if (field == "artwork") {              
                    mediaItem.fields["artworkUrl"] = currentItem->data(Qt::EditRole).value<KUrl>().url();
                } else {
                    mediaItem.fields[field] = currentItem->data(Qt::EditRole);
                }
            }
        }
        updatedList << mediaItem;
    }
    m_mediaList = updatedList;
    
    //Update File Metadata
    saveFileMetaData(m_mediaList);
    
    //Update source information
    m_sourceModel->updateSourceInfo(m_mediaList);
    
    //Ensure original values in model are updated to reflect saved(no-edits) state
    loadInfo(m_mediaList); 
}

void InfoItemModel::cancelChanges()
{
    loadInfo(m_mediaList);
}

QList<MediaItem> InfoItemModel::mediaList()
{
    return m_mediaList;
}

void InfoItemModel::setSourceModel(MediaItemModel * sourceModel)
{
    m_sourceModel = sourceModel;
}

void InfoItemModel::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable)
{
    QList<QStandardItem *> rowData;
    
    // Set field label (artwork and title fields span both columns)
    if (field != "artwork" && field != "title" && field != "description") {
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
        KUrl artworkUrl = KUrl(m_mediaList.at(0).fields["artworkUrl"].toString());
        fieldItem->setData(artworkUrl, Qt::DisplayRole);
        fieldItem->setData(artworkUrl, Qt::EditRole);
        fieldItem->setData(artworkUrl, InfoItemModel::OriginalValueRole); //stores copy of original data
        
        //Compose artwork slice for each selected item.
        QPixmap artwork(128,128);
        artwork.fill(Qt::transparent);
        QPainter p(&artwork);
        int totalSlices = qMin(12, m_mediaList.count());
        for (int i = 0; i < totalSlices; i++) {
            int sliceSourceRow = i * (m_mediaList.count()/totalSlices) + 0.5; 
            MediaItem sliceSourceItem = m_mediaList.at(sliceSourceRow);
            QPixmap itemArtwork = Utilities::getArtworkFromMediaItem(sliceSourceItem);
            if (!itemArtwork.isNull()) {
                QPixmap centeredArtwork(128,128);
                centeredArtwork.fill(Qt::transparent);
                QPainter p1(&centeredArtwork);
                QIcon(itemArtwork).paint(&p1, 0, 0, 128, 128, Qt::AlignCenter);
                itemArtwork = centeredArtwork;
            } else {
                itemArtwork = sliceSourceItem.artwork.pixmap(128,128);
            }
            qreal sliceWidth = 128.0/totalSlices;
            qreal sliceLeft = sliceWidth * i;
            p.drawPixmap(QRectF(sliceLeft, 0, sliceWidth, 128), itemArtwork, QRectF(sliceLeft, 0, sliceWidth, 128));
        }
        p.end();
        fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
            
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
        if (field == "url") {
            fieldItem->setData(value, Qt::ToolTipRole);
        }
    } else {
        //Set default field value
        QVariant value = m_mediaList.at(0).fields[field];
        if (value.type() == QVariant::String) {
            fieldItem->setData(QString(), Qt::EditRole);
            fieldItem->setData(valueList(field), InfoItemModel::ValueListRole);
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
        if (value.isNull()) {
            value = m_mediaList.at(i).fields.value(field);
        } else if (m_mediaList.at(i).fields.value(field) != value) {
            return true;
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

void InfoItemModel::saveFileMetaData(QList<MediaItem> mediaList)
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

        