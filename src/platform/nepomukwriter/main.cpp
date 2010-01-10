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

#include "../mediavocabulary.h"
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KLocalizedString>
#include <KAboutData>
#include <KDebug>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <KApplication>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QUrl>

static KAboutData aboutData( "bangarangnepomukwriter", 0,
                             KLocalizedString(), "0.92 (1.0~beta2)",
                             KLocalizedString(), KAboutData::License_GPL_V3,
        ki18n("Copyright 2009, Andrew Lake"), KLocalizedString(),
        "" );

void removeType(Nepomuk::Resource res, QUrl mediaType)
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

void writeProperty(MediaVocabulary mediaVocabulary,
                   Nepomuk::Resource res, QUrl property, Nepomuk::Variant value)
{
    QStringList storageProcedure = mediaVocabulary.storageProcedure(property);
    Nepomuk::Resource valueResource;
    for (int i = 0; i < storageProcedure.count(); i++) {
        QString step = storageProcedure.at(i);
        //kdebug() << step << "\n";
        if (step == "[Resource]::[Property]::[Value]") {
            res.setProperty(property, value);
        } else if (step.startsWith("[ResourceValue]") && step.endsWith("[Value]")) {
            QUrl propertyForValueResource = QUrl(step.split("::").at(1));
            //kDebug() << "Setting new resource property " << propertyForValueResource.toString() << "\n";
            valueResource.setProperty(propertyForValueResource, value);
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
                        //kdebug() << "Creating resource " << valueResourceIdentifier;
                        valueResource = Nepomuk::Resource(valueResourceIdentifier);
                        if (!valueResource.exists()) {
                            valueResource = Nepomuk::Resource(valueResourceIdentifier, valueResourceType);
                        }
                        
                        //Set property of primary resource to the new value resource
                        //kdebug() << "Pointing to new resource " << valueResource.resourceUri().toString() << "\n";
                        res.setProperty(property, Nepomuk::Variant(valueResource));
                    }
                }
            }
        }
    }
}

