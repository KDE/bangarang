#include "mediaitemmodelimageprovider.h"
#include "mediaitemmodel.h"
#include "utilities/artwork.h"

#include <KDebug>

MediaItemModelImageProvider::MediaItemModelImageProvider(QDeclarativeImageProvider::ImageType type) :
    QDeclarativeImageProvider(type)
{
    m_id = "imageProvider";
    m_model = 0;
}

void MediaItemModelImageProvider::setModel(MediaItemModel *model)
{
    m_model = model;
}

QImage MediaItemModelImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    return requestPixmap(id, size, requestedSize).toImage();
}

QPixmap MediaItemModelImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap pixmap;
    bool isReflection = false;
    QString finalId = id;
    if (id.endsWith("/reflection")) {
        isReflection = true;
        finalId = id.left(id.length() - 11);
    }
    int index = m_artworkIds.indexOf(finalId);
    kDebug() << QString("%1:%2").arg(index).arg(finalId);
    kDebug() << m_artworkIds;
    if (index >= 0 && index < m_model->rowCount()) {
        pixmap = m_model->mediaItemAt(index).artwork.pixmap(256, 256);
    } else {
        pixmap = QPixmap(256, 256);
        pixmap.fill(Qt::transparent);
    }

    if (isReflection) {
        pixmap = Utilities::reflection(pixmap);
    }
    size->setHeight(256);
    size->setWidth(256);
    if (requestedSize.isValid()) {
        return pixmap.scaled(requestedSize);
    } else {
        return pixmap;
    }
}

void MediaItemModelImageProvider::setId(const QString &id)
{
    m_id = id;
}

QString MediaItemModelImageProvider::id()
{
    return m_id;
}

QString MediaItemModelImageProvider::artworkIdForMediaItem(const MediaItem &mediaItem)
{
    return QString("%2:%3:%4").arg(mediaItem.url)
            .arg(mediaItem.subType())
            .arg(mediaItem.type);
}

QString MediaItemModelImageProvider::artworkUriForMediaItem(const MediaItem &mediaItem) {
    return QString("image://%1/%2").arg(this->id()).arg(artworkIdForMediaItem(mediaItem));
}

QList<QString> * MediaItemModelImageProvider::artworkIds()
{
    return &m_artworkIds;
}


