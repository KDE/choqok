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

#ifndef SEARCH_H
#define SEARCH_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QPair>
#include <KUrl>

#include "datacontainers.h"

class KJob;
class Account;

/**
    Base class for search feature.
    @author Stephen Henderson <hendersonsk@gmail.com>
*/
class Search : public QObject
{
    Q_OBJECT
public:
    explicit Search( Account* account, const QString searchUrl = QString(), QObject *parent=0 );
    virtual ~Search();

    QMap<int, QPair<QString, bool> > getSearchTypes();

private:
    virtual KUrl buildUrl( QString query, int option, uint sinceStatusId = 0, uint count = 0, uint page = 1 );

public slots:
    virtual void requestSearchResults( QString query,
                                       int option,
                                       uint sinceStatusId = 0,
                                       uint count = 0,
                                       uint page = 1 );

protected slots:
    virtual void searchResultsReturned( KJob *job );
    virtual void singleStatusReturned( KJob* job );

signals:
    void searchResultsReceived( QList<Status> &statusList );
    void error( QString message );

protected:
    // The QString in the QPair is a human readable string describing what the type searches for. The boolean value
    // determines whether or not the search type is traversable (if the forward and back buttons should be displayed).
    QMap<int, QPair<QString, bool> > mSearchTypes;
    uint mSinceStatusId;
    QString mSearchUrl;
    Account* mAccount;
};

#endif
