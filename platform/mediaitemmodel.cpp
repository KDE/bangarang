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

#include <QFontMetrics>
#include <QDateTime>
#include <KIcon>
#include <KDebug>

MediaItemModel::MediaItemModel(QObject * parent) : QStandardItemModel(parent) 
{
    m_parent = parent;
    m_mediaListProperties.lri = QString();
    m_filter = QString();
    m_listEngineFactory = new ListEngineFactory(this);
    m_emitChangedAfterDrop = false;
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
    return m_urlList.indexOf(url);
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

void MediaItemModel::reload()
{
    clearMediaListData();
    load();
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
    m_urlList.clear();
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
        if (m_mediaList.count() > 0) {
            removeMediaItemAt(row, false);
        }
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
    titleItem->setData(mediaItem.isSavedList, MediaItem::IsSavedListRole);
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
       markPlaylistItem->setData("Add to playlist/Remove from playlist", Qt::ToolTipRole);
       rowData << markPlaylistItem;
    } else if (mediaItem.type == "Category") {
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
    
    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled |defaultFlags;
    else
        return defaultFlags;
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
            indexList += QString("%1,").arg(index.row());
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
    QStringList rowsToMove = data->text().split(",", QString::SkipEmptyParts);

    //insert rows into model
    QList<MediaItem> mediaItemsToMove;
    int insertionRow = beginRow;
    for (int i = 0; i < urls.count(); i++) {
        if (rowsToMove.count() > 0) {
            int rowToMove = rowsToMove.at(i).toInt();
            MediaItem mediaItem = mediaItemAt(rowToMove);
            mediaItemsToMove << mediaItem;
            QList<QStandardItem *> rowItems = rowDataFromMediaItem(mediaItem);
            insertRow(insertionRow, rowItems);
            insertionRow = insertionRow + 1;
        }
    }
    
    //Update cached data to reflect inserted rows
    insertionRow = beginRow;
    for (int i = 0; i < urls.count(); i++) {
        MediaItem mediaItem = mediaItemsToMove.at(i);
        m_mediaList.insert(insertionRow, mediaItem);
        m_urlList.insert(insertionRow, mediaItem.url);
        insertionRow = insertionRow + 1;
    }
    m_emitChangedAfterDrop = true;
    
    return true;
}
