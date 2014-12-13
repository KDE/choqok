/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#include "imqdbus.h"
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>

#include <kdebug.h>

IMQDBus::IMQDBus ( const QString im, const QString statusMsg )
{
    /*
    TODO:
    - qutIM (>0.3)
    - gajim ( doesn't want work :( )
    */
    
    m_statusMsg = statusMsg;
    if ( im == "Kopete" ) useKopete();
    if ( im == "Psi" ) usePsi();
    if ( im == "Skype" ) useSkype();
    if ( im == "Pidgin" ) usePidgin();
}

void IMQDBus::useKopete()
{
    QDBusMessage msg = QDBusMessage::createMethodCall ( "org.kde.kopete", "/Kopete", "org.kde.Kopete", "setStatusMessage" );
    QList<QVariant> args;
    args.append ( QVariant ( m_statusMsg ) );
    msg.setArguments ( args );
    QDBusMessage rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }
}

void IMQDBus::usePsi()
{
    QDBusMessage msg = QDBusMessage::createMethodCall ( "org.psi-im.Psi", "/Main", "org.psi_im.Psi.Main", "setStatus" );
    QList<QVariant> args;
    args.append ( QVariant ( "online" ) );
    args.append ( QVariant ( m_statusMsg ) );
    msg.setArguments ( args );
    QDBusMessage rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }
}

void IMQDBus::useSkype()
{
    QDBusMessage msg = QDBusMessage::createMethodCall ( "com.Skype.API", "/com/Skype", "com.Skype.API", "Invoke" );
    QList<QVariant> args;
    args.append ( QVariant ( QString ( "NAME Choqok" ) ) );
    msg.setArguments ( args );
    QDBusMessage rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }

    args.clear();
    args.append ( QVariant ( QString ( "PROTOCOL 7" ) ) );
    msg.setArguments ( args );
    rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }

    args.clear();
    args.append ( QVariant ( QString ( "SET PROFILE MOOD_TEXT %1" ).arg ( m_statusMsg ) ) );
    msg.setArguments ( args );
    rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }
}

void IMQDBus::usePidgin()
{
    QDBusMessage msg = QDBusMessage::createMethodCall ( "im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject", "im.pidgin.purple.PurpleInterface", "PurpleSavedstatusGetCurrent" );
    QDBusReply<int> repUInt = QDBusConnection::sessionBus().call ( msg );
    if ( repUInt.error().isValid() ) {
        kDebug() << "ERROR:" << repUInt.error().message();
        return;
    }
    int IDCurrentStatus = repUInt.value();
    msg = QDBusMessage::createMethodCall ( "im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject", "im.pidgin.purple.PurpleInterface", "PurpleSavedstatusGetType" );
    QList<QVariant> args;
    args.append ( QVariant ( IDCurrentStatus ) );
    msg.setArguments ( args );
    repUInt = QDBusConnection::sessionBus().call ( msg );
    if ( repUInt.error().isValid() ) {
        kDebug() << "ERROR:" << repUInt.error().message();
        return;
    }
    int currentStatusType = repUInt.value();

    msg = QDBusMessage::createMethodCall ( "im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject", "im.pidgin.purple.PurpleInterface", "PurpleSavedstatusNew" );
    args.clear();
    args.append ( QVariant ( QString() ) );
    args.append ( QVariant ( currentStatusType ) );
    msg.setArguments ( args );
    repUInt = QDBusConnection::sessionBus().call ( msg );
    if ( repUInt.error().isValid() ) {
        kDebug() << "ERROR:" << repUInt.error().message();
        return;
    }
    IDCurrentStatus = repUInt.value(); //ID of new status

    msg = QDBusMessage::createMethodCall ( "im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject", "im.pidgin.purple.PurpleInterface", "PurpleSavedstatusSetMessage" );
    args.clear();
    args.append ( QVariant ( IDCurrentStatus ) );
    args.append ( QVariant ( m_statusMsg ) );
    msg.setArguments ( args );
    QDBusReply<void> repStr = QDBusConnection::sessionBus().call ( msg );
    if ( repStr.error().isValid() ) {
        kDebug() << "ERROR:" << repStr.error().message();
        return;
    }

    msg = QDBusMessage::createMethodCall ( "im.pidgin.purple.PurpleService", "/im/pidgin/purple/PurpleObject", "im.pidgin.purple.PurpleInterface", "PurpleSavedstatusActivate" );
    args.clear();
    args.append ( QVariant ( IDCurrentStatus ) );
    msg.setArguments ( args );
    repStr = QDBusConnection::sessionBus().call ( msg );
    if ( repStr.error().isValid() ) {
        kDebug() << "ERROR:" << repStr.error().message();
        return;
    }
}

IMQDBus::~IMQDBus()
{}


QStringList IMQDBus::scanForIMs()
{
    QStringList ims;
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "com.Skype.API" ).value() )
        ims << "Skype";
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "org.psi-im.Psi" ).value() )
        ims << "Psi";
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "org.kde.kopete" ).value() )
        ims << "Kopete";
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "im.pidgin.purple.PurpleService" ).value() )
        ims << "Pidgin";
    ims.sort();
    return ims;
}
