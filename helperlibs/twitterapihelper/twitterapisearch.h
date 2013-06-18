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

#ifndef TWITTERAPISEARCH_H
#define TWITTERAPISEARCH_H

#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QMap>
#include <QtCore/QRegExp>
#include <KDE/KUrl>
#include <account.h>
#include <accountmanager.h>
#include <choqoktypes.h>

class TwitterApiAccount;
namespace Choqok {
class Account;
}

class KJob;

class CHOQOK_HELPER_EXPORT SearchInfo
{
public:
    SearchInfo();
    SearchInfo( Choqok::Account *theAccount, const QString &queryStr,
                int optionCode, bool IsBrowsable = false);
    QString toString();
    bool fromString(const QString &str);

    Choqok::Account *account;
    /**
    option code
    */
    int option;

    /**
    Query text to search
    */
    QString query;

    /**
    Show if this search type is browsable, next and prev buttons should be displayed or not!
    */
    bool isBrowsable;
};

/**
    Base class for search feature.
    @author Stephen Henderson \<hendersonsk@gmail.com\>
    @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_HELPER_EXPORT TwitterApiSearch : public QObject
{
    Q_OBJECT
public:
    TwitterApiSearch(QObject* parent = 0);
    virtual ~TwitterApiSearch();

    /**
    The QString in the QPair is a human readable string describing what the type searches for.
    The boolean value determines whether or not the search type is traversable
    (if the forward and back buttons should be displayed).
    */
    QMap<int, QPair<QString, bool> > &getSearchTypes();

    /**
    Sub classes should implement this!
    Result will use on Timeline Widget tab name!
    Example:
        returned optionCode for option 1 is "#" and query was "Choqok", So tab name will be "#Choqok"
    */
    virtual QString optionCode(int option) = 0;

    QDateTime dateFromString( const QString &date );

public Q_SLOTS:
    virtual void requestSearchResults( const SearchInfo &searchInfo,
                                       const ChoqokId &sinceStatusId = QString(),
                                       uint count = 0,
                                       uint page = 1 ) = 0;
    /**
    This is for convenience
    */
    void requestSearchResults( Choqok::Account *theAccount, const QString &query, int option,
                                       const ChoqokId &sinceStatusId = QString(),
                                       uint count = 0,
                                       uint page = 1 );

Q_SIGNALS:
    void searchResultsReceived( const SearchInfo &searchInfo,
                                QList<Choqok::Post*> &postsList );
    void error( const QString &message );

protected:
    /**
    The QString in the QPair is a human readable string describing what the type searches for.
    The boolean value determines whether or not the search type is traversable
    (if the forward and back buttons should be displayed).
    */
    QMap<int, QPair<QString, bool> > mSearchTypes;

private:
    class Private;
    Private * const d;
};

#endif // TWITTERAPISEARCH_H
