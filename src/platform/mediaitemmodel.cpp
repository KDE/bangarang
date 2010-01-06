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

#include "mediaitemmodel.h"
#include "listengine.h"
#include "listenginefactory.h"
#include "medialistcache.h"
#include "utilities.h"

#include <QFontMetrics>
#include <QDateTime>
#include <KIcon>
#include <KDebug>
#include <KLocale>

MediaItemModel::MediaItemModel(QObject * parent) : QStandardItemModel(parent) 
{
    m_parent = parent;
    m_mediaListProperties.lri = QString();
    m_filter = QString();
    m_listEngineFactory = new ListEngineFactory(this);
    m_emitChangedAfterDrop = false;
    m_loadingState = false;
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(synchRemoveRows(const QModelIndex &, int, int)));
    m_mediaListCache = new MediaListCache(parent);
    m_cacheThreshold = 3000; //default to 3 second loading threshold for adding to cache
    m_forceRefreshFromSource = false;
    m_loadSources = false;
    m_reload = false;
}

MediaItemModel::~MediaItemModel() 
{
    delete m_listEngineFactory;
}

/*QVariant MediaItemModel::headerData (int section, Qt::Orientation orientation, int role) const 
{
    if (section == 0) {
        if (role == Qt::DisplayRole) {
            return m_mediaListProperties.name;
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        } else {
            return QVariant();
        }
    } else if (section == 1) {
        if (role == Qt::DecorationRole) {
            return KIcon("system-run");
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignRight;
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
    Q_UNUSED(orientation);
}*/

QString MediaItemModel::dataEngine()
{
    return m_mediaListProperties.engine();
}

QString MediaItemModel::filter()
{
    return m_mediaListProperties.engineFilter();
}

MediaListProperties MediaItemModel::mediaListProperties()
{
    return m_mediaListProperties;
}

void MediaItemModel::setMediaListProperties(const MediaListProperties &mediaListProperties)
{
    if (m_mediaListProperties.lri != mediaListProperties.lri) {
        setLoadingState(false);
    }
    m_mediaListProperties = mediaListProperties;
    emit propertiesChanged();
}

QList<MediaItem> MediaItemModel::mediaList()
{
    return m_mediaList;
}

MediaItem MediaItemModel::mediaItemAt(int row)
{
    return m_mediaList.at(row);
}

int MediaItemModel::rowOfUrl(const QString &url)
{
    return m_urlList.indexOf(url);
}

void MediaItemModel::load()
{
    if (!m_mediaListProperties.lri.isEmpty()) {
        if (m_mediaListCache->isInCache(m_mediaListProperties.lri) && !m_forceRefreshFromSource) {
            setLoadingState(true);
            // Load data from from the cache
            ListEngine * listEngine = m_listEngineFactory->availableListEngine("cache://");
            MediaListProperties cacheListProperties;
            cacheListProperties.lri = QString("cache://dummyarg?%1").arg(m_mediaListProperties.lri);
            m_requestSignature = m_listEngineFactory->generateRequestSignature();
            listEngine->setRequestSignature(m_requestSignature);
            listEngine->setMediaListProperties(cacheListProperties);
            listEngine->start();
            kDebug() << "loading from cache for " << m_mediaListProperties.lri;
        } else {    
            if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
                setLoadingState(true);
                if (m_lrisLoading.indexOf(m_mediaListProperties.lri) == -1) {
                    // Since this lri is not currently being loaded by any list engine
                    // go ahead and start a new load
                    ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
                    m_requestSignature = m_listEngineFactory->generateRequestSignature();
                    listEngine->setRequestSignature(m_requestSignature);
                    listEngine->setMediaListProperties(m_mediaListProperties);
                    m_lriStartTimes.insert(m_mediaListProperties.lri, QTime::currentTime());
                    m_lrisLoading.append(m_mediaListProperties.lri);
                    listEngine->start();
                    kDebug() << "started new load for " << m_mediaListProperties.lri;
                } else {
                    kDebug() << "waiting for " << m_mediaListProperties.lri;
                }
            } else {
                showNoResultsMessage();
            }
            m_forceRefreshFromSource = false;
        }
    }
    m_loadSources = false;
}

