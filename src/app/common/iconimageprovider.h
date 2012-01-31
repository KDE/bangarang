#ifndef ICONIMAGEPROVIDER_H
#define ICONIMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class IconImageProvider : public QDeclarativeImageProvider
{
public:
    IconImageProvider(QDeclarativeImageProvider::ImageType type);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // ICONIMAGEPROVIDER_H
