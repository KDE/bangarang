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

#ifndef INFOCATEGORYMODEL_H
#define INFOCATEGORYMODEL_H

#include <KIcon>
#include <kio/copyjob.h>
#include <Soprano/BindingSet>
#include <QtCore>
#include <QStandardItemModel>

class MediaItem;
class MediaItemModel;
class DBPediaQuery;

/*
* This model is responsible for 
*
*/ 
class InfoCategoryModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        enum InfoCategoryRole {FieldRole = Qt::UserRole,
                            MultipleValuesRole = Qt::UserRole + 1,
                            OriginalValueRole = Qt::UserRole +2,
                            ValueListRole = Qt::UserRole +3};
        enum InfoCategoryMode {DefaultMode = 0,
                                ArtistMode = 1,
                                AlbumMode = 2,
                                MusicGenreMode = 3,
                                AudioTagMode = 4,
                                TVSeriesMode = 5,
                                TVSeasonMode = 6,
                                VideoGenreMode = 7,
                                VideoTagMode = 8,
                                ActorMode = 9,
                                DirectorMode = 10};
        InfoCategoryModel(QObject * parent = 0);
        ~InfoCategoryModel();
        void loadInfo(const QList<MediaItem> & mediaList);
        void downloadInfo();
        void saveChanges();
        void cancelChanges();
        QList<MediaItem> mediaList();
        void setSourceModel(MediaItemModel *sourceModel);
        void setMode(InfoCategoryMode mode);
        InfoCategoryMode mode();
                   
    private:
        QList<MediaItem> m_mediaList;
        MediaItemModel * m_sourceModel;
        KUrl m_dbPediaDownloadUrl;
        InfoCategoryMode m_mode;
        DBPediaQuery * m_dbPediaQuery;
        
        void addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable = false);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        void saveFileMetaData(QList<MediaItem> mediaList);
        void getDBPediaInfo(const QString &artistName);
        InfoCategoryModel::InfoCategoryMode categoryModeFromCategoryType(const QString &categoryType);
        
    Q_SIGNALS:
        void infoChanged(bool modified);
        void modelDataChanged();
        
    private Q_SLOTS:
        void checkInfoModified(QStandardItem *changedItem);
        void gotPersonInfo(bool successful, const QList<Soprano::BindingSet> results, const QString &requestKey);
        void loadThumbnail(KIO::Job *job, const KUrl &from, const KUrl &to, time_t mtime, bool directory, bool renamed);
};

#endif // INFOARTISTMODEL_H
