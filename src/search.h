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

#ifndef SEARCH_H
#define SEARCH_H

#include <QString>
#include <QObject>
#include <QMap>
#include <KUrl>

#include "datacontainers.h"

class KJob;

/**
    Base class for search feature.
    @author Stephen Henderson <hendersonsk@gmail.com>
*/
class Search : public QObject
{
    Q_OBJECT
public:
    Search();
    virtual ~Search();

    QMap<int, QString> getSearchTypes();

private:
    virtual KUrl buildUrl( QString query, int option );

public slots:
    virtual void requestSearchResults( QString query, int option );

protected slots:
    virtual void searchResultsReturned( KJob *job );

signals:
    void searchResultsReceived( QList<Status> &statusList );
    void error( QString message );

protected:
    QMap<int, QString> mSearchTypes;
};

#endif
