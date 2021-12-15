/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTEREDITACCOUNT_H
#define TWITTEREDITACCOUNT_H

#include "editaccountwidget.h"

#include "ui_twittereditaccount_base.h"

class QProgressBar;
class TwitterAccount;
class TwitterMicroBlog;
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TwitterEditAccountWidget : public ChoqokEditAccountWidget, public Ui::TwitterEditAccountBase
{
    Q_OBJECT
public:
    TwitterEditAccountWidget(TwitterMicroBlog *microblog, TwitterAccount *account, QWidget *parent);

    /**
    * Destructor
    */
    ~TwitterEditAccountWidget();

    virtual bool validateData() override;

    /**
    * Create a new account if we are in the 'add account wizard',
    * otherwise update the existing account.
    * @Return new or modified account. OR nullptr on failure.
    */
    virtual Choqok::Account *apply() override;

protected Q_SLOTS:
    void authorizeUser();

protected:
    void loadTimelinesTableState();
    void saveTimelinesTableState();
    virtual void getPinCode();
    void setAuthenticated(bool authenticated);
    bool isAuthenticated;
    TwitterMicroBlog *mBlog;
    TwitterAccount *mAccount;
    QProgressBar *progress;

};

#endif
