#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class MediaItem;
class MediaItemModel;

class MediaItemModelImageProvider : public QDeclarativeImageProvider
{
public:
    explicit MediaItemModelImageProvider(QDeclarativeImageProvider::ImageType type);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
    void setModel(MediaItemModel *model);
    void setId(const QString &id);
    QString id();
    QString artworkIdForMediaItem(const MediaItem &mediaItem);
    QString artworkUriForMediaItem(const MediaItem &mediaItem);
    QList<QString> * artworkIds();

private:
    QString m_id;
    MediaItemModel * m_model;
    QList<QString> m_artworkIds;

};

#endif // IMAGEPROVIDER_H
