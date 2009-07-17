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
#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <QString>

/**
    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class Account
{
public:
    enum Service{ Twitter=0, Identica=1, Laconica=2 };
    Account();
    explicit Account( Service type, const QString &homepage = QString() );
    Account( const Account &account );

    ~Account();

    qulonglong userId() const;
    void setUserId( qulonglong id );

    QString username() const;
    void setUsername( const QString &name );

    QString password() const;
    void setPassword( const QString &pass );

    QString serviceName() const;

    QString alias() const;
    void setAlias( const QString &alias );

    Qt::LayoutDirection direction() const;
    void setDirection( const Qt::LayoutDirection &dir );

    QString apiPath() const;

    QString statusUrl( qulonglong statusId, const QString &userScreenName ) const;
    QString homepage() const;

    bool isError() const;
    void setError( bool isError );

    Service serviceType() const;
    void setServiceType( Service type, const QString &homepage = QString() );

private:
    qulonglong mUserId;
    QString mUsername;
    QString mPassword;
    QString mServiceName;
    QString mAlias;
    Qt::LayoutDirection mDirection;
    QString mApiPath;
    bool mIsError;
    Service mServiceType;
    QString mStatusUrlBase;
    QString mHomepage;
};

#endif
