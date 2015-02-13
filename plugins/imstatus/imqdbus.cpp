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

#if TELEPATHY_FOUND
    #include <TelepathyQt/AccountManager>
    #include <TelepathyQt/AccountSet>
    #include <TelepathyQt/Account>
    #include <TelepathyQt/PendingOperation>
    #include <TelepathyQt/PendingReady>
#endif

const QString IM_KOPETE = "Kopete";
const QString IM_PSI = "Psi";
const QString IM_SKYPE = "Skype";
const QString IM_PIDGIN = "Pidgin";
const QString IM_TELEPATHY = "Telepathy";

IMQDBus::IMQDBus (QObject *parent) : QObject(parent)
{
    /*
    TODO:
    - qutIM (>0.3)
    - gajim ( doesn't want work :( )
    */
#if TELEPATHY_FOUND
    m_accountManager =  Tp::AccountManager::create(Tp::AccountFactory::create(QDBusConnection::sessionBus(), Tp::Account::FeatureCore));
    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(slotFinished(Tp::PendingOperation*)));

    Tp::registerTypes();
#endif
}

void IMQDBus::updateStatusMessage(const QString& im, const QString& statusMessage)
{
    if ( im == IM_KOPETE ) useKopete(statusMessage);
    if ( im == IM_PSI ) usePsi(statusMessage);
    if ( im == IM_SKYPE ) useSkype(statusMessage);
    if ( im == IM_PIDGIN ) usePidgin(statusMessage);
#if TELEPATHY_FOUND
    if ( im == IM_TELEPATHY ) useTelepathy(statusMessage);
#endif
}

void IMQDBus::useKopete(const QString &statusMessage)
{
    QDBusMessage msg = QDBusMessage::createMethodCall ( "org.kde.kopete", "/Kopete", "org.kde.Kopete", "setStatusMessage" );
    QList<QVariant> args;
    args.append ( QVariant ( statusMessage ) );
    msg.setArguments ( args );
    QDBusMessage rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }
}

void IMQDBus::usePsi(const QString &statusMessage)
{
    QDBusMessage msg = QDBusMessage::createMethodCall ( "org.psi-im.Psi", "/Main", "org.psi_im.Psi.Main", "setStatus" );
    QList<QVariant> args;
    args.append ( QVariant ( "online" ) );
    args.append ( QVariant ( statusMessage ) );
    msg.setArguments ( args );
    QDBusMessage rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }
}

void IMQDBus::useSkype(const QString &statusMessage)
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
    args.append ( QVariant ( QString ( "SET PROFILE MOOD_TEXT %1" ).arg ( statusMessage ) ) );
    msg.setArguments ( args );
    rep = QDBusConnection::sessionBus().call ( msg );
    if ( rep.type() == QDBusMessage::ErrorMessage ) {
        kDebug() <<  "ERROR" << rep.errorMessage();
        return;
    }
}

void IMQDBus::usePidgin(const QString &statusMessage)
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
    args.append ( QVariant ( statusMessage ) );
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

#if TELEPATHY_FOUND
void IMQDBus::useTelepathy(const QString &statusMessage)
{
    if (m_accountManager->isReady()) {
        Tp::AccountSetPtr validAccountsPtr = m_accountManager->enabledAccounts();

        QList<Tp::AccountPtr> accountsList = validAccountsPtr->accounts();

        Q_FOREACH (const Tp::AccountPtr &account, accountsList) {
            if (account->isOnline() && account->isReady()) {
                Tp::Presence currentPresence = account->currentPresence();
                currentPresence.setStatusMessage(statusMessage);
                account->setRequestedPresence(currentPresence);
            }
        }
    }
}

void IMQDBus::slotFinished(Tp::PendingOperation* po)
{
    if (po->isError()) {
	kDebug() << "Telepathy AccountManager failed to get ready:" << po->errorMessage();
    }
}

#endif

IMQDBus::~IMQDBus()
{}


QStringList IMQDBus::scanForIMs()
{
    QStringList ims;
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "com.Skype.API" ).value() )
        ims << IM_SKYPE;
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "org.psi-im.Psi" ).value() )
        ims << IM_PSI;
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "org.kde.kopete" ).value() )
        ims << IM_KOPETE;
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered ( "im.pidgin.purple.PurpleService" ).value() )
        ims << IM_PIDGIN;
#if TELEPATHY_FOUND
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.freedesktop.Telepathy.AccountManager" ).value() )
        ims << IM_TELEPATHY;
#endif

    ims.sort();
    return ims;
}
