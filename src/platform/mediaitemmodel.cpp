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
#include "listengines/listengine.h"
#include "listengines/listenginefactory.h"
#include "medialistcache.h"
#include "utilities/utilities.h"

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
    m_cacheThreshold = 5000; //default to 5 second loading threshold for adding to cache
    m_forceRefreshFromSource = false;
    m_loadSources = false;
    m_reload = false;
    m_suppressNoResultsMessage = false;
    m_pendingUpdateRefresh = false;
    m_suppressTooltip = false;
    setSupportedDragActions(Qt::CopyAction);
    
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
    emit mediaListPropertiesChanged();
}

QList<MediaItem> MediaItemModel::mediaList()
{
    return m_mediaList;
}

MediaItem MediaItemModel::mediaItemAt(int row)
{
    if (row < 0 || row >= m_mediaList.size()) {
        return MediaItem();
    }
    return m_mediaList.at(row);
}

int MediaItemModel::rowOfUrl(const QString &url)
{
    return m_urlList.indexOf(url);
}

int MediaItemModel::rowOfResourceUri(const QString &resourceUri)
{
    return m_resourceUriList.indexOf(resourceUri);
}

void MediaItemModel::load()
{
    if (!m_mediaListProperties.lri.isEmpty()) {
        if (m_mediaListCache->isInCache(m_mediaListProperties.lri) && !m_forceRefreshFromSource) {
            setLoadingState(true);
            // Load data from from the cache
            ListEngine * listEngine = m_listEngineFactory->availableListEngine(EngineTypeCache);
            MediaListProperties cacheListProperties;
            cacheListProperties.lri = QString("cache://dummyarg?%1").arg(m_mediaListProperties.lri);
            m_requestSignature = m_listEngineFactory->generateRequestSignature();
            listEngine->setRequestSignature(m_requestSignature);
            listEngine->setMediaListProperties(cacheListProperties);
            listEngine->start();
            kDebug() << "loading from cache for " << m_mediaListProperties.lri;
        } else {
            EngineType type = m_listEngineFactory->engineTypeFromString(m_mediaListProperties.engine());
            if (m_listEngineFactory->engineExists(type)) {
                setLoadingState(true);
                m_listEngineFactory->stopAll();

                ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
                m_requestSignature = m_listEngineFactory->generateRequestSignature();
                listEngine->setRequestSignature(m_requestSignature);
                listEngine->setMediaListProperties(m_mediaListProperties);
                m_lriStartTimes.insert(m_mediaListProperties.lri, QTime::currentTime());
                listEngine->start();
                kDebug() << "started new load for " << m_mediaListProperties.lri;
            } else {
                showNoResultsMessage();
            }
            m_forceRefreshFromSource = false;
        }
    }
    m_loadSources = false;
}

void MediaItemModel::loadLRI(const QString &lri)
{
    clearMediaListData();
    setMediaListProperties(MediaListProperties(lri));
    load();
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
    if (rowOfUrl(mediaItem.url) == -1 || mediaItem.url.isEmpty()) {
        m_mediaList << mediaItem;
        m_urlList << mediaItem.url;
        m_resourceUriList << mediaItem.fields["resourceUri"].toString();
        appendRow(rowDataFromMediaItem(mediaItem));
        if (emitMediaListChanged) {
            emit mediaListChanged();
        }
    }
}   

void MediaItemModel::categoryActivated(QModelIndex index)
{
    MediaListProperties mediaListProperties;
    mediaListProperties.lri =  itemFromIndex(index)->data(MediaItem::UrlRole).toString();
    mediaListProperties.name =  m_mediaList.at(index.row()).title;
    mediaListProperties.category = m_mediaList.at(index.row());
    m_mediaListProperties = mediaListProperties;
    
    if (m_mediaListCache->isInCache(m_mediaListProperties.lri)) {
        removeRows(0, rowCount());
        setLoadingState(true);
        // Load data from from the cache
        m_listEngineFactory->stopAll();
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(EngineTypeCache);
        MediaListProperties cacheListProperties;
        cacheListProperties.lri = QString("cache://dummyarg?%1").arg(m_mediaListProperties.lri);
        m_requestSignature = m_listEngineFactory->generateRequestSignature();
        listEngine->setRequestSignature(m_requestSignature);
        listEngine->setMediaListProperties(cacheListProperties);
        listEngine->start();
    } else {
        EngineType type = m_listEngineFactory->engineTypeFromString(m_mediaListProperties.engine());
        if (m_listEngineFactory->engineExists(type)) {
            removeRows(0, rowCount());
            setLoadingState(true);
            m_listEngineFactory->stopAll();
            ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
            m_requestSignature = m_listEngineFactory->generateRequestSignature();
            listEngine->setRequestSignature(m_requestSignature);
            listEngine->setMediaListProperties(m_mediaListProperties);
            m_lriStartTimes.insert(m_mediaListProperties.lri, QTime::currentTime());
            listEngine->start();
            kDebug()<< "started load for " << m_mediaListProperties.lri;
        }
    }
}

