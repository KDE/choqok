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
#ifndef ACCOUNTSWIZARD_H
#define ACCOUNTSWIZARD_H

#include "ui_accounts_wizard_base.h"
#include "constants.h"
#include "account.h"
class QProgressBar;

class AccountsWizard: public KDialog
{
    Q_OBJECT
public:
    explicit AccountsWizard( QString alias = QString(), QWidget *parent = 0 );

signals:
    void accountAdded( const Account &account );
    void accountEdited( const Account &account );

protected slots:
    virtual void slotButtonClicked( int button );
    void slotUserVerified( Account *userAccount );
    void slotError( const QString &errMsg );
    void handleVerifyTimeout();

private:
    void loadAccount( QString &alias );

    Ui::accounts_wizard_base ui;
    QProgressBar *progress;
    QString mAlias;
    bool isEdit;
    Account mAccount;
};

#endif
