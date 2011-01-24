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

#include "infoitemmodel.h"
#include "mediaitemmodel.h"
#include "utilities/utilities.h"
#include "infofetchers/dbpediainfofetcher.h"
#include "infofetchers/feedinfofetcher.h"
#include "infofetchers/filenameinfofetcher.h"
#include "infofetchers/lastfminfofetcher.h"
#include "infofetchers/tmdbinfofetcher.h"
#include "infofetchers/tvdbinfofetcher.h"
#include "mediaindexer.h"
#include <KLocale>
#include <KDebug>
#include <KUrl>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <QApplication>
#include <QPainter>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

InfoItemModel::InfoItemModel(QObject *parent) : QStandardItemModel(parent)
{
    connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));

    m_defaultEditable = true;
    m_modified = false;
    m_suppressFetchOnLoad = false;
    m_fetchingStatus["progress"] = -1;
    m_utilThread = new Utilities::Thread(this);
    connect(m_utilThread, SIGNAL(gotArtworks(QList<QImage>,MediaItem)), this, SLOT(gotArtworks(QList<QImage>,MediaItem)));

    //Store field order
    m_fieldsOrder["Music"] = QStringList() << "audioType" << "artwork" << "title" << "rating" << "artist" << "composer" << "album" << "trackNumber" << "year" << "genre" << "description" << "tags" << "relatedTo" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Audio Clip"] = QStringList() << "audioType" << "artwork" << "title" << "rating" << "description" << "tags" << "relatedTo" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Audio Stream"] = QStringList() << "audioType" << "artwork" << "title" << "rating" << "description" << "tags" << "relatedTo" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Video Clip"] = QStringList() << "videoType" << "artwork" << "title" << "rating" << "description" << "tags" << "relatedTo" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Movie"] = QStringList() << "videoType" << "artwork" << "title" << "rating" << "description" << "actor" << "director" << "writer" << "producer" << "year" << "genre" << "tags" << "relatedTo" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["TV Show"] = QStringList() << "videoType" << "artwork" << "title" << "rating" << "description" << "seriesName" << "actor" << "director" << "writer" << "producer" << "season" << "episodeNumber" << "year" << "genre" << "tags" << "relatedTo" << "url" << "playCount" << "lastPlayed";
    m_fieldsOrder["Artist"] = QStringList() << "artwork" << "title" << "description" << "relatedTo";
    m_fieldsOrder["Album"] = QStringList() << "artwork" << "title";
    m_fieldsOrder["AudioGenre"] = QStringList() << "artwork" << "title";
    m_fieldsOrder["AudioTag"] = QStringList() << "title";
    m_fieldsOrder["TV Series"] = QStringList() << "artwork" << "title" << "description";
    m_fieldsOrder["TV Season"] = QStringList() << "artwork" << "title" << "season";
    m_fieldsOrder["VideoGenre"] = QStringList() << "artwork" << "title";
    m_fieldsOrder["Actor"] = QStringList() << "artwork" << "title" << "description" << "relatedTo";
    m_fieldsOrder["Director"] = QStringList() << "artwork" << "title" << "description" << "relatedTo";
    m_fieldsOrder["VideoTag"] = QStringList() << "title";
    m_fieldsOrder["Audio Feed"] = QStringList() << "artwork" << "title" << "description" << "url";
    m_fieldsOrder["Video Feed"] = QStringList() << "artwork" << "title" << "description" << "url";
    m_fieldsOrder["Basic"] = QStringList() << "title";
    m_fieldsOrder["Basic+Artwork"] = QStringList() << "artwork" << "title";

    //Store field names
    m_fieldNames["audioType"] = i18n("Type");
    m_fieldNames["artwork"] = i18n("Artwork");
    m_fieldNames["title"]= i18n("Title");
    m_fieldNames["artist"] = i18n("Artist");
    m_fieldNames["composer"] = i18n("Composer");
    m_fieldNames["album"] = i18n("Album");
    m_fieldNames["trackNumber"] = i18n("Track");
    m_fieldNames["year"] = i18n("Year");
    m_fieldNames["genre"] = i18n("Genre");
    m_fieldNames["tags"] = i18n("Tags");
    m_fieldNames["url"] = i18n("Location");
    m_fieldNames["videoType"] = i18n("Type");
    m_fieldNames["actor"] = i18n("Actor");
    m_fieldNames["director"] = i18n("Director");
    m_fieldNames["writer"] = i18n("Writer");
    m_fieldNames["producer"] = i18n("Producer");
    m_fieldNames["seriesName"] = i18n("Series");
    m_fieldNames["season"] = i18n("Season");
    m_fieldNames["episodeNumber"] = i18n("Episode");
    m_fieldNames["relatedTo"] = i18n("Links");
    m_fieldNames["playCount"] = i18n("Play Count");
    m_fieldNames["lastPlayed"] = i18n("Last Played");

    //Store restricted fields
    m_restrictedFields["Music"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Audio Clip"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Audio Stream"] = QStringList() << "playCount" << "lastPlayed";
    m_restrictedFields["Video Clip"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Movie"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["TV Show"] = QStringList() << "url" << "playCount" << "lastPlayed";
    m_restrictedFields["Artist"] = QStringList() << "title";
    m_restrictedFields["Album"] = QStringList() << "artwork" << "title";
    m_restrictedFields["AudioGenre"] = QStringList() << "title";
    m_restrictedFields["AudioTag"] = QStringList() << "artwork" << "title";
    m_restrictedFields["TV Series"] = QStringList() << "title";
    m_restrictedFields["TV Season"] = QStringList() << "artwork" << "title" << "season";
    m_restrictedFields["VideoGenre"] = QStringList() << "title";
    m_restrictedFields["Actor"] = QStringList() << "title";
    m_restrictedFields["Director"] = QStringList() << "title";
    m_restrictedFields["VideoTag"] = QStringList() << "artwork" << "title";
    m_restrictedFields["Basic"] = QStringList() << "title";
    m_restrictedFields["Basic+Artwork"] = QStringList() << "artwork" << "title";

    m_drillLris["Artist"] = "music://albums?artist=%1";
    m_drillLris["Album"] = "music://songs?album=%1";
    m_drillLris["AudioGenre"] = "music://artists?genre=%1";
    m_drillLris["VideoGenre"] = "video://sources?||genre=%1";
    m_drillLris["AudioTag"] = "tag://audio?tag=%1";
    m_drillLris["VideoTag"] = "tag://video?tag=%1";
    m_drillLris["TV Series"] = "video://seasons?||seriesName=%1";
    m_drillLris["Actor"] = "video://sources?||actor=%1";
    m_drillLris["Director"] = "video://sources?||director=%1";

    //Set up InfoFetchers
    TMDBInfoFetcher * tmdbInfoFetcher = new TMDBInfoFetcher(this);
    connect(tmdbInfoFetcher, SIGNAL(infoFetched(QList<MediaItem>)), this, SLOT(infoFetched(QList<MediaItem>)));
    connect(tmdbInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(tmdbInfoFetcher, SIGNAL(fetchComplete(InfoFetcher *)), this, SLOT(infoFetcherComplete(InfoFetcher *)));
    connect(tmdbInfoFetcher, SIGNAL(noResults(InfoFetcher *)), this, SLOT(noResults(InfoFetcher *)));
    connect(tmdbInfoFetcher, SIGNAL(updateFetchedInfo(int,MediaItem)), this, SLOT(updateFetchedInfo(int,MediaItem)));
    m_infoFetchers.append(tmdbInfoFetcher);

    TVDBInfoFetcher * tvdbInfoFetcher = new TVDBInfoFetcher(this);
    connect(tvdbInfoFetcher, SIGNAL(infoFetched(QList<MediaItem>)), this, SLOT(infoFetched(QList<MediaItem>)));
    connect(tvdbInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(tvdbInfoFetcher, SIGNAL(fetchComplete(InfoFetcher *)), this, SLOT(infoFetcherComplete(InfoFetcher *)));
    connect(tvdbInfoFetcher, SIGNAL(noResults(InfoFetcher *)), this, SLOT(noResults(InfoFetcher *)));
    connect(tvdbInfoFetcher, SIGNAL(updateFetchedInfo(int,MediaItem)), this, SLOT(updateFetchedInfo(int,MediaItem)));
    m_infoFetchers.append(tvdbInfoFetcher);

    LastfmInfoFetcher * lastfmInfoFetcher = new LastfmInfoFetcher(this);
    connect(lastfmInfoFetcher, SIGNAL(infoFetched(QList<MediaItem>)), this, SLOT(infoFetched(QList<MediaItem>)));
    connect(lastfmInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(lastfmInfoFetcher, SIGNAL(fetchComplete(InfoFetcher *)), this, SLOT(infoFetcherComplete(InfoFetcher *)));
    connect(lastfmInfoFetcher, SIGNAL(noResults(InfoFetcher *)), this, SLOT(noResults(InfoFetcher *)));
    connect(lastfmInfoFetcher, SIGNAL(updateFetchedInfo(int,MediaItem)), this, SLOT(updateFetchedInfo(int,MediaItem)));
    m_infoFetchers.append(lastfmInfoFetcher);

    /* NOTE: Results returned from DBPedia SPARQL frontend is inconsistent
             InfoFetcher is disabled until this problem is resolved
    DBPediaInfoFetcher * dbPediaInfoFetcher = new DBPediaInfoFetcher(this);
    connect(dbPediaInfoFetcher, SIGNAL(infoFetched(QList<MediaItem>)), this, SLOT(infoFetched(QList<MediaItem>)));
    connect(dbPediaInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(dbPediaInfoFetcher, SIGNAL(fetchComplete(InfoFetcher *)), this, SLOT(infoFetcherComplete(InfoFetcher *)));
    connect(dbPediaInfoFetcher, SIGNAL(noResults(InfoFetcher *)), this, SLOT(noResults(InfoFetcher *)));
    connect(dbPediaInfoFetcher, SIGNAL(updateFetchedInfo(int,MediaItem)), this, SLOT(updateFetchedInfo(int,MediaItem)));
    m_infoFetchers.append(dbPediaInfoFetcher);*/

    FeedInfoFetcher * feedInfoFetcher = new FeedInfoFetcher(this);
    connect(feedInfoFetcher, SIGNAL(infoFetched(QList<MediaItem>)), this, SLOT(infoFetched(QList<MediaItem>)));
    connect(feedInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(feedInfoFetcher, SIGNAL(fetchComplete(InfoFetcher *)), this, SLOT(infoFetcherComplete(InfoFetcher *)));
    connect(feedInfoFetcher, SIGNAL(noResults(InfoFetcher *)), this, SLOT(noResults(InfoFetcher *)));
    connect(feedInfoFetcher, SIGNAL(updateFetchedInfo(int,MediaItem)), this, SLOT(updateFetchedInfo(int,MediaItem)));
    m_infoFetchers.append(feedInfoFetcher);

    FileNameInfoFetcher * fileNameInfoFetcher = new FileNameInfoFetcher(this);
    connect(fileNameInfoFetcher, SIGNAL(infoFetched(QList<MediaItem>)), this, SLOT(infoFetched(QList<MediaItem>)));
    connect(fileNameInfoFetcher, SIGNAL(fetching()), this, SIGNAL(fetching()));
    connect(fileNameInfoFetcher, SIGNAL(fetchComplete(InfoFetcher *)), this, SLOT(infoFetcherComplete(InfoFetcher *)));
    connect(fileNameInfoFetcher, SIGNAL(noResults(InfoFetcher *)), this, SLOT(noResults(InfoFetcher *)));
    connect(fileNameInfoFetcher, SIGNAL(updateFetchedInfo(int,MediaItem)), this, SLOT(updateFetchedInfo(int,MediaItem)));
    m_infoFetchers.append(fileNameInfoFetcher);

    m_selectedFetchedMatch = -1;

    //Setup indexer
    m_indexer = new MediaIndexer(this);
}

InfoItemModel::~InfoItemModel()
{
}

void InfoItemModel::loadInfo(const QList<MediaItem> & mediaList) 
{
    m_modified = false;
    m_mediaList = mediaList;
    m_originalList = mediaList;
    m_fetchedMatches.clear();
    cancelFetching();
    
    if (m_mediaList.count() > 0) {
        Utilities::removeFromImageCache(m_mediaList.at(0));
        loadFieldsInOrder();
        emit infoChanged(false);

        //Upon selection of only one media item, launch Autofetch if NO info
        //is available in the fetchable fields of the media item.
        //NOTE: Automatic autofetching is more intrusive to the user experience than I'd like.
        //      So it is disabled below. For now I'd rather leave it to the user to decide
        //      when to fetch info.
        /*if (m_mediaList.count() == 1 && !m_suppressFetchOnLoad) {
            for (int i = 0; i < m_infoFetchers.count(); i++) {
                if (autoFetchIsAvailable(m_infoFetchers.at(i))) {
                    QStringList fetchableFields = m_infoFetchers.at(i)->fetchableFields(subType);
                    QStringList requiredFields = m_infoFetchers.at(i)->requiredFields(subType);
                    bool fetchableFieldsEmpty = true;
                    for (int j = 0; j < fetchableFields.count(); j++) {
                        if (!requiredFields.contains(fetchableFields.at(j)) &&
                            !isEmpty(fetchableFields.at(j))) {
                            fetchableFieldsEmpty = false;
                        }
                    }
                    if (fetchableFieldsEmpty) {
                        autoFetch(m_infoFetchers.at(i), false, false);
                        break;
                    }
                }
            }
        }*/
    }

    //Fetch on load can only be suppressed for one load
    m_suppressFetchOnLoad = false;
}

void InfoItemModel::saveChanges()
{
    //Update Custom Genre Info
    saveCustomGenreInfo(m_mediaList);
    
    //Update File Metadata
    saveFileMetaData(m_mediaList);
    
    //Update source information
    m_sourceModel->updateSourceInfo(m_mediaList);
    
    //Ensure original values in model are updated to reflect saved(no-edits) state
    loadInfo(m_mediaList); 
}

void InfoItemModel::cancelChanges()
{
    m_suppressFetchOnLoad = true;
    loadInfo(m_originalList);
}

QList<MediaItem> InfoItemModel::mediaList()
{
    return m_mediaList;
}

void InfoItemModel::setSourceModel(MediaItemModel * sourceModel)
{
    m_sourceModel = sourceModel;
}

QHash<QString, QVariant> InfoItemModel::fetchingStatus()
{
    return m_fetchingStatus;
}

QList<InfoFetcher *> InfoItemModel::infoFetchers()
{
    return m_infoFetchers;
}

QList<InfoFetcher *> InfoItemModel::availableInfoFetchers()
{
    QList<InfoFetcher *> availableFetchers;
    for (int i = 0; i < m_infoFetchers.count(); i++) {
        if (autoFetchIsAvailable(m_infoFetchers.at(i)) || fetchIsAvailable(m_infoFetchers.at(i))) {
            availableFetchers.append(m_infoFetchers.at(i));
        }
    }
    return availableFetchers;
}

bool InfoItemModel::autoFetchIsAvailable(InfoFetcher* infoFetcher)
{
    //Autofetch is only available when required info is available to make
    //fetch request AND there are no unsaved modifications.
    bool available = false;
    if (m_mediaList.count() == 0) {
        return available;
    }
    QString subType = m_mediaList.at(0).subType();
    if (!m_modified && infoFetcher->available(subType)) {
        QStringList requiredFields = infoFetcher->requiredFields(subType);
        available = true;
        for (int i =0; i < requiredFields.count(); i++) {
            if (isEmpty(requiredFields.at(i))) {
                available = false;
                break;
            }
        }
    }
    return available;
}

bool InfoItemModel::fetchIsAvailable(InfoFetcher* infoFetcher)
{
    //Fetch is only available when enough info is available to make
    //fetch request AND when only one media item is loaded.
    bool available = false;
    if (m_mediaList.count() == 0) {
        return available;
    }
    QString subType = m_mediaList.at(0).subType();
    if (m_mediaList.count() == 1  && infoFetcher->available(subType)) {
        QStringList requiredFields = infoFetcher->requiredFields(subType);
        available = true;
        for (int i =0; i < requiredFields.count(); i++) {
            if (isEmpty(requiredFields.at(i))) {
                available = false;
                break;
            }
        }
    }
    return available;
}

void InfoItemModel::autoFetch(InfoFetcher* infoFetcher, bool updateRequiredFields, bool updateArtwork)
{
    m_fetchType = AutoFetch;
    m_fetchedMatches.clear();
    m_selectedFetchedMatch = -1;
    m_itemsToFetch = m_mediaList;
    fetchBatch(infoFetcher, 1, updateRequiredFields, updateArtwork);
    m_fetchingStatus["description"] = i18np("Fetching info for %1 item...", "Fetching info for %1 items...", m_mediaList.count());
    emit fetchingStatusUpdated();
}

void InfoItemModel::fetch(InfoFetcher* infoFetcher)
{
    m_fetchType = Fetch;
    m_fetchedMatches.clear();
    m_selectedFetchedMatch = -1;
    m_itemsToFetch = m_mediaList;
    fetchBatch(infoFetcher, 4, true, true);
    m_fetchingStatus["description"] = i18np("Fetching info for %1 item...", "Fetching info for %1 items...", m_mediaList.count());
    emit fetchingStatusUpdated();
}

void InfoItemModel::fetchBatch(InfoFetcher *infoFetcher, int maxMatches, bool updateRequiredFields, bool updateArtwork)
{
    //Take next 4 items to fetch
    QList<MediaItem> mediaList;
    for (int i = 0; i < 4; i++) {
        if (!m_itemsToFetch.isEmpty()) {
            mediaList.append(m_itemsToFetch.takeFirst());
        }
    }

    if (!mediaList.isEmpty()) {
        m_isFetching = true;
        infoFetcher->fetchInfo(mediaList, maxMatches, updateRequiredFields, updateArtwork);
    }
}

QList<MediaItem> InfoItemModel::fetchedMatches()
{
    return m_fetchedMatches;
}

void InfoItemModel::selectFetchedMatch(int index)
{
    if (index >= m_fetchedMatches.count() ||
        index < 0) {
        return;
    }

    //Find corresponding media item
    int foundIndex = -1;
    MediaItem match = m_fetchedMatches.at(index);
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).url == match.url) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        return;
    }

    m_selectedFetchedMatch = index;
    m_mediaList.replace(foundIndex, match);

    //Update model data
    for (int i = 0; i < rowCount(); i++) {
        QString subType = match.subType();
        QString field = item(i)->data(InfoItemModel::FieldRole).toString();
        bool restricted = m_restrictedFields[subType].contains(field);
        if (!restricted) {
            if (field == "audioType" || field == "videoType") {
                QVariant value;
                if (subType == "Music" || subType == "Movie") {
                    value = QVariant(0);
                } else if (subType == "Audio Stream" || subType == "TV Show") {
                    value = QVariant(1);
                } else if (subType == "Audio Clip" || subType == "Video Clip") {
                    value = QVariant(2);
                }
                item(i)->setData(value, Qt::DisplayRole);
                item(i)->setData(value, Qt::EditRole);
            } else if (field == "artwork") {
                //disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
                item(i)->setData(match.fields["artworkUrl"], Qt::DisplayRole);
                item(i)->setData(match.fields["artworkUrl"], Qt::EditRole);
                item(i)->setData(match.artwork, Qt::DecorationRole);
                //connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            } else {
                item(i)->setData(match.fields[field], Qt::DisplayRole);
                item(i)->setData(match.fields[field], Qt::EditRole);
            }
        }
    }

}

void InfoItemModel::setRating(int rating)
{
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        mediaItem.fields["rating"] = rating;
        m_mediaList.replace(i, mediaItem);
        m_indexer->updateRating(mediaItem.fields["resourceUri"].toString(), rating);
    }
    m_originalList = m_mediaList;
    for (int i = 0 ; i < rowCount(); i++) {
        QStandardItem *currentItem = item(i);
        if (currentItem->data(InfoItemModel::FieldRole).toString() == "rating") {
            disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            currentItem->setData(rating, Qt::DisplayRole);
            currentItem->setData(rating, Qt::EditRole);
            currentItem->setData(rating, InfoItemModel::OriginalValueRole);
            connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            break;
        }
    }
}

