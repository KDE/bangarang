#include <QFontMetrics>
#include <QDateTime>
#include <KIcon>
#include "mediaitemmodel.h"
#include "listengine.h"
#include "listenginefactory.h"

MediaItemModel::MediaItemModel(QObject * parent) : QStandardItemModel(parent) 
{
    m_parent = parent;
    m_mediaListProperties.lri = QString();
    m_filter = QString();
    m_listEngineFactory = new ListEngineFactory(this);
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(synchRemoveRows(const QModelIndex &, int, int)));
}

MediaItemModel::~MediaItemModel() 
{
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

void MediaItemModel::setMediaListProperties(MediaListProperties mediaListProperties)
{
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

int MediaItemModel::rowOfUrl(QString url)
{
    return urlList.indexOf(url);
}

void MediaItemModel::load()
{
    if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
        showLoadingMessage();
        
        // Load data from engine
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
        m_requestSignature = m_listEngineFactory->generateRequestSignature();
        listEngine->setRequestSignature(m_requestSignature);
        listEngine->setMediaListProperties(m_mediaListProperties);
        listEngine->start();
    } else {
        showNoResultsMessage();
    }
}

void MediaItemModel::loadMediaList(QList<MediaItem> mediaList, bool emitMediaListChanged)
{
    for (int i = 0 ; i < mediaList.count() ; ++i) {
        loadMediaItem(mediaList.at(i));
    }
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::loadMediaItem(MediaItem mediaItem, bool emitMediaListChanged)
{
    /*QList<QStandardItem *> rowData;
    QStandardItem * titleItem = new QStandardItem(mediaItem.artwork, mediaItem.title);
    titleItem->setData(mediaItem.subTitle, MediaItem::SubTitleRole);
    titleItem->setData(mediaItem.url, MediaItem::UrlRole);
    titleItem->setData(mediaItem.type, MediaItem::TypeRole);
    titleItem->setData(mediaItem.duration, MediaItem::DurationRole);
    titleItem->setData(mediaItem.playlistIndex, MediaItem::PlaylistIndexRole);
    titleItem->setData(mediaItem.nowPlaying, MediaItem::NowPlayingRole);
    rowData << titleItem;
    
    //if ((mediaItem.type == "Audio") || (mediaItem.type == "Video") || (mediaItem.type == "Images")) {
        QStandardItem * markPlaylistItem = new QStandardItem(KIcon(), QString());
        markPlaylistItem->setData(mediaItem.url, MediaItem::UrlRole);
        markPlaylistItem->setData(mediaItem.type, MediaItem::TypeRole);   
        markPlaylistItem->setData("Mark for playlist/Unmark", Qt::ToolTipRole);
        rowData << markPlaylistItem;
    //}
   
    if (mediaItem.type == "Category") {
        KIcon categoryActionIcon;
        QString tooltip;
        categoryActionIcon = KIcon("system-run");
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
    }*/
    appendRow(rowDataFromMediaItem(mediaItem));
    m_mediaList << mediaItem;
    urlList << mediaItem.url;
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}   

void MediaItemModel::categoryActivated(QModelIndex index)
{
    MediaListProperties mediaListProperties;
    mediaListProperties.lri =  itemFromIndex(index)->data(MediaItem::UrlRole).toString();
    
    if (m_listEngineFactory->engineExists(mediaListProperties.engine())) {
        m_mediaListProperties = mediaListProperties;
        removeRows(0, rowCount());
        showLoadingMessage();
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
        m_requestSignature = m_listEngineFactory->generateRequestSignature();
        listEngine->setRequestSignature(m_requestSignature);
        listEngine->setMediaListProperties(m_mediaListProperties);
        listEngine->start();
    }
}

void MediaItemModel::actionActivated(QModelIndex index)
{
    MediaListProperties mediaListProperties;
    mediaListProperties.lri =  itemFromIndex(index)->data(MediaItem::UrlRole).toString();

    if (m_listEngineFactory->engineExists(mediaListProperties.engine())) {
        m_mediaListProperties = mediaListProperties;
        removeRows(0, rowCount());
        showLoadingMessage();
        ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
        m_requestSignature = m_listEngineFactory->generateRequestSignature();
        listEngine->setRequestSignature(m_requestSignature);
        listEngine->setMediaListProperties(m_mediaListProperties);
        listEngine->activateAction();
    }   
}

void MediaItemModel::loadSources(QList<MediaItem> mediaList)
{
    showLoadingMessage();
    
    //Load data only for media sources
    m_subRequestMediaLists.clear();
    m_subRequestSignatures.clear();
    m_subRequestsDone = 0;
    bool onlySources = true;
    m_requestSignature = m_listEngineFactory->generateRequestSignature();
    for (int i = 0; i < mediaList.count(); ++i) {
        if ((mediaList.at(i).type == "Audio") || (mediaList.at(i).type == "Video") || (mediaList.at(i).type == "Image")){
            hideLoadingMessage();
            loadMediaItem(mediaList.at(i));
        } else if (mediaList.at(i).type == "Category") {
            //Generate signatures and media list holders for each subrequest
            //This must be complete for all categories before launching subrequests
            //to ensure full order of subrequests are available when results are returned
            onlySources = false;
            if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
                QString subRequestSignature = m_listEngineFactory->generateRequestSignature();
                m_subRequestSignatures.append(subRequestSignature);
                QList<MediaItem> emptyList;
                m_subRequestMediaLists.append(emptyList);
            }
        }
    }
    if (onlySources) {
        if (rowCount() == 0) {
            showNoResultsMessage();
        }
        emit mediaListChanged();
    } else {
        //Launch subrequests
        for (int i = 0; i < mediaList.count(); ++i) {
            MediaListProperties mediaListProperties;
            mediaListProperties.lri = mediaList.at(i).url;
            if (m_listEngineFactory->engineExists(m_mediaListProperties.engine())) {
                ListEngine * listEngine = m_listEngineFactory->availableListEngine(m_mediaListProperties.engine());
                listEngine->setRequestSignature(m_requestSignature);
                listEngine->setSubRequestSignature(m_subRequestSignatures.at(i));
                listEngine->setFilterForSources(mediaListProperties.engineFilter());
                listEngine->start();
            }
        }
    }
}

void MediaItemModel::addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature)
{
    //Check request signature of results and ignore results with a different signature
    if (requestSignature == m_requestSignature) {
        if (m_subRequestSignatures.count() == 0) {
            hideLoadingMessage();
            loadMediaList(mediaList);
            m_mediaListProperties.name = mediaListProperties.name;
            if (done) {
                if (rowCount() == 0) {
                    showNoResultsMessage();
                }
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
                        hideLoadingMessage();
                        //All the subrequests results are in, go ahead and load results in correct order
                        for (int i = 0; i < m_subRequestMediaLists.count(); ++i) {
                            loadMediaList(m_subRequestMediaLists.at(i));
                        }
                        m_subRequestMediaLists.clear();
                        m_subRequestSignatures.clear();
                        m_subRequestsDone = 0;
                        if (rowCount() == 0) {
                            showNoResultsMessage();
                        }
                        emit mediaListChanged();
                    }
                }
            }
        }
    }
    
    Q_UNUSED(done);
}

