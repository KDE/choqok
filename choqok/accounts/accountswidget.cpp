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
#include <choqokuiglobal.h>
#include "ui_accountswidget_base.h"

class AccountsWidget::Private
{
public:
    Ui_AccountsWidgetBase ui;
    KMenu *mBlogMenu;
};

AccountsWidget::AccountsWidget( QWidget* parent )
        : KDialog(parent), d(new Private)
{
    kDebug();
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *wd = new QWidget(this);
    d->ui.setupUi( wd );
    setMainWidget(wd);
    setWindowTitle(i18n("Manage Accounts"));
    resize(530, 300);
//     connect( btnAdd, SIGNAL( clicked() ), this, SLOT( addAccount() ) );
    connect( d->ui.btnEdit, SIGNAL( clicked() ), this, SLOT( editAccount() ) );
    connect( d->ui.btnRemove, SIGNAL( clicked() ), this, SLOT( removeAccount() ) );
    connect( d->ui.accountsTable, SIGNAL( currentItemChanged( QTableWidgetItem *, QTableWidgetItem * ) ),
             this, SLOT( accountsTablestateChanged() ) );

    connect(Choqok::AccountManager::self(), SIGNAL(accountAdded(Choqok::Account*)),
             SLOT(slotAccountAdded(Choqok::Account*)) );
    connect(Choqok::AccountManager::self(), SIGNAL(accountRemoved(QString)),
             SLOT(slotAccountRemoved(QString)) );

    d->ui.btnAdd->setIcon( KIcon( "list-add" ) );
    d->ui.btnEdit->setIcon( KIcon( "edit-rename" ) );
    d->ui.btnEdit->hide();///FIXME Fix account modify function
    d->ui.btnRemove->setIcon( KIcon( "list-remove" ) );
    d->ui.btnAdd->setMenu( createAddAccountMenu() );
    load();
}

AccountsWidget::~AccountsWidget()
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
            AddAccountDialog *d = new AddAccountDialog(
                                       blog->createEditAccountWidget(0, Choqok::UI::Global::mainWindow()),
                                                        Choqok::UI::Global::mainWindow() );
            d->exec();
        } else {
            KMessageBox::sorry(this, i18n("Cannot load %1 plugin. Check your installation.", name));
        }
    }
}

void AccountsWidget::editAccount( QString alias )
{
    kDebug();
    if ( alias.isEmpty() ) {
        int currentRow = d->ui.accountsTable->currentRow();
        alias = d->ui.accountsTable->item( currentRow, 0 )->text();
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
        alias = d->ui.accountsTable->item( d->ui.accountsTable->currentRow(), 0 )->text();
    if ( Choqok::AccountManager::self()->removeAccount( alias ) ) {
//         accountsTable->removeRow( accountsTable->currentRow() );
    } else
        KMessageBox::detailedSorry( this, i18n( "Cannot remove the account." ), Choqok::AccountManager::self()->lastError() );
}

void AccountsWidget::slotAccountAdded( Choqok::Account *account )
{
    kDebug();
    addAccountToTable( account );
}

void AccountsWidget::slotAccountRemoved( const QString alias )
{
    kDebug();
    int count = d->ui.accountsTable->rowCount();
    for(int i = 0; i<count; ++i) {
        if(d->ui.accountsTable->item(i, 0)->text() == alias){
            d->ui.accountsTable->removeRow(i);
            break;
        }
    }
}

void AccountsWidget::addAccountToTable( Choqok::Account* account )
{
    kDebug();
    int row = d->ui.accountsTable->rowCount();
    d->ui.accountsTable->setRowCount( row + 1 );
//   accountsTable->insertRow(row);
//     QCheckBox *enable = new QCheckBox ( d->ui.accountsTable );
//     enable->setChecked ( account->isEnabled() );
//     d->ui.accountsTable->setCellWidget ( row, 0, enable );
    d->ui.accountsTable->setItem( row, 0, new QTableWidgetItem( account->alias() ) );
    d->ui.accountsTable->setItem( row, 1, new QTableWidgetItem( KIcon(account->microblog()->pluginIcon()), account->microblog()->serviceName() ) );
    QCheckBox *readOnly = new QCheckBox ( d->ui.accountsTable );
    readOnly->setChecked ( account->isReadOnly() );
    d->ui.accountsTable->setCellWidget ( row, 2, readOnly );
    QCheckBox *quick = new QCheckBox ( d->ui.accountsTable );
    quick->setChecked ( account->showInQuickPost() );
    d->ui.accountsTable->setCellWidget ( row, 3, quick );
}

void AccountsWidget::accountsTablestateChanged()
{
    kDebug();
    if ( d->ui.accountsTable->currentRow() >= 0 ) {
        d->ui.btnEdit->setEnabled( true );
        d->ui.btnRemove->setEnabled( true );
    } else {
        d->ui.btnEdit->setEnabled( false );
        d->ui.btnRemove->setEnabled( false );
    }
}

void AccountsWidget::load()
{
    d->ui.accountsTable->clearContents();
    QList<Choqok::Account*> ac = Choqok::AccountManager::self()->accounts();
    QListIterator<Choqok::Account*> it( ac );
    while ( it.hasNext() ) {
        Choqok::Account *current = it.next();
        addAccountToTable( current );
    }
    d->ui.accountsTable->resizeColumnsToContents();
}

void AccountsWidget::save()
{
    kDebug();
    int rowCount = d->ui.accountsTable->rowCount();
    bool changed;
    for(int i=0; i<rowCount; ++i){
        changed = false;
        Choqok::Account *acc = Choqok::AccountManager::self()->findAccount(d->ui.accountsTable->item(i, 0)->text());
        if(!acc)
            continue;
        QCheckBox *readOnly = qobject_cast<QCheckBox*>(d->ui.accountsTable->cellWidget(i, 2));
        if(readOnly && acc->isReadOnly() != readOnly->isChecked()){
            acc->setReadOnly(readOnly->isChecked());
            changed = true;
        }
        QCheckBox *showOnQuick= qobject_cast<QCheckBox*>(d->ui.accountsTable->cellWidget(i, 3));
        if(showOnQuick && acc->showInQuickPost() != showOnQuick->isChecked()){
            acc->setShowInQuickPost(showOnQuick->isChecked());
            changed = true;
        }
        if(changed)
            acc->writeConfig();
    }
}

KMenu * AccountsWidget::createAddAccountMenu()
{
    d->mBlogMenu = new KMenu(i18n("Select MicroBlog type"), this);
    const QList<KPluginInfo> list = Choqok::PluginManager::self()->availablePlugins("MicroBlogs");
    foreach(const KPluginInfo& info, list){
        KAction *act = new KAction(d->mBlogMenu);
        act->setText(info.name());
        act->setIcon( KIcon(info.icon()) );
        act->setData(info.pluginName());
        connect(act, SIGNAL(triggered(bool)), this, SLOT(addAccount()) );
        d->mBlogMenu->addAction(act);
    }
    return d->mBlogMenu;
}

void AccountsWidget::slotButtonClicked(int button)
{
    if(button == KDialog::Ok)
        save();
    KDialog::slotButtonClicked(button);
}

#include "accountswidget.moc"
