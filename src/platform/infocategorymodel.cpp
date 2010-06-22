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

#include "infocategorymodel.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include "dbpediaquery.h"
#include <KLocale>
#include <KDebug>
#include <KUrl>
#include <KStandardDirs>
#include <kio/netaccess.h>
#include <kio/copyjob.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>
#include <Solid/Networking>
#include <QDomDocument>

InfoCategoryModel::InfoCategoryModel(QObject *parent) : QStandardItemModel(parent)
{
    connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoModified(QStandardItem *)));
    m_mode = DefaultMode;
    m_dbPediaQuery = new DBPediaQuery(this);
}

InfoCategoryModel::~InfoCategoryModel()
{
}

void InfoCategoryModel::loadInfo(const QList<MediaItem> & mediaList) 
{
    m_mediaList = mediaList;
    clear();
    
    if (m_mediaList.count() > 0) {
        // Get fields shared by all media types
        QString type = m_mediaList.at(0).type;
        
        if (type == "Category") {
            QString subType = m_mediaList.at(0).fields["categoryType"].toString();
            setMode(categoryModeFromCategoryType(subType));
            addFieldToValuesModel(i18n("Image"), "artworkUrl");
            addFieldToValuesModel(i18n("Name"), "title");
            addFieldToValuesModel(i18n("Description"), "description");
            //addFieldToValuesModel(i18n("Rating"), "rating");
            
            if (subType == "Audio Feed" || subType == "Video Feed") {
                addFieldToValuesModel(i18n("Location"), "url");
            }
        }
    }
}

void InfoCategoryModel::downloadInfo()
{
    if (!hasMultipleValues("title")) {
        if(Solid::Networking::status() == Solid::Networking::Connected){
            m_gotDownloadedData = false;
            getDBPediaInfo(commonValue("title").toString());
        }
    }
}

void InfoCategoryModel::saveChanges()
{
    QList<MediaItem> updatedList;
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *currentItem = item(row, 0);
            QString field = currentItem->data(FieldRole).toString();
            //Save any field that does not have multiple values.
            //If multiple items are selected and a field is edited
            //then the edited field won't have multiple values
            bool multipleValues = currentItem->data(MultipleValuesRole).toBool();
            if (!multipleValues) { 
                mediaItem.fields[field] = currentItem->data(Qt::EditRole);
                if (field == "artworkUrl") {
                    mediaItem.artwork = currentItem->data(Qt::DecorationRole).value<QIcon>();
                }
            }
        }
        updatedList << mediaItem;
    }
    m_mediaList = updatedList;
    
    //Update source information
    m_sourceModel->updateSourceInfo(m_mediaList);
    
    //Ensure original values in model are updated to reflect saved(no-edits) state
    loadInfo(m_mediaList); 
}

void InfoCategoryModel::cancelChanges()
{
    loadInfo(m_mediaList);
}

QList<MediaItem> InfoCategoryModel::mediaList()
{
    return m_mediaList;
}

void InfoCategoryModel::setSourceModel(MediaItemModel * sourceModel)
{
    m_sourceModel = sourceModel;
}

void InfoCategoryModel::setMode(InfoCategoryMode mode)
{
    m_mode = mode;
}

InfoCategoryModel::InfoCategoryMode InfoCategoryModel::mode()
{
    return m_mode;
}

