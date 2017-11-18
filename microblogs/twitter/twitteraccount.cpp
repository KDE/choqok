/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twitteraccount.h"
#include "twitterdebug.h"
#include "twittermicroblog.h"

class TwitterAccount::Private
{
public:
    QString uploadHost;
    QUrl uploadUrl;
//     QStringList lists;
};

const char *twitterConsumerKey = "VyXMf0O7CvciiUQjliYtYg";
const char *twitterConsumerSecret = "uD2HvsOBjzt1Vs6SnouFtuxDeHmvOOVwmn3fBVyCw0";

TwitterAccount::TwitterAccount(TwitterMicroBlog *parent, const QString &alias)
    : TwitterApiAccount(parent, alias), d(new Private)
{
    setHost(QLatin1String("https://api.twitter.com"));
    setUploadHost(QLatin1String("https://api.twitter.com"));
    setApi(QLatin1String("1.1"));
    qCDebug(CHOQOK) << "Set API version to 1.1";
    setOauthConsumerKey(twitterConsumerKey);
    setOauthConsumerSecret(twitterConsumerSecret);
    setUsingOAuth(true);

    setPostCharLimit(280);    //TODO: See if we can ask twitter for the char limit and make it dynamic

    if (!oauthToken().isEmpty() && !oauthTokenSecret().isEmpty()) {
        // We trigger this to update the username
        parent->verifyCredentials(this);
    }

//     d->lists = configGroup()->readEntry("lists", QStringList());
    QStringList lists;
    for (const QString &tm: timelineNames()) {
        if (tm.startsWith(QLatin1Char('@'))) {
            lists.append(tm);
        }
    }
    if (!lists.isEmpty()) {
        parent->setListTimelines(this, lists);
    }
}

TwitterAccount::~TwitterAccount()
{
    delete d;
}

void TwitterAccount::setApi(const QString &api)
{
    TwitterApiAccount::setApi(api);
    generateUploadUrl();
}

QUrl TwitterAccount::uploadUrl() const
{
    return d->uploadUrl;
}

void TwitterAccount::setUploadUrl(const QUrl &url)
{
    d->uploadUrl = url;
}

QString TwitterAccount::uploadHost() const
{
    return d->uploadHost;
}

void TwitterAccount::setUploadHost(const QString &uploadHost)
{
    d->uploadHost = uploadHost;
}

void TwitterAccount::generateUploadUrl()
{
    if (!uploadHost().startsWith(QLatin1String("http"))) { //NOTE: This is for compatibility by prev versions. remove it after 1.0 release
        setUploadHost(uploadHost().prepend(QLatin1String("http://")));
    }
    QUrl url(uploadHost());

    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (api()));
    setUploadUrl(url);
}

/*
void TwitterAccount::writeConfig()
{
    qCDebug(CHOQOK)<<d->lists;
    configGroup()->writeEntry("lists", d->lists);
    TwitterApiAccount::writeConfig();
}

void TwitterAccount::addList(const QString& name)
{
    d->lists << name;
}

void TwitterAccount::removeList(const QString& name)
{
    d->lists.removeOne(name);
}*/