void InfoItemModel::clearArtwork()
{
    //Clear artwork in mediaItems
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        mediaItem.fields["artworkUrl"] = QString("");
        mediaItem.artwork = Utilities::defaultArtworkForMediaItem(mediaItem);
        m_mediaList.replace(i, mediaItem);
        Utilities::removeFromImageCache(mediaItem);
    }

    //Find artwork item and reset artwork;
    for (int i = 0; i < rowCount(); i++) {
        QString field = item(i)->data(InfoItemModel::FieldRole).toString();
        if (field == "artwork") {
            item(i)->setData(QString(""), Qt::EditRole);
            getArtwork(item(i),QString(""));
            break;
        }
    }
}

void InfoItemModel::infoFetched(QList<MediaItem> fetchedMatches)
{
    if (!m_isFetching || fetchedMatches.isEmpty()) {
        return;
    }

    //Find corresponding media item
    int foundIndex = -1;
    MediaItem match = fetchedMatches.at(0);
    for (int i = 0; i < m_mediaList.count(); i++) {
        if (m_mediaList.at(i).url == match.url) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex != -1 && m_fetchType == AutoFetch) {
        match = Utilities::makeSubtitle(match);
        m_mediaList.replace(foundIndex, match);
    } else if (foundIndex != -1 && m_fetchType == Fetch) {
        m_fetchedMatches = fetchedMatches;
        selectFetchedMatch(0);
    }
}

