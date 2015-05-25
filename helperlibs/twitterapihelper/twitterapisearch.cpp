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

#include "twitterapisearch.h"

#include "accountmanager.h"

#include <stdio.h>

class TwitterApiSearch::Private
{
public:
    Private()
    {
        monthes[QLatin1String("Jan")] = 1;
        monthes[QLatin1String("Feb")] = 2;
        monthes[QLatin1String("Mar")] = 3;
        monthes[QLatin1String("Apr")] = 4;
        monthes[QLatin1String("May")] = 5;
        monthes[QLatin1String("Jun")] = 6;
        monthes[QLatin1String("Jul")] = 7;
        monthes[QLatin1String("Aug")] = 8;
        monthes[QLatin1String("Sep")] = 9;
        monthes[QLatin1String("Oct")] = 10;
        monthes[QLatin1String("Nov")] = 11;
        monthes[QLatin1String("Dec")] = 12;
    }
    QMap<QString, int> monthes;
};

TwitterApiSearch::TwitterApiSearch(QObject *parent)
    : QObject(parent), d(new Private)
{

}

TwitterApiSearch::~TwitterApiSearch()
{
    delete d;
}

QMap< int, QPair<QString, bool> > &TwitterApiSearch::getSearchTypes()
{
    return mSearchTypes;
}

void TwitterApiSearch::requestSearchResults(Choqok::Account *theAccount, const QString &query,
        int option, const QString &sinceStatusId,
        uint count, uint page)
{
    bool isB = getSearchTypes()[option].second;
    SearchInfo info(theAccount, query, option, isB);
    requestSearchResults(info, sinceStatusId, count, page);
}

QDateTime TwitterApiSearch::dateFromString(const QString &date)
{
    char s[10];
    int year, day, hours, minutes, seconds, tz;
    sscanf(qPrintable(date), "%*s %s %d %d:%d:%d %d %d", s, &day, &hours, &minutes, &seconds, &tz, &year);
    int month = d->monthes[QLatin1String(s)];
    QDateTime recognized(QDate(year, month, day), QTime(hours, minutes, seconds));
    if (tz == 0) { //tz is the timezone, in Twitter it's always UTC(0) in Identica it's local +/-NUMBER
        recognized.setTimeSpec(Qt::UTC);
    }
    return recognized.toLocalTime();
}

SearchInfo::SearchInfo()
{

}

SearchInfo::SearchInfo(Choqok::Account *theAccount, const QString &queryStr, int optionCode, bool IsBrowsable)
    : account(theAccount), option(optionCode), query(queryStr), isBrowsable(IsBrowsable)
{

}

bool SearchInfo::fromString(const QString &str)
{
    QStringList lis = str.split(QLatin1String(",,,"));
    if (lis.count() != 4) {
        return false;
    }
    account = Choqok::AccountManager::self()->findAccount(lis[0]);
    option = lis[1].toInt();
    query = lis[2];
    isBrowsable = (bool)lis[3].toInt();
    return true;
}

QString SearchInfo::toString()
{
    return account->alias() + QLatin1String(",,,") + QString::number(option) + QLatin1String(",,,") + query + QLatin1String(",,,") + QString::number(isBrowsable);
}
