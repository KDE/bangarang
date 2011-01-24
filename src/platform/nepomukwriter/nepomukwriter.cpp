#include "../mediavocabulary.h"
#include "../mediaquery.h"
#include "nepomukwriter.h"
#include <KDebug>
#include <KUrl>
#include <KLocale>
#include <Soprano/Vocabulary/RDF>
#include <QDBusInterface>

NepomukWriter::NepomukWriter(QObject *parent) :
    QObject(parent)
{
}

void NepomukWriter::processJob(QFile *jobFile)
{
    //Parse job file and write each field value to nepomuk
    QTextStream in(jobFile);
    QHash <QString, QVariant> fields;
    QString line;
    int count = 0;
    int processed = 0;
    while (!in.atEnd()) {
        if (!line.isEmpty()) {
            if (line.startsWith("#Count")) {
                count = line.split("=").at(1).trimmed().toInt();
            } else if (line.startsWith("[") && line.endsWith("]")) {
                QString resourceUri = line.mid(1, line.length()-2);
                fields.insert("resourceUri",resourceUri);
            } else {
                int equalLocation = line.indexOf("=");
                QString field;
                QString value;
                if (equalLocation != -1) {
                    field = line.left(equalLocation).trimmed();
                    value = line.mid(equalLocation+1).trimmed();
                }
                if (field == "duration" ||
                    field == "trackNumber" ||
                    field == "year" ||
                    field == "season" ||
                    field == "episodeNumber" ||
                    field == "rating" ||
                    field == "playCount") {
                    fields.insert(field, value.toInt());
                } else if (field == "lastPlayed") {
                    fields.insert(field, QDateTime::fromString(value, "yyyyMMddhhmmss"));
                } else {
                    fields.insert(field, value);
                }
            }
        }

        line = in.readLine().trimmed();

        //Update the resource once all provided fields for a resource is read.
        if ((line.startsWith("[") && line.endsWith("]")) || in.atEnd()) {
            if (fields.count() > 0) {
                writeToNepomuk(fields);
                fields.clear();
                processed = processed + 1;
                if (count > 0) {
                    outputMessage(Progress, QString("%1").arg(processed*100/count));
                }
            }
        }
    }

    //Remove unused property resources
    removeUnusedPropertyResources();

    jobFile->close();

    //Consume job file when done
    jobFile->remove();
}

void NepomukWriter::writeToNepomuk(QHash <QString, QVariant> fields)
{
    if (fields["removeInfo"].toString() == "true") {
        removeInfo(fields);
    } else {
        updateInfo(fields);
    }
}

