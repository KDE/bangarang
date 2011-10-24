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
#include "sha256.h"
#include "../mediaitemmodel.h"
#include "../mediavocabulary.h"
#include "../mediaquery.h"

#include <KUrl>
#include <KDebug>
#include <KLocale>
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

#include <QtCore>

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
    query.addCondition(QString("?r rdf:type <%1> . ").arg(mediaVocabulary.typeTVSeries().toString()));
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

QString Utilities::removeRangesFromString(const QString &str, QString begin, QString end)
{
    QString edited = str;
    int bPos = edited.indexOf(begin), ePos, endLen = end.length();
    while ( bPos >= 0 ) {
        ePos = edited.indexOf(end, bPos);
        if ( ePos <= bPos ) { //e.g. -1 if not found
            break;
        }
        edited = edited.remove(bPos, ePos - bPos + endLen); //including the end str
        bPos = edited.indexOf(begin);
    }
    return edited.trimmed();
}

QString Utilities::titleForRequest(const QString& title)
{
    QString edited = title, tmp;

    //Chop filename extension
    int extLen = edited.length() - edited.lastIndexOf(".");
    //if no "." is found extLen is greater than title.length()
    if ( extLen < 5 && extLen < edited.length() ) {
        edited.chop(extLen);
    }

    //Remove square brackets as filenames may contain information about the track in it
    // as [1080p;x286;AUD_en,de;SUB_en]
    tmp = Utilities::removeRangesFromString(edited, "[", "]");
    if ( !tmp.isEmpty() ) {
        edited = tmp;
    }

    //As this is only for requesting data we will also remove normal brackets
    //user like to store year, etc in it which is NOT the real title
    //if they are relevant for the fetching the user will see the choices anyway.
    tmp = Utilities::removeRangesFromString(edited, "(", ")");
    if ( !tmp.isEmpty() ) {
        edited = tmp;
    }

    //Replace underscores with spaces
    tmp = edited.replace('_', ' ').trimmed();
    if ( !tmp.isEmpty() ) {
        edited = tmp;
    }

    //Remove "the" from front of name
    if (edited.startsWith("the", Qt::CaseInsensitive) && KGlobal::locale()->language().startsWith("en")) {
        tmp = edited.mid(4).trimmed();
    }
    if ( !tmp.isEmpty() ) {
        edited = tmp;
    }

    return edited;
}

QString Utilities::wordsForTimeSince(const QDateTime &dateTime)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    if (!dateTime.isValid() && dateTime > currentDateTime) {
        return QString();
    }

    int secsSince = dateTime.secsTo(currentDateTime);
    int minutesSince = secsSince/60;
    int hoursSince = minutesSince/60;
    int daysSince = dateTime.daysTo(currentDateTime);
    int weeksSince = daysSince/7;
    int monthsSince = 0;
    int yearsSince = (daysSince < 366) ? 0 : currentDateTime.date().year() - dateTime.date().year();
    if (currentDateTime.date().month() != dateTime.date().month() &&
        weeksSince >= 4) {
        if (yearsSince == 0) {
            monthsSince = currentDateTime.date().month() - dateTime.date().month();
        } else {
            monthsSince = 12*yearsSince + (currentDateTime.date().month() - dateTime.date().month());
        }
    }

    QString words;
    if (yearsSince > 0) {
        words = i18np("a year ago", "%1 years ago", yearsSince);
    } else if (monthsSince > 0){
        words = i18np("a month ago", "%1 months ago", monthsSince);
    } else if (weeksSince > 0) {
        words = i18np("a week ago", "%1 weeks ago", weeksSince);
    } else if (daysSince > 0) {
        words = i18np("a day ago", "%1 days ago", daysSince);
    } else if (hoursSince > 0) {
        words = i18np("an hour ago", "%1 hours ago", hoursSince);
    } else if (minutesSince > 0) {
        words = i18np("a minute ago", "%1 minutes ago", minutesSince);
    } else {
        words = i18n("a few seconds ago");
    }
    return words;
}

QString Utilities::capitalize(const QString &text)
{
    QStringList capWords;
    QStringList words = text.split(" ");
    for (int i=0; i < words.count(); i++) {
        QString capWord = words.at(i).left(1).toUpper() + words.at(i).mid(1);
        capWords.append(capWord);
    }
    return capWords.join(" ");
}

QHash<QString, QStringList> Utilities::multiValueAppend(QHash<QString, QStringList> multiValues, QString key, QString newValue)
{
    QStringList multiValue = multiValues.value(key);
    if (!multiValue.contains(newValue)) {
        multiValue.append(newValue);
        multiValues.insert(key, multiValue);
    }
    return multiValues;
}

QString Utilities::durationString(int seconds)
{
    QTime durTime(seconds/(60*60), (seconds / 60) % 60, (seconds) % 60);
    int minutes = 0;
    int remSeconds = 0;
    minutes = durTime.hour()*60 + durTime.minute();
    remSeconds = durTime.second();
    QString displayTime;
    if (remSeconds < 10) {
        displayTime = QString("%1:0%2").arg(minutes).arg(remSeconds);
    } else {
        displayTime = QString("%1:%2").arg(minutes).arg(remSeconds);
    }
    return displayTime;
}

QString Utilities::sha256Of( QString in )
{
    // Copied from Amarok /src/services/ampache/AmpacheAccountLogin.cpp
    unsigned char digest[ SHA512_DIGEST_SIZE];
    unsigned char* toHash = (unsigned char*)in.toUtf8().data();

    sha256( toHash , qstrlen( ( char* )toHash ), digest );

    // this part copied from main() in sha256.cpp
    unsigned char output[2 * SHA512_DIGEST_SIZE + 1];
    int i;

    output[2 * SHA256_DIGEST_SIZE ] = '\0';

    for (i = 0; i < SHA256_DIGEST_SIZE ; i++) {
        sprintf((char *) output + 2*i, "%02x", digest[i]);
    }

    return QString::fromAscii( (const char*)output );
}
#endif //UTILITIES_GENERAL_CPP


