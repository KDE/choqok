/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "passwordmanager.h"

#include <QApplication>

#include <KMessageBox>
#include <KWallet>

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "libchoqokdebug.h"

namespace Choqok
{
class PasswordManager::Private
{
public:
    Private()
        : wallet(nullptr), conf(nullptr), cfg(nullptr)
    {}

    ~Private()
    {
        if (cfg) {
            cfg->sync();
        }
        delete wallet;
        delete conf;
        delete cfg;
    }

    bool openWallet()
    {
        qCDebug(CHOQOK);
#pragma message("This segfaults on KF5")
//        if(kapp->sessionSaving())
//            return false;
        if ((wallet && wallet->isOpen())) {
            return true;
        }
        WId id = 0;
        if (Choqok::UI::Global::mainWindow()) {
            id = Choqok::UI::Global::mainWindow()->winId();
        }
        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), id);
        if (wallet) {
            if (!wallet->setFolder(QCoreApplication::applicationName())) {
                wallet->createFolder(QCoreApplication::applicationName());
                wallet->setFolder(QCoreApplication::applicationName());
            }
            qCDebug(CHOQOK) << "Wallet successfully opened.";
            return true;
        } else if (!conf) {
            cfg = new KConfig(QLatin1String("choqok/secretsrc"), KConfig::NoGlobals, QStandardPaths::DataLocation);
            conf = new KConfigGroup(cfg, QLatin1String("Secrets"));
            KMessageBox::information(Choqok::UI::Global::mainWindow(),
                                     i18n("Cannot open KDE Wallet manager, your secrets will be stored as plain text. You can install KWallet to fix this."), QString(), QLatin1String("DontShowKWalletProblem"),
                                     KMessageBox::Dangerous);
        }
        return false;
    }
    void syncConfigs()
    {
        cfg->sync();
    }
    KWallet::Wallet *wallet;
    KConfigGroup *conf;

private:
    KConfig *cfg;
};

PasswordManager::PasswordManager()
    : QObject(qApp), d(new Private)
{
    qCDebug(CHOQOK);
}

PasswordManager::~PasswordManager()
{
    delete d;
}

PasswordManager *PasswordManager::mSelf = nullptr;

PasswordManager *PasswordManager::self()
{
    if (!mSelf) {
        mSelf = new PasswordManager;
    }
    return mSelf;
}

QString PasswordManager::readPassword(const QString &alias)
{
    if (d->openWallet()) {
        QString pass;
        if (d->wallet->readPassword(alias, pass) == 0) {
            qCDebug(CHOQOK) << "Read password from wallet";
            return pass;
        } else {
            qCDebug(CHOQOK) << "Error on reading password from wallet";
            return QString();
        }
    } else {
        QByteArray pass = QByteArray::fromBase64(d->conf->readEntry(alias, QByteArray()));
        return QString::fromUtf8(pass.data(), pass.size());
    }
}

bool PasswordManager::writePassword(const QString &alias, const QString &password)
{
    if (d->openWallet()) {
        if (d->wallet->writePassword(alias, password) == 0) {
            qCDebug(CHOQOK) << "Password wrote to wallet successfuly";
            return true;
        } else {
            qCDebug(CHOQOK) << "Error on writing password to wallet";
            return false;
        }
    } else {
        d->conf->writeEntry(alias, password.toUtf8().toBase64());
        d->syncConfigs();
        return true;
    }
    return false;
}

bool PasswordManager::removePassword(const QString &alias)
{
    if (d->openWallet()) {
        if (!d->wallet->removeEntry(alias)) {
            return true;
        }
    } else {
        d->conf->deleteEntry(alias);
        return true;
    }
    return false;
}

}

#include "moc_passwordmanager.cpp"