void NepomukWriter::removeInfo(QHash <QString, QVariant> fields)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    QString resourceUri = fields["resourceUri"].toString();
    QString type = fields["type"].toString();
    Nepomuk::Resource res = Nepomuk::Resource(QUrl::fromEncoded(resourceUri.toUtf8()));

    if (fields["removeInfo"].toString() == "true") {

        //Remove metadata for media from nepomuk store
        if (!res.exists()) {
            return;
        }
        if (type == "Audio") {
            // Update the media type
            QUrl audioType;
            if (fields["audioType"] == "Music") {
                audioType = mediaVocabulary.typeAudioMusic();
            } else if (fields["audioType"] == "Audio Stream") {
                audioType = mediaVocabulary.typeAudioStream();
            } else if (fields["audioType"] == "Audio Clip") {
                audioType = mediaVocabulary.typeAudio();
            }
            if (res.hasType(audioType)) {
                removeType(res, audioType);
            }

            // Update the properties
            if (res.hasProperty(mediaVocabulary.title())){
                res.removeProperty(mediaVocabulary.title());
            }
            if (res.hasProperty(mediaVocabulary.description())) {
                res.removeProperty(mediaVocabulary.description());
            }
            if (res.hasProperty(mediaVocabulary.artwork())) {
                res.removeProperty(mediaVocabulary.artwork());
            }

            if (fields["audioType"] == "Music") {
                if (res.hasProperty(mediaVocabulary.musicArtist())) {
                    res.removeProperty(mediaVocabulary.musicArtist());
                }
                res.removeProperty(mediaVocabulary.musicPerformer());
                res.removeProperty(mediaVocabulary.musicComposer());;
                if (res.hasProperty(mediaVocabulary.musicAlbum())) {
                    res.removeProperty(mediaVocabulary.musicAlbum());
                }
                if (res.hasProperty(mediaVocabulary.genre())) {
                    res.removeProperty(mediaVocabulary.genre());
                }
                if (res.hasProperty(mediaVocabulary.musicTrackNumber())) {
                    res.removeProperty(mediaVocabulary.musicTrackNumber());
                }
                if (res.hasProperty(mediaVocabulary.duration())) {
                    res.removeProperty(mediaVocabulary.duration());
                }
                if (res.hasProperty(mediaVocabulary.created())) {
                    res.removeProperty(mediaVocabulary.created());
                }
                if (res.hasProperty(mediaVocabulary.releaseDate())) {
                    res.removeProperty(mediaVocabulary.releaseDate());
                }
            } else if ((fields["audioType"] == "Audio Stream") ||
                (fields["audioType"] == "Audio Clip")) {
            }
        } else if (type == "Video") {
            mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
            //Update the media type
            if (res.hasType(mediaVocabulary.typeVideo())) {
                removeType(res, mediaVocabulary.typeVideo());
            }
            if (res.hasType(mediaVocabulary.typeVideoMovie())) {
                removeType(res, mediaVocabulary.typeVideoMovie());
            }
            if (res.hasType(mediaVocabulary.typeVideoTVShow())) {
                removeType(res, mediaVocabulary.typeVideoTVShow());
            }

            //Update the properties
            if (res.hasProperty(mediaVocabulary.title())){
                res.removeProperty(mediaVocabulary.title());
            }
            if (res.hasProperty(mediaVocabulary.description())) {
                res.removeProperty(mediaVocabulary.description());
            }
            if (res.hasProperty(mediaVocabulary.artwork())) {
                res.removeProperty(mediaVocabulary.artwork());
            }
            if ((fields["videoType"] == "Movie") || (fields["videoType"] == "TV Show")) {
                if (res.hasProperty(mediaVocabulary.genre())) {
                    res.removeProperty(mediaVocabulary.genre());
                }
                if (res.hasProperty(mediaVocabulary.videoSynopsis())) {
                    res.removeProperty(mediaVocabulary.videoSynopsis());
                }
                if (res.hasProperty(mediaVocabulary.created())) {
                    res.removeProperty(mediaVocabulary.created());
                }
                if (res.hasProperty(mediaVocabulary.releaseDate())) {
                    res.removeProperty(mediaVocabulary.releaseDate());
                }
                if (res.hasProperty(mediaVocabulary.videoWriter())) {
                    res.removeProperty(mediaVocabulary.videoWriter());
                }
                if (res.hasProperty(mediaVocabulary.videoDirector())) {
                    res.removeProperty(mediaVocabulary.videoDirector());
                }
                if (res.hasProperty(mediaVocabulary.videoProducer())) {
                    res.removeProperty(mediaVocabulary.videoProducer());
                }
                if (res.hasProperty(mediaVocabulary.videoActor())) {
                    res.removeProperty(mediaVocabulary.videoActor());
                }

                if (fields["videoType"] == "TV Show") {
                    if (res.hasProperty(mediaVocabulary.videoSeries())) {
                        res.removeProperty(mediaVocabulary.videoSeries());
                    }
                    if (res.hasProperty(mediaVocabulary.videoSeason())) {
                        res.removeProperty(mediaVocabulary.videoSeason());
                    }
                    if (res.hasProperty(mediaVocabulary.videoEpisodeNumber())) {
                        res.removeProperty(mediaVocabulary.videoEpisodeNumber());
                    }
                }
            }
        } else if (type == "Category") {
            if (fields["categoryType"] == "Audio Feed") {
                removeType(res, mediaVocabulary.typeAudioFeed());
                //res.remove();
            } else if (fields["categoryType"] == "Video Feed") {
                removeType(res, mediaVocabulary.typeVideoFeed());
                //res.remove();
            }
            //Update the properties
            if (res.hasProperty(mediaVocabulary.title())){
                res.removeProperty(mediaVocabulary.title());
            }
            if (res.hasProperty(mediaVocabulary.description())) {
                res.removeProperty(mediaVocabulary.description());
            }
            if (res.hasProperty(mediaVocabulary.artwork())) {
                res.removeProperty(mediaVocabulary.artwork());
            }
        }
        outputMessage(InfoRemoved, QUrl::toPercentEncoding(resourceUri));
    }
}

