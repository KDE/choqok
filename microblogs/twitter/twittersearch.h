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

#ifndef TWITTERSEARCH_H
#define TWITTERSEARCH_H

#include "twitterapihelper/twitterapisearch.h"

/**
Twitter.com search API implementation.

    @author Stephen Henderson \<hendersonsk@gmail.com\>
    @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/

class TwitterSearch : public TwitterApiSearch
{
    Q_OBJECT
public:
    enum SearchType { CustomSearch = 0, ReferenceHashtag, FromUser, ToUser, ReferenceUser };

    TwitterSearch(QObject* parent = 0);
    ~TwitterSearch();

    virtual void requestSearchResults(const SearchInfo &searchInfo,
                                      const ChoqokId& sinceStatusId = ChoqokId(),
                                      uint count = 0, uint page = 1);
    virtual QString optionCode(int option);

protected slots:
    virtual void searchResultsReturned( KJob *job );

protected:
    QList< Choqok::Post* > parseJson(QByteArray buffer);
    Choqok::Post* readStatusesFromJsonMap(const QVariantMap& var);

    QMap<int, QString> mSearchCode;
    QMap<int, QString> mI18nSearchCode;
    QMap<KJob*, SearchInfo> mSearchJobs;
    static const QRegExp m_rId;
};

#endif // TWITTERSEARCH_H
