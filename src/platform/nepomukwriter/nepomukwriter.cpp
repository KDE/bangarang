#include "../mediavocabulary.h"
#include "nepomukwriter.h"
#include <KDebug>

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
    Nepomuk::Resource res = Nepomuk::Resource(QUrl::fromEncoded(resourceUri.toUtf8()));

    //Write media metadata info to nepomuk store;
    // Update the media type
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
    //Set properties common to all media types
    if (!res.hasProperty(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"))) {
        res.setProperty(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"),
                        QUrl(url));
    }
    if (fields.contains("rating")) {
        unsigned int rating = fields["rating"].toInt();
        if (rating != res.rating()) {
            res.setRating(rating);
        }
    }
    if (fields.contains("playCount")) {
        int playCount = fields["playCount"].toInt();
        writeProperty(mediaVocabulary, res, mediaVocabulary.playCount(), Nepomuk::Variant(playCount));
    }
    if (fields.contains("lastPlayed")) {
        Nepomuk::Variant value;
        QDateTime lastPlayed = fields["lastPlayed"].toDateTime();
        if (lastPlayed.isValid()) {
            value = Nepomuk::Variant(lastPlayed);
        }
        writeProperty(mediaVocabulary, res, mediaVocabulary.lastPlayed(), value);
    }
    if (fields.contains("title")) {
        QString title = fields["title"].toString();
        writeProperty(mediaVocabulary, res, mediaVocabulary.title(), Nepomuk::Variant(title));
    }
    if (fields.contains("description")) {
        QString description = fields["description"].toString();
        writeProperty(mediaVocabulary, res, mediaVocabulary.description(), Nepomuk::Variant(description));
    }
    if (fields.contains("tags")) {
        QStringList tagStrList = fields["tags"].toString().split(";", QString::SkipEmptyParts);
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

    //Set type-specific properties
    if (type == "Audio") {
        if (fields["audioType"] == "Music") {
            if (fields.contains("artist")) {
                QString artist  = fields["artist"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.musicArtist(), Nepomuk::Variant(artist));
            }
            if (fields.contains("album")) {
                QString album   = fields["album"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.musicAlbum(), Nepomuk::Variant(album));
            }
            if (fields.contains("genre")) {
                QString genre  = fields["genre"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.genre(), Nepomuk::Variant(genre));
            }
            if (fields.contains("trackNumber")) {
                int track   = fields["trackNumber"].toInt();
                if (track != 0) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
                } else {
                    if (res.hasProperty(mediaVocabulary.musicTrackNumber())) {
                        res.removeProperty(mediaVocabulary.musicTrackNumber());
                    }
                }
            }
            if (fields.contains("duration")) {
                int duration = fields["duration"].toInt();
                if (duration != 0) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.duration(), Nepomuk::Variant(duration));
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
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                } else {
                    if (res.hasProperty(mediaVocabulary.releaseDate())) {
                        res.removeProperty(mediaVocabulary.releaseDate());
                    }
                }
            }
        } else if ((fields["audioType"] == "Audio Stream") ||
            (fields["audioType"] == "Audio Clip")) {
        }
    } else if (type == "Video") {
        mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
        if ((fields["videoType"] == "Movie") || (fields["videoType"] == "TV Show")) {
            if (fields.contains("genre")) {
                QString genre   = fields["genre"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.genre(), Nepomuk::Variant(genre));
            }
            if (fields.contains("synopsis")) {
                QString synopsis   = fields["synopsis"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.videoSynopsis(), Nepomuk::Variant(synopsis));
            }
            if (fields.contains("year")) {
                int year = fields["year"].toInt();
                if (year != 0) {
                    QDate releaseDate = QDate(year, 1, 1);
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
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
                writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(value));
            }
            if (fields.contains("writer")) {
                QString writer   = fields["writer"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.videoWriter(), Nepomuk::Variant(writer));
            }
            if (fields.contains("director")) {
                QString director   = fields["director"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.videoDirector(), Nepomuk::Variant(director));
            }
            if (fields.contains("producer")) {
                QString producer   = fields["producer"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.videoProducer(), Nepomuk::Variant(producer));
            }
            if (fields.contains("actor")) {
                QString actor   = fields["actor"].toString();
                writeProperty(mediaVocabulary, res, mediaVocabulary.videoActor(), Nepomuk::Variant(actor));
            }

            if (fields["videoType"] == "TV Show") {
                if (fields.contains("seriesName")) {
                    QString seriesName = fields["seriesName"].toString();
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoSeries(), Nepomuk::Variant(seriesName));
                }
                if (fields.contains("season")) {
                    int season = fields["season"].toInt();
                    if (season != 0) {
                        writeProperty(mediaVocabulary, res, mediaVocabulary.videoSeason(), Nepomuk::Variant(season));
                    } else {
                        if (res.hasProperty(mediaVocabulary.videoSeason())) {
                            res.removeProperty(mediaVocabulary.videoSeason());
                        }
                    }
                }
                if (fields.contains("episodeNumber")) {
                    int episodeNumber = fields["episodeNumber"].toInt();
                    if (episodeNumber != 0) {
                        writeProperty(mediaVocabulary, res, mediaVocabulary.videoEpisodeNumber(), Nepomuk::Variant(episodeNumber));
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

void NepomukWriter::writeProperty(MediaVocabulary mediaVocabulary,
                   Nepomuk::Resource res, QUrl property, Nepomuk::Variant value)
{

    QTextStream cout(stdout, QIODevice::WriteOnly);
    QStringList storageProcedure = mediaVocabulary.storageProcedure(property);
    Nepomuk::Resource valueResource;
    for (int i = 0; i < storageProcedure.count(); i++) {
        QString step = storageProcedure.at(i);
        if (step == "[Resource]::[Property]::[Value]") {
            if ((value != res.property(property) ) ) {
                res.setProperty(property, value);
            }
        } else if (step.startsWith("[ResourceValue]") && step.endsWith("[Value]")) {
            QUrl propertyForValueResource = QUrl(step.split("::").at(1));
            if (value != valueResource.property(propertyForValueResource)) {
                valueResource.setProperty(propertyForValueResource, value);
            }
        } else {
            QString stepValue = step.split("::").at(2);
            if (stepValue == "[ResourceValue]") {
                i++;
                if (i < storageProcedure.count()) {
                    QString valueResourceTypeStep = storageProcedure.at(i);
                    if (valueResourceTypeStep.startsWith("[ResourceValue]::[Type]")) {
                        //Create value resource
                        QUrl valueResourceType = QUrl(valueResourceTypeStep.split("::").at(2));
                        QString valueResourceIdentifier;
                        if (property == mediaVocabulary.videoSeries()) {
                            valueResourceIdentifier = QString("nmm-tvseries-%1").arg(value.toString());
                        } else if (property == mediaVocabulary.musicArtist()) {
                            valueResourceIdentifier = QString("music-artist-%1").arg(value.toString());
                        } else if (property == mediaVocabulary.musicAlbum()) {
                            valueResourceIdentifier = QString("music-album-%1").arg(value.toString());
                        } else {
                            valueResourceIdentifier = value.toString();
                        }
                        valueResource = Nepomuk::Resource(valueResourceIdentifier);
                        if (!valueResource.exists()) {
                            valueResource = Nepomuk::Resource(valueResourceIdentifier, valueResourceType);
                        }

                        //Set property of primary resource to the new value resource
                        if (Nepomuk::Variant(valueResource) != res.property(property)) {
                            res.setProperty(property, Nepomuk::Variant(valueResource));
                        }
                    }
                }
            }
        }
    }
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
    } else if (messageType == Error) {
        cout << "BangarangError:" << urlOrProgressOrMessage << "\n";
    }
    cout.flush();
}
