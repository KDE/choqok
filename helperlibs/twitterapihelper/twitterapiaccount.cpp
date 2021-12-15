/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapiaccount.h"

#include <KIO/AccessManager>

#include "passwordmanager.h"

#include "twitterapidebug.h"
#include "twitterapimicroblog.h"
#include "twitterapioauth.h"

class TwitterApiAccount::Private
{
public:
    Private()
        : api(QLatin1Char('/')), usingOauth(true), qoauth(nullptr)
    {}
    QString userId;
    int count;
    QString host;
    QString api;
    QUrl apiUrl;
    QUrl homepageUrl;
    QStringList friendsList;
    QStringList followersList;
    QStringList timelineNames;
    QByteArray oauthToken;
    QByteArray oauthTokenSecret;
    QByteArray oauthConsumerKey;
    QByteArray oauthConsumerSecret;
    bool usingOauth;
    TwitterApiOAuth *qoauth;
};

TwitterApiAccount::TwitterApiAccount(TwitterApiMicroBlog *parent, const QString &alias)
    : Account(parent, alias), d(new Private)
{
    qCDebug(CHOQOK);
    d->usingOauth = configGroup()->readEntry("UsingOAuth", false);
    d->userId = configGroup()->readEntry("UserId", QString());
    d->count = configGroup()->readEntry("CountOfPosts", 20);
    d->host = configGroup()->readEntry("Host", QString());
    d->friendsList = configGroup()->readEntry("Friends", QStringList());
    d->followersList = configGroup()->readEntry("Followers", QStringList());
    d->timelineNames = configGroup()->readEntry("Timelines", QStringList());
    d->oauthToken = configGroup()->readEntry("OAuthToken", QByteArray());
    d->oauthConsumerKey = configGroup()->readEntry("OAuthConsumerKey", QByteArray());
    d->oauthConsumerSecret = Choqok::PasswordManager::self()->readPassword(
                                 QStringLiteral("%1_consumerSecret").arg(alias)).toUtf8();
    d->oauthTokenSecret = Choqok::PasswordManager::self()->readPassword(
                              QStringLiteral("%1_tokenSecret").arg(alias)).toUtf8();
    setApi(configGroup()->readEntry(QLatin1String("Api"), QString::fromLatin1("/")));

    qCDebug(CHOQOK) << "UsingOAuth:" << d->usingOauth;
    if (d->usingOauth) {
        initQOAuthInterface();
    }

    if (d->timelineNames.isEmpty()) {
        QStringList list = parent->timelineNames();
        list.removeOne(QLatin1String("Public"));
        list.removeOne(QLatin1String("Favorite"));
        list.removeOne(QLatin1String("ReTweets"));
        d->timelineNames = list;
    }

    if (d->friendsList.isEmpty()) {
        parent->listFriendsUsername(this);
        //Result will set on TwitterApiMicroBlog!
    }
}

TwitterApiAccount::~TwitterApiAccount()
{
    if(d->qoauth)
        d->qoauth->deleteLater();
    delete d;
}

void TwitterApiAccount::writeConfig()
{
    configGroup()->writeEntry("UsingOAuth", d->usingOauth);
    configGroup()->writeEntry("UserId", d->userId);
    configGroup()->writeEntry("CountOfPosts", d->count);
    configGroup()->writeEntry("Host", d->host);
    configGroup()->writeEntry("Api", d->api);
    configGroup()->writeEntry("Friends", d->friendsList);
    configGroup()->writeEntry("Followers", d->followersList);
    configGroup()->writeEntry("Timelines", d->timelineNames);
    configGroup()->writeEntry("OAuthToken", d->oauthToken);
    configGroup()->writeEntry("OAuthConsumerKey", d->oauthConsumerKey);
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("%1_consumerSecret").arg(alias()),
            QString::fromUtf8(d->oauthConsumerSecret));
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("%1_tokenSecret").arg(alias()),
            QString::fromUtf8(d->oauthTokenSecret));
    Choqok::Account::writeConfig();
}

QString TwitterApiAccount::userId() const
{
    return d->userId;
}

