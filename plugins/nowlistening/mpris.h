/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010-2011 Ramin Gomari <ramin.gomari@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/
#ifndef MPRIS_H
#define MPRIS_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

/**
implament of MPRIS
see more details @ http://xmms2.org/wiki/MPRIS
@author Ramin Gomari \<ramin.gomari@gmail.com\>
*/
class MPRIS
{
public:
    struct MprisStatusStruct
    {
        MprisStatusStruct() {
            Status = hasShuffle = hasRepeat = hasPlaylistRepeat = -1;
        };

        int Status;            // 0 = Playing, 1 = Paused, 2 = Stopped.
        int hasShuffle;        // 0 = Playing linearly , 1 = Playing randomly.
        int hasRepeat;         // 0 = Go to the next element once the current has finished playing , 1 = Repeat the current element
        int hasPlaylistRepeat; // 0 = Stop playing once the last element has been played, 1 = Never give up playing
    };
    MPRIS(const QString  PlayerName);
    ~MPRIS();

    static QStringList getRunningPlayers();

    /**

    @return true if this query was a valid query
    */
    bool isValid() {
        return valid;
    }


    bool isPlaying() {
        return status.Status==0;
    }
    bool isPaused() {
        return status.Status==1;
    }
    bool isStopped() {
        return status.Status==2;
    }
    bool hasRepeat() {
        return status.hasRepeat==1;
    }
    bool hasPlaylistRepeat() {
        return status.hasPlaylistRepeat==1;
    }

    const QString getLocation() {     // Url to the media (local files are represented as a file:// url)
        return trackInfo["location"].toString();
    }
    const QString getTitle() {        // Name
        return trackInfo["title"].toString();
    }
    const QString getArtist() {        // Name of artist or band performing the work
        return trackInfo["artist"].toString();
    }
    const QString getAlbum() {        // Name of compilation the work is part of
        return trackInfo["album"].toString();
    }
    const QString getTracknumber() {    // The position if it's part of a larger set, it may have to be converted to an integer. This MAY be extended with a '/' character and a numeric string containing the total number of elements in the set. E.g. '69/1337'.
        return trackInfo["tracknumber"].toString();
    }
    uint getTime() {        // The duration in seconds
        return trackInfo["time"].toUInt();
    }
    uint getMtime() {        // The duration in milliseconds
        return trackInfo["mtime"].toUInt();
    }
    const QString getGenre() {        // The genre. This MAY begin with a numerical value reflecting the genre in a previously known array of genres, such as ID3 genres. See http://www.linuxselfhelp.com/HOWTO/MP3-HOWTO-13.html#ss13.3
        return trackInfo["genre"].toString();
    }
    const QString getComment() {    // A comment about the work
        return trackInfo["comment"].toString();
    }
    uint getRating() {        // A 'taste' rate value, out of 5. 0-5 or 1-5?
        return trackInfo["rating"].toUInt();
    }
    uint getYear() {        // The year when the performing was realized, i.e. 2007.
        return trackInfo["year"].toUInt();
    }
    uint getDate() {        // When the performing was realized, for precise needs. It is represented as epoch, i.e. the number of seconds since « 00:00:00 1970-01-01 UTC »
        return trackInfo["date"].toUInt();
    }
    const QString getArturl() {        // An URI to an image associated with the work
        return trackInfo["arturl"].toString();
    }
    const QVariantMap getTrackMetadata() {
        return trackInfo;
    }
    const QString getPlayerIdentification() {
        return Identity;
    }
private:
    bool valid;
    MprisStatusStruct status;
    QVariantMap trackInfo;
    QString Identity;
};

#endif // MPRIS_H
