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

#ifndef PUMPIOACCOUNT_H
#define PUMPIOACCOUNT_H

#include <QtOAuth/QtOAuth>

#include "account.h"
#include "choqoktypes.h"

class PumpIOMicroBlog;

class PumpIOAccount : public Choqok::Account
{
    Q_OBJECT
public:
    explicit PumpIOAccount(PumpIOMicroBlog *parent, const QString &alias);
    virtual ~PumpIOAccount();

    virtual QStringList timelineNames() const;

    virtual void writeConfig();

    QString host();
    void setHost(const QString &host);

    QString consumerKey();
    void setConsumerKey(const QString &consumerKey);

    QString consumerSecret();
    void setConsumerSecret(const QString &consumerSecret);

    QString token();
    void setToken(const QString &token);

    QString tokenSecret();
    void setTokenSecret(const QString &tokenSecret);

    QStringList following();
    void setFollowing(const QStringList following);

    QVariantList lists();
    void setLists(const QVariantList lists);

    void setTimelineNames(const QStringList &list);

    QString webfingerID();
    QOAuth::Interface *oAuth();

private:
    class Private;
    Private *d;

};

#endif // PUMPIOACCOUNT_H