void NepomukWriter::updateInfo(QHash<QString, QVariant> fields)
{
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    QString resourceUri = fields["resourceUri"].toString();
    QString type = fields["type"].toString();
    QString url = fields["url"].toString();

    //Find corresponding Nepomuk resource
    Nepomuk::Resource res;
    if (!resourceUri.isEmpty()) {
        res = Nepomuk::Resource(QUrl::fromEncoded(resourceUri.toUtf8()));
    }

    //If Nepomuk resource doesn't exist for local file, ask Strigi to index file first
    //TODO: Disabled currently since it does not appear to work.  Investigate for Bangarang 2.1
    /*if (!res.exists() && KUrl(url).isValid() && KUrl(url).isLocalFile()) {
        outputMessage(Debug, "ASKING STRIGI TO INDEX FILE:" + url);
        QDBusInterface strigi(
                "org.kde.nepomuk.services.nepomukstrigiservice",
                "/nepomukstrigiservice",
                "org.kde.nepomuk.Strigi");
        if (strigi.isValid()) {
            strigi.call("indexFile", KUrl(url).path());
        }
        res = Nepomuk::Resource(QUrl::fromEncoded(url.toUtf8()));
        outputMessage(Debug, QString("RESOURCE EXISTS:%1").arg(res.exists()));
    } else {
        outputMessage(Debug, QString("RESOURCE EXISTS:%1").arg(res.exists()));
    }*/

    if (type == "Category" &&
        fields["categoryType"] != "Audio Feed" &&
        fields["categoryType"] != "Video Feed") {
        QUrl categoryProperty;
        if (fields["categoryType"] == "Artist") {
            categoryProperty = mediaVocabulary.musicArtist();
        } else if (fields["categoryType"] == "Album") {
            categoryProperty = mediaVocabulary.musicAlbum();
        } else if (fields["categoryType"] == "TV Series") {
            categoryProperty = mediaVocabulary.videoSeries();
        } else if (fields["categoryType"] == "Actor") {
            categoryProperty = mediaVocabulary.videoActor();
        } else if (fields["categoryType"] == "Director") {
            categoryProperty = mediaVocabulary.videoDirector();
        } else if (fields["categoryType"] == "Writer") {
            categoryProperty = mediaVocabulary.videoWriter();
        } else if (fields["categoryType"] == "Producer") {
            categoryProperty = mediaVocabulary.videoProducer();
        }
        res = findPropertyResourceByTitle(categoryProperty, fields["title"].toString(), true);
    }

    // Update the media type
    {
        if (type == "Audio") {
            QUrl audioType;
            if (fields["audioType"] == "Music") {
                audioType = mediaVocabulary.typeAudioMusic();
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, audioType);
                }
                removeType(res, mediaVocabulary.typeAudioStream());
                removeType(res, mediaVocabulary.typeAudio());
            } else if (fields["audioType"] == "Audio Stream") {
                audioType = mediaVocabulary.typeAudioStream();
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, audioType);
                }
                removeType(res, mediaVocabulary.typeAudioMusic());
                removeType(res, mediaVocabulary.typeAudio());
            } else if (fields["audioType"] == "Audio Clip") {
                audioType = mediaVocabulary.typeAudio();
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, audioType);
                }
                removeType(res, mediaVocabulary.typeAudioMusic());
                removeType(res, mediaVocabulary.typeAudioStream());
            }
            if (!res.hasType(audioType)) {
                res.addType(audioType);
            }
        } else if (type == "Video") {
            //Update the media type
            mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
            QUrl videoType;
            if (fields["videoType"] == "Movie") {
                videoType = mediaVocabulary.typeVideoMovie();
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, videoType);
                }
                removeType(res, mediaVocabulary.typeVideoTVShow());
                removeType(res, mediaVocabulary.typeVideo());
            } else if (fields["videoType"] == "TV Show") {
                videoType = mediaVocabulary.typeVideoTVShow();
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, videoType);
                }
                removeType(res, mediaVocabulary.typeVideoMovie());
                removeType(res, mediaVocabulary.typeVideo());
            } else if (fields["videoType"] == "Video Clip") {
                videoType = mediaVocabulary.typeVideo();
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, videoType);
                }
                removeType(res, mediaVocabulary.typeVideoMovie());
                removeType(res, mediaVocabulary.typeVideoTVShow());
            }
            if (!res.hasType(videoType)) {
                res.addType(videoType);
            }
        } else if (type == "Category") {
            if (fields["categoryType"] == "Audio Feed") {
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, mediaVocabulary.typeAudioFeed());
                } else {
                    if (!res.hasType(mediaVocabulary.typeAudioFeed())) {
                        res.addType(mediaVocabulary.typeAudioFeed());
                    }
                }
            } else if (fields["categoryType"] == "Video Feed") {
                if (!res.exists()) {
                    res = Nepomuk::Resource(url, mediaVocabulary.typeVideoFeed());
                } else {
                    if (!res.hasType(mediaVocabulary.typeVideoFeed())) {
                        res.addType(mediaVocabulary.typeVideoFeed());
                    }
                }
            }
        }
    }


    //Set properties common to all types
    {
        if (fields.contains("description")) {
            QString description = fields["description"].toString();
            res.setProperty(mediaVocabulary.description(), Nepomuk::Variant(description));
        }
        if (fields.contains("artworkUrl")) {
            QString artworkUrl = fields["artworkUrl"].toString();
            if (!artworkUrl.isEmpty()) {
                Nepomuk::Resource artworkRes(artworkUrl);
                if (!artworkRes.exists()) {
                    artworkRes = Nepomuk::Resource(QUrl(artworkUrl), QUrl("http://http://www.semanticdesktop.org/ontologies/nfo#Image"));
                }
                res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(artworkRes));
            } else {
                if (res.hasProperty(mediaVocabulary.artwork())) {
                    res.removeProperty(mediaVocabulary.artwork());
                }
            }
        }
        if (!url.isEmpty()) {
            //the url may has changed (e.g. with audio streams), so update it in any case
            res.setProperty(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"), QUrl(url));
        }
        if (fields.contains("title")) {
            QString title = fields["title"].toString();
            res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        }
        if (fields.contains("rating")) {
            unsigned int rating = fields["rating"].toInt();
            if (rating != res.rating()) {
                res.setRating(rating);
            }
        }
        if (fields.contains("playCount")) {
            int playCount = fields["playCount"].toInt();
            res.setProperty(mediaVocabulary.playCount(), Nepomuk::Variant(playCount));
        }
        if (fields.contains("duration")) {
            int duration = fields["duration"].toInt();
            res.setProperty(mediaVocabulary.duration(), Nepomuk::Variant(duration));
        }
        if (fields.contains("lastPlayed")) {
            Nepomuk::Variant value;
            QDateTime lastPlayed = fields["lastPlayed"].toDateTime();
            if (lastPlayed.isValid()) {
                value = Nepomuk::Variant(lastPlayed);
            }
            res.setProperty(mediaVocabulary.lastPlayed(), value);
        }
        if (fields.contains("tags")) {
            QStringList tagStrList = fields["tags"].toString().split("||", QString::SkipEmptyParts);
            QList<Nepomuk::Tag> tags;
            QList<Nepomuk::Tag> currentTags = res.tags();
            bool tagsChanged = false;
            for (int i = 0; i < tagStrList.count(); i++) {
                Nepomuk::Tag tag(tagStrList.at(i).trimmed());
                tag.setLabel(tagStrList.at(i).trimmed());
                tags.append(tag);
                if (currentTags.indexOf(tag) == -1) {
                    tagsChanged = true;
                }
            }
            if (tags.count() != currentTags.count()) {
                tagsChanged = true;
            }
            if (tagsChanged) {
                res.setTags(tags);
            }
        }
        if (fields.contains("associatedImage")) {
            QString associatedImage = fields["associatedImage"].toString();
            if (!associatedImage.isEmpty()) {
                Nepomuk::Resource associatedImageRes(associatedImage);
                if (!associatedImageRes.exists()) {
                    associatedImageRes = Nepomuk::Resource(QUrl(associatedImage), QUrl("http://http://www.semanticdesktop.org/ontologies/nfo#Image"));
                }
                res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(associatedImageRes));
            } else {
                if (res.hasProperty(mediaVocabulary.artwork())) {
                    res.removeProperty(mediaVocabulary.artwork());
                }
            }
        }
        if (fields.contains("dataSourceUrl")) {
            QString dataSourceUrl = fields["dataSourceUrl"].toString();
            QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#dataSource");
            if (!dataSourceUrl.isEmpty()) {
                Nepomuk::Resource dataSourceRes(dataSourceUrl);
                if (dataSourceRes.exists()) {
                    res.setProperty(property, Nepomuk::Variant(dataSourceRes));
                }
            } else {
                if (res.hasProperty(property)) {
                    res.removeProperty(property);
                }
            }
        }
        if (fields.contains("isLogicalPartOfUrl")) {
            QString isLogicalPartOfUrl = fields["isLogicalPartOfUrl"].toString();
            QUrl property = QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isLogicalPartOf");
            if (!isLogicalPartOfUrl.isEmpty()) {
                Nepomuk::Resource isLogicalPartOfRes(isLogicalPartOfUrl);
                if (isLogicalPartOfRes.exists()) {
                    res.setProperty(property, Nepomuk::Variant(isLogicalPartOfRes));
                }
            } else {
                if (res.hasProperty(property)) {
                    res.removeProperty(property);
                }
            }
        }
        if (fields.contains("relatedTo")) {
            QUrl relatedProperty("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#relatedTo");
            QStringList related  = fields["relatedTo"].toString().split("||", QString::SkipEmptyParts);
            res.removeProperty(relatedProperty);
            if (!related.isEmpty()) {
                QList<QUrl> relatedUrls;
                for (int i = 0; i < related.count(); i++) {
                    outputMessage(Debug, related.at(i));
                    relatedUrls.append(QUrl(related.at(i)));
                }
                res.setProperty(relatedProperty, relatedUrls);
            }
        }

    }


    //Set Audio properties
    if (type == "Audio") {
        if (fields["audioType"] == "Music") {
            if (fields.contains("artist")) {
                QStringList artists  = fields["artist"].toString().split("||", QString::SkipEmptyParts);
                res.removeProperty(mediaVocabulary.musicArtist());
                if (!artists.isEmpty()) {
                    QList<Nepomuk::Variant> artistResources;
                    for (int i = 0; i < artists.count(); i++) {
                        Nepomuk::Resource artistResource = findPropertyResourceByTitle(mediaVocabulary.musicArtist(), artists.at(i), true);
                        artistResources.append(artistResource);
                    }
                    res.setProperty(mediaVocabulary.musicPerformer(), artistResources);
                } else {
                    if (res.hasProperty(mediaVocabulary.musicPerformer())) {
                        res.removeProperty(mediaVocabulary.musicPerformer());
                    }
                }
            }
            if (fields.contains("composer")) {
                QStringList composers  = fields["composer"].toString().split("||", QString::SkipEmptyParts);
                if (!composers.isEmpty()) {
                    QList<Nepomuk::Variant> composerResources;
                    for (int i = 0; i < composers.count(); i++) {
                        if (!composers.at(i).isEmpty()) {
                            Nepomuk::Resource composerResource = findPropertyResourceByTitle(mediaVocabulary.musicComposer(), composers.at(i), true);
                            composerResources.append(composerResource);
                        }
                    }
                    res.setProperty(mediaVocabulary.musicComposer(), composerResources);
                } else {
                    if (res.hasProperty(mediaVocabulary.musicComposer())) {
                        res.removeProperty(mediaVocabulary.musicComposer());
                    }
                }
            }
            if (fields.contains("album")) {
                QString album   = fields["album"].toString();
                if (!album.isEmpty()) {
                    Nepomuk::Resource albumResource = findPropertyResourceByTitle(mediaVocabulary.musicAlbum(), album, true);
                    res.setProperty(mediaVocabulary.musicAlbum(), Nepomuk::Variant(albumResource));
                } else {
                    if (res.hasProperty(mediaVocabulary.musicAlbum())) {
                        res.removeProperty(mediaVocabulary.musicAlbum());
                    }
                }
            }
            if (fields.contains("genre")) {
                QStringList genres  = fields["genre"].toString().split("||", QString::SkipEmptyParts);
                res.setProperty(mediaVocabulary.genre(), variantListFromStringList(genres));
            }
            if (fields.contains("trackNumber")) {
                int track   = fields["trackNumber"].toInt();
                if (track != 0) {
                    res.setProperty(mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
                } else {
                    if (res.hasProperty(mediaVocabulary.musicTrackNumber())) {
                        res.removeProperty(mediaVocabulary.musicTrackNumber());
                    }
                }
            }
            if (fields.contains("duration")) {
                int duration = fields["duration"].toInt();
                if (duration != 0) {
                    res.setProperty(mediaVocabulary.duration(), Nepomuk::Variant(duration));
                } else {
                    if (res.hasProperty(mediaVocabulary.duration())) {
                        res.removeProperty(mediaVocabulary.duration());
                    }
                }
            }
            if (fields.contains("year")) {
                int year = fields["year"].toInt();
                if (year != 0) {
                    QDate releaseDate = QDate(year, 1, 1);
                    res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                } else {
                    if (res.hasProperty(mediaVocabulary.releaseDate())) {
                        res.removeProperty(mediaVocabulary.releaseDate());
                    }
                }
            }
        } else if ((fields["audioType"] == "Audio Stream") ||
            (fields["audioType"] == "Audio Clip")) {
        }
    }

    //Set Video properties
    if (type == "Video") {
        mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
        if ((fields["videoType"] == "Movie") || (fields["videoType"] == "TV Show")) {
            if (fields.contains("genre")) {
                QStringList genres  = fields["genre"].toString().split("||", QString::SkipEmptyParts);
                res.setProperty(mediaVocabulary.genre(), variantListFromStringList(genres));
            }
            if (fields.contains("synopsis")) {
                QString synopsis   = fields["synopsis"].toString();
                res.setProperty(mediaVocabulary.videoSynopsis(), Nepomuk::Variant(synopsis));
            }
            if (fields.contains("year")) {
                int year = fields["year"].toInt();
                if (year != 0) {
                    QDate releaseDate = QDate(year, 1, 1);
                    res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                } else {
                    if (res.hasProperty(mediaVocabulary.releaseDate())) {
                        res.removeProperty(mediaVocabulary.releaseDate());
                    }
                }
            }
            if (fields.contains("releaseDate")) {
                Nepomuk::Variant value;
                QDate releaseDate = fields["releaseDate"].toDate();
                if (releaseDate.isValid()) {
                    value = Nepomuk::Variant(releaseDate);
                }
                res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
            }
            if (fields.contains("writer")) {
                QStringList writers  = fields["writer"].toString().split("||", QString::SkipEmptyParts);
                if (!writers.isEmpty()) {
                    QList<Nepomuk::Variant> writerResources;
                    for (int i = 0; i < writers.count(); i++) {
                        Nepomuk::Resource writerResource = findPropertyResourceByTitle(mediaVocabulary.videoWriter(), writers.at(i), true);
                        writerResources.append(writerResource);
                    }
                    res.setProperty(mediaVocabulary.videoWriter(), writerResources);
                } else {
                    if (res.hasProperty(mediaVocabulary.videoWriter())) {
                        res.removeProperty(mediaVocabulary.videoWriter());
                    }
                }
            }
            if (fields.contains("director")) {
                QStringList directors  = fields["director"].toString().split("||", QString::SkipEmptyParts);
                if (!directors.isEmpty()) {
                    QList<Nepomuk::Variant> directorResources;
                    for (int i = 0; i < directors.count(); i++) {
                        Nepomuk::Resource directorResource = findPropertyResourceByTitle(mediaVocabulary.videoDirector(), directors.at(i), true);
                        directorResources.append(directorResource);
                    }
                    res.setProperty(mediaVocabulary.videoDirector(), directorResources);
                } else {
                    if (res.hasProperty(mediaVocabulary.videoDirector())) {
                        res.removeProperty(mediaVocabulary.videoDirector());
                    }
                }
            }
            if (fields.contains("producer")) {
                QStringList producers  = fields["producer"].toString().split("||", QString::SkipEmptyParts);
                if (!producers.isEmpty()) {
                    QList<Nepomuk::Variant> producerResources;
                    for (int i = 0; i < producers.count(); i++) {
                        Nepomuk::Resource producerResource = findPropertyResourceByTitle(mediaVocabulary.videoProducer(), producers.at(i), true);
                        producerResources.append(producerResource);
                    }
                    res.setProperty(mediaVocabulary.videoProducer(), producerResources);
                } else {
                    if (res.hasProperty(mediaVocabulary.videoProducer())) {
                        res.removeProperty(mediaVocabulary.videoProducer());
                    }
                }
            }
            if (fields.contains("actor")) {
                QStringList actors  = fields["actor"].toString().split("||", QString::SkipEmptyParts);
                if (!actors.isEmpty()) {
                    QList<Nepomuk::Variant> actorResources;
                    for (int i = 0; i < actors.count(); i++) {
                        Nepomuk::Resource actorResource = findPropertyResourceByTitle(mediaVocabulary.videoActor(), actors.at(i), true);
                        actorResources.append(actorResource);
                    }
                    res.setProperty(mediaVocabulary.videoActor(), actorResources);
                } else {
                    if (res.hasProperty(mediaVocabulary.videoActor())) {
                        res.removeProperty(mediaVocabulary.videoActor());
                    }
                }
            }

            if (fields["videoType"] == "TV Show") {
                if (fields.contains("seriesName")) {
                    QString seriesName = fields["seriesName"].toString();
                    if (!seriesName.isEmpty()) {
                        Nepomuk::Resource seriesResource = findPropertyResourceByTitle(mediaVocabulary.videoSeries(), seriesName, true);
                        res.setProperty(mediaVocabulary.videoSeries(), Nepomuk::Variant(seriesResource));
                    } else {
                        if (res.hasProperty(mediaVocabulary.videoSeries())) {
                            res.removeProperty(mediaVocabulary.videoSeries());
                        }
                    }
                }
                if (fields.contains("season")) {
                    int season = fields["season"].toInt();
                    if (season != 0) {
                        res.setProperty(mediaVocabulary.videoSeason(), Nepomuk::Variant(season));
                    } else {
                        if (res.hasProperty(mediaVocabulary.videoSeason())) {
                            res.removeProperty(mediaVocabulary.videoSeason());
                        }
                    }
                }
                if (fields.contains("episodeNumber")) {
                    int episodeNumber = fields["episodeNumber"].toInt();
                    if (episodeNumber != 0) {
                        res.setProperty(mediaVocabulary.videoEpisodeNumber(), Nepomuk::Variant(episodeNumber));
                    } else {
                        if (res.hasProperty(mediaVocabulary.videoEpisodeNumber())) {
                            res.removeProperty(mediaVocabulary.videoEpisodeNumber());
                        }
                    }
                }
            }
        }
    }

    outputMessage(InfoUpdated, QUrl::toPercentEncoding(url));
}