void InfoItemModel::infoFetcherComplete(InfoFetcher *infoFetcher)
{
    if (!m_isFetching) {
        return;
    }

    if (!m_itemsToFetch.isEmpty()) {
        //First argument controls the plural
        m_fetchingStatus["description"] = i18np("Fetched info for %2 of 1 item...", "Fetched info for %2 of %1 items...", m_mediaList.count(), (m_mediaList.count() - m_itemsToFetch.count()));
        emit fetchingStatusUpdated();

        //Fetch info for another batch of items
        if (m_fetchType == AutoFetch) {
            fetchBatch(infoFetcher, 1, true, true);
        } else {
            fetchBatch(infoFetcher, 4, true, true);
        }
    } else {
        m_isFetching = false;
        m_fetchingStatus["description"] = i18n("Complete");
        emit fetchingStatusUpdated();
        m_fetchingStatus["description"] = QString();
        emit fetchingStatusUpdated();
        emit fetchComplete();
        if (m_fetchType == AutoFetch) {
            QApplication::processEvents();  //make sure all infoFetched signals are processed before saving changes.
            saveChanges();
        }
    }
}
void InfoItemModel::noResults(InfoFetcher *infoFetcher)
{
    m_fetchingStatus["description"] = i18n("No Results");
    emit fetchingStatusUpdated();
    m_fetchingStatus["description"] = QString();
    emit fetchingStatusUpdated();
    Q_UNUSED(infoFetcher)
}

