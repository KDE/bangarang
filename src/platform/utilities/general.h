/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef UTILITIES_GENERAL_H
#define UTILITIES_GENERAL_H

#include <KUrl>
#include <phonon/Global>
#include <solid/opticaldisc.h>
#include <QtCore>
#include <QMutex>

namespace Phonon {
class MediaObject;
}

/**
* @def SAVE_DELETE_OBJ
* This macro deletes an object only if the pointer isn't NULL and sets it NULL after deletion
*/
//note that the do/while loop is only inserted as the macro should look as a function, terminated
//with ; (semicolon) which wouldn't be possible otherwise. The compiler will optimize it any way
//so it doesn't influence the speed
#define SAVE_DELETE_OBJ(obj) \
  do { \
    if (obj != NULL) { \
        delete obj; \
        obj = NULL; \
    } \
  } while( false )

/**
 * This namespace provides a list of convenience functions
 * used throughout bangarang.
 */
namespace Utilities {
    static QMutex mutex;
    QString mergeLRIs(const QString &lri, const QString &lriToMerge);
    QUrl artistResource(const QString &artistName);
    QUrl albumResource(const QString &albumName);
    QUrl TVSeriesResource(const QString &seriesName);
    QUrl actorResource(const QString &actorName);
    QUrl directorResource(const QString &directorName);
    KUrl deviceUrl(const QString &type, const QString &udi, const QString& name = QString(), QString content = QString(), int title = -1 );
    int deviceTitleFromUrl(const KUrl &url);
    QString deviceUdiFromUrl(const KUrl &url);
    QString deviceNameFromUrl(const KUrl &url);
    int invalidTitle();
    QString deviceName( QString udi, Phonon::MediaObject *mobj = NULL );
    bool nepomukInited();
    QStringList cleanStringList(QStringList stringList);
    QString removeRangesFromString(const QString& str, QString begin, QString end);
    QString titleForRequest(const QString &title);
    QString wordsForTimeSince(const QDateTime & dateTime);
    QString capitalize(const QString & text);
    QHash<QString, QStringList> multiValueAppend(QHash<QString, QStringList> multiValues, QString key, QString newValue);
    QString durationString(int seconds);
    QString sha256Of(QString in);
}
#endif //UTILITIES_GENERAL_H
