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
#include "dbpediainfofetcher.h"
#include "mediaindexer.h"
#include <KLocale>
#include <KDebug>
#include <KUrl>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <QPainter>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

InfoItemModel::InfoItemModel(QObject *parent) : QStandardItemModel(parent)
{
    connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoModified(QStandardItem *)));

    m_defaultEditable = true;
    m_modified = false;

    //Store field order
    m_fieldsOrder["Music"] = QStringList() << "audioType" << "artwork" << "title" << "artist" << "album" << "trackNumber" << "year" << "genre" << "description" << "tags" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Audio Clip"] = QStringList() << "audioType" << "artwork" << "title" << "description" << "tags" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Audio Stream"] = QStringList() << "audioType" << "artwork" << "title" << "description" << "tags" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Video Clip"] = QStringList() << "videoType" << "artwork" << "title" << "description" << "tags" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Movie"] = QStringList() << "videoType" << "artwork" << "title" << "description" << "tags" << "genre" << "year" << "actor" << "director" << "writer" << "producer" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["TV Show"] = QStringList() << "videoType" << "artwork" << "title" << "description" << "tags" << "seriesName" << "season" << "episodeNumber" << "genre" << "year" << "actor" << "director" << "writer" << "producer" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Artist"] = QStringList() << "artwork" << "title" << "description";
    m_fieldsOrder["Album"] = QStringList() << "artwork" << "title";
    m_fieldsOrder["AudioGenre"] = QStringList() << "artwork" << "title";
    m_fieldsOrder["AudioTag"] = QStringList() << "title";
    m_fieldsOrder["TV Series"] = QStringList() << "artwork" << "title" << "description";
    m_fieldsOrder["VideoGenre"] = QStringList() << "title";
    m_fieldsOrder["Actor"] = QStringList() << "artwork" << "title" << "description";
    m_fieldsOrder["Director"] = QStringList() << "artwork" << "title" << "description";
    m_fieldsOrder["VideoTag"] = QStringList() << "title";
    m_fieldsOrder["Audio Feed"] = QStringList() << "artwork" << "title" << "description" << "url";
    m_fieldsOrder["Video Feed"] = QStringList() << "artwork" << "title" << "description" << "url";
    m_fieldsOrder["Basic"] = QStringList() << "title";
    m_fieldsOrder["Basic+Artwork"] = QStringList() << "artwork" << "title";

    //Store field names
    m_fieldNames["audioType"] = i18n("Type");
    m_fieldNames["artwork"] = i18n("Artwork");
    m_fieldNames["title"]= i18n("Title");
    m_fieldNames["artist"] = i18n("Artist");
    m_fieldNames["album"] = i18n("Album");
    m_fieldNames["trackNumber"] = i18n("Track");
    m_fieldNames["year"] = i18n("Year");
    m_fieldNames["genre"] = i18n("Genre");
    m_fieldNames["tags"] = i18n("Tags");
    m_fieldNames["url"] = i18n("Location");
    m_fieldNames["videoType"] = i18n("Type");
    m_fieldNames["actor"] = i18n("Actor");
    m_fieldNames["director"] = i18n("Director");
    m_fieldNames["writer"] = i18n("Writer");
    m_fieldNames["producer"] = i18n("Producer");
    m_fieldNames["seriesName"] = i18n("Series");
    m_fieldNames["season"] = i18n("Season");
    m_fieldNames["episodeNumber"] = i18n("episodeNumber");
    m_fieldNames["playCount"] = i18n("Play Count");
    m_fieldNames["lastPlayed"] = i18n("Last Played");

    //Store restricted fields
    m_restrictedFields["Music"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Audio Clip"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Audio Stream"] = QStringList() << "playCount" << "lastPlayed";
    m_restrictedFields["Video Clip"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Movie"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["TV Show"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Artist"] = QStringList() << "title";
    m_restrictedFields["Album"] = QStringList() << "artwork" << "title";
    m_restrictedFields["AudioGenre"] = QStringList() << "title";
    m_restrictedFields["AudioTag"] = QStringList() << "artwork" << "title";
    m_restrictedFields["TV Series"] = QStringList() << "title";
    m_restrictedFields["VideoGenre"] = QStringList() << "title";
    m_restrictedFields["Actor"] = QStringList() << "title";
    m_restrictedFields["Director"] = QStringList() << "title";
    m_restrictedFields["VideoTag"] = QStringList() << "artwork" << "title";


    //Set up InfoFetchers
    DBPediaInfoFetcher * dbPediaInfoFetcher = new DBPediaInfoFetcher(this);
    connect(dbPediaInfoFetcher, SIGNAL(infoFetched(MediaItem)), this, SLOT(infoFetched(MediaItem)));
    connect(dbPediaInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(dbPediaInfoFetcher, SIGNAL(fetchComplete()), this, SIGNAL(fetchComplete()));
    m_infoFetchers.append(dbPediaInfoFetcher);

    //Setup indexer
    m_indexer = new MediaIndexer(this);
}

