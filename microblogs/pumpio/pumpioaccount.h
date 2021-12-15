/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOACCOUNT_H
#define PUMPIOACCOUNT_H

#include "account.h"
#include "choqoktypes.h"

#include "pumpiooauth.h"

class PumpIOMicroBlog;

class PumpIOAccount : public Choqok::Account
{
    Q_OBJECT
public:
    explicit PumpIOAccount(PumpIOMicroBlog *parent, const QString &alias);
    ~PumpIOAccount();

    virtual QStringList timelineNames() const override;

    virtual void writeConfig() override;

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
    PumpIOOAuth *oAuth();

private:
    class Private;
    Private *d;

};

#endif // PUMPIOACCOUNT_H
