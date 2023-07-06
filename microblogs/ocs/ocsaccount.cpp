/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ocsaccount.h"

#include <Attica/ProviderManager>

#include "ocsdebug.h"
#include "ocsmicroblog.h"

class OCSAccount::Private
{
public:
    QUrl providerUrl;
    Attica::Provider provider;
    OCSMicroblog *mBlog;
};

OCSAccount::OCSAccount(OCSMicroblog *parent, const QString &alias)
    : Account(parent, alias), d(new Private)
{
    qCDebug(CHOQOK) << alias;
    d->mBlog = parent;
    setProviderUrl(QUrl(configGroup()->readEntry("ProviderUrl", QString())));
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

void OCSAccount::setProviderUrl(const QUrl &url)
{
    qCDebug(CHOQOK) << url;
    d->providerUrl = url;
    if (d->mBlog->isOperational()) {
        slotDefaultProvidersLoaded();
    } else {
        connect(d->mBlog, &OCSMicroblog::initialized, this,
                &OCSAccount::slotDefaultProvidersLoaded);
    }
}

Attica::Provider OCSAccount::provider()
{
    return d->provider;
}

void OCSAccount::slotDefaultProvidersLoaded()
{
    d->provider = d->mBlog->providerManager()->providerByUrl(d->providerUrl);
}

#include "moc_ocsaccount.cpp"
