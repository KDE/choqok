/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mastodonaccount.h"

#include <KIO/AccessManager>

#include "passwordmanager.h"

#include "mastodonmicroblog.h"

class MastodonAccount::Private
{
public:
    QString consumerKey;
    QString consumerSecret;
    QString host;
    uint id;
    QString tokenSecret;
    QStringList followers;
    QStringList following;
    QVariantList lists;
    MastodonOAuth *oAuth;
    QStringList timelineNames;
};

MastodonAccount::MastodonAccount(MastodonMicroBlog *parent, const QString &alias):
    Account(parent, alias), d(new Private)
{
    d->host = configGroup()->readEntry("Host", QString());
    d->id = configGroup()->readEntry("Id", uint());
    d->followers = configGroup()->readEntry("Followers", QStringList());
    d->following = configGroup()->readEntry("Following", QStringList());
    d->lists = configGroup()->readEntry("Lists", QVariantList());
    d->tokenSecret = Choqok::PasswordManager::self()->readPassword(QStringLiteral("%1_tokenSecret").arg(alias));
    d->consumerKey = configGroup()->readEntry("ConsumerKey", QString());
    d->consumerSecret = Choqok::PasswordManager::self()->readPassword(QStringLiteral("%1_consumerSecret").arg(alias));
    d->oAuth = new MastodonOAuth(this);
    d->oAuth->setToken(d->tokenSecret);

    setPostCharLimit(500);

    parent->fetchFollowers(this, false);
    parent->fetchFollowing(this, false);
}

MastodonAccount::~MastodonAccount()
{
    d->oAuth->deleteLater();
    delete d;
}

void MastodonAccount::writeConfig()
{
    configGroup()->writeEntry("Host", d->host);
    configGroup()->writeEntry("Id", d->id);
    configGroup()->writeEntry("ConsumerKey", d->consumerKey);
    configGroup()->writeEntry("Followers", d->followers);
    configGroup()->writeEntry("Following", d->following);
    configGroup()->writeEntry("Lists", d->lists);

    Choqok::PasswordManager::self()->writePassword(QStringLiteral("%1_consumerSecret").arg(alias()),
            d->consumerSecret);
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("%1_tokenSecret").arg(alias()),
            d->tokenSecret);
    Choqok::Account::writeConfig();
}

QString MastodonAccount::host()
{
    return d->host;
}

void MastodonAccount::setHost(const QString &host)
{
    d->host = host;
}

uint MastodonAccount::id()
{
    return d->id;
}

void MastodonAccount::setId(const uint id)
{
    d->id = id;
}

QString MastodonAccount::consumerKey()
{
    return d->consumerKey;
}

void MastodonAccount::setConsumerKey(const QString &consumerKey)
{
    d->consumerKey = consumerKey;
}

QString MastodonAccount::consumerSecret()
{
    return d->consumerSecret;
}

void MastodonAccount::setConsumerSecret(const QString &consumerSecret)
{
    d->consumerSecret = consumerSecret;
}

QString MastodonAccount::tokenSecret()
{
    return d->tokenSecret;
}

void MastodonAccount::setTokenSecret(const QString &tokenSecret)
{
    d->tokenSecret = tokenSecret;
}

MastodonOAuth *MastodonAccount::oAuth()
{
    return d->oAuth;
}

QStringList MastodonAccount::followers() {
    return d->followers;
}

void MastodonAccount::setFollowers(const QStringList &followers) {
    d->followers = followers;
    writeConfig();
}

QStringList MastodonAccount::following() {
    return d->following;
}

void MastodonAccount::setFollowing(const QStringList &following) {
    d->following = following;
    writeConfig();
}

QVariantList MastodonAccount::lists() {
    return d->lists;
}

void MastodonAccount::setLists(const QVariantList &lists) {
    d->lists = lists;
    writeConfig();
}

#include "moc_mastodonaccount.cpp"
