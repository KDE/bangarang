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

#include "filenameinfofetcher.h"
#include "../mediaitemmodel.h"
#include "../utilities/utilities.h"

#include <KIcon>
#include <KLocale>

#include <QtCore>

FileNameInfoFetcher::FileNameInfoFetcher(QObject *parent) :
        InfoFetcher(parent)
{
    m_name = i18n("Filename Info");
    m_icon = KIcon("quickopen-file");
    m_about = i18n("This fetcher uses guesses the season and episode number based on a filename pattern. E.g.  2x23 or S02E23 or 2.23");

    //Define fetchable fields
    m_fetchableFields["TV Show"] = QStringList() << "season" << "episode";

    //Define required fields
    m_requiredFields["TV Show"] = QStringList() << "url";
}

FileNameInfoFetcher::~FileNameInfoFetcher()
{
}

bool FileNameInfoFetcher::available(const QString &subType)
{
    bool handlesType = (m_requiredFields[subType].count() > 0);
    return (handlesType);
}

void FileNameInfoFetcher::fetchInfo(QList<MediaItem> mediaList, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    /*
    *   Attempts to guess metadata from MediaItem's filename
    *   It currently only works for season and episode numbers for TV shows
    *   but could potentially be expanded
    *   it's not really necessary for music files as the indexer gets metadata from the files themselves
    *   (Contributed by: Miha Čančula)
    *   TODO: Add more search patterns to cover things like "SeasonXXEpisodeYY", etc.
    */
    Q_UNUSED(maxMatches);
    QList<MediaItem> updatedItems;
    setFetching();
    for (int i = 0; i < mediaList.size(); ++i)
    {
        bool updated = false;
        MediaItem item = mediaList.at(i);
        if (item.type == "Video" && item.fields["videoType"] == "TV Show")
        {
            QString fileName = KUrl(item.fields["url"].toString()).fileName();
            // If the url is empty or poinst to to nepomuk's location,
            // we can't get the filname that way.
            // So we use the title as a fall-back
            if (fileName.isEmpty() || fileName.startsWith("nepomuk:/"))
            {
                fileName = item.fields["title"].toString();
            }
            // correctly parses names like 2x23 or S02E23 or 2.23 or 2/23
            // \d = digit, \D = non-digit
            QRegExp seasonEpisodeRegex("(\\d+)\\D(\\d+)");
            if (seasonEpisodeRegex.indexIn(fileName) > -1) {
                item.fields["season"] = seasonEpisodeRegex.cap(1).toInt();
                item.fields["episodeNumber"] = seasonEpisodeRegex.cap(2).toInt();
                updated = true;
            } else {
                // for notations like 224 for episode 23 of season 2
                // some shows might have more than 10 seasons, but I am unaware of any with more than 100 episodes per season
                QRegExp noSeparatorRegex("(\\d{3,4})");
                if (noSeparatorRegex.indexIn(fileName) > -1) {
                    QString match = noSeparatorRegex.cap(1);
                    item.fields["episodeNumber"] = match.right(2).toInt();
                    match.chop(2);
                    item.fields["season"] = match.toInt();
                    updated = true;
                }
            }
            item = Utilities::makeSubtitle(item);
        }
        if (updated) {
            QList<MediaItem> matches;
            matches.append(item);
            emit infoFetched(matches);
        }
    }
    if (!m_timeout) {
        emit fetchComplete();
        emit fetchComplete(this);
    }
    Q_UNUSED(updateRequiredFields);
    Q_UNUSED(updateArtwork);
}
