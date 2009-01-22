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
#ifndef ACCOUNTSWIZARD_H
#define ACCOUNTSWIZARD_H

#include "ui_accounts_wizard_base.h"
#include "datacontainers.h"
#include "constants.h"
#include "account.h"


class AccountsWizard: public KDialog
{
    Q_OBJECT
public:
    explicit AccountsWizard ( QString alias = QString(), QWidget *parent = 0 );

signals:
    void accountAdded ( const Account &account );
    void accountEdited ( const Account &account );

protected slots:
    virtual void slotButtonClicked( int button );
    void slotUserVerified(Account *userAccount);
    void slotError(QString &errMsg);

private:
    void loadAccount ( QString &alias );

    Ui::accounts_wizard_base ui;

    QString mAlias;
    bool isEdit;
    Account mAccount;
};

#endif
