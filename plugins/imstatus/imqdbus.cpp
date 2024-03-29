/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "imqdbus.h"

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>

#ifdef TELEPATHY_FOUND
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>
#endif

const QString IM_KOPETE = QLatin1String("Kopete");
const QString IM_PSI = QLatin1String("Psi");
const QString IM_SKYPE = QLatin1String("Skype");
const QString IM_PIDGIN = QLatin1String("Pidgin");
const QString IM_TELEPATHY = QLatin1String("Telepathy");

IMQDBus::IMQDBus(QObject *parent) : QObject(parent)
{
    /*
    TODO:
    - qutIM (>0.3)
    - gajim ( doesn't want work :( )
    */
#ifdef TELEPATHY_FOUND
    m_accountManager =  Tp::AccountManager::create(Tp::AccountFactory::create(QDBusConnection::sessionBus(), Tp::Account::FeatureCore));
    connect(m_accountManager->becomeReady(), &Tp::PendingOperation::finished,
            this, &IMQDBus::slotFinished);;

    Tp::registerTypes();
#endif
}

void IMQDBus::updateStatusMessage(const QString &im, const QString &statusMessage)
{
    if (im == IM_KOPETE) {
        useKopete(statusMessage);
    }
    if (im == IM_PSI) {
        usePsi(statusMessage);
    }
    if (im == IM_SKYPE) {
        useSkype(statusMessage);
    }
    if (im == IM_PIDGIN) {
        usePidgin(statusMessage);
    }
#ifdef TELEPATHY_FOUND
    if (im == IM_TELEPATHY) {
        useTelepathy(statusMessage);
    }
#endif
}

void IMQDBus::useKopete(const QString &statusMessage)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String("org.kde.kopete"), QLatin1String("/Kopete"), QLatin1String("org.kde.Kopete"), QLatin1String("setStatusMessage"));
    QList<QVariant> args;
    args.append(QVariant(statusMessage));
    msg.setArguments(args);
    QDBusMessage rep = QDBusConnection::sessionBus().call(msg);
    if (rep.type() == QDBusMessage::ErrorMessage) {
        qWarning() <<  "Failed with error:" << rep.errorMessage();
        return;
    }
}

void IMQDBus::usePsi(const QString &statusMessage)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String("org.psi-im.Psi"), QLatin1String("/Main"), QLatin1String("org.psi_im.Psi.Main"), QLatin1String("setStatus"));
    QList<QVariant> args;
    args.append(QVariant(QLatin1String("online")));
    args.append(QVariant(statusMessage));
    msg.setArguments(args);
    QDBusMessage rep = QDBusConnection::sessionBus().call(msg);
    if (rep.type() == QDBusMessage::ErrorMessage) {
        qWarning() <<  "Failed with error:" << rep.errorMessage();
        return;
    }
}

void IMQDBus::useSkype(const QString &statusMessage)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String("com.Skype.API"), QLatin1String("/com/Skype"), QLatin1String("com.Skype.API"), QLatin1String("Invoke"));
    QList<QVariant> args;
    args.append(QVariant(QLatin1String("NAME Choqok")));
    msg.setArguments(args);
    QDBusMessage rep = QDBusConnection::sessionBus().call(msg);
    if (rep.type() == QDBusMessage::ErrorMessage) {
        qWarning() <<  "Failed with error:" << rep.errorMessage();
        return;
    }

    args.clear();
    args.append(QVariant(QLatin1String("PROTOCOL 7")));
    msg.setArguments(args);
    rep = QDBusConnection::sessionBus().call(msg);
    if (rep.type() == QDBusMessage::ErrorMessage) {
        qWarning() <<  "Failed with error:" << rep.errorMessage();
        return;
    }

    args.clear();
    args.append(QVariant(QStringLiteral("SET PROFILE MOOD_TEXT %1").arg(statusMessage)));
    msg.setArguments(args);
    rep = QDBusConnection::sessionBus().call(msg);
    if (rep.type() == QDBusMessage::ErrorMessage) {
        qWarning() <<  "Failed with error:" << rep.errorMessage();
        return;
    }
}

