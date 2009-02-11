/*
    This file is part of choqoK, the KDE mono-blogging client

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
#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <QString>

#define TWITTER_API_PATH "http://twitter.com/"
#define TWITTER_SERVICE_TEXT "Twitter.com"

#define IDENTICA_API_PATH "http://identi.ca/api/"
#define IDENTICA_SERVICE_TEXT "Identi.ca"

/**
    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class Account
{
public:
    enum Service{ Twitter=0, Identica=1  };
    Account();
    Account( Service type );
    Account( const Account &account );

    ~Account();

    uint userId() const;
    void setUserId( uint id );

    QString username() const;
    void setUsername( const QString &name );

    QString password() const;
    void setPassword( const QString &pass );

    QString serviceName() const;
//     void setServiceName( const QString &servicename );

    QString alias() const;
    void setAlias( const QString &alias );

    Qt::LayoutDirection direction() const;
    void setDirection( const Qt::LayoutDirection &dir );

    QString apiPath() const;
//     void setApiPath( const QString &apiPath );

    bool isError() const;
    void setError( bool isError );

    Service serviceType() const;
    void setServiceType( Service type );

private:
    uint mUserId;
    QString mUsername;
    QString mPassword;
    QString mServiceName;
    QString mAlias;
    Qt::LayoutDirection mDirection;
    QString mApiPath;
    bool mIsError;
    Service mServiceType;
};

#endif