InfoItemModel::~InfoItemModel()
{
}

void InfoItemModel::loadInfo(const QList<MediaItem> & mediaList) 
{
    m_modified = false;
    m_mediaList = mediaList;
    clear();
    
    if (m_mediaList.count() > 0) {
        QString type = m_mediaList.at(0).type;
        QString subType = m_mediaList.at(0).subType();

        //Load field info in order specified
        QStringList fieldsOrder = m_fieldsOrder.value(subType, m_fieldsOrder["Basic"]);
        for (int i = 0; i < fieldsOrder.count(); i++) {
            QString field = fieldsOrder.at(i);
            if ((Utilities::isCd(m_mediaList.at(0).url) || Utilities::isDvd(m_mediaList.at(0).url)) &&
                (field == "url")) {
                addFieldToValuesModel(i18n("Location"), "album", false); //or the user would see the ugly udi
            } else {
                bool isEditable = !m_restrictedFields[subType].contains(field);
                addFieldToValuesModel(m_fieldNames[field],field, isEditable);
            }
        }
        emit infoChanged(false);

        //Upon selection of only one media item, launch Autofetch if NO info
        //is available in the fetchable fields of the media item.
        if (m_mediaList.count() == 1) {
            for (int i = 0; i < m_infoFetchers.count(); i++) {
                if (autoFetchIsAvailable(m_infoFetchers.at(i))) {
                    QStringList fetchableFields = m_infoFetchers.at(i)->fetchableFields(subType);
                    QStringList requiredFields = m_infoFetchers.at(i)->requiredFields(subType);
                    bool fetchableFieldsEmpty = true;
                    for (int j = 0; j < fetchableFields.count(); j++) {
                        if (!requiredFields.contains(fetchableFields.at(j)) &&
                            !isEmpty(fetchableFields.at(j))) {
                            fetchableFieldsEmpty = false;
                        }
                    }
                    if (fetchableFieldsEmpty) {
                        autoFetch(m_infoFetchers.at(i), false, false);
                        break;
                    }
                }
            }
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
                    QString artworkUrl = currentItem->data(Qt::EditRole).toString();
                    mediaItem.fields["artworkUrl"] = currentItem->data(Qt::EditRole).toString();
                    if (!artworkUrl.isEmpty() && (mediaItem.subType() == "AudioGenre" || mediaItem.subType() == "VideoGenre")) {
                        mediaItem.artwork = currentItem->data(Qt::DecorationRole).value<QIcon>();
                        mediaItem.hasCustomArtwork = true;
                    } else if (mediaItem.subType() == "AudioGenre" || mediaItem.subType() == "VideoGenre"){
                        mediaItem.artwork = KIcon("flag-blue");
                        mediaItem.hasCustomArtwork = false;
                    }
                } else if (field == "tags") {
                    QStringList rawTagStrList = currentItem->data(Qt::EditRole).toString().split(";", QString::SkipEmptyParts);
                    QStringList tags;
                    for (int i = 0; i < rawTagStrList.count(); i++) {
                        tags.append(rawTagStrList.at(i).trimmed());
                    }
                    mediaItem.fields["tags"] = tags.join(";");
                } else {
                    mediaItem.fields[field] = currentItem->data(Qt::EditRole);
                }
            }
        }
        updatedList << mediaItem;
    }
    m_mediaList = updatedList;

    //Update Custom Genre Info
    saveCustomGenreInfo(m_mediaList);
    
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

