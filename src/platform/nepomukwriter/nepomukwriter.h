#ifndef NEPOMUKWRITER_H
#define NEPOMUKWRITER_H

#include "../mediavocabulary.h"
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <KApplication>
#include <QtCore>
#include <QFile>

class NepomukWriter : public QObject
{
    Q_OBJECT
public:
    enum MessageType {InfoRemoved = 1,
                      InfoUpdated = 2,
                      Progress = 3,
                      Error = 4};
    explicit NepomukWriter(QObject *parent = 0);
    void processJob(QFile *jobFile);

signals:

public slots:

private:
    void writeToNepomuk(QHash <QString, QVariant> fields);
    void removeInfo(QHash <QString, QVariant> fields);
    void updateInfo(QHash <QString, QVariant> fields);
    void writeProperty(MediaVocabulary mediaVocabulary,
                       Nepomuk::Resource res, QUrl property, Nepomuk::Variant value);
    void removeType(Nepomuk::Resource res, QUrl mediaType);
    void outputMessage(MessageType messageType, QString urlOrProgressOrMessage = QString());

};

#endif // NEPOMUKWRITER_H
