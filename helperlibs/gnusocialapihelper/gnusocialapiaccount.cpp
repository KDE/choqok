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

#include "gnusocialapiaccount.h"

#include <KIO/Global>

#include "gnusocialapidebug.h"
#include "gnusocialapimicroblog.h"

class GNUSocialApiAccount::Private
{
public:
    bool isChangeExclamationMark;
    QString changeExclamationMarkToText;
};

GNUSocialApiAccount::GNUSocialApiAccount(GNUSocialApiMicroBlog *parent, const QString &alias)
    : TwitterApiAccount(parent, alias), d(new Private)
{
    d->changeExclamationMarkToText = configGroup()->readEntry(QLatin1String("changeExclamationMarkText"), QString::fromLatin1("#"));
    d->isChangeExclamationMark = configGroup()->readEntry("isChangeExclamationMark", false);
}

GNUSocialApiAccount::~GNUSocialApiAccount()
{
    delete d;
}

void GNUSocialApiAccount::writeConfig()
{
    configGroup()->writeEntry("isChangeExclamationMark", d->isChangeExclamationMark);
    configGroup()->writeEntry("changeExclamationMarkText", d->changeExclamationMarkToText);
    TwitterApiAccount::writeConfig();
}

QString GNUSocialApiAccount::changeExclamationMarkToText() const
{
    return d->changeExclamationMarkToText;
}

void GNUSocialApiAccount::setChangeExclamationMarkToText(const QString &text)
{
    d->changeExclamationMarkToText = text;
}

bool GNUSocialApiAccount::isChangeExclamationMark() const
{
    return d->isChangeExclamationMark;
}

void GNUSocialApiAccount::setChangeExclamationMark(bool isChange)
{
    d->isChangeExclamationMark = isChange;
}

QUrl GNUSocialApiAccount::homepageUrl() const
{
    return KIO::upUrl(apiUrl());
}

