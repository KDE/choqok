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
#include "twittermicroblog.h"
#include <KDebug>
#include <iostream>


class TwitterAccount::Private
{
public:
    QString uploadHost;
    KUrl uploadUrl;
//     QStringList lists;
};

TwitterAccount::TwitterAccount(TwitterMicroBlog* parent, const QString &alias)
    : TwitterApiAccount(parent, alias), d(new Private)
{
    setHost("https://api.twitter.com");
    setUploadHost("https://upload.twitter.com");
    setApi("1.1");
    std::cout << "Set API version to 1.1" << std::endl;
//     d->lists = configGroup()->readEntry("lists", QStringList());
    QStringList lists;
    foreach(const QString & tm, timelineNames()){
        if(tm.startsWith('@'))
            lists.append(tm);
    }
    if(!lists.isEmpty())
        parent->setListTimelines(this, lists);
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

KUrl TwitterAccount::uploadUrl() const
{
    return d->uploadUrl;
}

void TwitterAccount::setUploadUrl(const KUrl &url)
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
    if(!uploadHost().startsWith(QLatin1String("http")))//NOTE: This is for compatibility by prev versions. remove it after 1.0 release
        setUploadHost(uploadHost().prepend("http://"));
    KUrl url(uploadHost());

    url.addPath(api());
    setUploadUrl(url);
}


/*
void TwitterAccount::writeConfig()
{
    kDebug()<<d->lists;
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

#include "twitteraccount.moc"