QList<InfoFetcher *> InfoItemModel::infoFetchers()
{
    return m_infoFetchers;
}

QList<InfoFetcher *> InfoItemModel::availableInfoFetchers()
{
    QList<InfoFetcher *> availableFetchers;
    for (int i = 0; i < m_infoFetchers.count(); i++) {
        if (autoFetchIsAvailable(m_infoFetchers.at(i)) || fetchIsAvailable(m_infoFetchers.at(i))) {
            availableFetchers.append(m_infoFetchers.at(i));
        }
    }
    return availableFetchers;
}

bool InfoItemModel::autoFetchIsAvailable(InfoFetcher* infoFetcher)
{
    //Autofetch is only available when required info is available to make
    //fetch request AND there are no unsaved modifications.
    bool available = false;
    if (m_mediaList.count() == 0) {
        return available;
    }
    QString subType = m_mediaList.at(0).subType();
    if (!m_modified && infoFetcher->available(subType)) {
        QStringList requiredFields = infoFetcher->requiredFields(subType);
        available = true;
        for (int i =0; i < requiredFields.count(); i++) {
            if (isEmpty(requiredFields.at(i))) {
                available = false;
                break;
            }
        }
    }
    return available;
}

bool InfoItemModel::fetchIsAvailable(InfoFetcher* infoFetcher)
{
    //Fetch is only available when enough info is available to make
    //fetch request AND when only one media item is loaded.
    bool available = false;
    if (m_mediaList.count() == 0) {
        return available;
    }
    QString subType = m_mediaList.at(0).subType();
    if (m_mediaList.count() == 1  && infoFetcher->available(subType)) {
        QStringList requiredFields = infoFetcher->requiredFields(subType);
        available = true;
        for (int i =0; i < requiredFields.count(); i++) {
            if (isEmpty(requiredFields.at(i))) {
                available = false;
                break;
            }
        }
    }
    return available;
}

void InfoItemModel::autoFetch(InfoFetcher* infoFetcher, bool updateRequiredFields, bool updateArtwork)
{
    m_fetchType = AutoFetch;
    infoFetcher->fetchInfo(m_mediaList, updateRequiredFields, updateArtwork);
}

void InfoItemModel::fetch(InfoFetcher* infoFetcher)
{
    m_fetchType = Fetch;
    infoFetcher->fetchInfo(m_mediaList, true, true);
}


void InfoItemModel::infoFetched(MediaItem mediaItem)
{
    //Find corresponding media item
    int foundIndex = -1;
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).url == mediaItem.url) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex != -1 && m_fetchType == AutoFetch) {
        m_mediaList.replace(foundIndex, mediaItem);

        //Cache autofetched info in nepomuk
        m_indexer->updateInfo(mediaItem);

        //Ensure original values in model are updated to reflect saved(no-edits) state
        loadInfo(m_mediaList);
        emit infoChanged(false);
    } else if (foundIndex != -1 && m_fetchType == Fetch) {
        //Update model data
        for (int i = 0; i < rowCount(); i++) {
            QString field = item(i)->data(InfoItemModel::FieldRole).toString();
            if (field == "audioType" || field == "videoType") {
                QString subType = mediaItem.subType();
                QVariant value;
                if (subType == "Music" || subType == "Movie") {
                    value = QVariant(0);
                } else if (subType == "Audio Stream" || subType == "TV Show") {
                    value = QVariant(1);
                } else if (subType == "Audio Clip" || subType == "Video Clip") {
                    value = QVariant(2);
                }
                item(i)->setData(value, Qt::DisplayRole);
                item(i)->setData(value, Qt::EditRole);
            } else if (field == "artwork") {
                item(i)->setData(mediaItem.artwork, Qt::DecorationRole);
                item(i)->setData(mediaItem.fields["artworkUrl"], Qt::DisplayRole);
            } else {
                item(i)->setData(mediaItem.fields[field], Qt::DisplayRole);
                item(i)->setData(mediaItem.fields[field], Qt::EditRole);
            }
        }
    }
}

