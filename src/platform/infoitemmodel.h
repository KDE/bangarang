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

#ifndef INFOITEMMODEL_H
#define INFOITEMMODEL_H

#include <KIcon>
#include <QtCore>
#include <QStandardItemModel>

class MediaItem;
class MediaItemModel;
class InfoFetcher;
class MediaIndexer;

/*
* This model is responsible for 
*
*/ 
class InfoItemModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        enum InfoItemRoles {FieldRole = Qt::UserRole,
                            FieldNameRole = Qt::UserRole + 1,
                            MultipleValuesRole = Qt::UserRole + 2,
                            OriginalValueRole = Qt::UserRole + 3,
                            ValueListRole = Qt::UserRole + 4,
                            ArtworkListRole = Qt::UserRole + 5};
        enum FetchType {AutoFetch = 0,
                        Fetch = 1};
        InfoItemModel(QObject * parent = 0);
        ~InfoItemModel();
        void loadInfo(const QList<MediaItem> & mediaList);
        void saveChanges();
        void cancelChanges();
        QList<MediaItem> mediaList();
        void setSourceModel(MediaItemModel *sourceModel);
        QList<InfoFetcher *> infoFetchers();
        QList<InfoFetcher *> availableInfoFetchers();
        bool autoFetchIsAvailable(InfoFetcher* infoFetcher);
        bool fetchIsAvailable(InfoFetcher* infoFetcher);
        void autoFetch(InfoFetcher* infoFetcher, bool updateRequiredFields = true, bool updateArtwork = true);
        void fetch(InfoFetcher* infoFetcher);
                   
    private:
        QList<MediaItem> m_mediaList;
        MediaItemModel * m_sourceModel;
        QHash<QString, QStringList> m_fieldsOrder;
        QHash<QString, QString> m_fieldNames;
        QHash<QString, QStringList> m_restrictedFields;
        bool m_defaultEditable;
        bool m_modified;
        QList<InfoFetcher *> m_infoFetchers;
        FetchType m_fetchType;
        MediaIndexer * m_indexer;
        void addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool isEditable = false);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        bool isEmpty(const QString &field);
        void saveFileMetaData(QList<MediaItem> mediaList);
        void saveCustomGenreInfo(QList<MediaItem> mediaList);
        bool getArtwork(QStandardItem *fieldItem, QString artworkUrlOverride = QString());

    Q_SIGNALS:
        void infoChanged(bool modified);
        void fetching();
        void fetchComplete();
        
    private Q_SLOTS:
        void checkInfoModified(QStandardItem *changedItem);
        void infoFetched(MediaItem mediaItem);
};

#endif // INFOITEMDELEGATE_H
