/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GNUSOCIALAPIACCOUNT_H
#define GNUSOCIALAPIACCOUNT_H

#include "gnusocialapihelper_export.h"

#include "twitterapiaccount.h"

class GNUSocialApiMicroBlog;
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class GNUSOCIALAPIHELPER_EXPORT GNUSocialApiAccount : public TwitterApiAccount
{
    Q_OBJECT
public:
    GNUSocialApiAccount(GNUSocialApiMicroBlog *parent, const QString &alias);
    ~GNUSocialApiAccount();

    virtual void writeConfig() override;

    bool isChangeExclamationMark() const;
    void setChangeExclamationMark(bool isChange);

    QString changeExclamationMarkToText() const;
    void setChangeExclamationMarkToText(const QString &text);

    virtual QUrl homepageUrl() const override;

private:
    class Private;
    Private *d;
};

#endif
