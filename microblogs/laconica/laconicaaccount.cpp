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

#include "laconicaaccount.h"

#include "choqokdebug.h"

#include "laconicamicroblog.h"

class LaconicaAccount::Private
{
public:
    bool isChangeExclamationMark;
    QString changeExclamationMarkToText;
};

LaconicaAccount::LaconicaAccount(LaconicaMicroBlog* parent, const QString &alias)
    : TwitterApiAccount(parent, alias), d(new Private)
{
    d->changeExclamationMarkToText = configGroup()->readEntry("changeExclamationMarkText", QString('#'));
    d->isChangeExclamationMark = configGroup()->readEntry("isChangeExclamationMark", false);
}

LaconicaAccount::~LaconicaAccount()
{
    delete d;
}

void LaconicaAccount::writeConfig()
{
    configGroup()->writeEntry("isChangeExclamationMark", d->isChangeExclamationMark);
    configGroup()->writeEntry("changeExclamationMarkText", d->changeExclamationMarkToText);
    TwitterApiAccount::writeConfig();
}

QString LaconicaAccount::changeExclamationMarkToText() const
{
    return d->changeExclamationMarkToText;
}

void LaconicaAccount::setChangeExclamationMarkToText(const QString& text)
{
    d->changeExclamationMarkToText = text;
}

bool LaconicaAccount::isChangeExclamationMark() const
{
    return d->isChangeExclamationMark;
}

void LaconicaAccount::setChangeExclamationMark(bool isChange)
{
    d->isChangeExclamationMark = isChange;
}

QUrl LaconicaAccount::homepageUrl() const
{
    return apiUrl().upUrl();
}

