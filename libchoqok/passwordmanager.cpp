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

#include "passwordmanager.h"
#include <kwallet.h>
#include "choqokbehaviorsettings.h"
#include <kdebug.h>
#include <QApplication>
#include "choqokuiglobal.h"
#include <KApplication>

namespace Choqok
{
class PasswordManager::Private
{
public:
    Private()
    : wallet(0), conf(0)
    {}

    ~Private() {
        delete wallet;
        wallet = 0L;
        delete conf;
        conf = 0L;
    }

    bool openWallet()
    {
        kDebug();
        if(kapp->sessionSaving())
            return false;
        if((wallet && wallet->isOpen()))
            return true;
        WId id = 0;
        if(Choqok::UI::Global::mainWindow())
            id = Choqok::UI::Global::mainWindow()->winId();
        wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), id);
        if ( wallet ) {
            if ( !wallet->setFolder( "choqok" ) ) {
                wallet->createFolder( "choqok" );
                wallet->setFolder( "choqok" );
            }
            kDebug() << "Wallet successfully opened.";
            return true;
        }
        if(!conf)
            conf = new KConfigGroup(KGlobal::config(), QString::fromLatin1( "Secrets" ));
        return false;
    }
    KWallet::Wallet *wallet;
    KConfigGroup *conf;
};

PasswordManager::PasswordManager()
    :QObject(qApp), d(new Private)
{
    kDebug();
}

PasswordManager::~PasswordManager()
{
    delete d;
}

PasswordManager *PasswordManager::mSelf = 0L;

PasswordManager *PasswordManager::self()
{
    if ( !mSelf )
        mSelf = new PasswordManager;
    return mSelf;
}

QString PasswordManager::readPassword(const QString &alias)
{
    if(d->openWallet()) {
        QString pass;
        if( d->wallet->readPassword(alias, pass) == 0 ) {
            kDebug()<<"Read password from wallet";
            return pass;
        } else {
            kDebug()<<"Error on reading password from wallet";
            return QString();
        }
    } else {
        QByteArray pass = QByteArray::fromBase64(d->conf->readEntry(alias, QByteArray()));
        return QString::fromUtf8(pass.data(), pass.size());
    }
}

bool PasswordManager::writePassword(const QString &alias, const QString &password)
{
    if(d->openWallet()) {
        if( d->wallet->writePassword(alias, password) == 0 ){
            kDebug()<<"Password wrote to wallet successfuly";
            return true;
        } else {
            kDebug()<<"Error on writing password to wallet";
            return false;
        }
    } else {
        d->conf->writeEntry(alias, password.toUtf8().toBase64());
        return true;
    }
    return false;
}

bool PasswordManager::removePassword(const QString& alias)
{
    if( d->openWallet() ) {
        if( !d->wallet->removeEntry(alias) )
            return true;
    } else {
        d->conf->deleteEntry(alias);
        return true;
    }
    return false;
}

}
