/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#include "account.h"

Account::Account()
{
}


Account::~Account()
{
}

Account::Account(const Account & account)
{
    mUserId = account.userId();
    mUsername = account.username();
    mPassword = account.password();
    mServiceName = account.serviceName();
    mAlias = account.alias();
    mDirection = account.direction();
    mApiPath = account.apiPath();
    mIsError = account.isError();
}

uint Account::userId() const
{
    return mUserId;
}

void Account::setUserId(uint id)
{
    mUserId = id;
}

QString Account::username() const
{
    return mUsername;
}

void Account::setUsername(const QString & name)
{
    mUsername = name;
}

QString Account::password() const
{
    return mPassword;
}

void Account::setPassword(const QString & pass)
{
    mPassword = pass;
}

QString Account::serviceName() const
{
    return mServiceName;
}

void Account::setServiceName(const QString & servicename)
{
    mServiceName = servicename;
}

QString Account::alias() const
{
    return mAlias;
}

void Account::setAlias(const QString & alias)
{
    mAlias = alias;
}

Qt::LayoutDirection Account::direction() const
{
    return mDirection;
}

void Account::setDirection(const Qt::LayoutDirection & dir)
{
    mDirection = dir;
}

QString Account::apiPath() const
{
    return mApiPath;
}

void Account::setApiPath(const QString & apiPath)
{
    mApiPath = apiPath;
}

void Account::setIsError(bool isError)
{
    mIsError = isError;
}

bool Account::isError() const
{
    return mIsError;
}