void InfoItemModel::updateFetchedInfo(int index, MediaItem match)
{
    if (!m_isFetching) {
        return;
    }
    if (index >= m_fetchedMatches.count() ||
        index < 0) {
        return;
    }

    //Update fetched matches
    m_fetchedMatches.replace(index, match);

    //If match is currently selected, update model to reflect updated fetched information
    if (index == m_selectedFetchedMatch) {
        selectFetchedMatch(index);
    }

}

void InfoItemModel::loadFieldsInOrder()
{
    clear();
    QString type = m_mediaList.at(0).type;
    QString subType = m_mediaList.at(0).subType();

    //Load field info in order specified
    QStringList fieldsOrder = m_fieldsOrder.value(subType, m_fieldsOrder["Basic"]);
    for (int i = 0; i < fieldsOrder.count(); i++) {
        QString field = fieldsOrder.at(i);
        if ((Utilities::isCd(m_mediaList.at(0).url) || Utilities::isDvd(m_mediaList.at(0).url)) &&
            (field == "url")) {
            addFieldToValuesModel(i18n("Location"), "album", false); //or the user would see the ugly udi
        } else {
            QStringList restrictedFields = m_restrictedFields.value(subType, m_restrictedFields["Basic"]);
            bool isEditable = !restrictedFields.contains(field);
            addFieldToValuesModel(m_fieldNames[field],field, isEditable);
        }
    }
}

