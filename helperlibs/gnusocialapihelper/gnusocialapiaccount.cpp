/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

