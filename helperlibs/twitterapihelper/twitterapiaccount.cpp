/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <KDebug>
#include "twitterapimicroblog.h"

class TwitterApiAccount::Private
{
public:
    bool secure;
    QString userId;
    int count;
};

TwitterApiAccount::TwitterApiAccount(TwitterApiMicroBlog* parent, const QString &alias)
    : Account(parent, alias), d(new Private)
{
    d->secure = configGroup()->readEntry("UseSecureConnection", false);
    d->userId = configGroup()->readEntry("UserId", QString());
    d->count = configGroup()->readEntry("CountOfPosts", 20);
}

TwitterApiAccount::~TwitterApiAccount()
{
    delete d;
}

void TwitterApiAccount::writeConfig()
{
    configGroup()->writeEntry("UseSecureConnection", d->secure);
    configGroup()->writeEntry("UserId", d->userId);
    configGroup()->writeEntry("CountOfPosts", d->count);
    Choqok::Account::writeConfig();
}

QString TwitterApiAccount::userId() const
{
    return d->userId;
}

void TwitterApiAccount::setUserId( const QString &id )
{
    kDebug();
    d->userId = id;
}

bool TwitterApiAccount::useSecureConnection() const
{
    return d->secure;
}

void TwitterApiAccount::setUseSecureConnection(bool use /*= true*/)
{
    d->secure = use;
}

int TwitterApiAccount::countOfPosts() const
{
    return d->count;
}

void TwitterApiAccount::setCountOfPosts(int count)
{
    d->count = count;
}

#include "twitterapiaccount.moc"
