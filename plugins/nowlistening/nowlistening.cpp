/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010 Ramin Gomari <ramin.gomari@gmail.com>

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

#include "nowlistening.h"
#include <KAction>
#include <KActionCollection>
#include <KAboutData>
#include <KGenericFactory>
#include <QDBusInterface>
#include <QDBusReply>
#include <choqokuiglobal.h>
#include <quickpost.h>
#include "nowlisteningsettings.h"
#include <KMessageBox>
#include <qdbusconnectioninterface.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < NowListening > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_nowlistening" ) )

NowListening::NowListening(QObject* parent, const QList<QVariant>& )
        :Choqok::Plugin(MyPluginFactory::componentData(), parent)
{
    KAction *action = new KAction(KIcon("media-playback-start"), i18n("Now Listening"), this);
    actionCollection()->addAction("nowListening", action);
    connect(action, SIGNAL(triggered(bool)), SLOT(slotPrepareNowListening()));
    setXMLFile("nowlisteningui.rc");
}

NowListening::~NowListening()
{

}

void NowListening::slotPrepareNowListening()
{
    QVariantMap trackInfo;
    QString player;
    bool playerFound = false;
    bool isPlaying = false;

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.amarok").value()) {

        QDBusInterface amarokPlayer ( "org.kde.amarok",
                                      "/Player",
                                      "org.freedesktop.MediaPlayer" );
        /*
        Amarok Player use this enum for GetCaps method
        see original code @ http://gitorious.org/amarok/amarok/blobs/master/src/dbus/PlayerDBusHandler.h
        61     enum DBusCaps {
        62      NONE                  = 0,
        63      CAN_GO_NEXT           = 1 << 0,
        64      CAN_GO_PREV           = 1 << 1,
        65      CAN_PAUSE             = 1 << 2,
        66      CAN_PLAY              = 1 << 3,
        67      CAN_SEEK              = 1 << 4,
        68      CAN_PROVIDE_METADATA  = 1 << 5,
        69      CAN_HAS_TRACKLIST     = 1 << 6
        70     };
    
        see original code @ http://gitorious.org/amarok/amarok/blobs/master/src/dbus/PlayerDBusHandler.cpp
        238 int PlayerDBusHandler::GetCaps()
        239 {
        240     int caps = NONE;
        241     Meta::TrackPtr track = The::engineController()->currentTrack();
        242     caps |= CAN_HAS_TRACKLIST;
        243     if ( track ) caps |= CAN_PROVIDE_METADATA;
        244     if ( GetStatus().Play == 0 /playing/ ) caps |= CAN_PAUSE;
        245     if ( ( GetStatus().Play == 1 /paused/ ) || ( GetStatus().Play == 2 /stoped/ ) ) caps |= CAN_PLAY;
        246     if ( ( GetStatus().Play == 0 /playing/ ) || ( GetStatus().Play == 1 /paused/ ) ) caps |= CAN_SEEK;
        247     if ( ( The::playlist()->activeRow() >= 0 ) && ( The::playlist()->activeRow() <= The::playlist()->qaim()->rowCount() ) )
        248     {
        249         caps |= CAN_GO_NEXT;
        250         caps |= CAN_GO_PREV;
        251     }
        252     return caps;
        253 }
        */
        if (((QDBusReply<int>)amarokPlayer.call ( "GetCaps" )).value()& (1 << 2 /* defined by amarok*/) ) {

            QDBusReply< QMap<QString, QVariant> > reply = amarokPlayer.call ( "GetMetadata" );
            trackInfo = reply.value();
            isPlaying=true;
        }
        playerFound=true;
        player="Amarok";
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.juk").value()) {
        QDBusInterface jukPlayer ( "org.kde.juk",
                                   "/Player",
                                   "org.kde.juk.player" );
        if ( ((QDBusReply<bool>)jukPlayer.call ( "playing" )).value() ) {
            QDBusReply< QString> reply = jukPlayer.call ("trackProperty","Title" );
            trackInfo.insert ( "title",reply.value() );

            reply = jukPlayer.call ( "trackProperty","Track" );
            trackInfo.insert ( "track",reply.value() );

            reply = jukPlayer.call ( "trackProperty","Album" );
            trackInfo.insert ( "album",reply.value() );

            reply = jukPlayer.call ( "trackProperty","Artist" );
            trackInfo.insert ( "artist",reply.value() );

            reply = jukPlayer.call ( "trackProperty","Year" );
            trackInfo.insert ( "year",reply.value() );

            reply = jukPlayer.call ( "trackProperty","Genre" );
            trackInfo.insert ( "genre",reply.value() );
            isPlaying=true;
        }
        playerFound=true;
        player="JuK";
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered("org.gnome.Rhythmbox").value())  {
        QDBusInterface rhythmboxPlayer ( "org.gnome.Rhythmbox" ,
                                         "/org/gnome/Rhythmbox/Player",
                                         "org.gnome.Rhythmbox.Player" );
        if (((QDBusReply<bool>)rhythmboxPlayer.call ( "getPlaying" )).value()) {
            QDBusReply<QString> uri = rhythmboxPlayer.call ( "getPlayingUri" );

            QDBusInterface rhythmboxShell ( "org.gnome.Rhythmbox" ,
                                            "/org/gnome/Rhythmbox/Shell",
                                            "org.gnome.Rhythmbox.Shell" );

            QDBusReply< QMap<QString, QVariant> > reply = rhythmboxShell.call ( "getSongProperties",uri.value() );
            trackInfo=reply.value();
            isPlaying=true;
        }
        playerFound=true;
        player="Rhythmbox";
    }


    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered("org.exaile.Exaile").value()) {

        QDBusInterface exailePlayer ( "org.exaile.Exaile",
                                      "/org/exaile/Exaile",
                                      "org.exaile.Exaile" );
        if (((QDBusReply<bool> ) exailePlayer.call ( "IsPlaying" )).value()) {
            QDBusReply<QString> reply = exailePlayer.call("GetTrackAttr", "tracknumber");
            trackInfo.insert("tracknumber",reply.value());
            reply = exailePlayer.call("GetTrackAttr", "title");
            trackInfo.insert("title",reply.value());
            reply = exailePlayer.call("GetTrackAttr", "album");
            trackInfo.insert("album",reply.value());
            reply = exailePlayer.call("GetTrackAttr", "artist");
            trackInfo.insert("artist",reply.value());
            reply = exailePlayer.call("GetTrackAttr", "year");
            trackInfo.insert("year",reply.value());
            reply = exailePlayer.call("GetTrackAttr", "genre");
            trackInfo.insert("genre",reply.value());
            isPlaying=true;
        }
        playerFound=true;
        player="Exaile";
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered("org.bansheeproject.Banshee").value()) {
        // provide for new interface in Banshee 1.0+
        QDBusInterface bansheePlayer ( "org.bansheeproject.Banshee",
                                       "/org/bansheeproject/Banshee/PlayerEngine",
                                       "org.bansheeproject.Banshee.PlayerEngine" );
        if (!((QDBusReply<QString>) bansheePlayer.call ( "GetCurrentState" )).value().compare ( "playing" )) {
            QDBusReply< QMap<QString, QVariant> > reply = bansheePlayer.call ( "GetCurrentTrack" );
            trackInfo = reply.value();
            trackInfo.insert ( "title", trackInfo["name"] );
            isPlaying=true;
        }
        playerFound=true;
        player="Banshee";
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered("org.atheme.audacious").value()) {
        QDBusInterface audaciousPlayer ( "org.atheme.audacious",
                                         "/org/atheme/audacious",
                                         "org.atheme.audacious" );

        if ( !((QDBusReply<QString>)audaciousPlayer.call ( "Status" )).value().compare ( "playing" ) ) { //if audacious is playing
            QDBusReply<uint> pos = audaciousPlayer.call ( "Position" );
            QDBusReply< QVariant> reply = audaciousPlayer.call ( "SongTuple",pos.value(),"title" );

            trackInfo.insert ( "title",reply.value() );
            reply = audaciousPlayer.call ( "SongTuple",pos.value(),"track-number" );
            trackInfo.insert ( "track",reply.value() );

            reply = audaciousPlayer.call ( "SongTuple",pos.value(),"album" );
            trackInfo.insert ( "album",reply.value() );

            reply = audaciousPlayer.call ( "SongTuple",pos.value(),"artist" );
            trackInfo.insert ( "artist",reply.value() );

            reply = audaciousPlayer.call ( "SongTuple",pos.value(),"year" );
            trackInfo.insert ( "year",reply.value() );

            reply = audaciousPlayer.call ( "SongTuple",pos.value(),"genre" );
            trackInfo.insert ( "genre",reply.value() );
            isPlaying=true;
        }
        playerFound=true;
        player="Audacious";
    }

    if (!isPlaying) {
        if (playerFound)
            KMessageBox::information(Choqok::UI::Global::mainWindow(),
                                     i18n("Play your music player."));
        else
            KMessageBox::sorry(Choqok::UI::Global::mainWindow(),
                               i18n("No supported player found."));
        return;
    }

    NowListeningSettings::self()->readConfig();
    QString text = NowListeningSettings::templateString();
    text.replace("%track%", trackInfo["tracknumber"].toString());
    text.replace("%title%", trackInfo["title"].toString());
    text.replace("%album%", trackInfo["album"].toString());
    text.replace("%artist%", trackInfo["artist"].toString());
    text.replace("%year%", trackInfo["year"].toString());
    text.replace("%genre%", trackInfo["genre"].toString());
    text.replace("%player%", player);

    if (Choqok::UI::Global::quickPostWidget() )
        Choqok::UI::Global::quickPostWidget()->setText(text);
}

// #include "nowlistening.moc"
