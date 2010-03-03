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

/*
* This model is responsible for 
*
*/ 
class InfoItemModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        enum InfoItemRoles {FieldRole = Qt::UserRole,
                            MultipleValuesRole = Qt::UserRole + 1,
                            OriginalValueRole = Qt::UserRole +2,
                            ValueListRole = Qt::UserRole +3};
        InfoItemModel(QObject * parent = 0);
        ~InfoItemModel();
        void loadInfo(const QList<MediaItem> & mediaList);
        void saveChanges();
        void cancelChanges();
        QList<MediaItem> mediaList();
        void setSourceModel(MediaItemModel *sourceModel);
                   
    private:
        QList<MediaItem> m_mediaList;
        MediaItemModel * m_sourceModel;
        void addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool forceEditable = false);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        void saveFileMetaData(QList<MediaItem> mediaList);

    Q_SIGNALS:
        void infoChanged(bool modified);
        
    private Q_SLOTS:
        void checkInfoModified(QStandardItem *changedItem);
};

#endif // INFOITEMMODEL_H