void MediaItemModel::reload()
{
    m_reload = true;
    if (m_loadSources) {
        // If the model was populated using loadSource then reload 
        QList<MediaItem> mediaList = m_mediaListForLoadSources;
        clearMediaListData();
        loadSources(mediaList);
    } else if (!m_mediaListProperties.lri.isEmpty()) {
        clearMediaListData();
        m_forceRefreshFromSource = true;
        load();
    }
}

void MediaItemModel::loadMediaList(const QList<MediaItem> &mediaList, bool emitMediaListChanged, bool updateExisting)
{
    for (int i = 0 ; i < mediaList.count() ; ++i) {
        if (updateExisting) {
            int rowOfExisting = rowOfUrl(mediaList.at(i).url);
            if (rowOfExisting != -1) {
                replaceMediaItemAt(rowOfExisting, mediaList.at(i));
            } else {
                loadMediaItem(mediaList.at(i));
            }
        } else {
            loadMediaItem(mediaList.at(i));
        }
    }
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::loadMediaItem(const MediaItem &mediaItem, bool emitMediaListChanged)
{
    m_mediaList << mediaItem;
    m_urlList << mediaItem.url;
    appendRow(rowDataFromMediaItem(mediaItem));
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}   

void MediaItemModel::categoryActivated(QModelIndex index)
{
    MediaListProperties mediaListProperties;
    mediaListProperties.lri =  itemFromIndex(index)->data(MediaItem::UrlRole).toString();
    mediaListProperties.name =  m_mediaList.at(index.row()).title;
    m_mediaListProperties = mediaListProperties;
    
    if (m_mediaListCache->isInCache(m_mediaListProperties.lri)) {
        removeRows(0, rowCount());
        setLoadingState(true);
        // Load data from from the cache
        ListEngine * listEngine = m_listEngineFactory->availableListEngine("cache://");
        MediaListProperties cacheListProperties;
        cacheListProperties.lri = QString("cache://dummyarg?%1").arg(m_mediaListProperties.lri);
        m_requestSignature = m_listEngineFactory->generateRequestSignature();
        listEngine->setRequestSignature(m_requestSignature);
        listEngine->setMediaListProperties(cacheListProperties);
        listEngine->start();
    } else {    
        if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
            removeRows(0, rowCount());
            setLoadingState(true);
            if (m_lrisLoading.indexOf(m_mediaListProperties.lri) == -1) {
                // Since this lri is not currently being loaded by any list engine
                // go ahead and start a new load
                ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
                m_requestSignature = m_listEngineFactory->generateRequestSignature();
                listEngine->setRequestSignature(m_requestSignature);
                listEngine->setMediaListProperties(m_mediaListProperties);
                m_lriStartTimes.insert(m_mediaListProperties.lri, QTime::currentTime());
                m_lrisLoading.append(m_mediaListProperties.lri);
                listEngine->start();
                kDebug()<< "started load for " << m_mediaListProperties.lri;
            }
        }
    }
}

void MediaItemModel::actionActivated(QModelIndex index)
{
    MediaListProperties mediaListProperties;
    mediaListProperties.lri =  itemFromIndex(index)->data(MediaItem::UrlRole).toString();

    if (m_listEngineFactory->engineExists(mediaListProperties.engine())) {
        m_mediaListProperties = mediaListProperties;
        removeRows(0, rowCount());
        setLoadingState(true);
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
        m_requestSignature = m_listEngineFactory->generateRequestSignature();
        listEngine->setRequestSignature(m_requestSignature);
        listEngine->setMediaListProperties(m_mediaListProperties);
        listEngine->activateAction();
    }   
}