void InfoItemModel::addFieldToValuesModel(const QString &fieldTitle, const QString &field, bool isEditable)
{
    //NOTE: This method must only be called by loadInfo.  The reason is that this
    // method as well as supporting method, commonValue(), use m_mediaList to
    // store the original value: loadInfo is the only place m_medialist and
    // m_originalList are set equal.

    QList<QStandardItem *> rowData;
    QStandardItem *fieldItem = new QStandardItem();
    fieldItem->setData(field, InfoItemModel::FieldRole);
    fieldItem->setData(fieldTitle, InfoItemModel::FieldNameRole);
    bool hasMultiple = hasMultipleValues(field);
    fieldItem->setData(hasMultiple, InfoItemModel::MultipleValuesRole);
    fieldItem->setEditable(isEditable);

    //Set artwork
    if (field == "artwork") {
        QString artworkUrl = m_mediaList.at(0).fields["artworkUrl"].toString();
        fieldItem->setData(artworkUrl, Qt::DisplayRole);
        fieldItem->setData(artworkUrl, Qt::EditRole);
        fieldItem->setData(artworkUrl, InfoItemModel::OriginalValueRole); //stores copy of original data

        //Get artwork for selected items
        if (getArtwork(fieldItem)) {
            rowData.append(fieldItem);
            appendRow(rowData);
        }
        return;
    }

    if (!hasMultiple) {
        //Set field value
        QVariant value = commonValue(field);
        if (field == "audioType" || field == "videoType") {
            if (value.toString() == "Music" || value.toString() == "Movie") {
                value = QVariant(0);
            } else if (value.toString() == "Audio Stream" || value.toString() == "TV Show") {
                value = QVariant(1);
            } else if (value.toString() == "Audio Clip" || value.toString() == "Video Clip") {
                value = QVariant(2);
            }
        }
        fieldItem->setData(value, Qt::DisplayRole);
        fieldItem->setData(value, Qt::EditRole);
        fieldItem->setData(value, InfoItemModel::OriginalValueRole); //stores copy of original data

        //Store drill lri(s)
        setDrill(fieldItem, field, value);

        if (field == "url" || field == "relatedTo") {
            fieldItem->setData(value, Qt::ToolTipRole);
        }
    } else {
        //Set default field value
        QVariant value = m_mediaList.at(0).fields[field];
        if (value.type() == QVariant::String) {
            fieldItem->setData(QString(), Qt::DisplayRole);
            fieldItem->setData(QString(), Qt::EditRole);
            fieldItem->setData(QString(), InfoItemModel::OriginalValueRole);
            fieldItem->setData(valueList(field), InfoItemModel::ValueListRole);
        } else if (value.type() == QVariant::StringList) {
            fieldItem->setData(QStringList(), Qt::DisplayRole);
            fieldItem->setData(QStringList(), Qt::EditRole);
            fieldItem->setData(QStringList(), InfoItemModel::OriginalValueRole);
            fieldItem->setData(valueList(field), InfoItemModel::ValueListRole);
        } else if (value.type() == QVariant::Int) {
            fieldItem->setData(0, Qt::DisplayRole);
            fieldItem->setData(0, Qt::EditRole);
            fieldItem->setData(0, InfoItemModel::OriginalValueRole);
        }
    }
    rowData.append(fieldItem);
    appendRow(rowData);
}

