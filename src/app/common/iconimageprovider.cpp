#include "iconimageprovider.h"

#include <KIcon>
#include <KDebug>

IconImageProvider::IconImageProvider(QDeclarativeImageProvider::ImageType type) :
    QDeclarativeImageProvider(type)
{
}

QPixmap IconImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString iconId = id;
    QIcon::Mode mode = QIcon::Normal;
    if (id.endsWith("/disabled")) {
        iconId = id.left(id.length() - 9);
        mode = QIcon::Disabled;
    }

    kDebug() << id << ":" << iconId;

    size->setHeight(128);
    size->setWidth(128);
    if (requestedSize.isValid()) {
        return KIcon(iconId).pixmap(requestedSize, mode);
    } else {
        return KIcon(iconId).pixmap(128, 128, mode);
    }
}