void MediaItemModel::loadSources(const QList<MediaItem> &mediaList)
{

    if (mediaList.count() == 0) {
        return;
    }
    
    setLoadingState(true);
    
    //Load data only for media sources
    m_subRequestMediaLists.clear();
    m_subRequestSignatures.clear();
    m_subRequestsDone = 0;
    bool onlySources = true;
    QList<MediaItem> categories;
    m_requestSignature = m_listEngineFactory->generateRequestSignature();
    for (int i = 0; i < mediaList.count(); ++i) {
        if ((mediaList.at(i).type == "Audio") || (mediaList.at(i).type == "Video") || (mediaList.at(i).type == "Image")){
            setLoadingState(false);
            if (!mediaList.at(i).url.isEmpty()) { //url of sources can't be empty
                loadMediaItem(mediaList.at(i));
            }
        } else if (mediaList.at(i).type == "Category") {
            onlySources = false;
            categories.append(mediaList.at(i));
            if (mediaList.count() == 1) {
                MediaListProperties mediaListProperties;
                mediaListProperties.lri =  mediaList.at(i).url;
                mediaListProperties.name = mediaList.at(i).title;
                
                // - Get the lri for loading sources using this category item
                ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
                listEngine->setMediaListProperties(m_mediaListProperties);
                listEngine->setFilterForSources(m_mediaListProperties.engineFilter());
                QString loadSourcesLri = listEngine->mediaListProperties().lri;
                mediaListProperties.lri = loadSourcesLri;
                
                //Just directly load the sources since it's one category
                m_mediaListProperties = mediaListProperties;
            } else {
                //Generate signatures and media list holders for each subrequest
                //This must be complete for all categories before launching subrequests
                //to ensure full order of subrequests are available when results are returned
                MediaListProperties mediaListProperties;
                mediaListProperties.lri = mediaList.at(i).url;
                mediaListProperties.name = mediaList.at(i).title;
                if (m_listEngineFactory->engineExists(mediaListProperties.engine())) {
                    QString subRequestSignature = m_listEngineFactory->generateRequestSignature();
                    m_subRequestSignatures.append(subRequestSignature);
                    QList<MediaItem> emptyList;
                    m_subRequestMediaLists.append(emptyList);
                }
            }
        } else {
            setLoadingState(false);
        }
    }
    if (onlySources) {
        if (rowCount() == 0) {
            showNoResultsMessage();
        }
        emit mediaListChanged();
    } else {
        //Launch load requests
        for (int i = 0; i < categories.count(); ++i) {
            MediaListProperties mediaListProperties;
            mediaListProperties.lri = categories.at(i).url;
            mediaListProperties.name = categories.at(i).title;
            if (m_listEngineFactory->engineExists(mediaListProperties.engine())) {
                
                ListEngine * listEngine = m_listEngineFactory->availableListEngine(mediaListProperties.engine());
                listEngine->setMediaListProperties(mediaListProperties);
                listEngine->setFilterForSources(mediaListProperties.engineFilter());
                QString loadSourcesLri = listEngine->mediaListProperties().lri;
                
                if (m_mediaListCache->isInCache(loadSourcesLri)) {
                    // Load data from from the cache
                    listEngine = m_listEngineFactory->availableListEngine("cache://");
                    MediaListProperties cacheListProperties;
                    cacheListProperties.lri = QString("cache://dummyarg?%1").arg(loadSourcesLri);
                    m_requestSignature = m_listEngineFactory->generateRequestSignature();
                    listEngine->setRequestSignature(m_requestSignature);
                    if (mediaList.count() > 1) {
                        listEngine->setSubRequestSignature(m_subRequestSignatures.at(i));
                    }
                    listEngine->setMediaListProperties(cacheListProperties);
                    listEngine->start();
                    
                } else {
                    if (m_lrisLoading.indexOf(loadSourcesLri) == -1) {
                        // Since this lri is not currently being loaded by any list engine
                        // go ahead and start a new load
                        listEngine->setRequestSignature(m_requestSignature);
                        if (mediaList.count() > 1) {
                            listEngine->setSubRequestSignature(m_subRequestSignatures.at(i));
                        }
                        m_lriStartTimes.insert(loadSourcesLri, QTime::currentTime());
                        m_lrisLoading.append(loadSourcesLri);
                        listEngine->start();
                        kDebug()<< "started load for " << loadSourcesLri;
                    } else {
                        kDebug()<< "waiting for " << mediaListProperties.lri;
                    }
                }
            }
        }
    }
    
    m_loadSources = true;
    m_mediaListForLoadSources = mediaList;
}

