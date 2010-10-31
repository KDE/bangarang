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

#ifndef UTILITIES_GENERAL_CPP
#define UTILITIES_GENERAL_CPP

#include "general.h"
#include "../mediaitemmodel.h"
#include "../mediavocabulary.h"
#include "../mediaquery.h"

#include <KUrl>
#include <KDebug>
#include <Solid/Device>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <Soprano/Model>
#include <Nepomuk/Resource>
#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Tag>

#include <phonon/backendcapabilities.h>
#include <phonon/MediaObject>

QString Utilities::mergeLRIs(const QString &lri, const QString &lriToMerge)
{
    QString mergedLRI;
    MediaListProperties targetProperties(lri);
    MediaListProperties sourceProperties(lriToMerge);
    if (targetProperties.engine() == sourceProperties.engine() && targetProperties.engineArg() == sourceProperties.engineArg()) {
        mergedLRI = targetProperties.engine() + targetProperties.engineArg() + QString("?");
        QStringList targetFilterList = targetProperties.engineFilterList();
        QStringList sourceFilterList = sourceProperties.engineFilterList();
        QString mergedFilter;
        for (int i = 0; i < targetFilterList.count(); i++) {
            QString targetFilter = targetFilterList.at(i);
            QString field = targetProperties.filterField(targetFilter);
            QString sourceFilter = sourceProperties.filterForField(field);
            if (sourceFilter.isEmpty() || sourceFilter == targetFilter) {
                mergedFilter = targetFilter;
            } else if (!sourceFilter.isEmpty()) {
                mergedFilter = field + targetProperties.filterOperator(targetFilter) + targetProperties.filterValue(targetFilter) + QString("|OR|") + sourceProperties.filterValue(sourceFilter);
            }
            if (!mergedFilter.isEmpty()) {
                mergedLRI += QString("%1||").arg(mergedFilter);
            }
        }
        MediaListProperties mergedProperties(mergedLRI);
        mergedFilter = QString();
        for (int i = 0; i < sourceFilterList.count(); i++) {
            QString sourceFilter = sourceFilterList.at(i);
            QString field = sourceProperties.filterField(sourceFilter);
            if (mergedProperties.filterForField(field).isEmpty() && mergedProperties.engineFilterList().indexOf(sourceFilter) == -1) {
                mergedFilter = sourceFilter;
            }
            if (!mergedFilter.isEmpty()) {
                mergedLRI += QString("%1||").arg(mergedFilter);
            }
        }
    }
    return mergedLRI;
}

QUrl Utilities::artistResource(const QString &artistName)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append("r");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(QString("{?pr <%1> ?r. } UNION {?pr <%2> ?r . } UNION {?pr <%3> ?r . } ")
                       .arg(mediaVocabulary.musicArtist().toString())
                       .arg(mediaVocabulary.musicPerformer().toString())
                       .arg(mediaVocabulary.musicComposer().toString()));
    query.addCondition(QString("?r <%1> ?name . ").arg(mediaVocabulary.ncoFullname().toString()));
    query.startFilter();
    query.addFilterConstraint("name", artistName, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();

    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);

    QUrl resource;
    while (it.next()) {
        resource = it.binding("r").uri();
    }
    return resource;
}

QUrl Utilities::albumResource(const QString &albumName)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append("r");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(QString("?r rdf:type <%1> . ").arg(mediaVocabulary.typeMusicAlbum().toString()));
    query.addCondition(QString("?r <%1> ?name . ").arg(mediaVocabulary.musicAlbumName().toString()));
    query.startFilter();
    query.addFilterConstraint("name", albumName, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();

    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);

    QUrl resource;
    while (it.next()) {
        resource = it.binding("r").uri();
    }
    return resource;
}