void MediaItemModel::actionActivated(QModelIndex index)
{
    MediaListProperties mediaListProperties;
    mediaListProperties.lri =  itemFromIndex(index)->data(MediaItem::UrlRole).toString();
    EngineType type = m_listEngineFactory->engineTypeFromString(mediaListProperties.engine());
    if (m_listEngineFactory->engineExists(type)) {
        m_mediaListProperties = mediaListProperties;
        removeRows(0, rowCount());
        setLoadingState(true);
        m_listEngineFactory->stopAll();
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
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
    
    //Load data only for media sources
    m_subRequestMediaLists.clear();
    m_subRequestSignatures.clear();
    m_subRequestsDone = 0;
    bool onlySources = true;
    m_remainingCatsForLoadSources.clear();
    m_requestSignature = m_listEngineFactory->generateRequestSignature();
    for (int i = 0; i < mediaList.count(); ++i) {
        if ((mediaList.at(i).type == "Audio") || (mediaList.at(i).type == "Video") || (mediaList.at(i).type == "Image")){
            if (!mediaList.at(i).url.isEmpty()) { //url of sources can't be empty
                loadMediaItem(mediaList.at(i));
            }
        } else if (mediaList.at(i).type == "Category") {
            onlySources = false;
            m_remainingCatsForLoadSources.append(mediaList.at(i));
            if (mediaList.count() == 1) {
                MediaListProperties mediaListProperties;
                mediaListProperties.lri =  mediaList.at(i).url;
                mediaListProperties.name = mediaList.at(i).title;
                
                // - Get the lri for loading sources using this category item
                EngineType type = m_listEngineFactory->engineTypeFromString(mediaListProperties.engine());
                ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
                listEngine->setMediaListProperties(mediaListProperties);
                listEngine->setFilterForSources(mediaListProperties.engineFilter());
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
                EngineType type = m_listEngineFactory->engineTypeFromString(mediaListProperties.engine());
                if (m_listEngineFactory->engineExists(type)) {
                    QString subRequestSignature = m_listEngineFactory->generateRequestSignature();
                    m_subRequestSignatures.append(subRequestSignature);
                    QList<MediaItem> emptyList;
                    m_subRequestMediaLists.append(emptyList);
                }
            }
        }
    }

    if (onlySources) {
        if (rowCount() == 0  && !m_suppressNoResultsMessage) {
            showNoResultsMessage();
        }
        emit mediaListChanged();
    } else if (!m_remainingCatsForLoadSources.isEmpty()){
        //Launch load request(s)
        m_listEngineFactory->stopAll();
        int maxSimultaneous = 1; //Limit threads to one.  May increase later if stability is better.
        setLoadingState(true);
        for (int i = 0; i < maxSimultaneous; i++) {
            loadSourcesForNextCat();
        }
    }
    
    m_loadSources = true;
    m_mediaListForLoadSources = mediaList;
}

void MediaItemModel::loadSourcesForNextCat()
{
    if (!m_remainingCatsForLoadSources.isEmpty()) {
        int subRequestSignatureIndex = m_subRequestSignatures.count() - m_remainingCatsForLoadSources.count();
        if (subRequestSignatureIndex < 0) {
            QString subRequestSignature = m_listEngineFactory->generateRequestSignature();
            m_subRequestSignatures.append(subRequestSignature);
            QList<MediaItem> emptyList;
            m_subRequestMediaLists.append(emptyList);
            subRequestSignatureIndex = 0;
        }
        MediaItem category = m_remainingCatsForLoadSources.takeFirst();
        MediaListProperties mediaListProperties;
        mediaListProperties.lri = category.url;
        mediaListProperties.name = category.title;
        EngineType type = m_listEngineFactory->engineTypeFromString(m_mediaListProperties.engine());
        if (m_listEngineFactory->engineExists(type)) {

            ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
            listEngine->setMediaListProperties(mediaListProperties);
            listEngine->setFilterForSources(mediaListProperties.engineFilter());
            QString loadSourcesLri = listEngine->mediaListProperties().lri;

            if (m_mediaListCache->isInCache(loadSourcesLri)) {
                // Load data from from the cache
                listEngine = m_listEngineFactory->availableListEngine(EngineTypeCache);
                MediaListProperties cacheListProperties;
                cacheListProperties.lri = QString("cache://dummyarg?%1").arg(loadSourcesLri);
                m_requestSignature = m_listEngineFactory->generateRequestSignature();
                listEngine->setRequestSignature(m_requestSignature);
                if (m_subRequestSignatures.count() > 0) {
                    listEngine->setSubRequestSignature(m_subRequestSignatures.at(subRequestSignatureIndex));
                }
                listEngine->setMediaListProperties(cacheListProperties);
                listEngine->start();

            } else {
                listEngine->setRequestSignature(m_requestSignature);
                if (m_subRequestSignatures.count() > 0) {
                    listEngine->setSubRequestSignature(m_subRequestSignatures.at(subRequestSignatureIndex));
                }
                m_lriStartTimes.insert(loadSourcesLri, QTime::currentTime());
                listEngine->start();
                kDebug()<< "started load for " << loadSourcesLri;
            }
        }
    }
}

void MediaItemModel::addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature)
{
    //Check request signature of results and ignore results with a different signature
   if (done) kDebug() << "results returned for " << mediaListProperties.lri;
   if ((mediaListProperties.lri == m_mediaListProperties.lri) || (requestSignature == m_requestSignature)) {
        if (m_subRequestSignatures.count() == 0) {
            hideLoadingMessage();
            loadMediaList(mediaList, false, true);
            m_mediaListProperties = mediaListProperties;
            emit mediaListPropertiesChanged();
            if (done) {
                m_listEngineFactory->resumeAll();
                if (rowCount() == 0 && !m_suppressNoResultsMessage) {
                    showNoResultsMessage();
                }
                m_reload = false;
                m_lriIsLoadable = true;
                setLoadingState(false);
                emit mediaListChanged();
            }
        } else {
            loadSourcesForNextCat();
            //Place subrequest results in the correct order
            int indexOfSubRequest = m_subRequestSignatures.indexOf(subRequestSignature);
            if (indexOfSubRequest != -1) {
                //Determine index of subrequest not loaded so far
                int indexOfFirstEmpty = -1;
                for (int i = 0; i < m_subRequestMediaLists.count(); i++) {
                    if (m_subRequestMediaLists.at(i).isEmpty()) {
                        indexOfFirstEmpty = i;
                        break;
                    }
                }

                //Store subrequest results
                QList<MediaItem> srMediaList = m_subRequestMediaLists.at(indexOfSubRequest);
                srMediaList.append(mediaList);
                m_subRequestMediaLists.replace(indexOfSubRequest, srMediaList);

                if (done) {
                    m_subRequestsDone = m_subRequestsDone + 1;
                    if (m_subRequestsDone == m_subRequestSignatures.count()) {
                        m_listEngineFactory->resumeAll();
                        setLoadingState(false);
                        //All the subrequests results are in, go ahead and load results in correct order
                        int count = 0;
                        for (int i = 0; i < m_subRequestMediaLists.count(); ++i) {
                            count += m_subRequestMediaLists.at(i).count();
                            if (i < indexOfFirstEmpty) {
                                continue;
                            }
                            loadMediaList(m_subRequestMediaLists.at(i), false, true);
                        }
                        if (rowCount() == 0 && !m_suppressNoResultsMessage) {
                            showNoResultsMessage();
                        }
                        //Need a basic lri so updateInfo and removeInfo can be performed by a list engine
                        m_mediaListProperties.lri = mediaListProperties.engine(); 
                        if (!m_reload && !m_mediaListProperties.name.contains(i18n("Multiple"))) {
                            m_mediaListProperties.name = i18n("Multiple %1", m_mediaListProperties.name);
                        }
                        m_mediaListProperties.summary = i18np("1 item", "%1 items", count);
                        m_subRequestMediaLists.clear();
                        m_subRequestSignatures.clear();
                        m_subRequestsDone = 0;
                        m_reload = false;
                        m_lriIsLoadable = false;
                        emit mediaListChanged();
                    } else if (indexOfSubRequest == indexOfFirstEmpty) {
                        int count = 0;
                        for (int i = 0; i < m_subRequestMediaLists.count(); i++) {
                            count += m_subRequestMediaLists.at(i).count();
                            if (i < indexOfFirstEmpty) {
                                continue;
                            }
                            if (m_subRequestMediaLists.at(i).count() > 0) {
                                hideLoadingMessage();
                                loadMediaList(m_subRequestMediaLists.at(i), false, true);
                                if (!m_reload && !m_mediaListProperties.name.contains(i18n("Multiple"))) {
                                    m_mediaListProperties.name = i18n("Multiple %1", m_mediaListProperties.name);
                                }
                                m_mediaListProperties.summary = i18np("1 item", "%1 items", count);
                                emit mediaListPropertiesChanged();
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    //Cache results if they took long enough to return
    if (done && m_lriStartTimes.contains(mediaListProperties.lri)) {
        int elapsedMSecs = m_lriStartTimes.value(mediaListProperties.lri).msecsTo(QTime::currentTime());
        if (elapsedMSecs > m_cacheThreshold) {
            m_mediaListCache->addMediaList(mediaListProperties, m_mediaList);
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

void MediaItemModel::updateArtwork(QImage artworkImage, MediaItem mediaItem)
{
    int row = rowOfUrl(mediaItem.url);
    if (row != -1) {
        MediaItem updatedMediaItem = mediaItemAt(row);
        updatedMediaItem.artwork = QIcon(QPixmap::fromImage(artworkImage));
        updatedMediaItem.hasCustomArtwork = mediaItem.hasCustomArtwork;
        replaceMediaItemAt(row, updatedMediaItem);
    }     
}

void MediaItemModel::updateMediaItem(MediaItem mediaItem)
{
    int row = rowOfUrl(mediaItem.url);
    if (row != -1) {
        replaceMediaItemAt(row, mediaItem);
    } else {
        if (mediaItem.fields["sourceLri"].toString() == m_mediaListProperties.lri) {
            m_pendingUpdateRefresh = true;
        }
    }
}

void MediaItemModel::updateMediaListPropertiesCategoryArtwork(QImage artworkImage, MediaItem mediaItem)
{
    if (m_mediaListProperties.category.url == mediaItem.url) {
        m_mediaListProperties.category.artwork = QIcon(QPixmap::fromImage(artworkImage));
        emit mediaListPropertiesChanged();
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

void MediaItemModel::removeMediaItemByResource(QString resourceUri)
{
    int row = rowOfResourceUri(resourceUri);
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
    m_listEngineFactory->stopAll();
    removeRows(0, rowCount());
    m_mediaList.clear();
    m_urlList.clear();
    m_resourceUriList.clear();
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
        m_resourceUriList.removeAt(row);
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
    m_resourceUriList.replace(row, mediaItem.fields["resourceUri"].toString());
    QList<QStandardItem *> rowData = rowDataFromMediaItem(mediaItem);
    for (int i = 0; i < rowData.count(); i++) {
        setItem(row, i, rowData.at(i));
    }
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::insertMediaItemAt(int row, const MediaItem &mediaItem, bool emitMediaListChanged)
{
    int existingRow = rowOfUrl(mediaItem.url);
    if (existingRow == row) {
        replaceMediaItemAt(row, mediaItem, emitMediaListChanged);
        return;
    }

    m_mediaList.insert(row, mediaItem);
    m_urlList.insert(row, mediaItem.url);
    m_resourceUriList.insert(row, mediaItem.fields["resourceUri"].toString());
    QList<QStandardItem *> rowData = rowDataFromMediaItem(mediaItem);
    insertRow(row, rowData);

    if (existingRow != -1) {
        if (row <= existingRow) {
            existingRow++;
        }
        removeMediaItemAt(existingRow, false);
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
        m_resourceUriList.removeAt(start);
    }
    if (m_emitChangedAfterDrop) {
        emit mediaListChanged();
        m_emitChangedAfterDrop = false;
    }
    Q_UNUSED(index);
}

void MediaItemModel::setLoadingState(bool state)
{
    bool stateChanged = (m_loadingState != state);
    m_loadingState = state;
    if (m_loadingState) {
        showLoadingMessage();
        emit loading();
    } else {
        hideLoadingMessage();
    }
    if (stateChanged) {
        emit loadingStateChanged(m_loadingState);
    }
}

bool MediaItemModel::isLoading()
{
    return m_loadingState;
}

void MediaItemModel::showLoadingMessage()
{
    if (m_loadingState ) {
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
        } else if (m_mediaList.at(0).type == "Message" &&
                   m_mediaList.at(0).fields["messageType"].toString() == "Loading") {
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

void MediaItemModel::updateStatus(QHash<QString, QVariant> updatedStatus)
{
    m_status = updatedStatus;
    emit statusUpdated();
}

void MediaItemModel::setSuppressTooltip(bool suppress)
{
    m_suppressTooltip = suppress;
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
    titleItem->setData(mediaItem.hasCustomArtwork, MediaItem::HasCustomArtworkRole);
    titleItem->setData(mediaItem.semanticComment, MediaItem::SemanticCommentRole);
    if (!m_suppressTooltip) {
        QString tooltip = QString("<b>%1</b>").arg(mediaItem.title);
        if (!mediaItem.subTitle.isEmpty()) {
            tooltip.append((QString("<br>%2").arg(mediaItem.subTitle)));
        }
        if (!mediaItem.fields["description"].toString().isEmpty()) {
            QString description = mediaItem.fields["description"].toString();
            if (description.length() > 300) {
                description.chop(300);
                description = description + QString("...");
            }
            tooltip.append(QString("<br>%1").arg(description));
        }
        if (!mediaItem.semanticComment.isEmpty()) {
            tooltip.append(QString("<br><i>%1</i>").arg(mediaItem.semanticComment));
        }
        titleItem->setData(tooltip, Qt::ToolTipRole);
    }
    if (!mediaItem.fields["rating"].isNull()) {
        titleItem->setData(mediaItem.fields["rating"].toInt(), MediaItem::RatingRole);
    }
    if (!mediaItem.fields["playCount"].isNull()) {
        titleItem->setData(mediaItem.fields["playCount"].toInt(), MediaItem::PlayCountRole);
    }
    if (!mediaItem.fields["lastPlayed"].isNull()) {
        titleItem->setData(mediaItem.fields["lastPlayed"].toDateTime(), MediaItem::LastPlayedRole);
    }
    if (mediaItem.type == "Category") {
        titleItem->setData(mediaItem.fields["categoryType"].toString(), MediaItem::SubTypeRole);
    }           
    rowData << titleItem;
    return rowData;
}

Qt::DropActions MediaItemModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags MediaItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags useFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    QString type = data(index, MediaItem::TypeRole).toString();
    if (index.isValid()) {
        useFlags |= Qt::ItemIsDropEnabled;
        if (Utilities::isMedia(type)) {
            useFlags |= Qt::ItemIsDragEnabled;
        }
    } else {
        useFlags |= Qt::ItemIsDropEnabled;
    }
    return useFlags;
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
            QString url = urls.at(i).toEncoded();
            if (Utilities::isAudio(url) || Utilities::isVideo(url)) {
                MediaItem mediaItem = Utilities::mediaItemFromUrl(KUrl(url));
                mediaItemsInserted << mediaItem;
                QList<QStandardItem *> rowItems = rowDataFromMediaItem(mediaItem);
                insertRow(insertionRow, rowItems);
                insertionRow = insertionRow + 1;
            } else if (Utilities::isFSDirectory(url)) {
                MediaItem mediaItem = Utilities::mediaItemFromUrl(KUrl(url));
                if (mediaItem.type == "Category") {
                    QList<MediaItem> mediaList;
                    mediaList.append(mediaItem);
                    loadSources(mediaList); // no insertion - always add to end.
                }
            }
        }
    }
    
    //Update cached data to reflect inserted rows
    insertionRow = beginRow;
    for (int i = 0; i < mediaItemsInserted.count(); i++) {
        MediaItem mediaItem = mediaItemsInserted.at(i);
        m_mediaList.insert(insertionRow, mediaItem);
        m_urlList.insert(insertionRow, mediaItem.url);
        m_resourceUriList.insert(insertionRow, mediaItem.fields["resourceUri"].toString());
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
    //Group items in list by list engine type of MediaItem sourceLris
    QHash<EngineType, QList<MediaItem> > typeLists;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaListProperties properties(mediaList.at(i).fields["sourceLri"].toString());
        EngineType currentType = m_listEngineFactory->engineTypeFromString(properties.engine());
        if (currentType == EngineTypeUnknown) {
            currentType = m_listEngineFactory->engineTypeFromString(m_mediaListProperties.engine());
        }
        if (currentType != EngineTypeUnknown) {
            QList<MediaItem> typeList = typeLists.value(currentType);
            typeList.append(mediaList.at(i));
            typeLists.insert(currentType, typeList);
        }
    }

    //Remove info using appropriate list engine type
    QList<EngineType> types = typeLists.keys();
    for (int i = 0; i < types.count(); i++) {
        EngineType type = types.at(i);
        QList<MediaItem> list = typeLists.value(type);
        if (m_listEngineFactory->engineExists(type)) {
            ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
            listEngine->removeSourceInfo(mediaList);
        }
    }
}

void MediaItemModel::updateSourceInfo(const QList<MediaItem> &mediaList, bool nepomukOnly)
{
    //Group items in list by list engine type of MediaItem sourceLris
    QHash<EngineType, QList<MediaItem> > typeLists;
    for (int i = 0; i < mediaList.count(); i++) {
        MediaListProperties properties(mediaList.at(i).fields["sourceLri"].toString());
        EngineType currentType = m_listEngineFactory->engineTypeFromString(properties.engine());
        if (currentType == EngineTypeUnknown) {
            currentType = m_listEngineFactory->engineTypeFromString(m_mediaListProperties.engine());
        }
        if (currentType != EngineTypeUnknown) {
            QList<MediaItem> typeList = typeLists.value(currentType);
            typeList.append(mediaList.at(i));
            typeLists.insert(currentType, typeList);
        }
    }

    //Update info using appropriate list engine type
    QList<EngineType> types = typeLists.keys();
    for (int i = 0; i < types.count(); i++) {
        EngineType type = types.at(i);
        QList<MediaItem> list = typeLists.value(type);
        if (m_listEngineFactory->engineExists(type)) {
            ListEngine * listEngine = m_listEngineFactory->availableListEngine(type);
            listEngine->updateSourceInfo(mediaList, nepomukOnly);
        }
    }

    //Always update model info in case list engine is unable to update source of media items
    //Note that if the list engine doesn't update source of information then data in model will
    //only last until data is reloaded.
    updateMediaItems(mediaList);

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

void MediaItemModel::setSuppressNoResultsMessage(bool suppress)
{
    m_suppressNoResultsMessage = suppress;
}

QHash<QString, QVariant> MediaItemModel::status()
{
    return m_status;
}

void MediaItemModel::updateRefresh()
{
    if (m_pendingUpdateRefresh) {
        m_pendingUpdateRefresh = false;
        reload();
    }
}

bool MediaItemModel::containsPlayable()
{
    if (rowCount() < 1)
        return false;
    MediaItem item = mediaItemAt(0);
    return ( Utilities::isMedia(item.type) ||
             Utilities::isCategory(item.type) ||
             Utilities::isFeed(item.fields["categoryType"].toString()) );
}

MediaSortFilterProxyModel::MediaSortFilterProxyModel(QObject* parent)
                          : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool MediaSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    MediaItemModel *model = (MediaItemModel *) sourceModel();
    QModelIndex index = model->index(sourceRow, 0, sourceParent);
    QList<QRegExp> search;
    QString data = model->data(index, Qt::DisplayRole).toString();
    QVariant type_variant = model->data(index, MediaItem::TypeRole);
    if ( type_variant.isValid() ) {
        QString type = type_variant.toString();
        bool isCat = Utilities::isCategory( type );
        if ( !Utilities::isMedia( type ) && !isCat )
            return true;
        if ( isCat && data == "Indexer" )
            return true;
        if ( Utilities::isMessage( type ) )
            return true;
    }
    QStringList pat = filterRegExp().pattern().split(" ", QString::SkipEmptyParts);
    Qt::CaseSensitivity case_sen = filterRegExp().caseSensitivity();
    
    if (model->data(index, MediaItem::SubTitleRole).isValid())
        data += " " + model->data(index, MediaItem::SubTitleRole).toString();
    foreach (QString str, pat) {
        search << QRegExp(str, case_sen);
    }
    foreach(QRegExp reg, search) {
        if (!data.contains(reg))
            return false;
    }
    return true;
};