void MediaItemModel::addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature)
{
    //Remove lri from loading list
    int lriIndex = m_lrisLoading.indexOf(mediaListProperties.lri);
    if (lriIndex != -1) {
        m_lrisLoading.removeAt(lriIndex);
    }
    
    //Check request signature of results and ignore results with a different signature
//    if (requestSignature == m_requestSignature) {
   kDebug() << "results returned for " << mediaListProperties.lri;
   if ((mediaListProperties.lri == m_mediaListProperties.lri) || (requestSignature == m_requestSignature)) {
        if (m_subRequestSignatures.count() == 0) {
            setLoadingState(false);
            loadMediaList(mediaList, false, true);
            m_mediaListProperties = mediaListProperties;
            if (done) {
                if (rowCount() == 0) {
                    showNoResultsMessage();
                }
                m_reload = false;
                m_lriIsLoadable = true;
                emit mediaListChanged();
            }
        } else {
            //Place subrequest results in the correct order
            int indexOfSubRequest = m_subRequestSignatures.indexOf(subRequestSignature);
            if (indexOfSubRequest != -1) {
                QList<MediaItem> srMediaList = m_subRequestMediaLists.at(indexOfSubRequest);
                srMediaList.append(mediaList);
                m_subRequestMediaLists.replace(indexOfSubRequest, srMediaList);
                if (done) {
                    m_subRequestsDone = m_subRequestsDone + 1;
                    if (m_subRequestsDone == m_subRequestSignatures.count()) {
                        setLoadingState(false);
                        //All the subrequests results are in, go ahead and load results in correct order
                        int count = 0;
                        for (int i = 0; i < m_subRequestMediaLists.count(); ++i) {
                            loadMediaList(m_subRequestMediaLists.at(i), false, true);
                            count += m_subRequestMediaLists.at(i).count();
                        }
                        if (rowCount() == 0) {
                            showNoResultsMessage();
                        }
                        //Need a basic lri so updateInfo and removeInfo can be performed by a list engine
                        m_mediaListProperties.lri = mediaListProperties.engine(); 
                        if (!m_reload) {
                            m_mediaListProperties.name = i18n("Multiple %1", m_mediaListProperties.name);
                        }
                        m_mediaListProperties.summary = i18np("1 item", "%1 items", count);
                        m_subRequestMediaLists.clear();
                        m_subRequestSignatures.clear();
                        m_subRequestsDone = 0;
                        m_reload = false;
                        m_lriIsLoadable = false;
                        emit mediaListChanged();
                    }
                }
            }
        }
    }
    
    //Cache results if they took long enough to return
    if (m_lriStartTimes.contains(mediaListProperties.lri)) {
        int elapsedMSecs = m_lriStartTimes.value(mediaListProperties.lri).msecsTo(QTime::currentTime());
        if (elapsedMSecs > m_cacheThreshold) {
            m_mediaListCache->addMediaList(mediaListProperties, mediaList);
            m_lriStartTimes.remove(mediaListProperties.lri);
        }
    }
}

void MediaItemModel::updateMediaItems(QList<MediaItem> mediaList)
{
    for (int i = 0; i < mediaList.count(); i++) {
        MediaItem mediaItem = mediaList.at(i);
        updateMediaItem(mediaItem);
    }
}

void MediaItemModel::updateMediaItem(MediaItem mediaItem)
{
    int row = rowOfUrl(mediaItem.url);
    if (row != -1) {
        replaceMediaItemAt(row, mediaItem);
    }
}

void MediaItemModel::removeMediaItem(QString url)
{
    int row = rowOfUrl(url);
    if (row != -1) {
        removeMediaItemAt(row, true);
        if (rowCount() == 0) {
            showNoResultsMessage();
        }
    }
}

void MediaItemModel::clearMediaListData(bool emitMediaListChanged)
{
    disconnect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(synchRemoveRows(const QModelIndex &, int, int)));
    removeRows(0, rowCount());
    m_mediaList.clear();
    m_urlList.clear();
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(synchRemoveRows(const QModelIndex &, int, int)));
    m_loadSources = false;
    m_mediaListForLoadSources.clear();
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::removeMediaItemAt(int row, bool emitMediaListChanged)
{
    if (row < rowCount()) {
        disconnect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(synchRemoveRows(const QModelIndex &, int, int)));
        removeRows(row, 1);
        m_urlList.removeAt(row);
        m_mediaList.removeAt(row);
        connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(synchRemoveRows(const QModelIndex &, int, int)));
        
    }
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::replaceMediaItemAt(int row, const MediaItem &mediaItem, bool emitMediaListChanged)
{
    m_mediaList.replace(row, mediaItem);
    m_urlList.replace(row, mediaItem.url);
    QList<QStandardItem *> rowData = rowDataFromMediaItem(mediaItem);
    for (int i = 0; i < rowData.count(); i++) {
        setItem(row, i, rowData.at(i));
    }
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::synchRemoveRows(const QModelIndex &index, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        m_mediaList.removeAt(start);
        m_urlList.removeAt(start);
    }
    if (m_emitChangedAfterDrop) {
        emit mediaListChanged();
        m_emitChangedAfterDrop = false;
    }
    Q_UNUSED(index);
}

