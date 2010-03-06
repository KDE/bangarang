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

#include "infoartistmodel.h"
#include "mediaitemmodel.h"
#include "utilities.h"
#include <KLocale>
#include <KDebug>
#include <KUrl>
#include <KStandardDirs>
#include <kio/netaccess.h>
#include <kio/copyjob.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

InfoArtistModel::InfoArtistModel(QObject *parent) : QStandardItemModel(parent)
{
    connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(checkInfoModified(QStandardItem *)));
}

InfoArtistModel::~InfoArtistModel()
{
}

void InfoArtistModel::loadInfo(const QList<MediaItem> & mediaList) 
{
    m_mediaList = mediaList;
    clear();
    
    if (m_mediaList.count() > 0) {
        // Get fields shared by all media types
        QString type = m_mediaList.at(0).type;
        
        if (type == "Category") {
            QString subType = m_mediaList.at(0).fields["categoryType"].toString();
            if (subType == "Artist") {
                addFieldToValuesModel(i18n("Image"), "associatedImage");
                addFieldToValuesModel(i18n("Name"), "fullName");
                addFieldToValuesModel(i18n("Description"), "description");
                addFieldToValuesModel(i18n("Rating"), "rating");
            }
        }
    }
}

void InfoArtistModel::downloadInfo()
{
    if (!hasMultipleValues("fullName")) {
        getDBPediaInfo(commonValue("fullName").toString());
    }
}

void InfoArtistModel::saveChanges()
{
}

void InfoArtistModel::cancelChanges()
{
    loadInfo(m_mediaList);
}

QList<MediaItem> InfoArtistModel::mediaList()
{
    return m_mediaList;
}

void InfoArtistModel::setSourceModel(MediaItemModel * sourceModel)
{
    m_sourceModel = sourceModel;
}

