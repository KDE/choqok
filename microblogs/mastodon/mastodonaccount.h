/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017  Andrea Scarpino <scarpino@kde.org>

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

#ifndef MASTODONACCOUNT_H
#define MASTODONACCOUNT_H

#include "account.h"
#include "choqoktypes.h"

#include "mastodonoauth.h"

class MastodonMicroBlog;

class MastodonAccount : public Choqok::Account
{
    Q_OBJECT
public:
    explicit MastodonAccount(MastodonMicroBlog *parent, const QString &alias);
    ~MastodonAccount();

    virtual void writeConfig() override;

    QString host();
    void setHost(const QString &host);

    uint id();
    void setId(const uint id);

    QString consumerKey();
    void setConsumerKey(const QString &consumerKey);

    QString consumerSecret();
    void setConsumerSecret(const QString &consumerSecret);

    QString tokenSecret();
    void setTokenSecret(const QString &tokenSecret);

    MastodonOAuth *oAuth();

    QStringList followers();
    void setFollowers(const QStringList &followers);

    QStringList following();
    void setFollowing(const QStringList &following);

    QVariantList lists();
    void setLists(const QVariantList &lists);

private:
    class Private;
    Private *d;

};

#endif // MASTODONACCOUNT_H
