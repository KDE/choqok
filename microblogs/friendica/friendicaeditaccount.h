/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2016 Andrea Scarpino <scarpino@kde.org>

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

#ifndef FRIENDICAEDITACCOUNT_H
#define FRIENDICAEDITACCOUNT_H

#include <QByteArray>

#include "editaccountwidget.h"

#include "ui_friendicaeditaccount_base.h"

namespace QOAuth
{
class Interface;
}

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

    QByteArray token;
    QByteArray tokenSecret;
    QByteArray oauthConsumerKey;
    QByteArray oauthConsumerSecret;
    QOAuth::Interface *qoauth;
};

#endif // FRIENDICAEDITACCOUNT_H
