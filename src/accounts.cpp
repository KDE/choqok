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
#include "accounts.h"
#include "accountswizard.h"
#include "accountmanager.h"
#include <kdebug.h>
#include <QCheckBox>
#include <KMessageBox>
Accounts::Accounts ( QWidget *parent )
        : QWidget ( parent )
{
    kDebug();
    setupUi ( this );

    connect ( btnAdd, SIGNAL ( clicked() ), this, SLOT ( addAccount() ) );
    connect ( btnEdit, SIGNAL ( clicked() ), this, SLOT ( editAccount() ) );
    connect ( btnRemove, SIGNAL ( clicked() ), this, SLOT ( removeAccount() ) );
    connect ( accountsTable, SIGNAL ( currentItemChanged ( QTableWidgetItem *, QTableWidgetItem * ) ),
              this, SLOT ( accountsTablestateChanged() ) );

    btnAdd->setIcon ( KIcon ( "list-add" ) );
    btnEdit->setIcon ( KIcon ( "edit-rename" ) );
    btnRemove->setIcon ( KIcon ( "list-remove" ) );
    loadAccountsData();
}

Accounts::~ Accounts()
{
    kDebug();
}

void Accounts::addAccount()
{
    kDebug();
    AccountsWizard *wz = new AccountsWizard ( QString(), this );
    connect ( wz, SIGNAL ( accountAdded ( const Account & ) ),
              this, SLOT ( slotAccountAdded ( const Account & ) ) );
    wz->show();
}

void Accounts::editAccount ( QString alias )
{
    kDebug();
    if ( alias.isEmpty() ) {
        int currentRow = accountsTable->currentRow();
        alias = accountsTable->item ( currentRow, 0 )->text();
    }

    AccountsWizard *wz = new AccountsWizard ( alias, this );

    connect ( wz, SIGNAL ( accountEdited ( const Account & ) ),
              this, SLOT ( slotAccountEdited ( const Account & ) ) );
    wz->show();
}

void Accounts::removeAccount ( QString alias )
{
    kDebug()<<alias;
    if(alias.isEmpty())
        alias = accountsTable->item(accountsTable->currentRow(), 0)->text();
    if(AccountManager::self()->removeAccount(alias)){
        accountsTable->removeRow ( accountsTable->currentRow() );
//         emit accountRemoved( alias );
    } else
        KMessageBox::sorry(this, i18n("Cannot remove the account, please try removing it manually."));
}

void Accounts::slotAccountAdded ( const Account &account )
{
    kDebug();
    addAccountToTable ( account.alias, account.serviceName );
//     emit accountAdded( account );
}

void Accounts::slotAccountEdited ( const Account &account )
{
    kDebug();
    int row = accountsTable->currentRow();
    accountsTable->item ( row, 0 )->setText ( account.alias );
    accountsTable->item ( row, 1 )->setText ( account.serviceName );
//     emit accountEdited(account);
}

void Accounts::addAccountToTable (/* bool isEnabled, */const QString & alias, const QString & service )
{
    kDebug();
    int row = accountsTable->rowCount();
    accountsTable->setRowCount ( row + 1 );
//   accountsTable->insertRow(row);
//     QCheckBox *check = new QCheckBox ( accountsTable );
//     check->setChecked ( isEnabled );
//     accountsTable->setCellWidget ( row, 0, check );
    accountsTable->setItem(row, 0, new QTableWidgetItem(alias));
    accountsTable->setItem(row, 1, new QTableWidgetItem(service));
}

void Accounts::accountsTablestateChanged()
{
    kDebug();
    if ( accountsTable->currentRow() >= 0 ) {
        btnEdit->setEnabled ( true );
        btnRemove->setEnabled ( true );
    } else {
        btnEdit->setEnabled ( false );
        btnRemove->setEnabled ( false );
    }
}

void Accounts::loadAccountsData()
{
    kDebug();
//     KConfigGroup grp(&conf, "Accounts");
    QList<Account> ac = AccountManager::self()->accounts();
    QListIterator<Account> it(ac);
    while(it.hasNext()){
        Account current = it.next();
        addAccountToTable(current.alias, current.serviceName);
    }
}

void Accounts::saveAccountsData()
{
}

#include "accounts.moc"
