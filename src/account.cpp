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
#include "account.h"
#include <KUrl>

Account::Account( Service type, const QString &homepage )
{
    setServiceType(type, homepage);
}

Account::Account( )
{
}

Account::~Account()
{
}

Account::Account( const Account & account )
{
    mUserId = account.userId();
    mUsername = account.username();
    mPassword = account.password();
    mAlias = account.alias();
    mDirection = account.direction();
    mIsError = account.isError();
    setServiceType( account.serviceType(), account.homepage() );
}

qulonglong Account::userId() const
{
    return mUserId;
}

void Account::setUserId( qulonglong id )
{
    mUserId = id;
}

QString Account::username() const
{
    return mUsername;
}

void Account::setUsername( const QString & name )
{
    mUsername = name;
}

QString Account::password() const
{
    return mPassword;
}

void Account::setPassword( const QString & pass )
{
    mPassword = pass;
}

QString Account::serviceName() const
{
    return mServiceName;
}

QString Account::alias() const
{
    return mAlias;
}

void Account::setAlias( const QString & alias )
{
    mAlias = alias;
}

Qt::LayoutDirection Account::direction() const
{
    return mDirection;
}

void Account::setDirection( const Qt::LayoutDirection & dir )
{
    mDirection = dir;
}

QString Account::apiPath() const
{
    return mApiPath;
}

QString Account::statusUrl( int statusId, const QString &userScreenName ) const
{
    if( mServiceType == Twitter ) {
        return mStatusUrlBase.arg( QString::number(statusId), userScreenName );
    } else {
        return mStatusUrlBase.arg( statusId );
    }
}

void Account::setError( bool isError )
{
    mIsError = isError;
}

bool Account::isError() const
{
    return mIsError;
}

Account::Service Account::serviceType() const
{
    return mServiceType;
}

QString Account::homepage() const
{
    return mHomepage;
}

void Account::setServiceType( Service type, const QString &homepage )
{
    mServiceType = type;
    switch( type ) {
        case Identica:
            mServiceName = "Identi.ca";
            mApiPath = "http://identi.ca/api/";
            mHomepage = "http://identi.ca/";
            mStatusUrlBase = "http://identi.ca/notice/%1";
            break;
        case Twitter:
            mServiceName = "Twitter.com";
            mApiPath = "http://twitter.com/";
            mHomepage = "http://twitter.com/";
            mStatusUrlBase = "http://twitter.com/%2/status/%1";
            break;
        case Laconica:
            mServiceName = "Custom Laconica";
            KUrl tmp( homepage );
            tmp.addPath( "/api/" );
            mApiPath = tmp.prettyUrl(KUrl::AddTrailingSlash);
            mHomepage = homepage;
            mStatusUrlBase = homepage + "/notice/%1";
            break;
    }
}