void InfoCategoryModel::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable)
{
    QList<QStandardItem *> rowData;
    
    QStandardItem *fieldItem = new QStandardItem();
    fieldItem->setData(field, InfoCategoryModel::FieldRole);
    bool hasMultiple = hasMultipleValues(field);
    fieldItem->setData(hasMultiple, InfoCategoryModel::MultipleValuesRole);
    if (m_mode == AudioFeedMode || m_mode == VideoFeedMode) {
        fieldItem->setEditable(true);
    } else {
        fieldItem->setEditable(false);
    }
    
    bool addRow = false;
    if (field == "artworkUrl") {
        if (m_mediaList.count() == 1) {
            if (m_mode == AlbumMode || m_mode == AudioFeedMode || m_mode == AudioFeedMode) {
                addRow = true;
                fieldItem->setData(m_mediaList.at(0).artwork, Qt::DecorationRole);
            }
        } else {
            //We should eventually check for common artwork and set it here.
        }
        rowData.append(fieldItem);
        if (addRow) {
            appendRow(rowData);
        }
        return;
    }

    if (!hasMultiple) {
        //Set field value
        QVariant value = commonValue(field);
        //if (!value.isNull()) {
            fieldItem->setData(value, Qt::DisplayRole);
            fieldItem->setData(value, Qt::EditRole);
            fieldItem->setData(value, InfoCategoryModel::OriginalValueRole); //stores copy of original data
            addRow = true;
        //}
    } else {
        //Set default field value
        QVariant value = m_mediaList.at(0).fields[field];
        if (value.type() == QVariant::String) {
            fieldItem->setData(QString(), Qt::EditRole);
            fieldItem->setData(valueList(field), InfoCategoryModel::ValueListRole);
        } else if (value.type() == QVariant::Int) {
            fieldItem->setData(0, Qt::EditRole);
        }
    }
    rowData.append(fieldItem);
    if (addRow) {
        appendRow(rowData);
    }
    
    Q_UNUSED(fieldTitle);
    Q_UNUSED(forceEditable);
}

