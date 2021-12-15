/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERSEARCH_H
#define TWITTERSEARCH_H

#include "twitterapisearch.h"

class KJob;

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

    TwitterSearch(QObject *parent = nullptr);
    ~TwitterSearch();

    virtual void requestSearchResults(const SearchInfo &searchInfo,
                                      const QString &sinceStatusId = QString(),
                                      uint count = 0, uint page = 1) override;
    virtual QString optionCode(int option) override;

protected Q_SLOTS:
    void searchResultsReturned(KJob *job);

protected:
    Choqok::Post *readStatusesFromJsonMap(const QVariantMap &var);

    QMap<int, QString> mSearchCode;
    QMap<int, QString> mI18nSearchCode;
    QMap<KJob *, SearchInfo> mSearchJobs;
    static const QRegExp m_rId;
};

#endif // TWITTERSEARCH_H
