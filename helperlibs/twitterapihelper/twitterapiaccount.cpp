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

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"
#include <passwordmanager.h>

class TwitterApiAccount::Private
{
public:
    Private()
        :api('/'), usingOauth(true)
    {}
    bool secure;
    QString userId;
    int count;
    QString host;
    QString api;
    KUrl apiUrl;
    KUrl homepageUrl;
    QStringList friendsList;
    QStringList timelineNames;
    QByteArray oauthToken;
    QByteArray oauthTokenSecret;
    bool usingOauth;
};

TwitterApiAccount::TwitterApiAccount(TwitterApiMicroBlog* parent, const QString &alias)
    : Account(parent, alias), d(new Private)
{
    d->secure = configGroup()->readEntry("UseSecureConnection", true);
    d->usingOauth = configGroup()->readEntry("UsingOAuth", true);
    d->userId = configGroup()->readEntry("UserId", QString());
    d->count = configGroup()->readEntry("CountOfPosts", 20);
    d->host = configGroup()->readEntry("Host", QString());
    d->friendsList = configGroup()->readEntry("Friends", QStringList());
    d->timelineNames = configGroup()->readEntry("Timelines", QStringList());
    d->oauthToken = Choqok::PasswordManager::self()->readPassword( QString("%1_token").arg(alias) ).toUtf8();
    d->oauthTokenSecret = Choqok::PasswordManager::self()->readPassword( QString("%1_tokenSecret").arg(alias) ).toUtf8();
    setApi( configGroup()->readEntry("Api", QString('/') ) );

    if( d->timelineNames.isEmpty() ){
        QStringList list = parent->timelineNames();
        list.removeOne("Public");
        list.removeOne("Favorite");
        list.removeOne("ReTweets");
        d->timelineNames = list;
    }

    if( d->friendsList.isEmpty() ){
        parent->listFriendsUsername(this);
        //Result will set on TwitterApiMicroBlog!
    }
}

TwitterApiAccount::~TwitterApiAccount()
{
    delete d;
}

void TwitterApiAccount::writeConfig()
{
    configGroup()->writeEntry("UseSecureConnection", d->secure);
    configGroup()->writeEntry("UsingOAuth", d->usingOauth);
    configGroup()->writeEntry("UserId", d->userId);
    configGroup()->writeEntry("CountOfPosts", d->count);
    configGroup()->writeEntry("Host", d->host);
    configGroup()->writeEntry("Api", d->api);
    configGroup()->writeEntry("Friends", d->friendsList);
    configGroup()->writeEntry("Timelines", d->timelineNames);
    Choqok::PasswordManager::self()->writePassword( QString("%1_token").arg(alias()), QString::fromUtf8(d->oauthToken) );
    Choqok::PasswordManager::self()->writePassword( QString("%1_tokenSecret").arg(alias()), QString::fromUtf8( d->oauthTokenSecret) );
    Choqok::Account::writeConfig();
}

QString TwitterApiAccount::userId() const
{
    return d->userId;
}

void TwitterApiAccount::setUserId( const QString &id )
{
    d->userId = id;
}

bool TwitterApiAccount::useSecureConnection() const
{
    return d->secure;
}

void TwitterApiAccount::setUseSecureConnection(bool use /*= true*/)
{
    d->secure = use;
    generateApiUrl();
}

int TwitterApiAccount::countOfPosts() const
{
    return d->count;
}

void TwitterApiAccount::setCountOfPosts(int count)
{
    d->count = count;
}

KUrl TwitterApiAccount::apiUrl() const
{
    return d->apiUrl;
}

QString TwitterApiAccount::host() const
{
    return d->host;
}

void TwitterApiAccount::setApiUrl(const KUrl& apiUrl)
{
    d->apiUrl = apiUrl;
}

QString TwitterApiAccount::api() const
{
    return d->api;
}

void TwitterApiAccount::setApi(const QString& api)
{
    d->api = api;
    generateApiUrl();
}

void TwitterApiAccount::setHost(const QString& host)
{
    d->host = host;
    generateApiUrl();
}

KUrl TwitterApiAccount::homepageUrl() const
{
    return d->homepageUrl;
}

void TwitterApiAccount::generateApiUrl()
{
    KUrl url;
    url.setScheme(useSecureConnection()?"https":"http");
    url.setHost(host());

    setHomepageUrl(url);

    url.addPath(api());
    setApiUrl(url);
}

void TwitterApiAccount::setHomepageUrl(const KUrl& homepageUrl)
{ 
    d->homepageUrl = homepageUrl;
}

QStringList TwitterApiAccount::friendsList() const
{
    return d->friendsList;
}

void TwitterApiAccount::setFriendsList(const QStringList& list)
{
    d->friendsList = list;
}

QStringList TwitterApiAccount::timelineNames() const
{
    return d->timelineNames;
}

void TwitterApiAccount::setTimelineNames(const QStringList& list)
{
    d->timelineNames.clear();
    foreach(const QString &name, list){
        if(microblog()->timelineNames().contains(name))
            d->timelineNames<<name;
    }
}

QByteArray TwitterApiAccount::oauthToken() const
{
    return d->oauthToken;
}

void TwitterApiAccount::setOauthToken(const QByteArray& token)
{
    d->oauthToken = token;
}

QByteArray TwitterApiAccount::oauthTokenSecret() const
{
    return d->oauthTokenSecret;
}

void TwitterApiAccount::setOauthTokenSecret(const QByteArray& tokenSecret)
{
    d->oauthTokenSecret = tokenSecret;
}

bool TwitterApiAccount::isUsingOAuth() const
{
    return d->usingOauth;
}

void TwitterApiAccount::setUsingOAuth(bool use)
{
    d->usingOauth = use;
}

#include "twitterapiaccount.moc"
