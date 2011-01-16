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

#ifndef MEDIAITEMMODEL_H
#define MEDIAITEMMODEL_H

/** @file
  * This file contains the definition of the MediaItem, MediaListProperties 
  * and MediaItemModel and MediaSortFilterProxyModel.
  *
  * @author Andrew Lake
  */

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QList>

class MusicListEngine;
class FileListEngine;
class ListEngineFactory;
class MediaListCache;


/**
 * MediaItem is a simple object that can represent any playable
 * item of media (audio clip, music, video clip, movie, etc.). 
 * MediaItem can also refer to a collection of MediaItems (Artist,
 * Album, Genre, Movies, TV series, etc.) or an action or a 
 * message.
 * Multiple MediaItems, a media list, are generally handled with 
 * QList< MediaItem >.
 */

class MediaItem {
public:
    enum MediaItemRole { UrlRole = Qt::UserRole + 1, /** QStandardItem role containing MediaItem url.*/
    
    SubTitleRole = Qt::UserRole + 2, /** QStandardItem role containing MediaItem sub title.*/
    
    SemanticCommentRole = Qt::UserRole + 3, /** QStandardItem role containing MediaItem sub title.*/

    DurationRole = Qt::UserRole + 4, /** QStandardItem role containing MediaItem duration.*/
    
    RatingRole = Qt::UserRole + 5, /** QStandardItem role containing MediaItem rating.*/
    
    PlayCountRole = Qt::UserRole + 6, /** QStandardItem role containing MediaItem play count.*/
    
    LastPlayedRole = Qt::UserRole + 7, /** QStandardItem role containing MediaItem last played time.*/
    
    TypeRole = Qt::UserRole + 8, /** QStandardItem role containing MediaItem type.*/
    
    SubTypeRole = Qt::UserRole + 9, /** QStandardItem role containing MediaItem type.*/
    
    FilterRole = Qt::UserRole + 10, /** QStandardItem role containing MediaItem filter.*/
    
    PlaylistIndexRole = Qt::UserRole + 11,  /** QStandardItem role containing Playlist 
                                             *index of MediaItem.*/
                                             
    NowPlayingRole = Qt::UserRole + 12, 
    
    IsSavedListRole = Qt::UserRole + 13, /** QStandardItem role containing whether
                                         *or not the media list represented by the 
                                         *MediaItem is a saved list.*/
    
    ExistsRole = Qt::UserRole + 14,  /** QStandardItem role containing whether or
                                        *the file the MediaItem.url refers to exists.*/
    
    HasCustomArtworkRole = Qt::UserRole + 15}; /** QStandardItem role containing whether
                                                 * not artwork is custom(true) or default(false).*/

    QString url; /** Url of MediaItem. The may be a standard url representing a 
                   * location of a media resource or a List Resource Identifier (lri).*/
                   
    QIcon artwork; /** Icon containing artwork representing MediaItem.*/
    
    QString title; /** Title of the Media Item. */
    
    QString subTitle; /** Subtitle of the Media Item. */
    
    QString semanticComment; /** Comment containing info like play count, lastplayed, etc. */
    
    QString duration;  /** Displayed representation of duration of the Media Item. */
    
    QString type;  /** Type of the Media Item. 
                     * "Audio"    - MediaItem is a playable audio resource.
                     * "Video"    - MediaItem is a playable video resource.
                     * "Category" - MediaItem is a category which refers to other
                     *              MediaItems.  The url for this type of MediaItem
                     *              is an lri that a MediaItemModel can use to
                     *              load a list of MediaItems
                     * "Action"   - MediaItem is an action. A MediaItemModel can use this
                     *              to load a list of MediaItems by means other than
                     *              an just an lri. e.g. Open a dialog, etc.
                     * "Message"  - MediaItem is a message. A MediaItemModel can use this
                     *              to display messages in associated views.
                     *              e.g. "Loading...", "No results", etc.
                     * */
                     
    QString filter;
    
    int playlistIndex; /** Row of Playlist::playListModel() that contains this MediaItem.*/
    
    bool nowPlaying;
    
    bool isSavedList; /** If the MediaItem.url is an lri, this bool is true if the lri 
                        * refers to a saved media list, otherwise is false.*/
                      
