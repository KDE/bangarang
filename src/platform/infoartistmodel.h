/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef INFOARTISTMODEL_H
#define INFOARTISTMODEL_H

#include <KIcon>
#include <kio/copyjob.h>
#include <Soprano/BindingSet>
#include <QtCore>
#include <QStandardItemModel>

class MediaItem;
class MediaItemModel;

/*
* This model is responsible for 
*
*/ 
class InfoArtistModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        enum InfoArtistRoles {FieldRole = Qt::UserRole,
                            MultipleValuesRole = Qt::UserRole + 1,
                            OriginalValueRole = Qt::UserRole +2,
                            ValueListRole = Qt::UserRole +3};
        InfoArtistModel(QObject * parent = 0);
        ~InfoArtistModel();
        void loadInfo(const QList<MediaItem> & mediaList);
        void downloadInfo();
        void saveChanges();
        void cancelChanges();
        QList<MediaItem> mediaList();
        void setSourceModel(MediaItemModel *sourceModel);
                   
    private:
        QList<MediaItem> m_mediaList;
        MediaItemModel * m_sourceModel;
        KUrl m_dbPediaDownloadUrl;
        
        void addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable = false);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        void saveFileMetaData(QList<MediaItem> mediaList);
        void getDBPediaInfo(const QString &artistName);
        
    Q_SIGNALS:
        void infoChanged(bool modified);
        void modelDataChanged();
        
    private Q_SLOTS:
        void checkInfoModified(QStandardItem *changedItem);
        void gotArtistInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey);
};

#endif // INFOARTISTMODEL_H
