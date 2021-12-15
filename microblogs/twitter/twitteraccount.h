/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERACCOUNT_H
#define TWITTERACCOUNT_H

#include <QUrl>

#include "account.h"

#include "twitterapiaccount.h"

class TwitterMicroBlog;
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TwitterAccount : public TwitterApiAccount
{
    Q_OBJECT
public:
    TwitterAccount(TwitterMicroBlog *parent, const QString &alias);
    ~TwitterAccount();

    virtual void setApi(const QString &api) override;

    QUrl uploadUrl() const;

    QString uploadHost() const;
    void setUploadHost(const QString &uploadHost);

protected:
    void setUploadUrl(const QUrl &url);
    void generateUploadUrl();

private:
    class Private;
    Private *d;
};

#endif // TWITTERACCOUNT_H
