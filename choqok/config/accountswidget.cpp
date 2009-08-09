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
#include "accountswidget.h"
#include "accountmanager.h"
#include <kdebug.h>
#include <QCheckBox>
#include <KMessageBox>
#include <microblog.h>
#include <KMenu>
#include <pluginmanager.h>
#include <KPluginInfo>
#include <KAction>
#include "addaccountdialog.h"
#include <accountmanager.h>
#include "editaccountwidget.h"
#include "editaccountdialog.h"

AccountsWidget::AccountsWidget( QWidget *parent )
        : QWidget( parent )
{
    kDebug();
    setupUi( this );

//     connect( btnAdd, SIGNAL( clicked() ), this, SLOT( addAccount() ) );
    connect( btnEdit, SIGNAL( clicked() ), this, SLOT( editAccount() ) );
    connect( btnRemove, SIGNAL( clicked() ), this, SLOT( removeAccount() ) );
    connect( accountsTable, SIGNAL( currentItemChanged( QTableWidgetItem *, QTableWidgetItem * ) ),
             this, SLOT( accountsTablestateChanged() ) );

    connect(Choqok::AccountManager::self(), SIGNAL(accountAdded(Choqok::Account*)),
             SLOT(slotAccountAdded(Choqok::Account*)) );
    connect(Choqok::AccountManager::self(), SIGNAL(accountRemoved(QString)),
             SLOT(slotAccountRemoved(QString)) );

    btnAdd->setIcon( KIcon( "list-add" ) );
    btnEdit->setIcon( KIcon( "edit-rename" ) );
    btnRemove->setIcon( KIcon( "list-remove" ) );
    btnAdd->setMenu( createAddAccountMenu() );
    loadAccountsData();
}

AccountsWidget::~ AccountsWidget()
{
    kDebug();
}

void AccountsWidget::addAccount()
{
    kDebug();
    KAction *act = qobject_cast<KAction*>( sender() );
    if(act) {
        QString name = act->data().toString();
        Choqok::MicroBlog *blog = qobject_cast<Choqok::MicroBlog *>(Choqok::PluginManager::self()->loadPlugin(name));
        if(blog){
            AddAccountDialog *d = new AddAccountDialog( blog->createEditAccountWidget(0, this), this );
            d->exec();
            blog->deleteLater();
        } else {
            KMessageBox::sorry(this, i18n("Cannot load %1 plugin. Check your installation.", name));
        }
    }
}

void AccountsWidget::editAccount( QString alias )
{
    kDebug();
    if ( alias.isEmpty() ) {
        int currentRow = accountsTable->currentRow();
        alias = accountsTable->item( currentRow, 0 )->text();
    }
    Choqok::Account *currentAccount = Choqok::AccountManager::self()->findAccount(alias);
    if(!currentAccount) {
        KMessageBox::detailedSorry(this, i18n("Cannot find desired account."),
                                    Choqok::AccountManager::self()->lastError());
        return;
    } else {
        ChoqokEditAccountWidget *eaw = currentAccount->microblog()->createEditAccountWidget(currentAccount, this);
        EditAccountDialog *d = new EditAccountDialog(eaw, this);
        d->exec();
    }
}

void AccountsWidget::removeAccount( QString alias )
{
    kDebug() << alias;
    if ( alias.isEmpty() )
        alias = accountsTable->item( accountsTable->currentRow(), 0 )->text();
    if ( Choqok::AccountManager::self()->removeAccount( alias ) ) {
        accountsTable->removeRow( accountsTable->currentRow() );
    } else
        KMessageBox::detailedSorry( this, i18n( "Cannot remove the account." ), Choqok::AccountManager::self()->lastError() );
}

void AccountsWidget::slotAccountAdded( Choqok::Account *account )
{
    kDebug();
    addAccountToTable( account->alias(), account->microblog()->serviceName() );
//     emit accountAdded( account );
}

void AccountsWidget::slotAccountRemoved( const QString alias )
{
    kDebug();
    int count = accountsTable->rowCount();
    for(int i = 0; i<count; ++i) {
        if(accountsTable->item(i, 0)->text() == alias)
            accountsTable->removeRow(i);
    }
}

void AccountsWidget::addAccountToTable( /* bool isEnabled, */const QString & alias, const QString & service )
{
    kDebug();
    int row = accountsTable->rowCount();
    accountsTable->setRowCount( row + 1 );
//   accountsTable->insertRow(row);
//     QCheckBox *check = new QCheckBox ( accountsTable );
//     check->setChecked ( isEnabled );
//     accountsTable->setCellWidget ( row, 0, check );
    accountsTable->setItem( row, 0, new QTableWidgetItem( alias ) );
    accountsTable->setItem( row, 1, new QTableWidgetItem( service ) );
}

void AccountsWidget::accountsTablestateChanged()
{
    kDebug();
    if ( accountsTable->currentRow() >= 0 ) {
        btnEdit->setEnabled( true );
        btnRemove->setEnabled( true );
    } else {
        btnEdit->setEnabled( false );
        btnRemove->setEnabled( false );
    }
}

void AccountsWidget::loadAccountsData()
{
    kDebug();
//     KConfigGroup grp(&conf, "Accounts");
    QList<Choqok::Account*> ac = Choqok::AccountManager::self()->accounts();
    QListIterator<Choqok::Account*> it( ac );
    while ( it.hasNext() ) {
        Choqok::Account *current = it.next();
        addAccountToTable( current->alias(), current->microblog()->serviceName() );
    }
}

void AccountsWidget::saveAccountsData()
{
}

KMenu * AccountsWidget::createAddAccountMenu()
{
    mBlogMenu = new KMenu(i18n("Select MicroBlog type"), this);
    const QList<KPluginInfo> list = Choqok::PluginManager::self()->availablePlugins("MicroBlogs");
    foreach(const KPluginInfo& info, list){
        KAction *act = new KAction(mBlogMenu);
        act->setText(info.name());
        act->setIcon( KIcon(info.icon()) );
        act->setData(info.pluginName());
        connect(act, SIGNAL(triggered(bool)), this, SLOT(addAccount()) );
        mBlogMenu->addAction(act);
    }
    return mBlogMenu;
}

#include "accountswidget.moc"
