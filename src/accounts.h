/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef ACCOUNTS_H
#define ACCOUNTS_H
#include "datacontainers.h"
#include "ui_accounts_base.h"
#include <QWidget>

class Accounts: public QWidget, public Ui::accounts_base
{
    Q_OBJECT
public:
    Accounts ( QWidget *parent = 0 );
    ~Accounts();

signals:
    void accountAdded(const Account &account);
    void accountRemoved(const QString &alias);
    void accountEdited(const Account &alias);

public slots:
    void addAccount();
    void editAccount ( QString alias = QString() );
    void removeAccount ( QString alias = QString() );

protected slots:
    void slotAccountAdded ( const Account &account );
    void slotAccountEdited ( const Account &account );
    void accountsTablestateChanged();

private:
    void addAccountToTable (/* bool isEnabled, */const QString &alias, const QString &service );
    void loadAccountsData();
    void saveAccountsData();
};

#endif
