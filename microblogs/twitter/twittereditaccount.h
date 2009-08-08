/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef TWITTEREDITACCOUNT_H
#define TWITTEREDITACCOUNT_H

#include "editaccountwidget.h"
#include <QWidget>
#include "ui_twittereditaccount_base.h"

class TwitterAccount;
class TwitterMicroBlog;
class TwitterEditAccountWidget : public ChoqokEditAccountWidget, public Ui::TwitterEditAccountBase
{
    Q_OBJECT
public:
    TwitterEditAccountWidget(TwitterMicroBlog *microblog, TwitterAccount* account, QWidget *parent);

    /**
    * Destructor
    */
    virtual ~TwitterEditAccountWidget();

    virtual bool validateData();
    /**
    * 
    */
    virtual bool verifyCredentials();

    /**
    * Create a new account if we are in the 'add account wizard',
    * otherwise update the existing account.
    * @Return new or modified account. OR 0L on failure.
    */
    virtual Choqok::Account *apply();

protected slots:
    virtual void slotRegisterNewAccount();

protected:
    TwitterMicroBlog *mBlog;
    TwitterAccount *mAccount;
};

#endif