bool InfoItemModel::hasMultipleValues(const QString &field)
{
    QVariant value;
    
    if (field == "artwork") {
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

QVariant InfoItemModel::commonValue(const QString &field)
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

QStringList InfoItemModel::valueList(const QString &field)
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

bool InfoItemModel::isEmpty(const QString &field)
{
    bool isEmpty = true;
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        if (mediaItem.fields.contains(field)) {
            QVariant::Type fieldType = mediaItem.fields[field].type();
            if (fieldType == QVariant::String) {
                isEmpty = mediaItem.fields[field].toString().isEmpty();
            } else if (fieldType == QVariant::StringList) {
                isEmpty = mediaItem.fields[field].toStringList().isEmpty();
            } else if (fieldType == QVariant::Date) {
                isEmpty = !mediaItem.fields[field].toDate().isValid();
            } else if (fieldType == QVariant::DateTime) {
                isEmpty = !mediaItem.fields[field].toDateTime().isValid();
            } else if (fieldType == QVariant::Int){
                isEmpty = !mediaItem.fields[field].isValid();
            } else {
                isEmpty = mediaItem.fields[field].isNull();
            }
            if (isEmpty) {
                break;
            }
        }
    }
    return isEmpty;
}

void InfoItemModel::updateMediaList()
{
    QList<MediaItem> updatedList;
    for (int i = 0; i < m_mediaList.count(); i++) {
        MediaItem mediaItem = m_mediaList.at(i);
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *currentItem = item(row, 0);
            QString field = currentItem->data(InfoItemModel::FieldRole).toString();

            //Save any field that does not have multiple values.
            //If multiple items are selected and a field is edited
            //then the edited field won't have multiple values
            bool multipleValues = currentItem->data(InfoItemModel::MultipleValuesRole).toBool();
            if (!multipleValues) {
                if (field == "audioType") {
                    int value = currentItem->data(Qt::EditRole).toInt();
                    if (value == 0) {
                        mediaItem.fields["audioType"] = "Music";
                    } else if (value == 1) {
                        mediaItem.fields["audioType"] = "Audio Stream";
                    } else if (value == 2) {
                        mediaItem.fields["audioType"] = "Audio Clip";
                    }
                } else if (field == "videoType") {
                    int value = currentItem->data(Qt::EditRole).toInt();
                    if (value == 0) {
                        mediaItem.fields["videoType"] = "Movie";
                    } else if (value == 1) {
                        mediaItem.fields["videoType"] = "TV Show";
                    } else if (value == 2) {
                        mediaItem.fields["videoType"] = "Video Clip";
                    }
                } else if (field == "title") {
                    mediaItem.fields["title"] = currentItem->data(Qt::EditRole);
                    mediaItem.title = currentItem->data(Qt::EditRole).toString();
                } else if (field == "duration") {
                    int duration = currentItem->data(Qt::EditRole).toInt();
                    mediaItem.fields["duration"] = currentItem->data(Qt::EditRole);
                    mediaItem.duration = Utilities::durationString(duration);
                } else if (field == "url") {
                    mediaItem.fields["url"] = currentItem->data(Qt::EditRole);
                    mediaItem.url = currentItem->data(Qt::EditRole).toString();
                } else if (field == "artwork") {
                    QString artworkUrl = currentItem->data(Qt::EditRole).toString();
                    mediaItem.fields["artworkUrl"] = currentItem->data(Qt::EditRole).toString();
                    if (!artworkUrl.isEmpty() && (mediaItem.subType() == "AudioGenre" || mediaItem.subType() == "VideoGenre")) {
                        mediaItem.artwork = currentItem->data(Qt::DecorationRole).value<QIcon>();
                        mediaItem.hasCustomArtwork = true;
                    } else if (mediaItem.subType() == "AudioGenre" || mediaItem.subType() == "VideoGenre"){
                        mediaItem.artwork = KIcon("flag-blue");
                        mediaItem.hasCustomArtwork = false;
                    }
                } else if (field == "year") {
                    mediaItem.fields["year"] = currentItem->data(Qt::EditRole);
                    if (!mediaItem.fields["year"].isNull() && mediaItem.fields["year"].toInt() !=0) {
                        mediaItem.fields["releaseDate"] = QDate(mediaItem.fields["year"].toInt(),1, 1);
                    } else {
                        mediaItem.fields["releaseDate"] = QVariant(QVariant::Date);
                    }
                    if (mediaItem.subType() == "Movie" && mediaItem.fields["year"].toInt() != 0) {
                    }
                } else {
                    mediaItem.fields[field] = currentItem->data(Qt::EditRole);
                }
            }
        }
        mediaItem = Utilities::makeSubtitle(mediaItem);
        updatedList << mediaItem;
    }
    m_mediaList = updatedList;
}

void InfoItemModel::itemChanged(QStandardItem *changedItem)
{
    if (changedItem->data(Qt::EditRole) != changedItem->data(InfoItemModel::OriginalValueRole)) {
        m_modified = true;
        QString field = changedItem->data(InfoItemModel::FieldRole).toString();
        if (field == "artwork") {
            disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            QString artworkUrl = changedItem->data(Qt::EditRole).toString();
            QString originalArtworkUrl = changedItem->data(InfoItemModel::OriginalValueRole).toString();
            for (int i = 0; i < m_mediaList.count(); i++) {
                MediaItem mediaItem = m_mediaList.at(i);
                if (artworkUrl != originalArtworkUrl && artworkUrl.isEmpty()) {
                    mediaItem.artwork = Utilities::defaultArtworkForMediaItem(mediaItem);
                    m_mediaList.replace(i, mediaItem);
                }
                Utilities::removeFromImageCache(mediaItem);
            }
            getArtwork(changedItem, artworkUrl);
            connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
        } else if (field == "url") {
            disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            QString url = changedItem->data(Qt::EditRole).toString();
            changedItem->setData(url, Qt::ToolTipRole);
            connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
        } else {
            //Update drill for changed item
            QVariant value = changedItem->data(Qt::EditRole);
            disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            setDrill(changedItem, field, value);
            connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
        }
        updateMediaList();
    } else {
        m_modified = false;
        for (int row = 0; row < rowCount(); row++) {
            QStandardItem *otherItem = item(row, 0);
            if (otherItem->data(InfoItemModel::FieldRole).toString() != "artwork") {
                if (otherItem->data(Qt::EditRole) != otherItem->data(InfoItemModel::OriginalValueRole)) {
                    m_modified = true;
                    break;
                }
            }
        }
    }
    emit infoChanged(m_modified);
    
}

