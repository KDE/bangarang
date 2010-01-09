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
  * and MediaItemModel.
  *
  * @author Andrew Lake
  */

#include <QStandardItemModel>
#include <QObject>
#include <QPixmap>
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

struct MediaItem {
    
    enum MediaItemRole { UrlRole = Qt::UserRole + 1, /** QStandardItem role containing MediaItem url.*/
    
    SubTitleRole = Qt::UserRole + 2, /** QStandardItem role containing MediaItem sub title.*/
    
    DurationRole = Qt::UserRole + 3, /** QStandardItem role containing MediaItem duration.*/
    
    RatingRole = Qt::UserRole + 4, /** QStandardItem role containing MediaItem rating.*/
    
    TypeRole = Qt::UserRole + 5, /** QStandardItem role containing MediaItem type.*/
    
    FilterRole = Qt::UserRole + 6, /** QStandardItem role containing MediaItem filter.*/
    
    PlaylistIndexRole = Qt::UserRole + 7,  /** QStandardItem role containing Playlist 
                                             *index of MediaItem.*/
                                             
    NowPlayingRole = Qt::UserRole + 8, 
    
    IsSavedListRole = Qt::UserRole + 9, /** QStandardItem role containing whether
                                         *or not the media list represented by the 
                                         *MediaItem is a saved list.*/
    
    ExistsRole = Qt::UserRole + 10 }; /** QStandardItem role containing whether or
                                        *the file the MediaItem.url refers to exists.*/
                                        
    QString url; /** Url of MediaItem. The may be a standard url representing a 
                   * location of a media resource or a List Resource Identifier (lri).*/
                   
    QIcon artwork; /** Icon containing artwork representing MediaItem.*/
    
    QString title; /** Title of the Media Item. */
    
    QString subTitle; /** Subtitle of the Media Item. */
    
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
                   * Note that this is only useful for file resources.*/
                   
    QHash <QString, QVariant> fields;  /** Collection of all key, value pairs containing
                                         * the metadata fields associated with this MediaItem.
                                         *  key - A string containing the field name.
                                         *  value - A variant conatining the value of the field.*/
    
    MediaItem() : nowPlaying(false), isSavedList(false), exists(true) {}
};

Q_DECLARE_METATYPE(MediaItem);


/** 
 * MediaListProperties contains the properties associated with a list of
 * MediaItems. 
 */

class MediaListProperties {

public:
    QString name; /** Name of media list */
    
    QString summary; /** Summary text describing the number of items in the media list */
    
    QString lri;  /** List Resource Identifier associated with media list.  This string
                    * is the essentially how the media list is retrieved. */
                    
    /** 
     * Returns the engine portion of the lri string.
     */
    QString engine() {
        if (lri.indexOf("://") != -1) {
            return lri.left(lri.indexOf("://") + 3);
        } else {
            return QString();
        }
    }

    /**
     * Returns the engine argument portion of the lri string. 
     */
    QString engineArg() {
        int endOfArg = (lri.indexOf("?") != -1) ? lri.indexOf("?") - 1: lri.size() - 1;
        if ((lri.indexOf("://") != -1) && (lri.indexOf("://") != lri.size() - 3)) {
            //return lri.mid(lri.indexOf("://") + 3, lri.size() - endOfArg + 1);
            return lri.mid(lri.indexOf("://") + 3, endOfArg - (lri.indexOf("://") + 2));
        } else {
            return QString();
        }
    }
    
    /**
     * Returns the engine filter portion of the lri string. 
     */
    QString engineFilter() {
        if ((lri.indexOf("://") != -1)  && (lri.indexOf("?") != -1) && (lri.indexOf("?") != lri.size() - 1)){
            return lri.right(lri.size() - (lri.indexOf("?") + 1));
        } else {
            return QString();
        }
    }
    
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
         * Loads list of MediaItems as specified by the MediaListProperties.lri
         *
         * Note: Loading is asynchronous. Use mediaListChanged() signal to detect
         *       when loading is complete.
         */
        void load();
        
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
         * Returns the DropActions supported by the model
         */
        Qt::DropActions supportedDropActions() const;
        
        /**
        * Update information associated with MediaItems in mediaList in the 
        * the source from which the model retrieved the MediaItems.
        *
        * @param mediaList list containing the MediaItems whose information
        *                  should be update in the source.
        *
        * Note: mediaList does not have to contain the same MediaItems contained
        *       in the model.  However, the current model MediaListProperties
        *       should refer to an lri whose ListEngine is capable of updating 
        *       information in the source.  As a general rule, if the lri
        *       can be used to retrieve the MediaItem, its ListEngine can
        *       update information for the MediaItem.
        */
        void updateSourceInfo(const QList<MediaItem> &mediaList);
        
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
         * Emmited when the source information for a MediaItem is updated.
         *
         * @param mediaItem MediaItem whose information was updated in
         *                  the source.
         */
        void sourceInfoUpdated(MediaItem mediaItem); 
        
        /**
         * Emitted when the source information for a MediaItem is updated
         *
         * @param percent percentage of MediaItems updated (from the 
         *                list of MediaItems being updated).
         */
        void sourceInfoUpdateProgress(int percent);
        
        /**
         * Emmited when the source information for MediaItem is removed.
         *
         * @param url url of MediaItem remove from the source
         */
        void sourceInfoRemoved(QString url);
        
        /**
        * Emitted when the source information for a MediaItem is removed
        *
        * @param percent percentage of MediaItems removed (of the 
        *                list of MediaItems being removed).
        */
        void sourceInfoRemovalProgress(int percent);
        
        /**
         * Emitted when the update or removal of the list MediaItems is complete.
         */
        void sourceInfoUpdateRemovalComplete();
        
        /**
        * Emitted when the update or removal of the list MediaItems has started.
        */
        void sourceInfoUpdateRemovalStarted();
        
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
        
        void addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature);
        
    private Q_SLOTS:
        void synchRemoveRows(const QModelIndex &index, int start, int end);
        void showLoadingMessage();
        
    private:
        void hideLoadingMessage();
        void showNoResultsMessage();
        QList<QStandardItem *> rowDataFromMediaItem(MediaItem mediaItem);
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
        QList<MediaItem> m_mediaList;
        bool m_emitChangedAfterDrop;
        int m_loadingProgress;
        bool m_loadingState;
        void setLoadingState(bool state);
        int m_cacheThreshold;
        MediaListCache * m_mediaListCache;
        bool m_forceRefreshFromSource;
        QHash<QString, QTime> m_lriStartTimes;
        QList<QString> m_lrisLoading; 
        bool m_loadSources;
        QList<MediaItem> m_mediaListForLoadSources;
        bool m_reload;
        bool m_lriIsLoadable;

};

#endif // MEDIAITEMMODEL_H
