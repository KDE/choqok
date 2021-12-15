/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GNUSOCIALAPISEARCH_H
#define GNUSOCIALAPISEARCH_H

#include "gnusocialapihelper_export.h"

#include "twitterapisearch.h"

class KJob;

/**
GNU social/StatatusNet/GNUSocialApi search API implementation.

@author Stephen Henderson \<hendersonsk@gmail.com\>
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class GNUSOCIALAPIHELPER_EXPORT GNUSocialApiSearch : public TwitterApiSearch
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
