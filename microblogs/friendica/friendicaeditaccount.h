/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2016 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FRIENDICAEDITACCOUNT_H
#define FRIENDICAEDITACCOUNT_H

#include <QByteArray>

#include "editaccountwidget.h"

#include "ui_friendicaeditaccount_base.h"

class QProgressBar;
class GNUSocialApiAccount;
class FriendicaMicroBlog;

class FriendicaEditAccountWidget : public ChoqokEditAccountWidget, public Ui::FriendicaEditAccountBase
{
    Q_OBJECT
public:
    FriendicaEditAccountWidget(FriendicaMicroBlog *microblog, GNUSocialApiAccount *account, QWidget *parent);

    /**
    * Destructor
    */
    ~FriendicaEditAccountWidget();

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

    FriendicaMicroBlog *mBlog;
    GNUSocialApiAccount *mAccount;
    QProgressBar *progress;

    bool isAuthenticated;
};

#endif // FRIENDICAEDITACCOUNT_H