void MediaItemModel::setLoadingState(bool state)
{
    m_loadingState = state;
    if (m_loadingState) {
        showLoadingMessage();
        emit loading();
    } else {
        hideLoadingMessage();
    }
}

void MediaItemModel::showLoadingMessage()
{
    if (m_loadingState) {
        MediaItem loadingMessage;
        m_loadingProgress += 1;
        if ((m_loadingProgress > 7) || (m_loadingProgress < 0)) {
            m_loadingProgress = 0;
        }
        QString iconName = QString("bangarang-loading-%1").arg(m_loadingProgress);
        loadingMessage.artwork = KIcon(iconName);
        loadingMessage.title = i18n("Loading...");
        loadingMessage.type = "Message";
        loadingMessage.fields["messageType"] = "Loading";
        if (rowCount() == 0) {
            loadMediaItem(loadingMessage, false);
        } else {
            replaceMediaItemAt(0, loadingMessage, false);
        }
        QTimer::singleShot(100, this, SLOT(showLoadingMessage()));
    }
}

void MediaItemModel::hideLoadingMessage()
{
    int row = -1;
    for (int i = 0; i < m_mediaList.count(); ++i) {
        if ((m_mediaList.at(i).fields["messageType"].toString() == "Loading") && (m_mediaList.at(i).type == "Message")) {
            row = i;
            break;
        }
    }
    if (row != -1) {
        if (m_mediaList.count() > 0) {
            removeMediaItemAt(row, false);
        }
    }
}

void MediaItemModel::showNoResultsMessage()
{
    MediaItem loadingMessage;
    loadingMessage.title = i18n("No results");
    loadingMessage.type = "Message";
    loadingMessage.fields["messageType"] = "No Results";
    loadMediaItem(loadingMessage, false);
}

QList<QStandardItem *> MediaItemModel::rowDataFromMediaItem(MediaItem mediaItem)
{
    QList<QStandardItem *> rowData;
    QStandardItem * titleItem = new QStandardItem(mediaItem.artwork, mediaItem.title);
    titleItem->setData(mediaItem.subTitle, MediaItem::SubTitleRole);
    titleItem->setData(mediaItem.url, MediaItem::UrlRole);
    titleItem->setData(mediaItem.type, MediaItem::TypeRole);
    titleItem->setData(mediaItem.duration, MediaItem::DurationRole);
    titleItem->setData(mediaItem.playlistIndex, MediaItem::PlaylistIndexRole);
    titleItem->setData(mediaItem.nowPlaying, MediaItem::NowPlayingRole);
    titleItem->setData(mediaItem.isSavedList, MediaItem::IsSavedListRole);
    titleItem->setData(mediaItem.exists, MediaItem::ExistsRole);
    if (!mediaItem.fields["description"].toString().isEmpty()) {
        QString tooltip = QString("<b>%1</b><br>%2")
                        .arg(mediaItem.title)
                        .arg(mediaItem.fields["description"].toString());
        titleItem->setData(tooltip, Qt::ToolTipRole);
    }
    titleItem->setData(mediaItem.fields["rating"].toInt(), MediaItem::RatingRole);
    rowData << titleItem;
    
    if ((mediaItem.type == "Audio") || (mediaItem.type == "Video") || (mediaItem.type == "Images")) {
       QStandardItem * markPlaylistItem = new QStandardItem(KIcon(), QString());
       markPlaylistItem->setData(mediaItem.url, MediaItem::UrlRole);
       markPlaylistItem->setData(mediaItem.type, MediaItem::TypeRole);   
       markPlaylistItem->setData(i18n("Add to playlist/Remove from playlist"), Qt::ToolTipRole);
       rowData << markPlaylistItem;
    } else if (mediaItem.type == "Category") {
       KIcon categoryActionIcon;
       QString tooltip;
       categoryActionIcon = KIcon("bangarang-category-browse");
       if (mediaItem.url.startsWith("music://songs")) {
           tooltip = "Show Songs";
       } else if (mediaItem.url.startsWith("music://albums")) {
           tooltip = "Show Albums";
       } else if (mediaItem.url.startsWith("music://artists")) {
           tooltip = "Show Artists";
       }
       QStandardItem * categoryItem = new QStandardItem(categoryActionIcon, QString());
       categoryItem->setData(mediaItem.url, MediaItem::UrlRole);
       categoryItem->setData(mediaItem.type, MediaItem::TypeRole);   
       categoryItem->setData(tooltip, Qt::ToolTipRole);
       rowData << categoryItem;
    } else {
        //Always return a second column. let the view figure out what to do with it
        QStandardItem * categoryItem = new QStandardItem(QIcon(), QString());
        rowData << categoryItem;
    }   
    return rowData;
}

