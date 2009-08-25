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
#include <KPluginFactory>

K_PLUGIN_FACTORY( ChoqokAccountsConfigFactory,
                  registerPlugin<AccountsWidget>(); )
K_EXPORT_PLUGIN( ChoqokAccountsConfigFactory("kcm_choqok_accountsconfig") )

AccountsWidget::AccountsWidget( QWidget* parent, const QVariantList& args )
        : KCModule( ChoqokAccountsConfigFactory::componentData(), parent, args )
{
    kDebug();
    setAttribute(Qt::WA_DeleteOnClose);
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
    btnUp->setIcon( KIcon("go-up") );
    btnDown->setIcon( KIcon("go-down") );
    btnAdd->setMenu( createAddAccountMenu() );
//     load();
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
        int currentRow = accountsTable->currentRow();
        alias = accountsTable->item( currentRow, 0 )->text();
    }
    Choqok::Account *currentAccount = Choqok::AccountManager::self()->findAccount(alias);
    if(!currentAccount) {
        KMessageBox::detailedSorry(this, i18n("Cannot find desired account."),
                                    Choqok::AccountManager::self()->lastError());
        return;
    } else {
        ChoqokEditAccountWidget *eaw = currentAccount->microblog()->createEditAccountWidget(currentAccount,
                                                                                            this);
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
//         accountsTable->removeRow( accountsTable->currentRow() );
    } else
        KMessageBox::detailedSorry( this, i18n( "Cannot remove the account." ), Choqok::AccountManager::self()->lastError() );
}

void AccountsWidget::slotAccountAdded( Choqok::Account *account )
{
    kDebug();
    addAccountToTable( account );
    emit changed(true);
}

void AccountsWidget::slotAccountRemoved( const QString alias )
{
    kDebug();
    int count = accountsTable->rowCount();
    for(int i = 0; i<count; ++i) {
        if(accountsTable->item(i, 0)->text() == alias){
            accountsTable->removeRow(i);
            emit changed(true);
            break;
        }
    }
}

void AccountsWidget::addAccountToTable( Choqok::Account* account )
{
    kDebug();
    int row = accountsTable->rowCount();
    accountsTable->setRowCount( row + 1 );
//   accountsTable->insertRow(row);
//     QCheckBox *enable = new QCheckBox ( accountsTable );
//     enable->setChecked ( account->isEnabled() );
//     accountsTable->setCellWidget ( row, 0, enable );
    accountsTable->setItem( row, 0, new QTableWidgetItem( account->alias() ) );
    accountsTable->setItem( row, 1, new QTableWidgetItem( KIcon(account->microblog()->pluginIcon()), account->microblog()->serviceName() ) );
    QCheckBox *readOnly = new QCheckBox ( accountsTable );
    readOnly->setChecked ( account->isReadOnly() );
    accountsTable->setCellWidget ( row, 2, readOnly );
    QCheckBox *quick = new QCheckBox ( accountsTable );
    quick->setChecked ( account->showInQuickPost() );
    accountsTable->setCellWidget ( row, 3, quick );
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

void AccountsWidget::load()
{
    kDebug();
    QList<Choqok::Account*> ac = Choqok::AccountManager::self()->accounts();
    QListIterator<Choqok::Account*> it( ac );
    while ( it.hasNext() ) {
        Choqok::Account *current = it.next();
        addAccountToTable( current );
    }
    accountsTable->resizeColumnsToContents();
}

void AccountsWidget::save()
{
    kDebug();
    int rowCount = accountsTable->rowCount();
    bool changed;
    for(int i=0; i<rowCount; ++i){
        changed = false;
        Choqok::Account *acc = Choqok::AccountManager::self()->findAccount(accountsTable->item(i, 0)->text());
        if(!acc)
            continue;
        QCheckBox *readOnly = qobject_cast<QCheckBox*>(accountsTable->cellWidget(i, 2));
        if(readOnly && acc->isReadOnly() != readOnly->isChecked()){
            acc->setReadOnly(readOnly->isChecked());
            changed = true;
        }
        QCheckBox *showOnQuick= qobject_cast<QCheckBox*>(accountsTable->cellWidget(i, 3));
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
    mBlogMenu = new KMenu(i18n("Select MicroBlog Service"), this);
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