void writeToNepomuk(QTextStream &cout, QHash <QString, QVariant> fields)
{
    //QTextStream cout(stdout, QIODevice::WriteOnly);
    
    MediaVocabulary mediaVocabulary = MediaVocabulary();
    QString resourceUri = fields["resourceUri"].toString();
    QString url = fields["url"].toString();
    QString type = fields["type"].toString();
    
    cout << QString("URL: %1\n").arg(url);
    
    //Find resource
    Nepomuk::Resource res = Nepomuk::Resource(QUrl::fromEncoded(resourceUri.toUtf8()));
    
    if (fields["removeInfo"].toString() == "true") {
        
        //Remove metadata for media from nepomuk store
        if (!res.exists()) {
            return;
        }
        if (type == "Audio") {
            // Update the media type
            cout << "Removing type...\n";
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
                cout << "Removing title...\n";
                res.removeProperty(mediaVocabulary.title());
            }
            if (res.hasProperty(mediaVocabulary.description())) {
                cout << "Removing description...\n";
                res.removeProperty(mediaVocabulary.description());
            }
            if (res.hasProperty(mediaVocabulary.artwork())) {
                cout << "Removing artwork...\n";
                res.removeProperty(mediaVocabulary.artwork());
            }
            
            if (fields["audioType"] == "Music") {
                if (res.hasProperty(mediaVocabulary.musicArtist())) {
                    cout << "Removing artist...\n";
                    res.removeProperty(mediaVocabulary.musicArtist());
                }
                if (res.hasProperty(mediaVocabulary.musicAlbum())) {
                    cout << "Removing album...\n";
                    res.removeProperty(mediaVocabulary.musicAlbum());
                }
                if (res.hasProperty(mediaVocabulary.genre())) {
                    cout << "Removing genre...\n";
                    res.removeProperty(mediaVocabulary.genre());
                }
                if (res.hasProperty(mediaVocabulary.musicTrackNumber())) {
                    cout << "Removing trackNumber...\n";
                    res.removeProperty(mediaVocabulary.musicTrackNumber());
                }
                if (res.hasProperty(mediaVocabulary.duration())) {
                    cout << "Removing duration...\n";
                    res.removeProperty(mediaVocabulary.duration());
                }
                if (res.hasProperty(mediaVocabulary.created())) {
                    cout << "Removing year...\n";
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
                cout << "Removing title...\n";
                res.removeProperty(mediaVocabulary.title());
            }
            if (res.hasProperty(mediaVocabulary.description())) {
                cout << "Removing description...\n";
                res.removeProperty(mediaVocabulary.description());
            }
            if (res.hasProperty(mediaVocabulary.artwork())) {
                cout << "Removing artwork...\n";
                res.removeProperty(mediaVocabulary.artwork());
            }
            if (fields["videoType"] == "Movie") {
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
                if (res.hasProperty(mediaVocabulary.videoAssistantDirector())) {
                    res.removeProperty(mediaVocabulary.videoAssistantDirector());
                }
                if (res.hasProperty(mediaVocabulary.videoProducer())) {
                    res.removeProperty(mediaVocabulary.videoProducer());
                }
                if (res.hasProperty(mediaVocabulary.videoActor())) {
                    res.removeProperty(mediaVocabulary.videoActor());
                }
                if (res.hasProperty(mediaVocabulary.videoCinematographer())) {
                    res.removeProperty(mediaVocabulary.videoCinematographer());
                }
                
            } else if (fields["videoType"] == "TV Show") {
                if (res.hasProperty(mediaVocabulary.videoSeries())) {
                    res.removeProperty(mediaVocabulary.videoSeries());
                }
                if (res.hasProperty(mediaVocabulary.videoSeason())) {
                    res.removeProperty(mediaVocabulary.videoSeason());
                }
                if (res.hasProperty(mediaVocabulary.videoEpisodeNumber())) {
                    res.removeProperty(mediaVocabulary.videoEpisodeNumber());
                }
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
                if (res.hasProperty(mediaVocabulary.videoAssistantDirector())) {
                    res.removeProperty(mediaVocabulary.videoAssistantDirector());
                }
                if (res.hasProperty(mediaVocabulary.videoProducer())) {
                    res.removeProperty(mediaVocabulary.videoProducer());
                }
                if (res.hasProperty(mediaVocabulary.videoActor())) {
                    res.removeProperty(mediaVocabulary.videoActor());
                }
                if (res.hasProperty(mediaVocabulary.videoCinematographer())) {
                    res.removeProperty(mediaVocabulary.videoCinematographer());
                }
            }
        }
        cout << "BangrangSignal:urlInfoRemoved:" << QUrl::toPercentEncoding(url) << "\n";
        cout.flush();
        //debug << QString("BangrangSignal:urlInfoRemoved:%1\n").arg(url);
        
    } else {
        //Write media metadata info to nepomuk store;
        // Update the media type
        if (type == "Audio") {
            cout << "Writing type...\n";
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
        }
        
        //Set properties common to all media types
        if (!res.hasProperty(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"))) {
            res.setProperty(QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"),
                            QUrl(url));
        }
        if (fields.contains("rating")) {
            cout << "Setting Rating...\n";
            int rating = fields["rating"].toInt();
            res.setRating(rating);
        }
        if (fields.contains("playCount")) {
            cout << "Setting Play Count...\n";
            int playCount = fields["playCount"].toInt();
            writeProperty(mediaVocabulary, res, mediaVocabulary.playCount(), Nepomuk::Variant(playCount));
            //res.setProperty(mediaVocabulary.playCount(), Nepomuk::Variant(playCount));
        }
        if (fields.contains("lastPlayed")) {
            cout << "Setting Last Played...\n";
            QDateTime lastPlayed = fields["lastPlayed"].toDateTime();
            writeProperty(mediaVocabulary, res, mediaVocabulary.lastPlayed(), Nepomuk::Variant(lastPlayed));
            //res.setProperty(mediaVocabulary.lastPlayed(), Nepomuk::Variant(lastPlayed));
        }
        QString title = fields["title"].toString();
        if (!title.isEmpty()) {
            cout << "Writing title...\n";
            writeProperty(mediaVocabulary, res, mediaVocabulary.title(), Nepomuk::Variant(title));
            //res.setProperty(mediaVocabulary.title(), Nepomuk::Variant(title));
        }
        QString description = fields["description"].toString();
        if (!description.isEmpty()) {
            cout << "Writing description...\n";
            writeProperty(mediaVocabulary, res, mediaVocabulary.description(), Nepomuk::Variant(description));
            //res.setProperty(mediaVocabulary.description(), Nepomuk::Variant(description));
        }
        QString artworkUrl = fields["artworkUrl"].toString();
        if (!artworkUrl.isEmpty()) {
            Nepomuk::Resource artworkRes(artworkUrl);
            cout << "Writing artworkurl...\n";
            if (!artworkRes.exists()) {
                artworkRes = Nepomuk::Resource(QUrl(artworkUrl), QUrl("http://http://www.semanticdesktop.org/ontologies/nfo#Image"));
            }
            res.setProperty(mediaVocabulary.artwork(), Nepomuk::Variant(artworkRes));
        }
        
        //Set type-specific properties
        if (type == "Audio") {
            if (fields["audioType"] == "Music") {
                QString artist  = fields["artist"].toString();
                if (!artist.isEmpty()) {
                    cout << "Writing artist...\n";
                    writeProperty(mediaVocabulary, res, mediaVocabulary.musicArtist(), Nepomuk::Variant(artist));
                    //res.setProperty(mediaVocabulary.musicArtist(), Nepomuk::Variant(artist));
                }
                QString album   = fields["album"].toString();
                if (!album.isEmpty()) {
                    cout << "Writing album...\n";
                    writeProperty(mediaVocabulary, res, mediaVocabulary.musicAlbum(), Nepomuk::Variant(album));
                    //res.setProperty(mediaVocabulary.musicAlbumName(), Nepomuk::Variant(album));
                }
                QString genre  = fields["genre"].toString();
                if (!genre.isEmpty()) {
                    cout << "Writing genre...\n";
                    writeProperty(mediaVocabulary, res, mediaVocabulary.genre(), Nepomuk::Variant(genre));
                    //res.setProperty(mediaVocabulary.genre(), Nepomuk::Variant(genre));
                }
                int track   = fields["trackNumber"].toInt();
                if (track != 0) {
                    cout << "Writing track...\n";
                    writeProperty(mediaVocabulary, res, mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
                    //res.setProperty(mediaVocabulary.musicTrackNumber(), Nepomuk::Variant(track));
                }
                int duration = fields["duration"].toInt();
                if (duration != 0) {
                    cout << "Writing duration...\n";
                    writeProperty(mediaVocabulary, res, mediaVocabulary.duration(), Nepomuk::Variant(duration));
                    //res.setProperty(mediaVocabulary.duration(), Nepomuk::Variant(duration));
                }
                int year = fields["year"].toInt();
                if (year != 0) {
                    QDate releaseDate = QDate(year, 1, 1);
                    cout << "Writing year..." << year << "\n";
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                    //res.setProperty(mediaVocabulary.created(), Nepomuk::Variant(created));
                }
            } else if ((fields["audioType"] == "Audio Stream") ||
                (fields["audioType"] == "Audio Clip")) {
            }
        } else if (type == "Video") {
            mediaVocabulary.setVocabulary(MediaVocabulary::nmm);
            if (fields["videoType"] == "Movie") {
                QString genre   = fields["genre"].toString();
                if (!genre.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.genre(), Nepomuk::Variant(genre));
                    //res.setProperty(mediaVocabulary.genre(), Nepomuk::Variant(genre));
                }
                QString synopsis   = fields["synopsis"].toString();
                if (!synopsis.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoSynopsis(), Nepomuk::Variant(synopsis));
                    //res.setProperty(mediaVocabulary.videoSynopsis(), Nepomuk::Variant(synopsis));
                }
                int year = fields["year"].toInt();
                if (year != 0) {
                    QDate releaseDate = QDate(year, 1, 1);
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                    //res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                }
                QDate releaseDate = fields["releaseDate"].toDate();
                if (releaseDate.isValid()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                    //res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                }
                QString writer   = fields["writer"].toString();
                if (!writer.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoWriter(), Nepomuk::Variant(writer));
                    //res.setProperty(mediaVocabulary.videoWriter(), Nepomuk::Variant(writer));
                }
                QString director   = fields["director"].toString();
                if (!director.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoDirector(), Nepomuk::Variant(director));
                    //res.setProperty(mediaVocabulary.videoDirector(), Nepomuk::Variant(director));
                }
                QString assistantDirector   = fields["assistantDirector"].toString();
                if (!assistantDirector.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoAssistantDirector(), Nepomuk::Variant(assistantDirector));
                    //res.setProperty(mediaVocabulary.videoAssistantDirector(), Nepomuk::Variant(assistantDirector));
                }
                QString producer   = fields["producer"].toString();
                if (!producer.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoProducer(), Nepomuk::Variant(producer));
                    //res.setProperty(mediaVocabulary.videoProducer(), Nepomuk::Variant(producer));
                }
                QString actor   = fields["actor"].toString();
                if (!actor.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoActor(), Nepomuk::Variant(actor));
                    //res.setProperty(mediaVocabulary.videoActor(), Nepomuk::Variant(actor));
                }
                QString cinematographer   = fields["cinematographer"].toString();
                if (!cinematographer.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoCinematographer(), Nepomuk::Variant(cinematographer));
                    //res.setProperty(mediaVocabulary.videoCinematographer(), Nepomuk::Variant(cinematographer));
                }
                
            } else if (fields["videoType"] == "TV Show") {
                QString seriesName = fields["seriesName"].toString();
                if (!seriesName.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoSeries(), Nepomuk::Variant(seriesName));
                    /*Nepomuk::Resource series(fields["seriesName"].toString());
                    if (!series.exists()) {
                        Nepomuk::Resource(fields["seriesName"].toString(), mediaVocabulary.typeTVSeries());
                    }
                    res.setProperty(mediaVocabulary.videoSeries(), Nepomuk::Variant(series));
                    series.setProperty(mediaVocabulary.title(), Nepomuk::Variant(seriesName));*/
                }
                int season = fields["season"].toInt();
                if (season != 0) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoSeason(), Nepomuk::Variant(season));
                    //res.setProperty(mediaVocabulary.videoSeason(), Nepomuk::Variant(season));
                } else {
                    if (res.hasProperty(mediaVocabulary.videoSeason())) {
                        res.removeProperty(mediaVocabulary.videoSeason());
                    }
                }
                int episodeNumber = fields["episodeNumber"].toInt();
                if (episodeNumber != 0) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoEpisodeNumber(), Nepomuk::Variant(episodeNumber));
                    //res.setProperty(mediaVocabulary.videoEpisodeNumber(), Nepomuk::Variant(episodeNumber));
                } else {
                    if (res.hasProperty(mediaVocabulary.videoEpisodeNumber())) {
                        res.removeProperty(mediaVocabulary.videoEpisodeNumber());
                    }
                }
                QString genre   = fields["genre"].toString();
                if (!genre.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.genre(), Nepomuk::Variant(genre));
                    //res.setProperty(mediaVocabulary.genre(), Nepomuk::Variant(genre));
                }
                QString synopsis   = fields["synopsis"].toString();
                if (!synopsis.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoSynopsis(), Nepomuk::Variant(synopsis));
                    //res.setProperty(mediaVocabulary.videoSynopsis(), Nepomuk::Variant(synopsis));
                }
                int year = fields["year"].toInt();
                if (year != 0) {
                    QDate releaseDate = QDate(year, 1, 1);
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                    //res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                }
                QDate releaseDate = fields["releaseDate"].toDate();
                if (releaseDate.isValid()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                    //res.setProperty(mediaVocabulary.releaseDate(), Nepomuk::Variant(releaseDate));
                }
                QString writer   = fields["writer"].toString();
                if (!writer.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoWriter(), Nepomuk::Variant(writer));
                    //res.setProperty(mediaVocabulary.videoWriter(), Nepomuk::Variant(writer));
                }
                QString director   = fields["director"].toString();
                if (!director.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoDirector(), Nepomuk::Variant(director));
                    //res.setProperty(mediaVocabulary.videoDirector(), Nepomuk::Variant(director));
                }
                QString assistantDirector   = fields["assistantDirector"].toString();
                if (!assistantDirector.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoAssistantDirector(), Nepomuk::Variant(assistantDirector));
                    //res.setProperty(mediaVocabulary.videoAssistantDirector(), Nepomuk::Variant(assistantDirector));
                }
                QString producer   = fields["producer"].toString();
                if (!producer.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoProducer(), Nepomuk::Variant(producer));
                    //res.setProperty(mediaVocabulary.videoProducer(), Nepomuk::Variant(producer));
                }
                QString actor   = fields["actor"].toString();
                if (!actor.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoActor(), Nepomuk::Variant(actor));
                    //res.setProperty(mediaVocabulary.videoActor(), Nepomuk::Variant(actor));
                }
                QString cinematographer   = fields["cinematographer"].toString();
                if (!cinematographer.isEmpty()) {
                    writeProperty(mediaVocabulary, res, mediaVocabulary.videoCinematographer(), Nepomuk::Variant(cinematographer));
                    //res.setProperty(mediaVocabulary.videoCinematographer(), Nepomuk::Variant(cinematographer));
                }
            }
        }
        cout << "BangarangSignal:sourceInfoUpdated:" << QUrl::toPercentEncoding(url) << "\n";
        cout.flush();
        //debug << QString("BangarangSignal:sourceInfoUpdated:%1\n").arg(url);
    }
}


