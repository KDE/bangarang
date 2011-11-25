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

#include "mediaitemmodel.h"
class InfoFetcher;
class MediaIndexer;
namespace Utilities {
    class Thread;
}

/*
* This model is responsible for storing information contained in a MediaItem or list of MediaItems.
* It recognizes shared values across media items and provides InfoFetchers for updating MediaItem
* information.  It is useful for displaying and editing MediaItem information.  If a MediaItemModel
* is provided it can request changes to be saved to the source of the MediaItem information.
*/ 
class InfoItemModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        /**
          * Roles for storing and accessing data stored by this model.
          **/
        enum InfoItemRoles {FieldRole = Qt::UserRole,
                            FieldNameRole = Qt::UserRole + 1,
                            MultipleValuesRole = Qt::UserRole + 2,
                            OriginalValueRole = Qt::UserRole + 3,
                            ValueListRole = Qt::UserRole + 4,
                            ArtworkListRole = Qt::UserRole + 5,
                            DrillRole = Qt::UserRole + 6};

        /**
          * Type of fetch.
          **/
        enum FetchType {AutoFetch = 0,
                        Fetch = 1};

        /**
          * Constructor
          **/
        InfoItemModel(QObject * parent = 0);

        /**
          * Destructor
          **/
        ~InfoItemModel();

        /**
          * Load information into the model for the provided list of MediaItems
          **/
        void loadInfo(const QList<MediaItem> & mediaList);

        /**
          * Save changes made to the data contained in the model
          **/
        void saveChanges();

        /**
          * Cancel changes made to the data contained in the model
          **/
        void cancelChanges();

        /**
          * Returns the list of MediaItems for which information is stored in the model
          **/
        QList<MediaItem> mediaList();

        /**
          * Sets the MediaItemModel which generated the MediaItems provide to this model.
          * This is the model that is used to save changes made to the information for
          * provided MediaItems
          **/
        void setSourceModel(MediaItemModel *sourceModel);

        /**
          * Returns list of InfoFetchers provided by this model
          **/
        QList<InfoFetcher *> infoFetchers();

        /**
          * Returns list of InfoFetchers that are available for fetching info for the
          * currently load MediaItems.
          **/
        QList<InfoFetcher *> availableInfoFetchers();

        /**
          * Returns true if provided InfoFetcher can automatically fetch information for the currently
          * loaded MediaItems.
          **/
        bool autoFetchIsAvailable(InfoFetcher* infoFetcher);

        /**
          * Returns true if provided InfoFetcher can fetch information for the currently
          * loaded MediaItems.
          **/
        bool fetchIsAvailable(InfoFetcher* infoFetcher);

        /**
          * Autofetch information for the currently loaded MediaItems.  Changes
          * will automatically be saved.
          **/
        void autoFetch(InfoFetcher* infoFetcher, bool updateRequiredFields = true, bool updateArtwork = true);

        /**
          * Fetch information for the currently loaded MediaItems. Fetched information will
          * loaded into model as changes - changes are not automatically saved.
          **/
        void fetch(InfoFetcher* infoFetcher);

        /**
          * Return list of MediaItems containing matching fetched information.
          */
        QList<MediaItem> fetchedMatches();

        Qt::DropActions supportedDropActions() const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent);
        QStringList mimeTypes() const;
        QHash<QString, QVariant> fetchingStatus();

    public slots:
        /**
          * Selects the index of the MediaItem containing fetched information to load into the model
          */
        void selectFetchedMatch(int index);

        /**
          * Set the rating for all currently loaded MediaItems. Rating changes are automatically saved.
          **/

        void setRating(int rating);

        /**
         * Clears the current artwork in the artwork field
         */
        void clearArtwork();

        /**
         * Loads fields for currently selected items in order
         */
        void loadFieldsInOrder();


    Q_SIGNALS:
        /**
          * Emitted when information contained in the model might have been changed.
          **/
        void infoChanged(bool modified);

        /**
          * Emitted when information is being fetched for MediaItems contained in the model.
          **/
        void fetching();

        /**
          * Emitted when information fetching has completed. Note that this signal is
          * also emitted if fetching times out.
          **/
        void fetchComplete();

        void fetchingStatusUpdated();

    private:
        QList<MediaItem> m_mediaList;
        QList<MediaItem> m_originalList;
        MediaItemModel * m_sourceModel;
        QHash<QString, QStringList> m_fieldsOrder;
        QHash<QString, QString> m_fieldNames;
        QHash<QString, QStringList> m_restrictedFields;
        QHash<QString, QString> m_drillLris;
        bool m_defaultEditable;
        bool m_modified;
        bool m_isFetching;
        QList<InfoFetcher *> m_infoFetchers;
        FetchType m_fetchType;
        MediaIndexer * m_indexer;
        bool m_suppressFetchOnLoad;
        QList<MediaItem> m_itemsToFetch;
        QList<MediaItem> m_fetchedMatches;
        int m_selectedFetchedMatch;
        Utilities::Thread * m_utilThread;
        MediaItemModel * m_valueListLoader;
        QHash<QString, QString> m_valueListLris;
        QStringList m_loadedValueLists;
        QHash<QString, QStringList> m_valueLists;
        void fetchBatch(InfoFetcher *infoFetcher, int maxMatches, bool updateRequiredFields, bool updateArtwork);
        QHash<QString, QVariant> m_fetchingStatus;
        void addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool isEditable = false);
        bool hasMultipleValues(const QString &field);
        QVariant commonValue(const QString &field);
        QStringList valueList(const QString &field);
        bool isEmpty(const QString &field);
        void updateMediaList();
        void saveFileMetaData(QList<MediaItem> mediaList);
        void saveCustomGenreInfo(QList<MediaItem> mediaList);
        bool getArtwork(QStandardItem *fieldItem, QString artworkUrlOverride = QString());
        void setDrill(QStandardItem *item, const QString &field, const QVariant &value);
        QString categoryTypeForField(const QString &field, const QString &type);
        MediaItem createDrillItem(const QString &field, const QString &type, const QString &value);

    private Q_SLOTS:
        void itemChanged(QStandardItem *changedItem);
        void infoFetched(QList<MediaItem> fetchedMatches);
        void updateFetchedInfo(int index, MediaItem match);
        void infoFetcherComplete(InfoFetcher *infoFetcher);
        void noResults(InfoFetcher *infoFetcher);
        void gotArtworks(QList<QImage> artworks, MediaItem mediaItem);
        void cancelFetching();
        void loadNextValueList();
        void reloadValueLists();
};

#endif // INFOITEMDELEGATE_H