void NepomukWriter::removeType(Nepomuk::Resource res, QUrl mediaType)
{
    QList<QUrl> types = res.types();
    for (int i = 0; i < types.count(); i++) {
        if (types.at(i).toString() == mediaType.toString()) {
            types.removeAt(i);
            break;
        }
    }
    res.setTypes(types);
}

void NepomukWriter::outputMessage(MessageType messageType, QString urlOrProgressOrMessage)
{
    QTextStream cout(stdout, QIODevice::WriteOnly);
    if (messageType == InfoRemoved) {
        cout << "BangrangSignal:urlInfoRemoved:" << urlOrProgressOrMessage << "\n";
    } else if (messageType == InfoUpdated) {
        cout << "BangarangSignal:sourceInfoUpdated:" << urlOrProgressOrMessage << "\n";
    } else if (messageType == Progress) {
        cout << "BangarangProgress:" << urlOrProgressOrMessage << "\n";
    } else if (messageType == Message) {
        cout << "BangarangMessage:" << urlOrProgressOrMessage << "\n";
    } else if (messageType == Debug) {
        cout << "BangarangDebug:" << urlOrProgressOrMessage << "\n";
    } else if (messageType == Error) {
        cout << "BangarangError:" << urlOrProgressOrMessage << "\n";
    }
    cout.flush();
}

