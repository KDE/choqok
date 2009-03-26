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

#ifndef IDENTICASEARCH_H
#define IDENTICASEARCH_H

#include <QList>

#include "search.h"
#include "datacontainers.h"

/**
Twitter.com search API implementation.

@author Stephen Henderson <hendersonsk@gmail.com>
*/
class IdenticaSearch : public Search
{
    Q_OBJECT
public:
    enum SearchType { ToUser = 0, FromUser, ReferenceGroup, ReferenceHashtag };

    explicit IdenticaSearch( Account* account, const QString searchUrl = QString(), QObject *parent=0 );
    virtual ~IdenticaSearch();

private:
    virtual KUrl buildUrl( QString query, int option, uint sinceStatusId = 0, uint count = 0, uint page = 1 );
    QList<Status>* parseRss( const QByteArray &buffer );

public slots:
    virtual void requestSearchResults( QString query,
                                       int option,
                                       uint sinceStatusId = 0,
                                       uint count = 0,
                                       uint page = 1 );

protected slots:
    virtual void searchResultsReturned( KJob *job );

};

#endif
