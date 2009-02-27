/*
    This file is part of choqoK, the KDE micro-blogging client

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
#ifndef ACCOUNTS_H
#define ACCOUNTS_H
#include "ui_accounts_base.h"
#include <QWidget>
#include "account.h"

class Accounts: public QWidget, public Ui::accounts_base
{
    Q_OBJECT
public:
    Accounts( QWidget *parent = 0 );
    ~Accounts();

public slots:
    void addAccount();
    void editAccount( QString alias = QString() );
    void removeAccount( QString alias = QString() );

protected slots:
    void slotAccountAdded( const Account &account );
    void slotAccountEdited( const Account &account );
    void accountsTablestateChanged();

private:
    void addAccountToTable( /* bool isEnabled, */const QString &alias, const QString &service );
    void loadAccountsData();
    void saveAccountsData();
};

#endif
