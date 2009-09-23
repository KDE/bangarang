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
    NowPlayingRole = Qt::UserRole + 8 };
    QIcon artwork;
    QString title;
    QString subTitle;
    QString duration;
    QString type;
    QString url;
    QString filter;
    int playlistIndex;
    bool nowPlaying;
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
        void load();
        QList<MediaItem> mediaList();
        MediaItem mediaItemAt(int row);
        void loadMediaList(QList<MediaItem>, bool emitMediaListChanged = false);
        void loadMediaItem(MediaItem mediaItem, bool emitMediaListChanged = false);
        MediaListProperties mediaListProperties();
        void setMediaListProperties(MediaListProperties mediaListProperties);
        int rowOfUrl(QString url);
        void loadSources(QList<MediaItem> mediaList);
        //QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        void clearMediaListData();
        void removeMediaItemAt(int row, bool emitMediaListChanged = false);
        void replaceMediaItemAt(int row, MediaItem mediaItem, bool emitMediaListChanged = false);
        void setListEngineFactory(ListEngineFactory * listEngineFactory);
        QObject * m_parent;
        
    private:
        QString m_dataEngine;
        QString m_filter;
        MediaListProperties m_mediaListProperties;
        MusicListEngine * m_musicListEngine;
        ListEngineFactory * m_listEngineFactory;
        QString m_requestSignature;
        QStringList m_subRequestSignatures;
        QList< QList<MediaItem> > m_subRequestMediaLists; //stores correctly ordered medialists for selected catgories
        int m_subRequestsDone;
        QStringList urlList;
        QList<MediaItem> m_mediaList;
        void showLoadingMessage();
        void hideLoadingMessage();
        void showNoResultsMessage();
        QList<QStandardItem *> rowDataFromMediaItem(MediaItem mediaItem);
        
    Q_SIGNALS:
        void propertiesChanged(); 
        void mediaListChanged();
        void loading();
        
    protected Q_SLOTS:
        void categoryActivated(QModelIndex index);
        void actionActivated(QModelIndex index);
        void synchRemoveRows(const QModelIndex &index, int start, int end);
    
    public Q_SLOTS:
        void addResults(QString requestSignature, QList<MediaItem> mediaList, MediaListProperties mediaListProperties, bool done, QString subRequestSignature);
        
};

#endif // MEDIAITEMMODEL_H