void InfoItemModel::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool isEditable)
{
    QList<QStandardItem *> rowData;
    QStandardItem *fieldItem = new QStandardItem();
    fieldItem->setData(field, InfoItemModel::FieldRole);
    fieldItem->setData(fieldTitle, InfoItemModel::FieldNameRole);
    bool hasMultiple = hasMultipleValues(field);
    fieldItem->setData(hasMultiple, InfoItemModel::MultipleValuesRole);
    fieldItem->setEditable(isEditable);
    if (isEditable) {
        fieldItem->setData(i18n("Double-click to edit"), Qt::ToolTipRole);
    }

    //Set artwork
    if (field == "artwork") {
        QString artworkUrl = m_mediaList.at(0).fields["artworkUrl"].toString();
        fieldItem->setData(artworkUrl, Qt::DisplayRole);
        fieldItem->setData(artworkUrl, Qt::EditRole);
        fieldItem->setData(artworkUrl, InfoItemModel::OriginalValueRole); //stores copy of original data

        //Get artwork for selected items
        if (getArtwork(fieldItem)) {
            rowData.append(fieldItem);
            appendRow(rowData);
        }
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

bool InfoItemModel::isEmpty(const QString &field)
{
    bool isEmpty = true;
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        if (mediaItem.fields.contains(field)) {
            QVariant::Type fieldType = mediaItem.fields[field].type();
            if (fieldType == QVariant::String) {
                isEmpty = mediaItem.fields[field].toString().isEmpty();
            } else if (fieldType == QVariant::StringList) {
                isEmpty = mediaItem.fields[field].toStringList().isEmpty();
            } else if (fieldType == QVariant::Date) {
                isEmpty = !mediaItem.fields[field].toDate().isValid();
            } else if (fieldType == QVariant::DateTime) {
                isEmpty = !mediaItem.fields[field].toDateTime().isValid();
            } else if (fieldType == QVariant::Int){
                isEmpty = !mediaItem.fields[field].isValid();
            }
            if (isEmpty) {
                break;
            }
        }
    }
    return isEmpty;
}

void InfoItemModel::checkInfoModified(QStandardItem *changedItem)
{
    if (changedItem->data(Qt::EditRole) != changedItem->data(InfoItemModel::OriginalValueRole)) {
        m_modified = true;
        QString field = changedItem->data(InfoItemModel::FieldRole).toString();
        if (field == "artwork") {
            disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoModified(QStandardItem *)));
            QString artworkUrl = changedItem->data(Qt::EditRole).toString();
            getArtwork(changedItem, artworkUrl);
            connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoModified(QStandardItem *)));
        }
    } else {
        m_modified = false;
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *otherItem = item(row, 0);
            if (otherItem->data(InfoItemModel::FieldRole).toString() != "artwork") {
                if (otherItem->data(Qt::DisplayRole) != otherItem->data(InfoItemModel::OriginalValueRole)) {
                    m_modified = true;
                    break;
                }
            }
        }
    }
    emit infoChanged(m_modified);
    
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
                TagLib::FileRef file(KUrl(mediaList.at(i).url).path().toLocal8Bit().constData());
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

void InfoItemModel::saveCustomGenreInfo(QList<MediaItem> mediaList)
{
    if (mediaList.count() > 0) {
        if (mediaList.at(0).type == "Category" &&
            (mediaList.at(0).subType() == "AudioGenre" || mediaList.at(0).subType() == "VideoGenre")) {
            QString localGenreFile = KGlobal::dirs()->locateLocal("data","bangarang/genrerc", true);
            if (localGenreFile.isEmpty()) {
                return;
            }

            KConfig genreConfig(localGenreFile);
            for (int i = 0; i < mediaList.count(); i++) {
                MediaItem mediaItem = mediaList.at(i);
                if (mediaItem.type == "Category" &&
                    (mediaItem.subType() == "AudioGenre" || mediaItem.subType() == "VideoGenre")) {
                    KConfigGroup genreGroup(&genreConfig, mediaItem.title);
                    QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
                    if (artworkUrl.isEmpty()) {
                        if (genreGroup.exists()) {
                            genreGroup.deleteEntry("artworkUrl");
                        }
                    } else {
                        genreGroup.writeEntry("artworkUrl", artworkUrl);
                    }
                }
            }
            genreConfig.sync();
        }
    }
}

