/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "ocsaccount.h"
#include <attica/provider.h>
#include "ocsmicroblog.h"
#include <attica/providermanager.h>
#include <KDebug>

class OCSAccount::Private
{
public:
    QUrl providerUrl;
    Attica::Provider provider;
    OCSMicroblog* mBlog;
};

OCSAccount::OCSAccount(OCSMicroblog* parent, const QString& alias)
: Account(parent, alias), d(new Private)
{
    kDebug()<<alias;
    d->mBlog = parent;
    setProviderUrl( QUrl(configGroup()->readEntry("ProviderUrl", QString())) );
}

OCSAccount::~OCSAccount()
{
    delete d;
}

void OCSAccount::writeConfig()
{
    configGroup()->writeEntry("ProviderUrl", d->providerUrl.toString());
    Choqok::Account::writeConfig();
}

QUrl OCSAccount::providerUrl() const
{
    return d->providerUrl;
}

void OCSAccount::setProviderUrl(const QUrl& url)
{
    kDebug()<<url;
    d->providerUrl = url;
    if(d->mBlog->isOperational())
        slotDefaultProvidersLoaded();
    else
        connect(d->mBlog, SIGNAL(initialized()), SLOT(slotDefaultProvidersLoaded()));
}

Attica::Provider OCSAccount::provider()
{
    return d->provider;
}

void OCSAccount::slotDefaultProvidersLoaded()
{
    d->provider = d->mBlog->providerManager()->providerByUrl(d->providerUrl);
}

#include "ocsaccount.moc"
