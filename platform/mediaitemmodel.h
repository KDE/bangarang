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

#include <QStandardItemModel>
#include <QObject>
#include <QPixmap>
#include <QList>

class MusicListEngine;
class FileListEngine;
class ListEngineFactory;

struct MediaItem {
    enum MediaItemRole { UrlRole = Qt::UserRole + 1,
    SubTitleRole = Qt::UserRole + 2,
    DurationRole = Qt::UserRole + 3,
    RatingRole = Qt::UserRole + 4,
    TypeRole = Qt::UserRole + 5,
    FilterRole = Qt::UserRole + 6,
    PlaylistIndexRole = Qt::UserRole + 7, 
    NowPlayingRole = Qt::UserRole + 8,
    IsSavedListRole = Qt::UserRole + 9 };
    QIcon artwork;
    QString title;
    QString subTitle;
    QString duration;
    QString type;
    QString url;
    QString filter;
    int playlistIndex;
    bool nowPlaying;
    bool isSavedList;
    QHash <QString, QVariant> fields;
};

class MediaListProperties {

public:
    QString name;
    QString lri;  //List Resource Identifier
    QString engine() {
        if (lri.indexOf("://") != -1) {
            return lri.left(lri.indexOf("://") + 3);
        } else {
            return QString();
        }
    }
    QString engineArg() {
        int endOfArg = (lri.indexOf("?") != -1) ? lri.indexOf("?") - 1: lri.size() - 1;
        if ((lri.indexOf("://") != -1) && (lri.indexOf("://") != lri.size() - 3)) {
            //return lri.mid(lri.indexOf("://") + 3, lri.size() - endOfArg + 1);
            return lri.mid(lri.indexOf("://") + 3, endOfArg - (lri.indexOf("://") + 2));
        } else {
            return QString();
        }
    }
    QString engineFilter() {
        if ((lri.indexOf("://") != -1)  && (lri.indexOf("?") != -1) && (lri.indexOf("?") != lri.size() - 1)){
            return lri.right(lri.size() - (lri.indexOf("?") + 1));
        } else {
            return QString();
        }
    }
    QString type;
};

class MediaList : QList<MediaItem>{};

class MediaItemModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        MediaItemModel(QObject * parent);
        ~MediaItemModel();
        QString dataEngine();
        QString filter();
        void clearMediaListData();
        void load();
        QList<MediaItem> mediaList();
        void loadMediaList(QList<MediaItem>, bool emitMediaListChanged = false);
        MediaItem mediaItemAt(int row);
        void loadMediaItem(MediaItem mediaItem, bool emitMediaListChanged = false);
        MediaListProperties mediaListProperties();
        void setMediaListProperties(MediaListProperties mediaListProperties);
        int rowOfUrl(QString url);
        void loadSources(QList<MediaItem> mediaList);
        void removeMediaItemAt(int row, bool emitMediaListChanged = false);
        void replaceMediaItemAt(int row, MediaItem mediaItem, bool emitMediaListChanged = false);
        void setListEngineFactory(ListEngineFactory * listEngineFactory);

        Qt::DropActions supportedDropActions() const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        QStringList mimeTypes() const;
        QMimeData *mimeData(const QModelIndexList &indexes) const;
        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
        
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
        
    Q_SIGNALS:
        void propertiesChanged(); 
        void mediaListChanged();
        void loading();
        
    protected Q_SLOTS:
        void categoryActivated(QModelIndex index);
        void actionActivated(QModelIndex index);
        void synchRemoveRows(const QModelIndex &index, int start, int end);
        void showLoadingMessage();
    
    public Q_SLOTS:
        void addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature);
        void reload();
};

#endif // MEDIAITEMMODEL_H
