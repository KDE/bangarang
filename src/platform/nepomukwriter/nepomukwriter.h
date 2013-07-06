#ifndef NEPOMUKWRITER_H
#define NEPOMUKWRITER_H

#include "../mediavocabulary.h"
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Variant>
#include <Nepomuk2/Tag>
#include <KApplication>
#include <KUrl>
#include <QtCore/QFile>

class NepomukWriter : public QObject
{
    Q_OBJECT
public:
    enum MessageType {InfoRemoved = 1,
                      InfoUpdated = 2,
                      Progress = 3,
                      Message = 4,
                      Debug = 5,
                      Error = 6};
    explicit NepomukWriter(QObject *parent = 0);
    void processJob(QFile *jobFile);

signals:

public slots:

private:
    QHash<QString, KUrl> m_propertyResourceCache;
    void writeToNepomuk(QHash <QString, QVariant> fields);
    void removeInfo(QHash <QString, QVariant> fields);
    void updateInfo(QHash <QString, QVariant> fields);
    void removeType(Nepomuk2::Resource res, QUrl mediaType);
    void outputMessage(MessageType messageType, QString urlOrProgressOrMessage = QString());
    Nepomuk2::Resource findPropertyResourceByTitle(QUrl property, QString title, bool createIfMissing = false);
    void removeUnusedPropertyResources();
    QList<Nepomuk2::Variant> variantListFromStringList(const QStringList &stringList);

};

#endif // NEPOMUKWRITER_H
