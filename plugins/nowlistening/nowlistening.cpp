/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>
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
#include "mpris.h"

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

    MPRIS amarok ("amarok");
    if (amarok.isValid()) {
        if (amarok.isPlaying()) {
            trackInfo=amarok.getTrackMetadata();
            isPlaying=true;
        }
        playerFound=true;
        player="Amarok";
    }

    MPRIS audacious ("audacious");
    if (!isPlaying && audacious.isValid()) {
        if (audacious.isPlaying()) {
            trackInfo=audacious.getTrackMetadata();
            isPlaying=true;
        }
        playerFound=true;
        player="Audacious";
    }
    
    // MPRIS id of Dragon Player contain pid of it.
    QStringList playersList = MPRIS::getRunningPlayers();
    if ( !playersList.isEmpty() && playersList.indexOf( QRegExp( "dragonplayer(.*)" ) ) > -1) {
      int i = playersList.indexOf( QRegExp( "dragonplayer(.*)" ) );
      MPRIS dragon ( playersList.at( i ) );
      if ( !isPlaying && dragon.isValid() ) {
          if ( dragon.isPlaying() ) {
              trackInfo = dragon.getTrackMetadata();
              isPlaying = true;
          }
          playerFound = true;
          player = "Dragon Player";
      }
    }

    //need to enable MPRIS Plugin (Qmmp +0.4)
    MPRIS qmmp ("qmmp");
    if (!isPlaying && qmmp.isValid()) {
        if (qmmp.isPlaying()) {
            trackInfo=qmmp.getTrackMetadata();
            isPlaying=true;
        }
        playerFound=true;
        player="Qmmp";
    }
    
    // only works if enabled D-BUS control interface in VLC (VLC 0.9.0+)
    MPRIS vlc ( "vlc" );
    if ( !isPlaying && vlc.isValid() ) {
        if ( vlc.isPlaying() ) {
            trackInfo = vlc.getTrackMetadata();
            isPlaying = true;
        }
        playerFound = true;
        player = "VLC";
    }

    //Mpris not complete supported by Kaffeine Version 1.0-svn3
    /*
    MPRIS kaffeine("kaffeine");
    if(!isPlaying && kaffeine.isValid()){
      if(kaffeine.isPlaying()){
    trackInfo=kaffeine.getTrackMetadata();
    isPlaying=true;
      }
      playerFound=true;
      player="Kaffeine";
    }
    */

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

    //trying to find not supported players that implamented the MPRIS-Dbus interface
    if (!isPlaying && !MPRIS::getRunningPlayers().isEmpty()) {
        QStringList players = MPRIS::getRunningPlayers();

        for (int i=0; i<players.size(); i++) { //looking for the first playing player
            playerFound=true;
            QString playerName = players.at(i);
            MPRIS mprisPlayer (playerName);
            if (mprisPlayer.isValid() && mprisPlayer.isPlaying()) {
                trackInfo=mprisPlayer.getTrackMetadata();
                isPlaying=true;
                player = mprisPlayer.getPlayerIdentification().left(
                             mprisPlayer.getPlayerIdentification().lastIndexOf(" ")); //remove the version of player
                break;
            }
        }
    }
    if (!isPlaying) {
        if (playerFound)
            KMessageBox::information(Choqok::UI::Global::mainWindow(),
                                     i18nc("Player is running, But it's not playing.",
                                           "Play your desired music player."));
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