void InfoItemModel::saveFileMetaData(QList<MediaItem> mediaList)
{
    Utilities::saveAllInfoToTag(mediaList);
}

void InfoItemModel::saveCustomGenreInfo(QList<MediaItem> mediaList)
{
    if (mediaList.count() > 0) {
        if (mediaList.at(0).type == "Category" &&
            (mediaList.at(0).subType() == "AudioGenre" || mediaList.at(0).subType() == "VideoGenre")) {
            QString localGenreFile = KGlobal::dirs()->locateLocal("data","bangarang/genrerc", true);
            if (localGenreFile.isEmpty()) {
                return;
            }

            KConfig genreConfig(localGenreFile);
            for (int i = 0; i < mediaList.count(); i++) {
                MediaItem mediaItem = mediaList.at(i);
                if (mediaItem.type == "Category" &&
                    (mediaItem.subType() == "AudioGenre" || mediaItem.subType() == "VideoGenre")) {
                    KConfigGroup genreGroup(&genreConfig, mediaItem.fields["title"].toString());
                    QString artworkUrl = mediaItem.fields["artworkUrl"].toString();
                    if (artworkUrl.isEmpty()) {
                        if (genreGroup.exists()) {
                            genreGroup.deleteEntry("artworkUrl");
                        }
                    } else {
                        genreGroup.writeEntry("artworkUrl", artworkUrl);
                    }
                }
            }
            genreConfig.sync();
        }
    }
}

bool InfoItemModel::getArtwork(QStandardItem *fieldItem, QString artworkUrlOverride)
{
    bool artworkExists = true;
    bool ignoreCache = m_modified;
    QList<QVariant> emptyArtworkPixmaps;
    fieldItem->setData(emptyArtworkPixmaps, InfoItemModel::ArtworkListRole);
    if (m_mediaList.count() == 1) {
        MediaItem mediaItem = m_mediaList.at(0);
        if (!artworkUrlOverride.isNull()) {
            mediaItem.fields["artworkUrl"] = artworkUrlOverride;
        }
        if (mediaItem.type == "Category") {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(mediaItem, ignoreCache);
            if (!artwork.isNull()) {
                fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
            } else {
                fieldItem->setData(mediaItem.artwork, Qt::DecorationRole);
                m_utilThread->getArtworksFromMediaItem(mediaItem, ignoreCache);
            }
        } else {
            QPixmap artwork = Utilities::getArtworkFromMediaItem(mediaItem, ignoreCache);
            if (!artwork.isNull()) {
                fieldItem->setData(QIcon(artwork), Qt::DecorationRole);
            } else {
                fieldItem->setData(mediaItem.artwork, Qt::DecorationRole);
            }
        }
    } else {
        if (m_mediaList.at(0).type == "Audio" || m_mediaList.at(0).type == "Video") {
            QList<QVariant> artworkPixmaps;
            QImage lastItemArtwork;
            for (int i = 0; i < qMin(10, m_mediaList.count()); i++) {
                MediaItem mediaItem = m_mediaList.at(i);
                if (!artworkUrlOverride.isNull()) {
                    mediaItem.fields["artworkUrl"] = artworkUrlOverride;
                }
                QImage itemArtwork = Utilities::getArtworkImageFromMediaItem(mediaItem, ignoreCache);
                if (!lastItemArtwork.isNull() && !itemArtwork.isNull()) {
                    bool sameImage = Utilities::compareImage(lastItemArtwork, itemArtwork, 50);
                    if (!sameImage) {
                        artworkPixmaps.append(QPixmap::fromImage(itemArtwork));
                    }
                } else if (lastItemArtwork.isNull() && !itemArtwork.isNull()) {
                    artworkPixmaps.append(QPixmap::fromImage(itemArtwork));
                } else if (itemArtwork.isNull()) {
                    itemArtwork = mediaItem.artwork.pixmap(128,128).toImage();
                    artworkPixmaps.append(mediaItem.artwork.pixmap(128,128));
                }
                lastItemArtwork = itemArtwork;
            }
            if (artworkPixmaps.count() > 0) {
                fieldItem->setData(artworkPixmaps, InfoItemModel::ArtworkListRole);
            } else {
                artworkExists = false;
            }
        } else {
            artworkExists = false;
        }
    }
    return artworkExists;
}

void InfoItemModel::gotArtworks(QList<QImage> artworks, MediaItem mediaItem)
{
    if (artworks.isEmpty()) {
        return;
    }

    if (m_mediaList.isEmpty()) {
        return;
    }

    if (m_mediaList.at(0).url != mediaItem.url) {
        return;
    }

    for (int i = 0; i < rowCount(); i++) {
        QStandardItem * fieldItem = item(i);
        QString field = fieldItem->data(InfoItemModel::FieldRole).toString();
        if (field == "artwork") {
            //Convert Image list to Pixmap list
            QList<QVariant> artworkPixmaps;
            for (int i = 0; i < artworks.count(); i++) {
                if (!artworks.at(i).isNull()) {
                    artworkPixmaps.append(QPixmap::fromImage(artworks.at(i)));
                }
            }
            if (!artworkPixmaps.isEmpty()) {
                disconnect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
                fieldItem->setData(artworkPixmaps, InfoItemModel::ArtworkListRole);
                connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));
            }
            break;
        }
    }
}