int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "File directive" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication application;
    
    QTextStream cout(stdout, QIODevice::WriteOnly);
    
    bool nepomukInited;
    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        nepomukInited = true; //resource manager inited successfully
    } else {
        nepomukInited = false; //no resource manager
    }
    
    if (args->count() > 0) {
        /*QFile debugFile(QString("%1.debug").arg(args->arg(0)));
        debugFile.open(QIODevice::WriteOnly);
        QTextStream debug(&debugFile);*/
        
        QFile file(args->arg(0));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            cout << QString("BangarangError:Couldn't open %1\n").arg(args->arg(0));
            //debug << QString("BangarangError:Couldn't open %1\n").arg(args->arg(0));
            return 0;
        }
        QTextStream in(&file);
        
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
                        //debug << field << ":" << value << "\n";
                    }
                }
            }
            
            line = in.readLine().trimmed();
            if ((line.startsWith("[") && line.endsWith("]")) || in.atEnd()) {
                if (fields.count() > 0) {
                    writeToNepomuk(cout, fields);
                    fields.clear();
                    processed = processed + 1;
                    if (count > 0) {
                        cout << QString("BangarangProgress:%1\n").arg(processed*100/count);
                        cout.flush();
                    }
                }
                //debug << QString("BangarangProgress:%1\n").arg(processed*100/count);
            }
        }
        file.close();
        cout << "Done!\n";
        cout.flush();
    } else {
        cout << "You didn't provide an argument!\n";
    }
    
    return 0;
}
