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

#ifndef TWITTERAPIACCOUNT_H
#define TWITTERAPIACCOUNT_H

#include <account.h>
#include <choqok_export.h>

namespace QOAuth {
class Interface;
}

class TwitterApiMicroBlog;
/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_HELPER_EXPORT TwitterApiAccount : public Choqok::Account
{
    Q_OBJECT
public:
    TwitterApiAccount(TwitterApiMicroBlog* parent, const QString& alias);
    ~TwitterApiAccount();
    virtual void writeConfig();

    QString userId() const;
    void setUserId( const QString &id );

    int countOfPosts() const;
    void setCountOfPosts(int count);

    QString host() const;
    void setHost( const QString &host );

    /**
    @return api path
    It's defer from apiUrl.
    For example: in http://identi.ca/api/
    identi.ca is @ref host()
    api is @ref api()
    http://identi.ca/api/ is @ref apiUrl()
    */
    QString api() const;
    virtual void setApi( const QString &api );

    /**
    Combined from @ref host and @ref api to use for connections and queries
    */
    KUrl apiUrl() const;
    virtual KUrl homepageUrl() const;

    QStringList friendsList() const;

    void setFriendsList( const QStringList &list );

    virtual QStringList timelineNames() const;

    virtual void setTimelineNames(const QStringList &list);

    QByteArray oauthToken() const;
    void setOauthToken( const QByteArray &token );

    QByteArray oauthTokenSecret() const;
    void setOauthTokenSecret( const QByteArray &tokenSecret );

    QByteArray oauthConsumerKey() const;
    void setOauthConsumerKey( const QByteArray &consumerKey );

    QByteArray oauthConsumerSecret() const;
    void setOauthConsumerSecret( const QByteArray &consumerSecret );

    bool usingOAuth() const;
    void setUsingOAuth( bool use = true );

    QOAuth::Interface *oauthInterface();
protected:
    void setApiUrl( const KUrl &apiUrl );
    void setHomepageUrl( const KUrl& homepageUrl );
    void generateApiUrl();
    void initQOAuthInterface();

private:
    class Private;
    Private * const d;
};

#endif // TWITTERACCOUNT_H