void InfoItemModel::setDrill(QStandardItem *item, const QString &field, const QVariant &value)
{
    if (value.type() == QVariant::StringList &&
        !categoryTypeForField(field, m_mediaList.at(0).type).isEmpty()) {
        QStringList values = value.toStringList();
        QList<QVariant> drillItems;
        for (int i = 0; i < values.count(); i++) {
            MediaItem drillItem = createDrillItem(field, m_mediaList.at(0).type, values.at(i));
            if (!drillItem.url.isEmpty()) {
                drillItems.append(QVariant::fromValue(drillItem));
            } else {
                drillItems.append(QVariant());
            }
        }
        item->setData(drillItems, InfoItemModel::DrillRole);
    } else if (!categoryTypeForField(field, m_mediaList.at(0).type).isEmpty()) {
        MediaItem drillItem = createDrillItem(field, m_mediaList.at(0).type, value.toString());
        if (!drillItem.url.isEmpty()) {
            item->setData(QVariant::fromValue(drillItem), InfoItemModel::DrillRole);
        } else {
            item->setData(QVariant(), InfoItemModel::DrillRole);
        }
    }

}

QString InfoItemModel::categoryTypeForField(const QString &field, const QString &type)
{
    QString categoryType;
    if (field == "artist") {
        categoryType = "Artist";
    } else if (field == "composer") {
        categoryType = "Artist";
    } else if (field == "album") {
        categoryType = "Album";
    } else if (field == "genre" && type == "Audio") {
        categoryType = "AudioGenre";
    } else if (field == "genre" && type == "Video") {
        categoryType = "VideoGenre";
    } else if (field == "tags" && type == "Audio") {
        categoryType = "AudioTag";
    } else if (field == "tags" && type == "Video") {
        categoryType = "VideoTag";
    } else if (field == "seriesName") {
        categoryType = "TV Series";
    } else if (field == "actor") {
        categoryType = "Actor";
    } else if (field == "director") {
        categoryType = "Director";
    }
    return categoryType;
}

MediaItem InfoItemModel::createDrillItem(const QString &field, const QString &type, const QString &value)
{
    MediaItem mediaItem;
    QString categoryType = categoryTypeForField(field, type);
    if (categoryType == "Artist") {
        Nepomuk::Resource res(Utilities::artistResource(value));
        mediaItem = Utilities::categoryMediaItemFromNepomuk(res, categoryType);
        mediaItem.url = m_drillLris[categoryType].arg(value);
    } else if (categoryType == "Actor") {
        Nepomuk::Resource res(Utilities::actorResource(value));
        mediaItem = Utilities::categoryMediaItemFromNepomuk(res, categoryType);
        mediaItem.url = m_drillLris[categoryType].arg(value);
    } else if (categoryType == "Director") {
        Nepomuk::Resource res(Utilities::directorResource(value));
        mediaItem = Utilities::categoryMediaItemFromNepomuk(res, categoryType);
        mediaItem.url = m_drillLris[categoryType].arg(value);
    } else if (categoryType == "Album") {
        Nepomuk::Resource res(Utilities::albumResource(value));
        mediaItem = Utilities::categoryMediaItemFromNepomuk(res, categoryType);
        mediaItem.url = m_drillLris[categoryType].arg(value);
    } else if (categoryType == "TV Series") {
        Nepomuk::Resource res(Utilities::TVSeriesResource(value));
        mediaItem = Utilities::categoryMediaItemFromNepomuk(res, categoryType);
        mediaItem.url = m_drillLris[categoryType].arg(value);
    } else if (categoryType == "AudioGenre" ||
               categoryType == "VideoGenre") {
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = categoryType;
        mediaItem.title = value;
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = m_drillLris[categoryType].arg(value);
        mediaItem.fields["artworkUrl"] = Utilities::getGenreArtworkUrl(value);
        if (categoryType == "AudioGenre") {
            mediaItem.addContext(i18n("Recently Played Songs"), QString("semantics://recent?audio||limit=4||genre=%1").arg(value));
            mediaItem.addContext(i18n("Highest Rated Songs"), QString("semantics://highest?audio||limit=4||genre=%1").arg(value));
            mediaItem.addContext(i18n("Frequently Played Songs"), QString("semantics://frequent?audio||limit=4||genre=%1").arg(value));
        } else {
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||genre=%1").arg(value));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||genre=%1").arg(value));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?video||limit=4||genre=%1").arg(value));
        }
    } else if (categoryType == "AudioTag" ||
               categoryType == "VideoTag") {
        mediaItem.type = "Category";
        mediaItem.fields["categoryType"] = categoryType;
        mediaItem.title = value;
        mediaItem.fields["title"] = mediaItem.title;
        mediaItem.url = m_drillLris[categoryType].arg(value);
        if (categoryType == "AudioTag") {
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?audio||limit=4||tag=%1").arg(value));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?audio||limit=4||tag=%1").arg(value));
            mediaItem.addContext(i18n("Frequently Played"), QString("semantics://frequent?audio||limit=4||tag=%1").arg(value));
        } else {
            mediaItem.addContext(i18n("Recently Played"), QString("semantics://recent?video||limit=4||tag=%1").arg(value));
            mediaItem.addContext(i18n("Highest Rated"), QString("semantics://highest?video||limit=4||tag=%1").arg(value));
            mediaItem.addContext(i18n("Frequently Played"),QString("semantics://frequent?video||limit=4||tag=%1").arg(value));
        }
    }

    if (mediaItem.title.isEmpty()) {
        mediaItem.title = value;
        mediaItem.fields["title"] = value;
    }
    return mediaItem;
}

void InfoItemModel::cancelFetching()
{
    if (m_isFetching) {
        m_isFetching = false;
        m_fetchingStatus["description"] = i18n("Fetch Canceled");
        emit fetchingStatusUpdated();
        m_fetchingStatus["description"] = QString();
        emit fetchingStatusUpdated();
        emit fetchComplete();
    }
}