void TwitterApiAccount::setUserId(const QString &id)
{
    d->userId = id;
}

int TwitterApiAccount::countOfPosts() const
{
    return d->count;
}

void TwitterApiAccount::setCountOfPosts(int count)
{
    d->count = count;
}

QUrl TwitterApiAccount::apiUrl() const
{
    return d->apiUrl;
}

QString TwitterApiAccount::host() const
{
    return d->host;
}

void TwitterApiAccount::setApiUrl(const QUrl &apiUrl)
{
    d->apiUrl = apiUrl;
}

QString TwitterApiAccount::api() const
{
    return d->api;
}

void TwitterApiAccount::setApi(const QString &api)
{
    d->api = api;
    generateApiUrl();
}

void TwitterApiAccount::setHost(const QString &host)
{
    d->host = host;
    generateApiUrl();
}

QUrl TwitterApiAccount::homepageUrl() const
{
    return d->homepageUrl;
}

void TwitterApiAccount::generateApiUrl()
{
    if (!host().startsWith(QLatin1String("http"))) { //NOTE: This is for compatibility by prev versions. remove it after 1.0 release
        setHost(host().prepend(QLatin1String("http://")));
    }
    QUrl url(host());

    setHomepageUrl(url);

    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (api()));
    setApiUrl(url);
}

void TwitterApiAccount::setHomepageUrl(const QUrl &homepageUrl)
{
    d->homepageUrl = homepageUrl;
}

QStringList TwitterApiAccount::friendsList() const
{
    return d->friendsList;
}

void TwitterApiAccount::setFriendsList(const QStringList &list)
{
    d->friendsList = list;
    writeConfig();
}

QStringList TwitterApiAccount::followersList() const
{
    return d->followersList;
}

void TwitterApiAccount::setFollowersList(const QStringList& list)
{
    d->followersList = list;
    writeConfig();
}

QStringList TwitterApiAccount::timelineNames() const
{
    return d->timelineNames;
}

void TwitterApiAccount::setTimelineNames(const QStringList &list)
{
    d->timelineNames.clear();
    for (const QString &name: list) {
        if (microblog()->timelineNames().contains(name)) {
            d->timelineNames << name;
        }
    }
}

QByteArray TwitterApiAccount::oauthToken() const
{
    return d->oauthToken;
}

void TwitterApiAccount::setOauthToken(const QByteArray &token)
{
    d->oauthToken = token;
}

QByteArray TwitterApiAccount::oauthTokenSecret() const
{
    return d->oauthTokenSecret;
}

void TwitterApiAccount::setOauthTokenSecret(const QByteArray &tokenSecret)
{
    d->oauthTokenSecret = tokenSecret;
}

QByteArray TwitterApiAccount::oauthConsumerKey() const
{
    return d->oauthConsumerKey;
}

void TwitterApiAccount::setOauthConsumerKey(const QByteArray &consumerKey)
{
    d->oauthConsumerKey = consumerKey;
}

QByteArray TwitterApiAccount::oauthConsumerSecret() const
{
    return d->oauthConsumerSecret;
}

void TwitterApiAccount::setOauthConsumerSecret(const QByteArray &consumerSecret)
{
    d->oauthConsumerSecret = consumerSecret;
}

bool TwitterApiAccount::usingOAuth() const
{
    return d->usingOauth;
}

void TwitterApiAccount::setUsingOAuth(bool use)
{
    if (use) {
        initQOAuthInterface();
    } else {
        delete d->qoauth;
        d->qoauth = nullptr;
    }
    d->usingOauth = use;
}

TwitterApiOAuth *TwitterApiAccount::oauthInterface()
{
    return d->qoauth;
}

void TwitterApiAccount::initQOAuthInterface()
{
    qCDebug(CHOQOK);
    if (!d->qoauth) {
        d->qoauth = new TwitterApiOAuth(this);
    }
    d->qoauth->setToken(QLatin1String(d->oauthToken));
    d->qoauth->setTokenSecret(QLatin1String(d->oauthTokenSecret));
}