Qt::DropActions MediaItemModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags MediaItemModel::flags(const QModelIndex &index) const
{
    //Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
    Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}

QStringList MediaItemModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list" << "text/plain";
    return types;
}

QMimeData *MediaItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    QString indexList;
    foreach (QModelIndex index, indexes) {
        if (index.isValid() && index.column() != 1) {
            QUrl url = QUrl(data(index, MediaItem::UrlRole).toString());
            urls << url;
            indexList += QString("BangarangRow:%1,").arg(index.row());
        }
    }
    
    mimeData->setUrls(urls);
    mimeData->setText(indexList);
    return mimeData;
}

bool MediaItemModel::dropMimeData(const QMimeData *data,
                                     Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
     return true;

    if (!data->hasFormat("text/uri-list"))
    return false;

    if (column > 0)
     return false;
    
    int beginRow;
    
    if (row != -1) {
        beginRow = row;
    } else if (parent.isValid()) {
        beginRow = parent.row();
    } else {
        beginRow = rowCount(QModelIndex());
    }
    
    QList<QUrl> urls = data->urls();
    
    bool internalMove = false;
    QStringList rowsToMove;
    if (data->text().startsWith("BangarangRow:")) {
        rowsToMove = data->text().split(",", QString::SkipEmptyParts);
        internalMove = true;
    }

    //insert rows into model
    QList<MediaItem> mediaItemsInserted;
    int insertionRow = beginRow;
    for (int i = 0; i < urls.count(); i++) {
        if (internalMove) {
            QString rowEntry = rowsToMove.at(i);
            int rowToMove = rowEntry.remove("BangarangRow:").toInt();
            MediaItem mediaItem = mediaItemAt(rowToMove);
            mediaItemsInserted << mediaItem;
            QList<QStandardItem *> rowItems = rowDataFromMediaItem(mediaItem);
            insertRow(insertionRow, rowItems);
            insertionRow = insertionRow + 1;
        } else {
            QString url = QUrl::fromPercentEncoding(urls.at(i).toString().toUtf8());
            if (Utilities::isAudio(url) || Utilities::isVideo(url)) {
                MediaItem mediaItem = Utilities::mediaItemFromUrl(KUrl(url));
                mediaItemsInserted << mediaItem;
                QList<QStandardItem *> rowItems = rowDataFromMediaItem(mediaItem);
                insertRow(insertionRow, rowItems);
                insertionRow = insertionRow + 1;
            }
        }
    }
    
    //Update cached data to reflect inserted rows
    insertionRow = beginRow;
    for (int i = 0; i < mediaItemsInserted.count(); i++) {
        MediaItem mediaItem = mediaItemsInserted.at(i);
        m_mediaList.insert(insertionRow, mediaItem);
        m_urlList.insert(insertionRow, mediaItem.url);
        insertionRow = insertionRow + 1;
    }
    
    if (internalMove) {
        m_emitChangedAfterDrop = true;
    } else {
        emit mediaListChanged();
    }
    
    return true;
}

void MediaItemModel::removeSourceInfo(const QList<MediaItem> &mediaList)
{
    //Assumes that items in mediaList are items currently in model
    if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
        listEngine->removeSourceInfo(mediaList);
    }
}

void MediaItemModel::updateSourceInfo(const QList<MediaItem> &mediaList)
{
    //Assumes that items in mediaList are items currently in model
    if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
        listEngine->updateSourceInfo(mediaList);
    }
}

void MediaItemModel::setCacheThreshold(int msec)
{
    m_cacheThreshold = msec;
}

int MediaItemModel::cacheThreshold()
{
    return m_cacheThreshold;
}

void MediaItemModel::setMediaListCache(MediaListCache * mediaListCache)
{
    m_mediaListCache = mediaListCache;
}

MediaListCache * MediaItemModel::mediaListCache()
{
    return m_mediaListCache;
}

bool MediaItemModel::lriIsLoadable()
{
    return m_lriIsLoadable;
}