bool InfoItemModel::getArtwork(QStandardItem *fieldItem, QString artworkUrlOverride)
{
    bool artworkExists = true;
    QList<QVariant> emptyArtworkPixmaps;
    fieldItem->setData(emptyArtworkPixmaps, InfoItemModel::ArtworkListRole);
    if (m_mediaList.count() == 1) {
        MediaItem mediaItem = m_mediaList.at(0);
        if (!artworkUrlOverride.isNull()) {
            mediaItem.fields["artworkUrl"] = artworkUrlOverride;
        }
        if (mediaItem.type == "Category") {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(mediaItem);
            if (!artwork.isNull()) {
                fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
            } else {
                QList<QImage> artworks;
                if (mediaItem.subType() == "AudioGenre") {
                    artworks = Utilities::getGenreArtworks(mediaItem.title, "audio");
                } else if (mediaItem.subType() == "VideoGenre") {
                    artworks = Utilities::getGenreArtworks(mediaItem.title, "video");
                } else if (mediaItem.subType() == "Artist") {
                    artworks = Utilities::getArtistArtworks(mediaItem.title);
                } else if (mediaItem.subType() == "AudioTag") {
                    artworks = Utilities::getTagArtworks(mediaItem.title, "audio");
                } else if (mediaItem.subType() == "VideoTag") {
                    artworks = Utilities::getTagArtworks(mediaItem.title, "video");
                } else if (mediaItem.subType() == "TV Series") {
                    artworks = Utilities::getTVSeriesArtworks(mediaItem.title);
                } else if (mediaItem.subType() == "Actor") {
                    artworks = Utilities::getActorArtworks(mediaItem.title);
                } else if (mediaItem.subType() == "Director") {
                    artworks = Utilities::getDirectorArtworks(mediaItem.title);
                }
                if (artworks.count() > 0 ) {
                    //Convert to Pixmap list
                    QList<QVariant> artworkPixmaps;
                    for (int i = 0; i < artworks.count(); i++) {
                        artworkPixmaps.append(QPixmap::fromImage(artworks.at(i)));
                    }
                    fieldItem->setData(artworkPixmaps, InfoItemModel::ArtworkListRole);
                } else {
                    fieldItem->setData(mediaItem.artwork, Qt::DecorationRole);
                }
            }
        } else {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(mediaItem);
            if (!artwork.isNull()) {
                fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
            } else {
                fieldItem->setData(mediaItem.artwork, Qt::DecorationRole);
            }
        }
    } else {
        if (m_mediaList.at(0).type == "Audio" || m_mediaList.at(0).type == "Video") {
            QList<QVariant> artworkPixmaps;
            QImage lastItemArtwork;
            for (int i = 0; i < qMin(10, m_mediaList.count()); i++) {
                MediaItem mediaItem = m_mediaList.at(i);
                if (!artworkUrlOverride.isNull()) {
                    mediaItem.fields["artworkUrl"] = artworkUrlOverride;
                }
                QImage itemArtwork = Utilities::getArtworkImageFromMediaItem(mediaItem);
                if (!lastItemArtwork.isNull() && !itemArtwork.isNull()) {
                    bool sameImage = Utilities::compareImage(lastItemArtwork, itemArtwork, 50);
                    if (!sameImage) {
                        artworkPixmaps.append(QPixmap::fromImage(itemArtwork));
                    }
                } else if (lastItemArtwork.isNull() && !itemArtwork.isNull()) {
                    artworkPixmaps.append(QPixmap::fromImage(itemArtwork));
                } else if (itemArtwork.isNull()) {
                    itemArtwork = mediaItem.artwork.pixmap(128,128).toImage();
                    artworkPixmaps.append(mediaItem.artwork.pixmap(128,128));
                }
                lastItemArtwork = itemArtwork;
            }
            if (artworkPixmaps.count() > 0) {
                fieldItem->setData(artworkPixmaps, InfoItemModel::ArtworkListRole);
            } else {
                artworkExists = false;
            }
        } else {
            artworkExists = false;
        }
    }
    return artworkExists;
}

        