bool InfoCategoryModel::hasMultipleValues(const QString &field)
{
    QVariant value;
    
    if (field == "artworkUrl") {
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

QVariant InfoCategoryModel::commonValue(const QString &field)
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

QStringList InfoCategoryModel::valueList(const QString &field)
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

void InfoCategoryModel::checkInfoModified(QStandardItem *changedItem)
{
    bool modified;
    if (changedItem->data(Qt::DisplayRole) != changedItem->data(InfoCategoryModel::OriginalValueRole)) {
        modified = true;
    } else {
        modified = false;
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *otherItem = item(row, 0);
            if (otherItem->data(InfoCategoryModel::FieldRole).toString() != "artworkUrl") {
                if (otherItem->data(Qt::DisplayRole) != otherItem->data(InfoCategoryModel::OriginalValueRole)) {
                    modified = true;
                    break;
                }
            }
        }
    }
    
    //Populate other fields when mediaFeed url is set
    QString field = changedItem->data(InfoCategoryModel::FieldRole).toString();
    if ((m_mode == AudioFeedMode || m_mode == VideoFeedMode) && field == "url") {
        getFeedInfo();
    }

    emit infoChanged(modified);
    
}

void InfoCategoryModel::saveFileMetaData(QList<MediaItem> mediaList)
{
    Q_UNUSED(mediaList);
}

void InfoCategoryModel::getDBPediaInfo(const QString &title)
{
    if (m_mode == ArtistMode) {
        kDebug() << "getting artist info";
        connect (m_dbPediaQuery, SIGNAL(gotArtistInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
        m_dbPediaQuery->getArtistInfo(title);
    }  else if (m_mode == ActorMode) {
        kDebug() << "getting actor info";
        connect (m_dbPediaQuery, SIGNAL(gotActorInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
        m_dbPediaQuery->getActorInfo(title);
    } else if (m_mode == DirectorMode) {
        kDebug() << "getting director info";
        connect (m_dbPediaQuery, SIGNAL(gotDirectorInfo(bool , const QList<Soprano::BindingSet>, const QString)), this, SLOT(gotPersonInfo(bool , const QList<Soprano::BindingSet>, const QString)));
        m_dbPediaQuery->getDirectorInfo(title);
    } 
}

void InfoCategoryModel::gotPersonInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey)
{
    //Determine request key for current mode
    QString keyPrefix;
    if (m_mode == ArtistMode) {
        keyPrefix = "Artist:";
    } else if (m_mode == ActorMode) {
        keyPrefix = "Actor:";
    } else if (m_mode == DirectorMode) {
        keyPrefix = "Director:";
    } 
    
    QString keyForCurrentData = keyPrefix + commonValue("title").toString();
    
    
    if (successful && requestKey == keyForCurrentData && !m_gotDownloadedData) {
        m_gotDownloadedData = true;
        if (results.count() > 0) {
            Soprano::BindingSet binding = results.at(0);
            
            clear();
            //Get Thumbnail
            KUrl thumbnailUrl = KUrl(binding.value("thumbnail").uri());
            if (thumbnailUrl.isValid()) {
                //Create placeholder in model for thumbnail
                QList<QStandardItem *> rowData;
                QStandardItem *fieldItem = new QStandardItem();
                fieldItem->setData("artworkUrl", InfoCategoryModel::FieldRole);
                fieldItem->setData(QIcon(), Qt::DecorationRole);
                rowData.append(fieldItem);
                appendRow(rowData);
                
                getThumbnail(thumbnailUrl);
            }

            {
                QList<QStandardItem *> rowData;
                QString title = commonValue("title").toString();
                QStandardItem *fieldItem = new QStandardItem();
                fieldItem->setData("title", InfoCategoryModel::FieldRole);
                fieldItem->setData(title, Qt::DisplayRole);
                fieldItem->setData(title, Qt::EditRole);
                fieldItem->setData(title, InfoCategoryModel::OriginalValueRole);
                rowData.append(fieldItem);
                appendRow(rowData);
            }
            
            //Get Description
            QString description = binding.value("description").literal().toString();
            if (!description.isEmpty()) {
                QList<QStandardItem *> rowData;
                QStandardItem *fieldItem = new QStandardItem();
                fieldItem->setData("description", InfoCategoryModel::FieldRole);
                fieldItem->setData(description, Qt::DisplayRole);
                fieldItem->setData(description, Qt::EditRole);
                fieldItem->setData(description, InfoCategoryModel::OriginalValueRole);
                rowData.append(fieldItem);
                appendRow(rowData);
            }
            
            emit modelDataChanged();
            
        }
    }
}

void InfoCategoryModel::getThumbnail(KUrl thumbnailUrl, bool keepThumbnail)
{
    //Prepare job to retrieve thumbnail
    m_keepThumbnail = keepThumbnail;
    QString folder = "temp";
    if (m_keepThumbnail) {
        folder = "thumbnails";
    }
    QString thumbnailTargetFile = QString("bangarang/%1/%2").arg(folder).arg(thumbnailUrl.fileName());
    KUrl thumbnailTargetUrl = KUrl(KStandardDirs::locateLocal("data", thumbnailTargetFile, true));
    QFile downloadTarget(thumbnailTargetUrl.path());
    downloadTarget.remove();
    KIO::CopyJob *copyJob = KIO::copyAs(thumbnailUrl, thumbnailTargetUrl, KIO::Overwrite | KIO::HideProgressInfo);
    copyJob->setUiDelegate(0);
    copyJob->setAutoDelete(true);
    connect (copyJob, 
             SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
             this,
             SLOT(gotThumbnail(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
}

void InfoCategoryModel::gotThumbnail(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(from);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);
    
    QString thumbnailFile = to.path();
    QPixmap thumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!thumbnail.isNull())  {
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *rowItem = item(row, 0);
            QString field = rowItem->data(InfoCategoryModel::FieldRole).toString();
            if (field == "artworkUrl") {
                rowItem->setData(QIcon(thumbnail), Qt::DecorationRole);
                if (m_keepThumbnail) {
                    rowItem->setData(to.prettyUrl(), Qt::EditRole);
                }
                break;
            }
        }
    }
    if (!m_keepThumbnail) {
        QFile(to.path()).remove();
    }
    m_keepThumbnail = false;
}

void InfoCategoryModel::getFeedInfo()
{
    if (m_mode == AudioFeedMode || m_mode == VideoFeedMode) {
        QString feedUrlStr;
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *rowItem = item(row, 0);
            if (rowItem->data(InfoCategoryModel::FieldRole).toString() == "url") {
                feedUrlStr = rowItem->data(Qt::EditRole).toString();
                break;
            }
        }
        KUrl feedUrl(feedUrlStr);
        if (!feedUrl.isEmpty()) {
            QString feedFilePath;
            
           //Prepare job to retrieve feed info
            QString feedTargetFile = QString("bangarang/temp/%1").arg(feedUrl.fileName());
            KUrl feedTargetUrl = KUrl(KStandardDirs::locateLocal("data", feedTargetFile, true));
            QFile feedTarget(feedTargetUrl.path());
            feedTarget.remove();
            KIO::CopyJob *copyJob = KIO::copyAs(feedUrl, feedTargetUrl, KIO::Overwrite | KIO::HideProgressInfo);
            copyJob->setUiDelegate(0);
            copyJob->setAutoDelete(true);
            connect (copyJob, 
                     SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
                     this,
                     SLOT(gotFeedInfo(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
                     
        }
    }
}

void InfoCategoryModel::gotFeedInfo(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);

    if (m_mode != AudioFeedMode && m_mode != VideoFeedMode) {
        return;
    }
    
    QString feedUrl;
    for (int row = 0; row < rowCount(); row++) {
        QStandardItem *rowItem = item(row, 0);
        if (rowItem->data(InfoCategoryModel::FieldRole).toString() == "url") {
            feedUrl = rowItem->data(Qt::EditRole).toString();
            break;
        }
    }
    if (from != KUrl(feedUrl)) {
        return;
    }
    
    QFile file(to.path());
    QDomDocument feedDoc("feed");
    feedDoc.setContent(&file);
    
    QString title;
    QString description;
    
    //Iterate through item nodes of the XML document
    QDomNodeList channels = feedDoc.elementsByTagName("channel");
    for (int i = 0; i < channels.count(); i++) {
        QDomNodeList nodes = channels.at(i).childNodes();
        for (int j = 0; j < nodes.count(); j++) {
            if (nodes.at(j).isElement()) {
                QDomElement element = nodes.at(j).toElement();
                if (element.tagName() == "title") {
                    title = element.text();
                } else if (element.tagName() == "description") {
                    description = element.text();
                } else if (element.tagName() == "itunes:image") {
                    getThumbnail(KUrl(element.attribute("href")), true);
                }
            }
        }
    }
    QFile(to.path()).remove();
    
    //Set fields in model
    for (int row = 0; row < rowCount(); row++) {
        QStandardItem *rowItem = item(row, 0);
        if (rowItem->data(InfoCategoryModel::FieldRole).toString() == "title") {
            rowItem->setData(title, Qt::EditRole);
        } else if (rowItem->data(InfoCategoryModel::FieldRole).toString() == "description") {
            rowItem->setData(description, Qt::EditRole);
        }
    }
}

InfoCategoryModel::InfoCategoryMode InfoCategoryModel::categoryModeFromCategoryType(const QString &categoryType)
{
    if (categoryType == "Artist") {
        return InfoCategoryModel::ArtistMode;
    } else if (categoryType == "Album") {
        return InfoCategoryModel::AlbumMode;
    } else if (categoryType == "MusicGenre") {
        return InfoCategoryModel::MusicGenreMode;
    } else if (categoryType == "AudioTag") {
        return InfoCategoryModel::AudioTagMode;
    } else if (categoryType == "TVSeries") {
        return InfoCategoryModel::TVSeriesMode;
    } else if (categoryType == "TVSeason") {
        return InfoCategoryModel::TVSeasonMode;
    } else if (categoryType == "Actor") {
        return InfoCategoryModel::ActorMode;
    } else if (categoryType == "Director") {
        return InfoCategoryModel::DirectorMode;
    } else if (categoryType == "VideoGenre") {
        return InfoCategoryModel::VideoGenreMode;
    } else if (categoryType == "VideoTag") {
        return InfoCategoryModel::VideoTagMode;
    } else if (categoryType == "Audio Feed") {
        return InfoCategoryModel::AudioFeedMode;
    } else if (categoryType == "Video Feed") {
        return InfoCategoryModel::VideoFeedMode;
    } else {
        return InfoCategoryModel::DefaultMode;
    }
}