void IMQDBus::usePidgin(const QString &statusMessage)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String("im.pidgin.purple.PurpleService"), QLatin1String("/im/pidgin/purple/PurpleObject"), QLatin1String("im.pidgin.purple.PurpleInterface"), QLatin1String("PurpleSavedstatusGetCurrent"));
    QDBusReply<int> repUInt = QDBusConnection::sessionBus().call(msg);
    if (repUInt.error().isValid()) {
        qWarning() <<  "Failed with error:" << repUInt.error().message();
        return;
    }
    int IDCurrentStatus = repUInt.value();
    msg = QDBusMessage::createMethodCall(QLatin1String("im.pidgin.purple.PurpleService"), QLatin1String("/im/pidgin/purple/PurpleObject"), QLatin1String("im.pidgin.purple.PurpleInterface"), QLatin1String("PurpleSavedstatusGetType"));
    QList<QVariant> args;
    args.append(QVariant(IDCurrentStatus));
    msg.setArguments(args);
    repUInt = QDBusConnection::sessionBus().call(msg);
    if (repUInt.error().isValid()) {
        qWarning() <<  "Failed with error:" << repUInt.error().message();
        return;
    }
    int currentStatusType = repUInt.value();

    msg = QDBusMessage::createMethodCall(QLatin1String("im.pidgin.purple.PurpleService"), QLatin1String("/im/pidgin/purple/PurpleObject"), QLatin1String("im.pidgin.purple.PurpleInterface"), QLatin1String("PurpleSavedstatusNew"));
    args.clear();
    args.append(QVariant(QString()));
    args.append(QVariant(currentStatusType));
    msg.setArguments(args);
    repUInt = QDBusConnection::sessionBus().call(msg);
    if (repUInt.error().isValid()) {
        qWarning() <<  "Failed with error:" << repUInt.error().message();
        return;
    }
    IDCurrentStatus = repUInt.value(); //ID of new status

    msg = QDBusMessage::createMethodCall(QLatin1String("im.pidgin.purple.PurpleService"), QLatin1String("/im/pidgin/purple/PurpleObject"), QLatin1String("im.pidgin.purple.PurpleInterface"), QLatin1String("PurpleSavedstatusSetMessage"));
    args.clear();
    args.append(QVariant(IDCurrentStatus));
    args.append(QVariant(statusMessage));
    msg.setArguments(args);
    QDBusReply<void> repStr = QDBusConnection::sessionBus().call(msg);
    if (repStr.error().isValid()) {
        qWarning() <<  "Failed with error:" << repStr.error().message();
        return;
    }

    msg = QDBusMessage::createMethodCall(QLatin1String("im.pidgin.purple.PurpleService"), QLatin1String("/im/pidgin/purple/PurpleObject"), QLatin1String("im.pidgin.purple.PurpleInterface"), QLatin1String("PurpleSavedstatusActivate"));
    args.clear();
    args.append(QVariant(IDCurrentStatus));
    msg.setArguments(args);
    repStr = QDBusConnection::sessionBus().call(msg);
    if (repStr.error().isValid()) {
        qWarning() <<  "Failed with error:" << repStr.error().message();
        return;
    }
}

#ifdef TELEPATHY_FOUND
void IMQDBus::useTelepathy(const QString &statusMessage)
{
    if (m_accountManager->isReady()) {
        Tp::AccountSetPtr validAccountsPtr = m_accountManager->enabledAccounts();

        QList<Tp::AccountPtr> accountsList = validAccountsPtr->accounts();

        for (const Tp::AccountPtr &account: accountsList) {
            if (account->isOnline() && account->isReady()) {
                Tp::Presence currentPresence = account->currentPresence();
                currentPresence.setStatusMessage(statusMessage);
                account->setRequestedPresence(currentPresence);
            }
        }
    }
}

void IMQDBus::slotFinished(Tp::PendingOperation *po)
{
    if (po->isError()) {
        qCritical() << "Telepathy AccountManager failed to get ready:" << po->errorMessage();
    }
}

#endif

IMQDBus::~IMQDBus()
{}

QStringList IMQDBus::scanForIMs()
{
    QStringList ims;
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("com.Skype.API")).value()) {
        ims << IM_SKYPE;
    }
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.psi-im.Psi")).value()) {
        ims << IM_PSI;
    }
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.kopete")).value()) {
        ims << IM_KOPETE;
    }
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("im.pidgin.purple.PurpleService")).value()) {
        ims << IM_PIDGIN;
    }
#ifdef TELEPATHY_FOUND
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.Telepathy.AccountManager")).value()) {
        ims << IM_TELEPATHY;
    }
#endif

    ims.sort();
    return ims;
}

#include "moc_imqdbus.cpp"