    bool exists; /** If the MediaItem.url point to a playable media resource, this
                   * bool is true if the playable media resource exists, otherwise is false.
                   * On DVDs or CDs this value depends on whether the DVD/CD is inserted or not.*/
    
    bool hasCustomArtwork; /** If the artwork property is a default icon this bool is false,
                            * otherwise if artwork property has custom artwork this bool is
                            * true. */
                   
    QHash <QString, QVariant> fields;  /** Collection of all key, value pairs containing
                                         * the metadata fields associated with this MediaItem.
                                         *  key - A string containing the field name.
                                         *  value - A variant conatining the value of the field.*/
                                         
    void addContext(const QString title, const QString lri)
    {
        QStringList contextTitles = fields["contextTitles"].toStringList();
        QStringList contextLRIs = fields["contextLRIs"].toStringList();
        contextTitles.append(title);
        contextLRIs.append(lri);
        fields["contextTitles"] = contextTitles;
        fields["contextLRIs"] = contextLRIs;
    }

    void clearContexts()
    {
        fields["contextTitles"] = QStringList();
        fields["contextLRIs"] = QStringList();
    }

    const QString subType() const
    {
        if (type == "Audio") {
            return fields["audioType"].toString();
        } else if (type == "Video"){
            return fields["videoType"].toString();
        } else {
            return fields["categoryType"].toString();
        }
    }
    
    MediaItem() : nowPlaying(false), isSavedList(false), exists(true), hasCustomArtwork(false) {}
};

Q_DECLARE_METATYPE(MediaItem);


/** 
 * MediaListProperties contains the properties associated with a list of
 * MediaItems. 
 */

class MediaListProperties {

public:
    MediaListProperties(QString startingLri = QString()) 
    {
        lri = startingLri;
        filterOperators << "!=" << ">=" << "<=" << "=" << ">" << "<" << ".contains.";
    }
    
    ~MediaListProperties(){}
    
    QString name; /** Name of media list */
    
    QString summary; /** Summary text describing the number of items in the media list */
    
    QString lri;  /** List Resource Identifier associated with media list.  This string
                    * is the essentially how the media list is retrieved. */
    MediaItem category;  /** Category mediaItem associated with the media list. This is
                           * the category mediaItem that was categoryActivated. */
                    
    /** 
     * Returns the engine portion of the lri string.
     */
    QString engine() {
        int idxOfSlashes = lri.indexOf("://");
        if (idxOfSlashes != -1) {
            return lri.left(idxOfSlashes + 3);
        } else {
            return QString();
        }
    }

    /**
     * Returns the engine argument portion of the lri string. 
     */
    QString engineArg() {
        int endOfArg = (lri.indexOf("?") != -1) ? lri.indexOf("?") - 1: lri.size() - 1;
        int idxOfSlashes = lri.indexOf("://");
        if ((idxOfSlashes != -1) && (idxOfSlashes != lri.size() - 3)) {
            //return lri.mid(lri.indexOf("://") + 3, lri.size() - endOfArg + 1);
            return lri.mid(idxOfSlashes + 3, endOfArg - (lri.indexOf("://") + 2));
        } else {
            return QString();
        }
    }
    
    /**
     * Returns the engine filter portion of the lri string. 
     */
    QString engineFilter() {
        int idxOfArgSep = lri.indexOf("?");
        if ((lri.indexOf("://") != -1)  && (idxOfArgSep != -1) && (idxOfArgSep != lri.size() - 1)){
            return lri.right(lri.size() - (idxOfArgSep + 1));
        } else {
            return QString();
        }
    }
    
    /**
     * Returns a parsed list of filters from the enginefilter portion
     * of the lri string
     **/
    QStringList engineFilterList() {
        QStringList lriFilterList;
        QString filter = engineFilter();
        if (!filter.isNull()) {
            QStringList argList = filter.split("||");
            for (int i = 0; i < argList.count(); i++) {
                lriFilterList << argList.at(i);
            }
        }
        return lriFilterList;
    }
    
