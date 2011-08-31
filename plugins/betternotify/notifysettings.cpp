/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011  Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "notifysettings.h"
#include <QMap>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>
#include <KSharedConfigPtr>
#include <accountmanager.h>

class NotifySettings::Private
{
public:
    QMap<QString, QStringList> accounts;
    KConfigGroup *accountsConf;
    KConfigGroup *conf;
    int interval;
};

NotifySettings::NotifySettings(QObject* parent)
:QObject(parent), d(new Private)
{
    d->conf = new KConfigGroup(KGlobal::config(), QString::fromLatin1( "BetterNotify Plugin" ));
    d->accountsConf = new KConfigGroup(KGlobal::config(), QString::fromLatin1( "BetterNotify Accounts Config" ));
    d->interval = 5;
    load();
}

NotifySettings::~NotifySettings()
{
    save();
    delete d->accountsConf;
}

QMap< QString, QStringList > NotifySettings::accounts()
{
    return d->accounts;
}

void NotifySettings::setAccounts(QMap< QString, QStringList > accounts)
{
    d->accounts = accounts;
}

void NotifySettings::load()
{
    d->accounts.clear();
    d->accountsConf->sync();
    foreach(Choqok::Account* acc, Choqok::AccountManager::self()->accounts()) {
        d->accounts.insert( acc->alias(), d->accountsConf->readEntry(acc->alias(), QStringList()));
    }
    d->conf->sync();
    d->interval = d->conf->readEntry("Interval", 7);
}

void NotifySettings::save()
{
    foreach(Choqok::Account* acc, Choqok::AccountManager::self()->accounts()) {
        d->accountsConf->writeEntry(acc->alias(), d->accounts.value(acc->alias()));
    }
    d->conf->writeEntry("Interval", d->interval);
    d->accountsConf->sync();
}

int NotifySettings::notifyInterval()
{
    return d->interval;
}

void NotifySettings::setNotifyInterval(int interval)
{
    d->interval = interval;
}