Nepomuk::Resource NepomukWriter::findPropertyResourceByTitle(QUrl property, QString title, bool createIfMissing)
{
    //First look in cache to see if this resource was previously found
    QString cacheKey = QString("%1:%2").arg(property.toString()).arg(title);
    if (m_propertyResourceCache.contains(cacheKey)) {
        return Nepomuk::Resource::fromResourceUri(m_propertyResourceCache.value(cacheKey));
    }

    //Query nepomuk store for resource
    MediaVocabulary mediaVocabulary;
    QUrl resourceProperty;
    QUrl resourceType;
    if (property == mediaVocabulary.musicArtist()) {
        resourceProperty = mediaVocabulary.musicArtistName();
        resourceType = mediaVocabulary.typeMusicArtist();
    } else if (property == mediaVocabulary.musicComposer()) {
        resourceProperty = mediaVocabulary.musicArtistName();
        resourceType = mediaVocabulary.typeMusicArtist();
    } else if (property == mediaVocabulary.musicAlbum()) {
        resourceProperty = mediaVocabulary.musicAlbumName();
        resourceType = mediaVocabulary.typeMusicAlbum();
    } else if (property == mediaVocabulary.videoSeries()) {
        resourceProperty = mediaVocabulary.videoSeriesTitle();
        resourceType = mediaVocabulary.typeTVSeries();
    } else if (property == mediaVocabulary.videoWriter() ||
               property == mediaVocabulary.videoDirector() ||
               property == mediaVocabulary.videoAssistantDirector() ||
               property == mediaVocabulary.videoProducer() ||
               property == mediaVocabulary.videoActor() ||
               property == mediaVocabulary.videoCinematographer() ) {
        resourceProperty = mediaVocabulary.ncoFullname();
        resourceType = mediaVocabulary.typeNCOContact();
    }

    QString resourceBinding = "pres";
    QString valueBinding = "title";
    QString typeBinding = "type";

    MediaQuery query;
    QStringList bindings;
    bindings.append(resourceBinding);
    bindings.append(typeBinding);
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    query.addCondition(MediaQuery::hasProperty(resourceBinding, resourceProperty, valueBinding));
    query.addCondition(MediaQuery::addOptional(MediaQuery::hasProperty(resourceBinding, Soprano::Vocabulary::RDF::type(), typeBinding)));
    query.startFilter();
    query.addFilterConstraint(valueBinding, title, MediaQuery::Equal);
    query.endFilter();
    query.endWhere();

    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());

    KUrl propertyResource;
    QUrl type;
    while( it.next() ) {
        propertyResource = it.binding(resourceBinding).uri();
        type = it.binding(typeBinding).uri();
        if (!type.isEmpty() && type == resourceType) {
            break; //There should only be one resource of the specified type with the specified title.
        }
    }

    if (!propertyResource.isEmpty()) {
        m_propertyResourceCache[cacheKey] = propertyResource;
        Nepomuk::Resource resource;
        resource =  Nepomuk::Resource::fromResourceUri(propertyResource);
        //If found resource has no type, assign the correct type and use it.
        if (type.isEmpty()) {
            resource.addType(resourceType);
        }
        return resource;
    } else if (createIfMissing){
        Nepomuk::Resource resource;
        resource.setProperty(resourceProperty, Nepomuk::Variant(title));
        resource.addType(resourceType);
        m_propertyResourceCache[cacheKey] = resource.resourceUri();
        return resource;
    } else {
        return Nepomuk::Resource();
    }
}

