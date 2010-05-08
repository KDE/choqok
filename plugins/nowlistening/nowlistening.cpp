/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
//     QDBusInterface rhythmboxPlayer ( "org.gnome.Rhythmbox" ,
//                                      "/org/gnome/Rhythmbox/Player",
//                                      "org.gnome.Rhythmbox.Player" );

    QDBusInterface amarokPlayer ( "org.kde.amarok",
                                  "/Player",
                                  "org.freedesktop.MediaPlayer" );

    QDBusInterface exailePlayer ( "org.exaile.Exaile",
                                  "/org/exaile/Exaile",
                                  "org.exaile.Exaile" );

    // provide for new interface in Banshee 1.0+
//     QDBusInterface bansheePlayer ( "org.bansheeproject.Banshee",
//                                    "/org/bansheeproject/Banshee/PlayerEngine",
//                                    "org.bansheeproject.Banshee.PlayerEngine" );

    QDBusInterface audaciousPlayer ( "org.atheme.audacious",
                                     "/org/atheme/audacious",
                                     "org.atheme.audacious" );

//     QDBusReply<bool> rhythmboxRunning = rhythmboxPlayer.call ( "getPlaying" );
    QDBusReply<bool> exaileRunning = exailePlayer.call ( "IsPlaying" );
//     QDBusReply<QString> bansheeRunning = bansheePlayer.call ( "GetCurrentState" );
    QDBusReply<QString> audaciousRunning = audaciousPlayer.call ( "Status" );
    //TODO Find a way to know if Amarok is playing!?

/*    if ( rhythmboxRunning.value() ) {
        QDBusReply<QString> uri = rhythmboxPlayer.call ( "getPlayingUri" );

        QDBusInterface rhythmboxShell ( "org.gnome.Rhythmbox" ,
                                        "/org/gnome/Rhythmbox/Shell",
                                        "org.gnome.Rhythmbox.Shell" );

        QDBusReply< QMap<QString, QVariant> > reply = rhythmboxShell.call ( "getSongProperties",uri.value() );
        trackInfo=reply.value();
        player="Rhythmbox";
    }else*/ if(exaileRunning.value()){
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
        player="Exaile";
    } /*else if ( !bansheeRunning.value().compare ( "playing" ) ) { //if banshee is playing
        QDBusReply< QMap<QString, QVariant> > reply = bansheePlayer.call ( "GetCurrentTrack" );
        trackInfo = reply.value();
        trackInfo.insert ( "title", trackInfo["name"] );
        player="Banshee";
    }*/ else if ( !audaciousRunning.value().compare ( "playing" ) ) { //if audacious is playing
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

        player="Audacious";
    } else if ( amarokPlayer.isValid() ) {
        QDBusReply< QMap<QString, QVariant> > reply = amarokPlayer.call ( "GetMetadata" );
        trackInfo = reply.value();
        player="Amarok";
    }
    if(player.isEmpty()){
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