void MediaItemModel::clearMediaListData()
{
    removeRows(0, rowCount());
    m_mediaList.clear();
    urlList.clear();
}

void MediaItemModel::removeMediaItemAt(int row, bool emitMediaListChanged)
{
    if (row < rowCount()) {
        removeRows(row, 1);
    }
    if (emitMediaListChanged) {
        emit mediaListChanged();
    }
}

void MediaItemModel::replaceMediaItemAt(int row, MediaItem mediaItem, bool emitMediaListChanged)
{
    m_mediaList.replace(row, mediaItem);
    urlList.replace(row, mediaItem.url);
    QList<QStandardItem *> rowData = rowDataFromMediaItem(mediaItem);
    for (int i = 0; i < rowData.count(); i++) {
        setItem(row, i, rowData.at(i));
    }
}

void MediaItemModel::synchRemoveRows(const QModelIndex &index, int start, int end)
{
    for (int i = start; i <= end; ++i) {
        m_mediaList.removeAt(start);
        urlList.removeAt(start);
    }
    Q_UNUSED(index);
}

void MediaItemModel::showLoadingMessage()
{
    MediaItem loadingMessage;
    loadingMessage.title = "Loading...";
    loadingMessage.type = "Message";
    loadMediaItem(loadingMessage, false);
    emit loading();
}

void MediaItemModel::hideLoadingMessage()
{
    MediaItem loadingMessage;
    loadingMessage.title = "Loading...";
    loadingMessage.type = "Message";
    int row = -1;
    for (int i = 0; i < m_mediaList.count(); ++i) {
        if ((m_mediaList.at(i).title == "Loading...") && (m_mediaList.at(i).type == "Message")) {
            row = i;
            break;
        }
    }
    if (row != -1) {
        removeMediaItemAt(row, false);
    }
}

void MediaItemModel::showNoResultsMessage()
{
    MediaItem loadingMessage;
    loadingMessage.title = "No results";
    loadingMessage.type = "Message";
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
    rowData << titleItem;
    
    //if ((mediaItem.type == "Audio") || (mediaItem.type == "Video") || (mediaItem.type == "Images")) {
   QStandardItem * markPlaylistItem = new QStandardItem(KIcon(), QString());
   markPlaylistItem->setData(mediaItem.url, MediaItem::UrlRole);
   markPlaylistItem->setData(mediaItem.type, MediaItem::TypeRole);   
   markPlaylistItem->setData("Mark for playlist/Unmark", Qt::ToolTipRole);
   rowData << markPlaylistItem;
   //}
   
   if (mediaItem.type == "Category") {
       KIcon categoryActionIcon;
       QString tooltip;
       categoryActionIcon = KIcon("system-run");
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
   }
   return rowData;
}
