/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013  Andrea Scarpino <scarpino@kde.org>

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
    QOAuth::Interface *oAuth;
    QStringList timelineNames;
};

PumpIOAccount::PumpIOAccount(PumpIOMicroBlog* parent, const QString& alias):
    Account(parent, alias), d(new Private)
{
    d->host = configGroup()->readEntry("Host", QString());
    d->token = configGroup()->readEntry("Token", QString());
    d->consumerKey = configGroup()->readEntry("ConsumerKey", QString());
    d->consumerSecret = Choqok::PasswordManager::self()->readPassword(QString("%1_consumerSecret").arg(alias));
    d->tokenSecret = Choqok::PasswordManager::self()->readPassword(QString("%1_tokenSecret").arg(alias));
    d->oAuth = new QOAuth::Interface(new KIO::AccessManager(this), this);
    d->oAuth->setConsumerKey(d->consumerKey.toLocal8Bit());
    d->oAuth->setConsumerSecret(d->consumerSecret.toLocal8Bit());
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
    Choqok::PasswordManager::self()->writePassword(QString("%1_consumerSecret").arg(alias()),
                                                   d->consumerSecret);
    Choqok::PasswordManager::self()->writePassword(QString("%1_tokenSecret").arg(alias()),
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

void PumpIOAccount::setHost(const QString& host)
{
    d->host = host;
}

QString PumpIOAccount::consumerKey()
{
    return d->consumerKey;
}

void PumpIOAccount::setConsumerKey(const QString& consumerKey)
{
    d->consumerKey = consumerKey;
    d->oAuth->setConsumerKey(consumerKey.toLocal8Bit());
}

QString PumpIOAccount::consumerSecret()
{
    return d->consumerSecret;
}

void PumpIOAccount::setConsumerSecret(const QString& consumerSecret)
{
    d->consumerSecret = consumerSecret;
    d->oAuth->setConsumerSecret(consumerSecret.toLocal8Bit());
}

QString PumpIOAccount::token()
{
    return d->token;
}

void PumpIOAccount::setToken(const QString& token)
{
    d->token = token;
}

QString PumpIOAccount::tokenSecret()
{
    return d->tokenSecret;
}

void PumpIOAccount::setTokenSecret(const QString& tokenSecret)
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
    d->following.sort();
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
    publicCollection.insert("id", PumpIOMicroBlog::PublicCollection);
    publicCollection.insert("name", "Public");
    d->lists.append(publicCollection);
    QVariantMap followersCollection;
    followersCollection.insert("id", QString(d->host + "/api/user/" + username() + "/followers"));
    followersCollection.insert("name", "Followers");
    d->lists.append(followersCollection);
}

void PumpIOAccount::setTimelineNames(const QStringList& list)
{
    d->timelineNames.clear();
    Q_FOREACH (const QString &name, list) {
        if (microblog()->timelineNames().contains(name)) {
            d->timelineNames.append(name);
        }
    }
}

QString PumpIOAccount::webfingerID()
{
    return username() + '@' + QString(d->host).remove("https://");
}

QOAuth::Interface* PumpIOAccount::oAuth()
{
    return d->oAuth;
}