QUrl Utilities::TVSeriesResource(const QString &seriesName)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append("r");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(QString("?r rdf:type %1 . ").arg(mediaVocabulary.typeTVSeries().toString()));
    query.addCondition(QString("?r <%1> ?name . ").arg(mediaVocabulary.videoSeriesTitle().toString()));
    query.startFilter();
    query.addFilterConstraint("name", seriesName, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();

    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);

    QUrl resource;
    while (it.next()) {
        resource = it.binding("r").uri();
    }
    return resource;
}

QUrl Utilities::actorResource(const QString &actorName)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append("r");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(QString("?pr <%1> ?r . ").arg(mediaVocabulary.videoActor().toString()));
    query.addCondition(QString("?r <%1> ?name . ").arg(mediaVocabulary.ncoFullname().toString()));
    query.startFilter();
    query.addFilterConstraint("name", actorName, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();

    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);

    QUrl resource;
    while (it.next()) {
        resource = it.binding("r").uri();
    }
    return resource;
}

QUrl Utilities::directorResource(const QString &directorName)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    MediaQuery query;
    QStringList bindings;
    bindings.append("r");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(QString("?pr <%1> ?r . ").arg(mediaVocabulary.videoDirector().toString()));
    query.addCondition(QString("?r <%1> ?name . ").arg(mediaVocabulary.ncoFullname().toString()));
    query.startFilter();
    query.addFilterConstraint("name", directorName, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();

    Soprano::Model * mainModel = Nepomuk::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = query.executeSelect(mainModel);

    QUrl resource;
    while (it.next()) {
        resource = it.binding("r").uri();
    }
    return resource;
}


KUrl Utilities::deviceUrl(const QString &type, const QString& udi, const QString& name, QString content, int title )
{
    KUrl url = QString("device://%1%2").arg(type, udi);
    QString query;
    if (!name.isEmpty())
        query += QString("?name=%1").arg(name);
    if (!content.isEmpty()) {
        if ( query.isEmpty() )
            query = "?";
        else
            query += "&";
        query += QString("content=%1").arg(content);
    }
    if (!query.isEmpty())
        url.setQuery(query);
    if (title != invalidTitle())
        url.setFragment(QString("%1").arg(title));
    return url;
}

QString Utilities::deviceNameFromUrl(const KUrl& url)
{
    return url.queryItemValue("name");
}

int Utilities::deviceTitleFromUrl(const KUrl& url)
{
    if (!url.hasFragment())
        return invalidTitle();
    bool ok = false;
    int title = url.fragment().toInt(&ok, 0);
    return ok ? title : invalidTitle();
}

QString Utilities::deviceUdiFromUrl(const KUrl& url)
{
    return url.path();
}

int Utilities::invalidTitle()
{
    return -1;
}

QString Utilities::deviceName(QString udi, Phonon::MediaObject *mobj)
{
    QString name;
    const Solid::OpticalDisc *disc = Solid::Device( udi ).as<const Solid::OpticalDisc>();
    if ( disc != NULL )
        name = disc->label();
    if ( !name.isEmpty() || mobj == NULL)
        return name;
    else if (!mobj->metaData("TITLE").isEmpty())
        return mobj->metaData("TITLE").join("");
    else
        return QString();
}

QStringList Utilities::availableDiscUdis(Solid::OpticalDisc::ContentType type)
{
    QStringList udis;
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString()))
    {
        const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc>();
        if (disc != NULL && disc->availableContent() & type)
            udis << device.udi();
    }
    return udis;
}

bool Utilities::nepomukInited()
{
    bool nepomukInited = Nepomuk::ResourceManager::instance()->initialized();
    if (!nepomukInited) {
        Nepomuk::ResourceManager::instance()->init();
        nepomukInited = Nepomuk::ResourceManager::instance()->initialized();
    }
    return nepomukInited;
}

QStringList Utilities::cleanStringList(QStringList stringList)
{
    QStringList returnList;
    stringList.removeDuplicates();
    for (int i = 0; i < stringList.count(); i++) {
        QString string = stringList.at(i);
        if (!string.isEmpty()) {
            returnList.append(string);
        }
    }
    return returnList;
}

#endif //UTILITIES_GENERAL_CPP


