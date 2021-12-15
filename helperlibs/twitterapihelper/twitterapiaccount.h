/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIACCOUNT_H
#define TWITTERAPIACCOUNT_H

#include <QUrl>

#include "account.h"
#include "twitterapihelper_export.h"

#include "twitterapimicroblog.h"
#include "twitterapioauth.h"

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TWITTERAPIHELPER_EXPORT TwitterApiAccount : public Choqok::Account
{
    Q_OBJECT
public:
    TwitterApiAccount(TwitterApiMicroBlog *parent, const QString &alias);
    ~TwitterApiAccount();
    virtual void writeConfig() override;

    QString userId() const;
    void setUserId(const QString &id);

    int countOfPosts() const;
    void setCountOfPosts(int count);

    QString host() const;
    void setHost(const QString &host);

    /**
    @return api path
    It's defer from apiUrl.
    For example: in http://identi.ca/api/
    identi.ca is @ref host()
    api is @ref api()
    http://identi.ca/api/ is @ref apiUrl()
    */
    QString api() const;
    virtual void setApi(const QString &api);

    /**
    Combined from @ref host and @ref api to use for connections and queries
    */
    QUrl apiUrl() const;
    virtual QUrl homepageUrl() const;

    QStringList friendsList() const;

    void setFriendsList(const QStringList &list);

    QStringList followersList() const;

    void setFollowersList(const QStringList &list);

    virtual QStringList timelineNames() const override;

    virtual void setTimelineNames(const QStringList &list);

    QByteArray oauthToken() const;
    void setOauthToken(const QByteArray &token);

    QByteArray oauthTokenSecret() const;
    void setOauthTokenSecret(const QByteArray &tokenSecret);

    QByteArray oauthConsumerKey() const;
    void setOauthConsumerKey(const QByteArray &consumerKey);

    QByteArray oauthConsumerSecret() const;
    void setOauthConsumerSecret(const QByteArray &consumerSecret);

    bool usingOAuth() const;
    void setUsingOAuth(bool use = true);

    TwitterApiOAuth *oauthInterface();
protected:
    void setApiUrl(const QUrl &apiUrl);
    void setHomepageUrl(const QUrl &homepageUrl);
    void generateApiUrl();
    void initQOAuthInterface();

private:
    class Private;
    Private *const d;
};

#endif // TWITTERACCOUNT_H
