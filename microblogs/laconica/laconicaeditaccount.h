/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LACONICAEDITACCOUNT_H
#define LACONICAEDITACCOUNT_H

#include <QByteArray>

#include "editaccountwidget.h"

#include "ui_laconicaeditaccount_base.h"

class QProgressBar;
class GNUSocialApiAccount;
class LaconicaMicroBlog;
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class LaconicaEditAccountWidget : public ChoqokEditAccountWidget, public Ui::LaconicaEditAccountBase
{
    Q_OBJECT
public:
    LaconicaEditAccountWidget(LaconicaMicroBlog *microblog, GNUSocialApiAccount *account, QWidget *parent);

    /**
    * Destructor
    */
    ~LaconicaEditAccountWidget();

    virtual bool validateData() override;

    /**
    * Create a new account if we are in the 'add account wizard',
    * otherwise update the existing account.
    * @Return new or modified account. OR nullptr on failure.
    */
    virtual Choqok::Account *apply() override;

protected Q_SLOTS:
//     virtual void authorizeUser();
//     void slotAuthMethodChanged(int);
    void slotCheckHostUrl();
//     void getAccessToken();

protected:
//     virtual void getPinCode();
    void loadTimelinesTableState();
    void saveTimelinesTableState();
//     void setAuthenticated(bool authenticated);
    void setTextLimit();

    LaconicaMicroBlog *mBlog;
    GNUSocialApiAccount *mAccount;
    QProgressBar *progress;

    bool isAuthenticated;
};

#endif // LACONICAEDITACCOUNT_H
