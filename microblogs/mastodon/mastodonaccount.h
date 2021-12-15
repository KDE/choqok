/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