    /**
     * Returns field name of specified filter
     **/
    QString filterField(const QString &filter)
    {
        QString field;
        for (int j = 0; j < filterOperators.count(); j ++) {
            QString oper = filterOperators.at(j);
            if (filter.indexOf(oper) != -1) {
                field = filter.left(filter.indexOf(oper)).trimmed();
                break;
            }
        }
        return field;
    }
    
    /**
     * Returns operator of specified filter
     **/
    QString filterOperator(const QString &filter)
    {
        QString oper;
        for (int j = 0; j < filterOperators.count(); j ++) {
            oper = filterOperators.at(j);
            if (filter.indexOf(oper) != -1) {
                break;
            }
        }
        return oper;
    }
    
    /**
    * Returns value of specified filter
    **/
    QString filterValue(const QString &filter)
    {
        QString value;
        for (int j = 0; j < filterOperators.count(); j ++) {
            QString oper = filterOperators.at(j);
            if (filter.indexOf(oper) != -1) {
                value = filter.mid(filter.indexOf(oper) + oper.length()).trimmed();
                break;
            }
        }
        return value;
    }
    
    /**
     * Returns first value of specified field in filter
     */
    QString filterFieldValue(const QString &field)
    {
        QString value;
        QStringList filterList = engineFilterList();
        for (int i = 0; i < filterList.count(); i++) {
            if (filterField(filterList.at(i)) == field) {
                value = filterValue(filterList.at(i));
                break;
            }
        }
        return value;
    }
    
    /**
     * Returns first filter corresponding to specified field
     */
    QString filterForField(const QString &field)
    {
        QString filter;
        QStringList filterList = engineFilterList();
        for (int i = 0; i < filterList.count(); i++) {
            if (filterField(filterList.at(i)) == field) {
                filter = filterList.at(i);
                break;
            }
        }
        return filter;
    }
    
    QStringList filterOperators;
    
    QString type; /** The type of media list.
                   * "Categories" - a list of MediaItems that are categories.
                   * "Sources" - a list of MediaItems that are playable media 
                   *             resources (files, streams, etc.). */
    
};
Q_DECLARE_METATYPE(MediaListProperties);


class MediaList : QList<MediaItem>{};


/**
 * MediaItemModel contains information for a list of MediaItems 
 * repesented in the form of a model.
 * It provides facilities for loading a list of MediaItems from a 
 * variety of sources as well as adding, updating or removing 
 * information associated with any MediaItem it contains.
 * A subclass of QStandardItemModel, MediaItemModel may be used 
 * with a view to visually respresent the information it contains.
 */

class MediaItemModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        /**
         * Default constructor.
         */
        MediaItemModel(QObject * parent);
        
        /**
         * Default destructor
         */
        ~MediaItemModel();
        
        /**
         * Clears all information contained in the model.
         * This includes MediaListProperties which a call to removeRows()
         * will not clear.
         *
         * @param emitMediaListChanged set true to emit the mediaListChanged()
         *                             signal after clearing data. Default is
         *                             false.
         */
        void clearMediaListData(bool emitMediaListChanged = false);
        
        /**
         * Returns the threshold, in milliseconds, used to cache data loaded by
         * by the model.
         *
         */
        int cacheThreshold();
        
        /**
         * Handler for data that is dropped onto a view associated with the
         * mdel.
         */
        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
        
        /**
         * Flags for items dragged from a view associated with the model.
         */
        Qt::ItemFlags flags(const QModelIndex &index) const;
        
        /**
         * Insert MediaItem at the specified row in the model with the one
         * provided.
         *
         * @param row row of model
         * @param mediaItem MediaItem to replace with
         * @param emitMediaListChanged emits mediaListChanged() signal if true,
         *                             otherwise don't emit mediaListChanged().
         *
         */
        void insertMediaItemAt(int row, const MediaItem &mediaItem, bool emitMediaListChanged = false);

        /**
         * Loads list of MediaItems as specified by the MediaListProperties.lri
         *
         * Note: Loading is asynchronous. Use mediaListChanged() signal to detect
         *       when loading is complete.
         */
        void load();

        /**
         * Returns loading state.
         *
         */
        bool isLoading();
        
        /**
        * Loads list of MediaItems as specified by the provided lri
        *
        * Note: Loading is asynchronous. Use mediaListChanged() signal to detect
        *       when loading is complete. This method will clear existing data.
        */
        void loadLRI(const QString &lri);
        
        /**
         * Loads a list of MediaItems directly into the model
         * 
         * @param mediaList list of MediaItems to load
         *
         * Note: After using this method, MediaListProperties returned by
         *       mediaListProperties() is likely stale.  
         *       MediaListProperties.lri should either be updated to correspond 
         *       to mediaList or set to QString().
         */
        void loadMediaList(const QList<MediaItem> &mediaList, bool emitMediaListChanged = false, bool updateExisting = false);
        
        /**
         * Loads a MediaItem directly into the model
         *
         * @param mediaItem MediaItem to load
         *
         * Note: After using this method, MediaListProperties returned by
         *       mediaListProperties() is likely stale.  
         *       MediaListProperties.lri should either be updated to correspond 
         *       to mediaList or set to QString().
         */
        void loadMediaItem(const MediaItem &mediaItem, bool emitMediaListChanged = false);
        
        /**
         * Loads playable MediaItems into the model.
         * 
         * @param mediaList mediaList containing MediaItems to load.
         *                  If mediaList contains "Category" type MediaItems
         *                  then the playable MediaItems associated with that
         *                  category (as specified by the lri in MediaItem.url)
         *                  will be loaded.
         *                  If the mediaList contains playable MediaItems
         *                  (type = "Audio" or "Video") then they will be directly 
         *                  loaded into the model.
         */
        void loadSources(const QList<MediaItem> &mediaList);
        
        /**
         * Returns true if lri is loadable
         **/
        bool lriIsLoadable();
        
        /**
         * Returns the MediaItem associated with the specified row in the model.
         */
        MediaItem mediaItemAt(int row);
        
        /**
         * Returns the list of MediaItems contained in the model.
         */
        QList<MediaItem> mediaList();
        
        /** 
         * Returns a pointer to the MediaListCache used by the model.
         * This is useful for sharing a common cache between different 
         * models.
         */
        MediaListCache * mediaListCache();
        
        /**
         * Returns the MediaListProperties associated with the media list
         * contained in the model
         */
        MediaListProperties mediaListProperties();
        
        /**
         * Returns the QMimeData associated with a list of model indexes
         */
        QMimeData *mimeData(const QModelIndexList &indexes) const; 
        
        /**
         * Returns a list of mimetypes associated with items in the model
         */
        QStringList mimeTypes() const;
        
        /**
         * Remove MediaItem at the specified row of the model
         * 
         * @param row row of model
         * @param emitMediaListChanged emits mediaListChanged() signal if true,
         *                             otherwise don't emit mediaListChanged().
         */
        void removeMediaItemAt(int row, bool emitMediaListChanged = false);
        
        /**
         * Remove information associated with MediaItems in mediaList from 
         * the source from which the model retrieved the MediaItems.
         *
         * @param mediaList list containing the MediaItems whose information
         *                  should be removed from the source.
         *
         * Note: mediaList does not have to contain the same MediaItems contained
         *       in the model.  However, the current model MediaListProperties
         *       should refer to an lri whose ListEngine is capable of removing 
         *       information from the source.  As a general rule, if the lri
         *       can be used to retrieve the MediaItem, its ListEngine can
         *       remove information for the MediaItem.
         */
        void removeSourceInfo(const QList<MediaItem> &mediaList);
        
        /**
         * Replace MediaItem at the specified row in the model with the one
         * provided.
         *
         * @param row row of model
         * @param mediaItem MediaItem to replace with
         * @param emitMediaListChanged emits mediaListChanged() signal if true,
         *                             otherwise don't emit mediaListChanged().
         *
         */
        void replaceMediaItemAt(int row, const MediaItem &mediaItem, bool emitMediaListChanged = false);
        
        /**
         * Return the row of a MediaItem whose url matches the one provided
         *
         * @param url url to match
         *
         * @return row of model
         */
        int rowOfUrl(const QString &url);
        
        /**
        * Return the row of a MediaItem whose resource uri matches the one provided
        *
        * @param resourceUri uri to match
        *
        * @return row of model
        */
        int rowOfResourceUri(const QString &resourceUri);
        
        /**
         * Sets the threshold for the cache.
         * 
         * @param msec cache threshold in milliseconds.
         *             If any load the model performs takes longer
         *             than this threshold it will be stored in the
         *             cache.
         */         
        void setCacheThreshold(int msec);
        
        /**
         * Sets the MediaListProperties for the model
         *
         * @param mediaListProperties MediaListProperties to be used by the
         *                            model.  A subsequent load() call will 
         *                            use the MediaListProperties.lri.
         */
        void setMediaListProperties(const MediaListProperties &mediaListProperties);
        
        /**
         * Sets the cache for the model to use.
         * This is useful for sharing a common cache between different 
         * models.
         *
         * @param mediaListCache pointer to MediaListCache for the 
         *                       model to use.
         */
        void setMediaListCache(MediaListCache * mediaListCache);
        
        /**
         * Sets the ListEngineFactory for the model to use.
         * This is mostly useful if using a custom ListEngineFactory.
         *
         * @param listEngineFactory ListEngineFactory for the model to use
         */
        void setListEngineFactory(ListEngineFactory * listEngineFactory);
        
        /**
         * Sets suppression of no results message
         * @param suppress true to suppress message, false otherwise
         */
        void setSuppressNoResultsMessage(bool suppress);

        /**
         * Returns model status information.  This contains for underlying model
         * status info like indexing status.
         */
        QHash<QString, QVariant> status();
        
        /**
         * Returns the DropActions supported by the model
         */
        Qt::DropActions supportedDropActions() const;
        
        /**
        * Update information associated with MediaItems in mediaList in the 
        * the source from which the model retrieved the MediaItems.
        *
        * @param mediaList list containing the MediaItems whose information
        *                  should be update in the source.
        * @param nepomukOnly only update the nepomuk store.
        *
        * Note: mediaList does not have to contain the same MediaItems contained
        *       in the model.  However, the current model MediaListProperties
        *       should refer to an lri whose ListEngine is capable of updating 
        *       information in the source.  As a general rule, if the lri
        *       can be used to retrieve the MediaItem, its ListEngine can
        *       update information for the MediaItem.
        */
        void updateSourceInfo(const QList<MediaItem> &mediaList, bool nepomukOnly = false);
        
        QString dataEngine();
        QString filter();
                
    Q_SIGNALS:
        /**
         * Emitted when the model's MediaListProperties have changed.
         */
        void propertiesChanged(); 
        
        /**
         * Emitted when the list of MediaItems in the model have changed.
         * This signal may be suppressed when using certain methods.
         */
        void mediaListChanged();
        
        /**
         * Emitted when the model is loading a list of MediaItems.
         */
        void loading();

        /**
         * Emitted when the loading state changes
         */
        void loadingStateChanged(bool loadingState);
        
        /**
         * Emitted when status is updated
         */
        void statusUpdated();
        
        /**
         * Emitted when MediaListProperties for this model has changed.
         * Emitted only when changed independent of media list load.
         */
        void mediaListPropertiesChanged();
        
    public Q_SLOTS:
        /**
        * Activate the action associated with "Action" mediaItem
        * at the specified model index.
        *
        * @param index QModelIndex of action. 
        *              (This slot is useful for View ItemDelegates to tell the
        *              model to activate the action when clicked. e.g. Open File.)
        */
        void actionActivated(QModelIndex index);
        
        /**
         * Loads the list of MediaItems associated with "Category" mediaItem
         * at the specified model index.
         *
         * @param index QModelIndex of category.  The url of the "Category" type
         *              MediaItem associated with the row of this index contains
         *              the lri representing the list of MediaItems to load.
         *              (This slot is useful for View ItemDelegates to tell the
         *              model to load the category when clicked.)
         */
        void categoryActivated(QModelIndex index);
        
        /**
         * Reloads the information in the current model.
         */
        void reload();
        
        /**
        * Update MediaItem contained in the model to the MediaItem specified
        * Only MediaItems in the model whose url matches the url of the
        * specifed MediaItem will be updated.
        * 
        * @param mediaItem MediaItem with updated information.
        *
        * Note: This method only updates the information in the model. It
        *       does not update the source from which the MediaItems were
        *       retrieved. Use updateSourceInfo() method to do that.
        */
        void updateMediaItem(MediaItem mediaItem);
        
        /**
         * Update MediaItems contained in the model to those in the specified
         * list of MediaItems. Only MediaItems in the model whose url matches
         * those in the list of MediaItems will be updated.
         * 
         * @param mediaList List of MediaItems with updated information.
         *
         * Note: This method only updates the information in the model. It
         *       does not update the source from which the MediaItems were
         *       retrieved. Use updateSourceInfo() method to do that.
         */
        void updateMediaItems(QList<MediaItem> mediaList);
        
        /**
        * Checks if the model contains playble items (media items or feeds)
        */
        bool containsPlayable();
        
        /**
         * Remove MediaItem in model that matches the url specified.
         *
         * @param url url of MediaItem to remove.
         *
         * Note: This method only removes the information in the model. It
         *       does not remove information in the source from which the 
         *       MediaItems were retrieved. Use removeSourceInfo() method 
         *       to do that.
         */
        void removeMediaItem(QString url);
        
        /**
        * Remove MediaItem in model that matches the resource uri specified.
        *
        * @param resourceUri uri of MediaItem to remove.
        *
        * Note: This method only removes the information in the model. It
        *       does not remove information in the source from which the 
        *       MediaItems were retrieved. Use removeSourceInfo() method 
        *       to do that.
        */
        void removeMediaItemByResource(QString resourceUri);
        
        /**
        * Update artwork for mediaItem corresponding to the specified MediaItem.
        * Only MediaItems in the model whose url matches the url of the
        * specifed MediaItem will be updated.
        * 
        * @param artworkImage Image of updated artwork.
        * @param mediaItem MediaItem to update the artwork for.
        *
        * Note: This method only updates the information in the model. It
        *       does not update the source from which the MediaItems were
        *       retrieved. Use updateSourceInfo() method to do that.
        */
        void updateArtwork(QImage artworkImage, MediaItem mediaItem);
        
        void updateRefresh();
        
        void updateMediaListPropertiesCategoryArtwork(QImage artworkImage, MediaItem mediaItem);
        
        void addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature);

        /**
         * Update model status
         */
        void updateStatus(QHash<QString, QVariant> updatedStatus);

        /**
         * Suppress tooltips
         */
        void setSuppressTooltip(bool suppress = true);
        
    private Q_SLOTS:
        void synchRemoveRows(const QModelIndex &index, int start, int end);
        void showLoadingMessage();
        
    private:
        void hideLoadingMessage();
        void showNoResultsMessage();
        QList<QStandardItem *> rowDataFromMediaItem(MediaItem mediaItem);
        void loadSourcesForNextCat();
        QObject * m_parent;
        QString m_dataEngine;
        QString m_filter;
        MediaListProperties m_mediaListProperties;
        ListEngineFactory * m_listEngineFactory;
        QString m_requestSignature;
        QStringList m_subRequestSignatures;
        QList< QList<MediaItem> > m_subRequestMediaLists;
        int m_subRequestsDone;
        QStringList m_urlList;
        QStringList m_resourceUriList;
        QList<MediaItem> m_mediaList;
        bool m_emitChangedAfterDrop;
        int m_loadingProgress;
        bool m_loadingState;
        void setLoadingState(bool state);
        int m_cacheThreshold;
        MediaListCache * m_mediaListCache;
        bool m_forceRefreshFromSource;
        QHash<QString, QTime> m_lriStartTimes;
        bool m_loadSources;
        QList<MediaItem> m_mediaListForLoadSources;
        QList<MediaItem> m_remainingCatsForLoadSources;
        bool m_reload;
        bool m_lriIsLoadable;
        bool m_suppressNoResultsMessage;
        bool m_pendingUpdateRefresh;
        bool m_suppressTooltip;
        QHash<QString, QVariant> m_status;

};

class MediaSortFilterProxyModel : public QSortFilterProxyModel
{
    public:
        MediaSortFilterProxyModel(QObject* parent = 0);

        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // MEDIAITEMMODEL_H
