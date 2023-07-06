/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pumpioaccount.h"

#include <KIO/AccessManager>

#include "passwordmanager.h"

#include "pumpiomicroblog.h"

class PumpIOAccount::Private
{
public:
    QString consumerKey;
    QString consumerSecret;
    QString host;
    QString token;
    QString tokenSecret;
    QStringList following;
    QVariantList lists;
    PumpIOOAuth *oAuth;
    QStringList timelineNames;
};

PumpIOAccount::PumpIOAccount(PumpIOMicroBlog *parent, const QString &alias):
    Account(parent, alias), d(new Private)
{
    d->host = configGroup()->readEntry("Host", QString());
    d->token = configGroup()->readEntry("Token", QString());
    d->consumerKey = configGroup()->readEntry("ConsumerKey", QString());
    d->consumerSecret = Choqok::PasswordManager::self()->readPassword(QStringLiteral("%1_consumerSecret").arg(alias));
    d->tokenSecret = Choqok::PasswordManager::self()->readPassword(QStringLiteral("%1_tokenSecret").arg(alias));
    d->oAuth = new PumpIOOAuth(this);
    d->oAuth->setToken(d->token);
    d->oAuth->setTokenSecret(d->tokenSecret);
    d->following = configGroup()->readEntry("Following", QStringList());
    d->lists = QVariantList();
    d->timelineNames = configGroup()->readEntry("Timelines", QStringList());

    if (d->timelineNames.isEmpty()) {
        d->timelineNames = microblog()->timelineNames();
    }

    parent->fetchFollowing(this);
    parent->fetchLists(this);

    setPostCharLimit(0);
}

PumpIOAccount::~PumpIOAccount()
{
    d->oAuth->deleteLater();
    delete d;
}

QStringList PumpIOAccount::timelineNames() const
{
    return d->timelineNames;
}

void PumpIOAccount::writeConfig()
{
    configGroup()->writeEntry("Host", d->host);
    configGroup()->writeEntry("Token", d->token);
    configGroup()->writeEntry("ConsumerKey", d->consumerKey);
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("%1_consumerSecret").arg(alias()),
            d->consumerSecret);
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("%1_tokenSecret").arg(alias()),
            d->tokenSecret);
    configGroup()->writeEntry("Following", d->following);
    configGroup()->writeEntry("Timelines", d->timelineNames);
    //TODO: write accounts lists
    Choqok::Account::writeConfig();
}

QString PumpIOAccount::host()
{
    return d->host;
}

void PumpIOAccount::setHost(const QString &host)
{
    d->host = host;
}

QString PumpIOAccount::consumerKey()
{
    return d->consumerKey;
}

void PumpIOAccount::setConsumerKey(const QString &consumerKey)
{
    d->consumerKey = consumerKey;
}

QString PumpIOAccount::consumerSecret()
{
    return d->consumerSecret;
}

void PumpIOAccount::setConsumerSecret(const QString &consumerSecret)
{
    d->consumerSecret = consumerSecret;
}

QString PumpIOAccount::token()
{
    return d->token;
}

void PumpIOAccount::setToken(const QString &token)
{
    d->token = token;
}

QString PumpIOAccount::tokenSecret()
{
    return d->tokenSecret;
}

void PumpIOAccount::setTokenSecret(const QString &tokenSecret)
{
    d->tokenSecret = tokenSecret;
}

QStringList PumpIOAccount::following()
{
    return d->following;
}

void PumpIOAccount::setFollowing(const QStringList following)
{
    d->following = following;
    d->following.sort(Qt::CaseInsensitive);
    writeConfig();
}

QVariantList PumpIOAccount::lists()
{
    return d->lists;
}

void PumpIOAccount::setLists(const QVariantList lists)
{
    d->lists = lists;
    QVariantMap publicCollection;
    publicCollection.insert(QLatin1String("id"), PumpIOMicroBlog::PublicCollection);
    publicCollection.insert(QLatin1String("name"), QLatin1String("Public"));
    d->lists.append(publicCollection);
    QVariantMap followersCollection;
    followersCollection.insert(QLatin1String("id"), QString(d->host + QLatin1String("/api/user/") + username() + QLatin1String("/followers")));
    followersCollection.insert(QLatin1String("name"), QLatin1String("Followers"));
    d->lists.append(followersCollection);
}

void PumpIOAccount::setTimelineNames(const QStringList &list)
{
    d->timelineNames.clear();
    for (const QString &name: list) {
        if (microblog()->timelineNames().contains(name)) {
            d->timelineNames.append(name);
        }
    }
}

QString PumpIOAccount::webfingerID()
{
    return username() + QLatin1Char('@') + QString(d->host).remove(QLatin1String("https://"));
}

PumpIOOAuth *PumpIOAccount::oAuth()
{
    return d->oAuth;
}

#include "moc_pumpioaccount.cpp"