void NepomukWriter::removeUnusedPropertyResources()
{
    outputMessage(Message, i18n("Identifying unused resources..."));
    MediaVocabulary mediaVocabulary;
    QString resourceBinding = "pres";
    QString playableResourceBinding = "r";
    QString playableResourceProperty = "p";
    MediaQuery query;
    QStringList bindings;
    bindings.append(resourceBinding);
    bindings.append("name");
    query.select(bindings, MediaQuery::Distinct);
    query.startWhere();
    QString typeUnion = QString("{%1} UNION {%2} UNION {%3} UNION {%4} ")
                        .arg(MediaQuery::hasType(resourceBinding, mediaVocabulary.typeMusicArtist()))
                        .arg(MediaQuery::hasType(resourceBinding, mediaVocabulary.typeMusicAlbum()))
                        .arg(MediaQuery::hasType(resourceBinding, mediaVocabulary.typeTVSeries()))
                        .arg(MediaQuery::hasType(resourceBinding, mediaVocabulary.typeNCOContact()));
    query.addCondition(typeUnion);
    query.addCondition(MediaQuery::addOptional(MediaQuery::hasProperty(resourceBinding, mediaVocabulary.musicArtistName(), "name")));
    query.addCondition(MediaQuery::addOptional(QString("?%1 ?%2 ?%3 . ")
                                               .arg(playableResourceBinding)
                                               .arg(playableResourceProperty)
                                               .arg(resourceBinding)));
    query.startFilter();
    query.addFilterConstraint(playableResourceProperty, QString(), MediaQuery::NotBound);
    query.endFilter();
    query.endWhere();

    Soprano::QueryResultIterator it = query.executeSelect(Nepomuk::ResourceManager::instance()->mainModel());
    outputMessage(Message, i18n("Tidying up unused resources..."));
    while( it.next() ) {
        QUrl propertyResource = it.binding(resourceBinding).uri();
        Nepomuk::Resource resource = Nepomuk::Resource::fromResourceUri(propertyResource);
        QString name = it.binding("name").literal().toString().trimmed();
        outputMessage(Message, i18n("Tidying up unused resources: %1", name));
        resource.remove();
    }

}

QList<Nepomuk::Variant> NepomukWriter::variantListFromStringList(const QStringList &stringList)
{
    QList<Nepomuk::Variant> variantList;
    for (int i = 0; i < stringList.count(); i++) {
        variantList.append(Nepomuk::Variant(stringList.at(i)));
    }
    return variantList;
}