void InfoArtistModel::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable)
{
    QList<QStandardItem *> rowData;
    
    QStandardItem *fieldItem = new QStandardItem();
    fieldItem->setData(field, InfoArtistModel::FieldRole);
    bool hasMultiple = hasMultipleValues(field);
    fieldItem->setData(hasMultiple, InfoArtistModel::MultipleValuesRole);
    bool isEditable = true;
    fieldItem->setEditable(isEditable);
    if (isEditable) {
        //fieldItem->setData(i18n("Double-click to edit"), Qt::ToolTipRole);
    }

    if (field == "associatedImage") {
        if (m_mediaList.count() == 1) {
            KUrl artworkUrl = KUrl(m_mediaList.at(0).fields["associatedImage"].toString());
            fieldItem->setData(artworkUrl, Qt::DisplayRole);
            fieldItem->setData(artworkUrl, Qt::EditRole);
            fieldItem->setData(artworkUrl, InfoArtistModel::OriginalValueRole); //stores copy of original data
            if (artworkUrl.isValid()) {
                kDebug() << "artworkUrl:" << artworkUrl.path();
                QPixmap artwork = QPixmap(artworkUrl.path()).scaled(128,128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                if (!artwork.isNull()) {
                    fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
                } else {
                    fieldItem->setData(KIcon("system-users"), Qt::DecorationRole);
                }
            } else {
                fieldItem->setData(KIcon("system-users"), Qt::DecorationRole);
            }
        } else {
            //We should eventually check for common artwork and set it here.
            fieldItem->setData(KIcon("system-users"), Qt::DecorationRole);
        }
        rowData.append(fieldItem);
        appendRow(rowData);
        return;
    }

    if (!hasMultiple) {
        //Set field value
        QVariant value = commonValue(field);
        fieldItem->setData(value, Qt::DisplayRole);
        fieldItem->setData(value, Qt::EditRole);
        fieldItem->setData(value, InfoArtistModel::OriginalValueRole); //stores copy of original data
    } else {
        //Set default field value
        QVariant value = m_mediaList.at(0).fields[field];
        if (value.type() == QVariant::String) {
            fieldItem->setData(QString(), Qt::EditRole);
            fieldItem->setData(valueList(field), InfoArtistModel::ValueListRole);
        } else if (value.type() == QVariant::Int) {
            fieldItem->setData(0, Qt::EditRole);
        }
    }
    rowData.append(fieldItem);
    appendRow(rowData);
    
    Q_UNUSED(fieldTitle);
    Q_UNUSED(forceEditable);
}

bool InfoArtistModel::hasMultipleValues(const QString &field)
{
    QVariant value;
    
    if (field == "associatedImage") {
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

QVariant InfoArtistModel::commonValue(const QString &field)
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

QStringList InfoArtistModel::valueList(const QString &field)
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

void InfoArtistModel::checkInfoModified(QStandardItem *changedItem)
{
    bool modified;
    if (changedItem->data(Qt::DisplayRole) != changedItem->data(InfoArtistModel::OriginalValueRole)) {
        modified = true;
    } else {
        modified = false;
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *otherItem = item(row, 0);
            if (otherItem->data(Qt::UserRole).toString() != "associatedImage") {
                if (otherItem->data(Qt::DisplayRole) != otherItem->data(InfoArtistModel::OriginalValueRole)) {
                    modified = true;
                    break;
                }
            }
        }
    }
    emit infoChanged(modified);
    
}

void InfoArtistModel::saveFileMetaData(QList<MediaItem> mediaList)
{
    Q_UNUSED(mediaList);
}

void InfoArtistModel::getDBPediaInfo(const QString &artistName)
{
    QString dbPediaSPARQL = QString("PREFIX owl: <http://www.w3.org/2002/07/owl#> "
                            "PREFIX xsd: <http://www.w3.org/2001/XMLSchema#> "
                            "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> "
                            "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
                            "PREFIX foaf: <http://xmlns.com/foaf/0.1/> "
                            "PREFIX dc: <http://purl.org/dc/elements/1.1/> "
                            "PREFIX : <http://dbpedia.org/resource/> "
                            "PREFIX dbpedia2: <http://dbpedia.org/property/> "
                            "PREFIX dbpedia: <http://dbpedia.org/> "
                            "PREFIX skos: <http://www.w3.org/2004/02/skos/core#> "
                            "PREFIX dbo: <http://dbpedia.org/ontology/> "
                            "SELECT DISTINCT ?name ?description ?thumbnail "
                            "WHERE { "
                            "{ ?person rdf:type dbo:Band . } "
                            "UNION "
                            "{?person rdf:type dbo:MusicalArtist . } "
                            "?person foaf:name ?name . "
                            "?person rdfs:comment ?description . "
                            "OPTIONAL {?person dbo:thumbnail ?thumbnail . } "
                            "FILTER (?name ='%1') . "
                            "FILTER (LANG(?description) = 'en') . "
                            "} "
                            "ORDER BY ?name ")
                            .arg(artistName);
     dbPediaSPARQL = QString(QUrl::toPercentEncoding(dbPediaSPARQL));
     QString dbPediaUrlString= QString("http://dbpedia.org/sparql/?query=%1").arg(dbPediaSPARQL);
     KUrl dbPediaUrl = KUrl(dbPediaUrlString);
     
     m_dbPediaDownloadUrl = KUrl(KStandardDirs::locateLocal("data", QString("bangarang/%1.tmp").arg(artistName.trimmed()), true));
     QFile downloadTarget(m_dbPediaDownloadUrl.path());
     downloadTarget.remove();
     KIO::CopyJob *copyJob = KIO::copy(dbPediaUrl, m_dbPediaDownloadUrl, KIO::Overwrite | KIO::HideProgressInfo);
     connect (copyJob, 
              SIGNAL(copyingDone(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)),
              this,
              SLOT(dbPediaDownloadComplete(KIO::Job *, const KUrl, const KUrl, time_t, bool, bool)));
}



void InfoArtistModel::dbPediaDownloadComplete(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);
    
    QFile file(to.path());
    file.rename(QString("consumable%1").arg(file.fileName()));
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kDebug() << QString("Couldn't open dbpedia query result file:%1").arg(from.path());
        return;
    }
    QTextStream in(&file);
    int fields = 3;
    QString line;

    //read past headers
    for (int i = 0; i < fields + 4; i++) {
        line = in.readLine();
    }

    //read fields
    if (!in.atEnd()) {
        line = in.readLine(); //read past artist name - we already have it
    }

    //Get Description
    if (!in.atEnd()) {
        QString dbPediaDescription = in.readLine().trimmed(); 
        dbPediaDescription.remove(0, 4); //remove <tr>
        dbPediaDescription.chop(5);
        dbPediaDescription = dbPediaDescription.trimmed(); //remove </tr>
        if (!dbPediaDescription.isEmpty()) {
            item(2)->setData(dbPediaDescription, Qt::DisplayRole);
            item(2)->setData(dbPediaDescription, Qt::EditRole);
        } else {
            kDebug() << "No description found";
        } 
    }

    //Get thumbnail
    if (!in.atEnd()) {
        QString thumbnailUrlString = in.readLine().trimmed(); //read past artist name - we already have it
        thumbnailUrlString.remove(0, 4); //remove <tr>
        thumbnailUrlString.chop(5); //remove </tr>
        KUrl thumbnailUrl = KUrl(thumbnailUrlString);
        if (thumbnailUrl.isValid()) {
            QString thumbnailFile;
            QPixmap dbPediaThumbnail = QPixmap();
            if (KIO::NetAccess::download(thumbnailUrl, thumbnailFile, 0)) {
                kDebug() << thumbnailFile;
                dbPediaThumbnail = QPixmap(thumbnailFile).scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            if (!dbPediaThumbnail.isNull())  {
                item(0)->setData(QIcon(dbPediaThumbnail), Qt::DecorationRole);
            }
        } else {
            kDebug() << "No thumbnail found";
        }
    }
    
    file.remove();

}