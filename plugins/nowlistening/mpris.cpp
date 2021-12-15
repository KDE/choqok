/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010-2011 Ramin Gomari <ramin.gomari@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

    This code is loosely inspired by the "now listening" plugin of KMess.
    Therefore some parts are also
    SPDX-FileCopyrightText: 2006 Diederik van der Boor <vdboor--at--codingdomain.com>
*/

#include "mpris.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>

Q_DECLARE_METATYPE(MPRIS::MprisStatusStruct)

/**
 * Marshall the MprisStatus structure into a DBus argument
 */
QDBusArgument &operator<<(QDBusArgument &argument, const MPRIS::MprisStatusStruct  &status)
{
    argument.beginStructure();
    argument << status.Status << status.hasShuffle << status.hasRepeat << status.hasPlaylistRepeat;
    argument.endStructure();
    return argument;
}

/**
 * Demarshall the MprisPlayerStatus structure from a DBus argument
 */
const QDBusArgument &operator>>(const QDBusArgument &argument, MPRIS::MprisStatusStruct  &status)
{
    argument.beginStructure();
    argument >> status.Status >> status.hasShuffle >> status.hasRepeat >> status.hasPlaylistRepeat;
    argument.endStructure();
    return argument;
}

MPRIS::MPRIS(const QString PlayerName)
{
    qDBusRegisterMetaType<MprisStatusStruct>();
    const QString serviceName = QStringLiteral("org.mpris.%1").arg(PlayerName);
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName).value()) {
        valid = true;
    } else {
        valid = false;
        return;
    }
    QDBusInterface playerInterface(serviceName,
                                   QLatin1String("/Player"),
                                   QLatin1String("org.freedesktop.MediaPlayer"));
    QDBusReply<MprisStatusStruct> getStatus = playerInterface.call(QLatin1String("GetStatus"));
    QDBusReply< QMap<QString, QVariant> > getMetadata = playerInterface.call(QLatin1String("GetMetadata"));

    if (!getStatus.isValid() || !getMetadata.isValid()) {
        valid = false;
        return;
    }
    status = getStatus.value();
    trackInfo = getMetadata.value();
    QDBusInterface rootInterface(serviceName,
                                 QLatin1String("/"),
                                 QLatin1String("org.freedesktop.MediaPlayer"));
    QDBusReply<QString> getIdentity = rootInterface.call(QLatin1String("Identity"));
    if (getIdentity.isValid()) {
        Identity = getIdentity.value();
    } else {
        valid = false;
        return;
    }
}

MPRIS::~MPRIS()
{

}

QStringList MPRIS::getRunningPlayers()
{
    QStringList services = QDBusConnection::sessionBus().interface()->registeredServiceNames().value().filter(QLatin1String("org.mpris."));
    services.removeDuplicates();
    services.replaceInStrings(QLatin1String("org.mpris."), QString());
    return services;
}

