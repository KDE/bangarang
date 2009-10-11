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

#include "mediaitemmodel.h"
#include "savedlistsengine.h"
#include "listenginefactory.h"
#include "utilities.h"
#include "mediavocabulary.h"

#include <KIcon>
#include <KMimeType>
#include <KStandardDirs>
#include <KUrl>
#include <nepomuk/resource.h>
#include <nepomuk/variant.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <id3v2tag.h>

SavedListsEngine::SavedListsEngine(ListEngineFactory * parent) : ListEngine(parent)
{
    m_parent = parent;
    
    
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        //resource manager inited successfully
    } else {
        //no resource manager
    };
    
    m_requestSignature = QString();
    m_subRequestSignature = QString();

}

SavedListsEngine::~SavedListsEngine()
{
}

void SavedListsEngine::run()
{
    QList<MediaItem> mediaList;
    
    if (!m_mediaListProperties.engineArg().isEmpty()) {
        QFile file(KStandardDirs::locateLocal("data", QString("bangarang/%1").arg(m_mediaListProperties.engineArg()), false));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        
        //Make sure it's a valid M3U fileref
        QTextStream in(&file);
        bool valid = false;
        if (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed() == "#EXTM3U") {
                valid = true;
            }
        }
        
        //Create a MediaItem for each entry
        if (valid) {
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("#EXTINF:")) {
                    line = line.replace("#EXTINF:","");
                    QStringList durTitle = line.split(",");
                    QString title;
                    int duration;
                    if (durTitle.count() == 1) {
                        //No title
                        duration = 0;
                        title = durTitle.at(0);
                    } else {
                        duration = durTitle.at(0).toInt();
                        title = durTitle.at(1);
                    }
                    QString url = in.readLine().trimmed();
                    MediaItem mediaItem;
                    if (!url.isEmpty()) {
                        mediaItem = Utilities::mediaItemFromUrl(KUrl(url));
                    } else {
                        continue;
                    }
                    if ((!mediaItem.title.isEmpty()) && (url.contains(mediaItem.title))) {
                        mediaItem.title = title;
                    }
                    if ((duration > 0) && (mediaItem.fields["duration"].toInt() <= 0)) {
                        mediaItem.duration = QTime(0,0,0,0).addSecs(duration).toString("m:ss");
                        mediaItem.fields["duration"] = duration;
                    }
                    mediaList << mediaItem;
                }
            }
        }
        
        /*MediaItem mediaItem;
        mediaItem.url = "-";
        mediaItem.title = QString("Saved List Engine test").arg(m_mediaListProperties.engineArg());
        mediaItem.type = "Audio";
        mediaList << mediaItem;*/
        
        
        model()->addResults(m_requestSignature, mediaList, m_mediaListProperties, true, m_subRequestSignature);
        m_requestSignature = QString();
        m_subRequestSignature = QString();
    }
    //exec();    
}

void SavedListsEngine::setMediaListProperties(MediaListProperties mediaListProperties)
{
    m_mediaListProperties = mediaListProperties;
}

MediaListProperties SavedListsEngine::mediaListProperties()
{
    return m_mediaListProperties;
}

void SavedListsEngine::setFilterForSources(QString engineFilter)
{
    Q_UNUSED(engineFilter);
}

void SavedListsEngine::setRequestSignature(QString requestSignature)
{
    m_requestSignature = requestSignature;
}

void SavedListsEngine::setSubRequestSignature(QString subRequestSignature)
{
    m_subRequestSignature = subRequestSignature;
}

void SavedListsEngine::activateAction()
{
        
}

