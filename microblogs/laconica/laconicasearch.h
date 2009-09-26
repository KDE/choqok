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

#ifndef LACONICASEARCH_H
#define LACONICASEARCH_H

#include "twitterapihelper/twitterapisearch.h"


/**
Laconica/StatusNet search API implementation.

@author Stephen Henderson \<hendersonsk@gmail.com\>
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class LaconicaSearch : public TwitterApiSearch
{
    Q_OBJECT
public:
    enum SearchType { ReferenceHashtag = 0, ReferenceGroup, FromUser, ToUser };
    LaconicaSearch(QObject* parent = 0);
    ~LaconicaSearch();
    virtual void requestSearchResults(TwitterApiAccount* theAccount, const QString& query, int option,
                                      const Choqok::ChoqokId& sinceStatusId = QString(),
                                      uint count = 0, uint page = 1);
    virtual QString optionCode(int option);

protected:
    virtual KUrl buildUrl( TwitterApiAccount* theAccount, QString query, int option,
                           Choqok::ChoqokId sinceStatusId = Choqok::ChoqokId(),
                           uint count = 0, uint page = 1 );
    QList<Choqok::Post*> parseRss( const QByteArray &buffer );
    QList<Choqok::Post*> parseAtom( const QByteArray &buffer );

protected slots:
    virtual void searchResultsReturned( KJob *job );

private:
    QMap<int, QString> mSearchCode;
    QMap<KJob*, AccountQueryOptionContainer> mSearchJobs;
    static const QRegExp mIdRegExp;
};

#endif // LACONICASEARCH_H
