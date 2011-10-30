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

#include "infofetcher.h"
#include "../mediaitemmodel.h"
#include "../utilities/utilities.h"
#include <KIcon>
#include <KDebug>

InfoFetcher::InfoFetcher(QObject * parent) : QObject(parent)
{
    m_name = QString();
    m_icon = KIcon("run-build");
    m_url = KUrl();
    m_isFetching = false;
    m_timeout = false;
    m_timeoutLength = 6000;
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

}

InfoFetcher::~InfoFetcher()
{
}

QString InfoFetcher::name()
{
    return m_name;
}

QIcon InfoFetcher::icon()
{
    return m_icon;
}

KUrl InfoFetcher::url()
{
    return m_url;
}

QString InfoFetcher::about()
{
    return m_about;
}

bool InfoFetcher::hasMultipleValues(const QString &field)
{
    QVariant value;
    
    if (field == "artworkUrl") {
        if (m_mediaList.count() == 1) {
            return false;
        } else {
            return true;
        }
    }

    for (int i = 0; i < m_mediaList.count(); i++) {
        if (value.isNull()) {
            value = m_mediaList.at(i).fields.value(field);
        } else if (m_mediaList.at(i).fields.value(field) != value) {
            return true;
        }
    }
    return false;
}

QVariant InfoFetcher::commonValue(const QString &field)
{
    QVariant value;
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).fields.contains(field)) {
            if (value.isNull()) {
                value = m_mediaList.at(i).fields.value(field);
            } else if (m_mediaList.at(i).fields.value(field) != value) {
                value = QVariant();
                break;
            }
        }
    }
        return value;
}
            
            
QStringList InfoFetcher::valueList(const QString &field)
{
    QStringList value;
    value << QString();
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).fields.contains(field)) {
            if (value.indexOf(m_mediaList.at(i).fields.value(field).toString()) == -1) {
                value << m_mediaList.at(i).fields.value(field).toString();
            }
        }
    }
    return value;   
}

void InfoFetcher::setFetching()
{
    m_isFetching = true;
    m_timeout = false;
    m_timer->start(m_timeoutLength);
    emit fetching();
}

void InfoFetcher::timeout()
{
    kDebug() << "TIMEOUT";
    m_timeout = true;
    m_isFetching = false;
    emit fetchComplete();
    emit fetchComplete(this);
}

