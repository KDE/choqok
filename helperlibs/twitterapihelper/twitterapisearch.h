/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPISEARCH_H
#define TWITTERAPISEARCH_H

#include <QDateTime>
#include <QMap>
#include <QPair>

#include "account.h"
#include "choqoktypes.h"

class CHOQOK_HELPER_EXPORT SearchInfo
{
public:
    SearchInfo();
    SearchInfo(Choqok::Account *theAccount, const QString &queryStr,
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
    TwitterApiSearch(QObject *parent = nullptr);
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

    QDateTime dateFromString(const QString &date);

public Q_SLOTS:
    virtual void requestSearchResults(const SearchInfo &searchInfo,
                                      const QString &sinceStatusId = QString(),
                                      uint count = 0,
                                      uint page = 1) = 0;
    /**
    This is for convenience
    */
    void requestSearchResults(Choqok::Account *theAccount, const QString &query, int option,
                              const QString &sinceStatusId = QString(),
                              uint count = 0,
                              uint page = 1);

Q_SIGNALS:
    void searchResultsReceived(const SearchInfo &searchInfo,
                               QList<Choqok::Post *> &postsList);
    void error(const QString &message);

protected:
    /**
    The QString in the QPair is a human readable string describing what the type searches for.
    The boolean value determines whether or not the search type is traversable
    (if the forward and back buttons should be displayed).
    */
    QMap<int, QPair<QString, bool> > mSearchTypes;

private:
    class Private;
    Private *const d;
};

#endif // TWITTERAPISEARCH_H
