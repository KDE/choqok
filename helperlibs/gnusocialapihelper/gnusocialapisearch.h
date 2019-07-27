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

#ifndef GNUSOCIALAPISEARCH_H
#define GNUSOCIALAPISEARCH_H

#include "twitterapisearch.h"

class KJob;

/**
GNU social/StatatusNet/GNUSocialApi search API implementation.

@author Stephen Henderson \<hendersonsk@gmail.com\>
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_HELPER_EXPORT GNUSocialApiSearch : public TwitterApiSearch
{
    Q_OBJECT
public:
    enum SearchType { ReferenceHashtag = 0, ReferenceGroup, FromUser, ToUser };
    GNUSocialApiSearch(QObject *parent = nullptr);
    ~GNUSocialApiSearch();
    virtual void requestSearchResults(const SearchInfo &searchInfo,
                                      const QString &sinceStatusId = QString(),
                                      uint count = 0, uint page = 1) override;
    virtual QString optionCode(int option) override;

protected:
    QUrl buildUrl(const SearchInfo &searchInfo,
                  QString sinceStatusId = QString(),
                  uint count = 0, uint page = 1);
    QList<Choqok::Post *> parseRss(const QByteArray &buffer);
    QList<Choqok::Post *> parseAtom(const QByteArray &buffer);

protected Q_SLOTS:
    void searchResultsReturned(KJob *job);

private:
    QMap<int, QString> mSearchCode;
    QMap<KJob *, SearchInfo> mSearchJobs;
    static const QRegExp mIdRegExp;
    static const QRegExp m_rId;
};

#endif // GNUSOCIALAPISEARCH_H
