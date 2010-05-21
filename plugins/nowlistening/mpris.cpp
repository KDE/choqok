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
// This code is loosely inspired by the "now listening" plugin of KMess.
// Therefore some parts are also
// (C) 2006 by Diederik van der Boor vdboor" --at-- "codingdomain.com"

#include <QDBusMetaType>
#include "mpris.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusConnectionInterface>


Q_DECLARE_METATYPE( MPRIS::MprisStatusStruct )

/**
 * Marshall the MprisStatus structure into a DBus argument
 */
QDBusArgument &operator<<( QDBusArgument &argument, const MPRIS::MprisStatusStruct  &status )
{
    argument.beginStructure();
    argument << status.Status << status.hasShuffle << status.hasRepeat << status.hasPlaylistRepeat;
    argument.endStructure();
    return argument;
}



/**
 * Demarshall the MprisPlayerStatus structure from a DBus argument
 */
const QDBusArgument &operator>>( const QDBusArgument &argument, MPRIS::MprisStatusStruct  &status )
{
    argument.beginStructure();
    argument >> status.Status >> status.hasShuffle >> status.hasRepeat >> status.hasPlaylistRepeat;
    argument.endStructure();
    return argument;
}

MPRIS::MPRIS(const QString PlayerName)
{
    qDBusRegisterMetaType<MprisStatusStruct>();
    const QString serviceName = QString("org.mpris.").append(PlayerName);
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName).value())
        valid=true;
    else {
        valid=false;
        return;
    }
    QDBusInterface playerInterface (serviceName,
                                    "/Player",
                                    "org.freedesktop.MediaPlayer");
    QDBusReply<MprisStatusStruct> getStatus = playerInterface.call("GetStatus");
    QDBusReply< QMap<QString, QVariant> > getMetadata = playerInterface.call ( "GetMetadata" );

    if (!getStatus.isValid() || !getMetadata.isValid()) {
        valid=false;
        return;
    }
    status = getStatus.value();
    trackInfo=getMetadata.value();
    QDBusInterface rootInterface(serviceName,
                                 "/",
                                 "org.freedesktop.MediaPlayer");
    QDBusReply<QString> getIdentity = rootInterface.call("Identity");
    if (getIdentity.isValid()) {
        Identity=getIdentity.value();
    } else {
        valid=false;
        return;
    }
}

MPRIS::~MPRIS()
{

}

QStringList MPRIS::getRunningPlayers()
{
    QStringList services = QDBusConnection::sessionBus().interface()->registeredServiceNames().value().filter("org.mpris.");
    services.removeDuplicates();
    services.replaceInStrings("org.mpris.","");
    return services;
}

