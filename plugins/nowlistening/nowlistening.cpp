/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010-2011 Ramin Gomari <ramin.gomari@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "nowlistening.h"

#include <QAction>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KMessageBox>

#include "choqokuiglobal.h"
#include "nowlisteningsettings.h"
#include "quickpost.h"

#include "mpris.h"

K_PLUGIN_CLASS_WITH_JSON(NowListening, "choqok_nowlistening.json")

NowListening::NowListening(QObject *parent, const QList<QVariant> &)
    : Choqok::Plugin(QLatin1String("choqok_nowlistening"), parent)
{
    QAction *action = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")), i18n("Now Listening"), this);
    actionCollection()->addAction(QLatin1String("nowListening"), action);
    connect(action, &QAction::triggered, this, &NowListening::slotPrepareNowListening);
    setXMLFile(QLatin1String("nowlisteningui.rc"));
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

    MPRIS amarok(QLatin1String("amarok"));
    if (amarok.isValid()) {
        if (amarok.isPlaying()) {
            trackInfo = amarok.getTrackMetadata();
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("Amarok");
    }

    MPRIS audacious(QLatin1String("audacious"));
    if (!isPlaying && audacious.isValid()) {
        if (audacious.isPlaying()) {
            trackInfo = audacious.getTrackMetadata();
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("Audacious");
    }

    // MPRIS id of Dragon Player contain pid of it.
    QStringList playersList = MPRIS::getRunningPlayers();
    if (!playersList.isEmpty() && playersList.indexOf(QRegExp(QLatin1String("dragonplayer(.*)"))) > -1) {
        int i = playersList.indexOf(QRegExp(QLatin1String("dragonplayer(.*)")));
        MPRIS dragon(playersList.at(i));
        if (!isPlaying && dragon.isValid()) {
            if (dragon.isPlaying()) {
                trackInfo = dragon.getTrackMetadata();
                isPlaying = true;
            }
            playerFound = true;
            player = QLatin1String("Dragon Player");
        }
    }

    //need to enable MPRIS Plugin (Qmmp +0.4)
    MPRIS qmmp(QLatin1String("qmmp"));
    if (!isPlaying && qmmp.isValid()) {
        if (qmmp.isPlaying()) {
            trackInfo = qmmp.getTrackMetadata();
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("Qmmp");
    }

    // only works if enabled D-BUS control interface in VLC (VLC 0.9.0+)
    MPRIS vlc(QLatin1String("vlc"));
    if (!isPlaying && vlc.isValid()) {
        if (vlc.isPlaying()) {
            trackInfo = vlc.getTrackMetadata();
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("VLC");
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

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.juk")).value()) {
        QDBusInterface jukPlayer(QLatin1String("org.kde.juk"),
                                 QLatin1String("/Player"),
                                 QLatin1String("org.kde.juk.player"));
        if (((QDBusReply<bool>)jukPlayer.call(QLatin1String("playing"))).value()) {
            QDBusReply< QString> reply = jukPlayer.call(QLatin1String("trackProperty"), QLatin1String("Title"));
            trackInfo.insert(QLatin1String("title"), reply.value());

            reply = jukPlayer.call(QLatin1String("trackProperty"), QLatin1String("Track"));
            trackInfo.insert(QLatin1String("track"), reply.value());

            reply = jukPlayer.call(QLatin1String("trackProperty"), QLatin1String("Album"));
            trackInfo.insert(QLatin1String("album"), reply.value());

            reply = jukPlayer.call(QLatin1String("trackProperty"), QLatin1String("Artist"));
            trackInfo.insert(QLatin1String("artist"), reply.value());

            reply = jukPlayer.call(QLatin1String("trackProperty"), QLatin1String("Year"));
            trackInfo.insert(QLatin1String("year"), reply.value());

            reply = jukPlayer.call(QLatin1String("trackProperty"), QLatin1String("Genre"));
            trackInfo.insert(QLatin1String("genre"), reply.value());
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("JuK");
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.gnome.Rhythmbox")).value())  {
        QDBusInterface rhythmboxPlayer(QLatin1String("org.gnome.Rhythmbox") ,
                                       QLatin1String("/org/gnome/Rhythmbox/Player"),
                                       QLatin1String("org.gnome.Rhythmbox.Player"));
        if (((QDBusReply<bool>)rhythmboxPlayer.call(QLatin1String("getPlaying"))).value()) {
            QDBusReply<QString> uri = rhythmboxPlayer.call(QLatin1String("getPlayingUri"));

            QDBusInterface rhythmboxShell(QLatin1String("org.gnome.Rhythmbox") ,
                                          QLatin1String("/org/gnome/Rhythmbox/Shell"),
                                          QLatin1String("org.gnome.Rhythmbox.Shell"));

            QDBusReply< QMap<QString, QVariant> > reply = rhythmboxShell.call(QLatin1String("getSongProperties"), uri.value());
            trackInfo = reply.value();
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("Rhythmbox");
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.exaile.Exaile")).value()) {

        QDBusInterface exailePlayer(QLatin1String("org.exaile.Exaile"),
                                    QLatin1String("/org/exaile/Exaile"),
                                    QLatin1String("org.exaile.Exaile"));
        if (((QDBusReply<bool>) exailePlayer.call(QLatin1String("IsPlaying"))).value()) {
            QDBusReply<QString> reply = exailePlayer.call(QLatin1String("GetTrackAttr"), QLatin1String("tracknumber"));
            trackInfo.insert(QLatin1String("tracknumber"), reply.value());
            reply = exailePlayer.call(QLatin1String("GetTrackAttr"), QLatin1String("title"));
            trackInfo.insert(QLatin1String("title"), reply.value());
            reply = exailePlayer.call(QLatin1String("GetTrackAttr"), QLatin1String("album"));
            trackInfo.insert(QLatin1String("album"), reply.value());
            reply = exailePlayer.call(QLatin1String("GetTrackAttr"), QLatin1String("artist"));
            trackInfo.insert(QLatin1String("artist"), reply.value());
            reply = exailePlayer.call(QLatin1String("GetTrackAttr"), QLatin1String("year"));
            trackInfo.insert(QLatin1String("year"), reply.value());
            reply = exailePlayer.call(QLatin1String("GetTrackAttr"), QLatin1String("genre"));
            trackInfo.insert(QLatin1String("genre"), reply.value());
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("Exaile");
    }

    if (!isPlaying && QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.bansheeproject.Banshee")).value()) {
        // provide for new interface in Banshee 1.0+
        QDBusInterface bansheePlayer(QLatin1String("org.bansheeproject.Banshee"),
                                     QLatin1String("/org/bansheeproject/Banshee/PlayerEngine"),
                                     QLatin1String("org.bansheeproject.Banshee.PlayerEngine"));
        if (!((QDBusReply<QString>) bansheePlayer.call(QLatin1String("GetCurrentState"))).value().compare(QLatin1String("playing"))) {
            QDBusReply< QMap<QString, QVariant> > reply = bansheePlayer.call(QLatin1String("GetCurrentTrack"));
            trackInfo = reply.value();
            trackInfo.insert(QLatin1String("title"), trackInfo[QLatin1String("name")]);
            isPlaying = true;
        }
        playerFound = true;
        player = QLatin1String("Banshee");
    }

    //trying to find not supported players that implamented the MPRIS-Dbus interface
    if (!isPlaying && !MPRIS::getRunningPlayers().isEmpty()) {

        for (const QString &playerName: MPRIS::getRunningPlayers()) {
            playerFound = true;
            MPRIS mprisPlayer(playerName);
            if (mprisPlayer.isValid() && mprisPlayer.isPlaying()) {
                trackInfo = mprisPlayer.getTrackMetadata();
                isPlaying = true;
                player = mprisPlayer.getPlayerIdentification().left(
                             mprisPlayer.getPlayerIdentification().lastIndexOf(QLatin1Char(' '))); //remove the version of player
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
          KMessageBox::error(Choqok::UI::Global::mainWindow(),
                             i18n("No supported player found."));
        return;
    }

    NowListeningSettings::self()->load();
    QString text = NowListeningSettings::templateString();
    text.replace(QLatin1String("%track%"), trackInfo[QLatin1String("tracknumber")].toString());
    text.replace(QLatin1String("%title%"), trackInfo[QLatin1String("title")].toString());
    text.replace(QLatin1String("%album%"), trackInfo[QLatin1String("album")].toString());
    text.replace(QLatin1String("%artist%"), trackInfo[QLatin1String("artist")].toString());
    text.replace(QLatin1String("%year%"), trackInfo[QLatin1String("year")].toString());
    text.replace(QLatin1String("%genre%"), trackInfo[QLatin1String("genre")].toString());
    text.replace(QLatin1String("%player%"), player);

    if (Choqok::UI::Global::quickPostWidget()) {
        Choqok::UI::Global::quickPostWidget()->setText(text);
    }
}

#include "nowlistening.moc"
