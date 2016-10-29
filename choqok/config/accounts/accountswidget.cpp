/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <QAction>
#include <QCheckBox>
#include <QMenu>

#include <KAboutData>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPluginInfo>

#include "accountmanager.h"
#include "accountsdebug.h"
#include "addaccountdialog.h"
#include "choqokuiglobal.h"
#include "editaccountwidget.h"
#include "editaccountdialog.h"
#include "microblog.h"
#include "pluginmanager.h"

K_PLUGIN_FACTORY_WITH_JSON(ChoqokAccountsConfigFactory, "choqok_accountsconfig.json",
                           registerPlugin<AccountsWidget>();)

AccountsWidget::AccountsWidget(QWidget *parent, const QVariantList &args)
    : KCModule(KAboutData::pluginData(QLatin1String("kcm_choqok_accountsconfig")) , parent, args)
{
    qCDebug(CHOQOK);
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    connect(accountsTable, &QTableWidget::cellDoubleClicked,this, &AccountsWidget::accountsTableCellDoubleClicked);
    connect(accountsTable, &QTableWidget::cellClicked,      this, &AccountsWidget::accountsTableCellClicked);
    accountsTable->horizontalHeader()->setStretchLastSection(true);
    connect(btnUp,     &QPushButton::clicked, this, &AccountsWidget::moveCurrentRowUp);
    connect(btnDown,   &QPushButton::clicked, this, &AccountsWidget::moveCurrentRowDown);
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(editAccount()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeAccount()));
    connect(accountsTable, &QTableWidget::currentItemChanged, this, &AccountsWidget::accountsTablestateChanged);

    connect(Choqok::AccountManager::self(), &Choqok::AccountManager::accountAdded,   this, &AccountsWidget::slotAccountAdded);
    connect(Choqok::AccountManager::self(), &Choqok::AccountManager::accountRemoved, this, &AccountsWidget::slotAccountRemoved);

    btnAdd->setMenu(createAddAccountMenu());
//     load();
}

AccountsWidget::~AccountsWidget()
{
    qCDebug(CHOQOK);
}

void AccountsWidget::addAccount()
{
    qCDebug(CHOQOK);
    QAction *act = qobject_cast<QAction *>(sender());
    if (act) {
        QString name = act->data().toString();
        Choqok::MicroBlog *blog = qobject_cast<Choqok::MicroBlog *>(Choqok::PluginManager::self()->loadPlugin(name));
        if (blog) {
            QPointer<AddAccountDialog> d = new AddAccountDialog(
                blog->createEditAccountWidget(0, Choqok::UI::Global::mainWindow()),
                Choqok::UI::Global::mainWindow());
            d->setModal(true);
            d->exec();
        } else {
            KMessageBox::sorry(this, i18n("Cannot load the %1 plugin. Please check your installation.", name));
        }
    }
}

void AccountsWidget::editAccount(QString alias)
{
    qCDebug(CHOQOK);
    int currentRow = accountsTable->currentRow();
    if (alias.isEmpty()) {
        alias = accountsTable->item(currentRow, 0)->text();
    }

    QPointer<Choqok::Account> currentAccount = Choqok::AccountManager::self()->findAccount(alias);
    if (!currentAccount) {
        KMessageBox::detailedSorry(this, i18n("Cannot find the desired account."),
                                   Choqok::AccountManager::self()->lastError());
        return;
    } else {
        ChoqokEditAccountWidget *eaw = currentAccount->microblog()->createEditAccountWidget(currentAccount,
                                       this);
        QPointer<EditAccountDialog> d = new EditAccountDialog(eaw, this);
        d->setModal(true);
        d->exec();

        emitChanged();

        // Needs for update alias after editing account
        accountsTable->setItem(currentRow, 0, new QTableWidgetItem(currentAccount->alias()));
    }
}

void AccountsWidget::removeAccount(QString alias)
{
    qCDebug(CHOQOK) << alias;
    if (KMessageBox::warningYesNoCancel(this, i18n("Are you sure you want to remove the selected account?"))
            == KMessageBox::Yes) {
        if (alias.isEmpty()) {
            alias = accountsTable->item(accountsTable->currentRow(), 0)->text();
        }
        if (!Choqok::AccountManager::self()->removeAccount(alias)) {
            KMessageBox::detailedSorry(this, i18n("Cannot remove the account."), Choqok::AccountManager::self()->lastError());
        }
    }
}

void AccountsWidget::slotAccountAdded(Choqok::Account *account)
{
    qCDebug(CHOQOK);
    addAccountToTable(account);
    emitChanged();
}

void AccountsWidget::slotAccountRemoved(const QString alias)
{
    qCDebug(CHOQOK);
    int count = accountsTable->rowCount();
    for (int i = 0; i < count; ++i) {
        if (accountsTable->item(i, 0)->text() == alias) {
            accountsTable->removeRow(i);
            emitChanged();
            break;
        }
    }
}

void AccountsWidget::addAccountToTable(Choqok::Account *account)
{
    qCDebug(CHOQOK);
    int row = accountsTable->rowCount();
    accountsTable->setRowCount(row + 1);
//   accountsTable->insertRow(row);
//     QCheckBox *enable = new QCheckBox ( accountsTable );
//     enable->setChecked ( account->isEnabled() );
//     accountsTable->setCellWidget ( row, 0, enable );
    accountsTable->setItem(row, 0, new QTableWidgetItem(account->alias()));
    accountsTable->setItem(row, 1, new QTableWidgetItem(QIcon::fromTheme(account->microblog()->pluginIcon()), account->microblog()->serviceName()));
    QCheckBox *enabled = new QCheckBox(accountsTable);
    enabled->setChecked(account->isEnabled());
    accountsTable->setCellWidget(row, 2, enabled);
    QCheckBox *readOnly = new QCheckBox(accountsTable);
    readOnly->setChecked(account->isReadOnly());
    accountsTable->setCellWidget(row, 3, readOnly);
    QCheckBox *quick = new QCheckBox(accountsTable);
    quick->setChecked(account->showInQuickPost());
    accountsTable->setCellWidget(row, 4, quick);
    connect(enabled, &QCheckBox::toggled, this, &AccountsWidget::emitChanged);
    connect(readOnly, &QCheckBox::toggled, this, &AccountsWidget::emitChanged);
    connect(quick,    &QCheckBox::toggled, this, &AccountsWidget::emitChanged);
}

void AccountsWidget::accountsTablestateChanged()
{
    qCDebug(CHOQOK);
    int current = accountsTable->currentRow();
    qCDebug(CHOQOK) << current;
    if (current >= 0 && accountsTable->selectedItems().count() > 0) {
        btnEdit->setEnabled(true);
        btnRemove->setEnabled(true);
        btnUp->setEnabled(current != 0);
        btnDown->setEnabled(current != accountsTable->rowCount() - 1);
    } else {
        btnEdit->setEnabled(false);
        btnRemove->setEnabled(false);
        btnUp->setEnabled(false);
        btnDown->setEnabled(false);
    }
}

void AccountsWidget::load()
{
    qCDebug(CHOQOK);

    accountsTable->clearContents();
    accountsTable->setRowCount(0);

    for (Choqok::Account *ac: Choqok::AccountManager::self()->accounts()) {
       addAccountToTable(ac);
    }
    accountsTable->resizeColumnsToContents();
}

void AccountsWidget::save()
{
    qCDebug(CHOQOK);

    const int rowCount = accountsTable->rowCount();
    bool changed;
    for (int i = 0; i < rowCount; ++i) {
        changed = false;
        Choqok::Account *acc = Choqok::AccountManager::self()->findAccount(accountsTable->item(i, 0)->text());
        if (!acc) {
            continue;
        }
        if (acc->priority() != (uint)i) {
            acc->setPriority((uint)i);
            changed = true;
        }
        QCheckBox *enabled = qobject_cast<QCheckBox *>(accountsTable->cellWidget(i, 2));
        if (enabled && acc->isEnabled() != enabled->isChecked()) {
            acc->setEnabled(enabled->isChecked());
            changed = true;
        }
        QCheckBox *readOnly = qobject_cast<QCheckBox *>(accountsTable->cellWidget(i, 3));
        if (readOnly && acc->isReadOnly() != readOnly->isChecked()) {
            acc->setReadOnly(readOnly->isChecked());
            changed = true;
        }
        QCheckBox *showOnQuick = qobject_cast<QCheckBox *>(accountsTable->cellWidget(i, 4));
        if (showOnQuick && acc->showInQuickPost() != showOnQuick->isChecked()) {
            acc->setShowInQuickPost(showOnQuick->isChecked());
            changed = true;
        }
        if (changed) { //Maybe we should call writeConfig() from setShowInQuickPost(), setReadOnly() and setPriority() -Mehrdad
            acc->writeConfig();
        }
    }
}

QMenu *AccountsWidget::createAddAccountMenu()
{
    mBlogMenu = new QMenu(i18n("Select Micro-Blogging Service"), this);
    const QList<KPluginInfo> list = Choqok::PluginManager::self()->availablePlugins(QLatin1String("MicroBlogs"));
    for (const KPluginInfo &info: list) {
        QAction *act = new QAction(mBlogMenu);
        act->setText(info.name());
        act->setIcon(QIcon::fromTheme(info.icon()));
        act->setData(info.pluginName());
        connect(act, &QAction::triggered, this, &AccountsWidget::addAccount);
        mBlogMenu->addAction(act);
    }
    return mBlogMenu;
}

void AccountsWidget::moveCurrentRowUp()
{
    move(true);
}

void AccountsWidget::moveCurrentRowDown()
{
    move(false);
}

void AccountsWidget::move(bool up)
{
    if (accountsTable->selectedItems().count() <= 0) {
        return;
    }
    emitChanged();
    const int sourceRow = accountsTable->row(accountsTable->selectedItems().at(0));
    bool sourceEnabled = qobject_cast<QCheckBox *>(accountsTable->cellWidget(sourceRow, 2))->isChecked();
    bool sourceReadOnly = qobject_cast<QCheckBox *>(accountsTable->cellWidget(sourceRow, 3))->isChecked();
    bool sourceQuickPost = qobject_cast<QCheckBox *>(accountsTable->cellWidget(sourceRow, 4))->isChecked();
    const int destRow = (up ? sourceRow - 1 : sourceRow + 1);

    if (destRow < 0  || (destRow >= accountsTable->rowCount())) {
        return;
    }

    bool destEnabled = qobject_cast<QCheckBox *>(accountsTable->cellWidget(destRow, 2))->isChecked();
    bool destReadOnly = qobject_cast<QCheckBox *>(accountsTable->cellWidget(destRow, 3))->isChecked();
    bool destQuickPost = qobject_cast<QCheckBox *>(accountsTable->cellWidget(destRow, 4))->isChecked();

    // take whole rows
    QList<QTableWidgetItem *> sourceItems = takeRow(sourceRow);
    QList<QTableWidgetItem *> destItems = takeRow(destRow);

    // set back in reverse order
    setRow(sourceRow, destItems);
    setRow(destRow, sourceItems);

    // taking whole row doesn't work! so changing value of checkBoxes take place here.
    qobject_cast<QCheckBox *>(accountsTable->cellWidget(sourceRow, 2))->setChecked(destEnabled);
    qobject_cast<QCheckBox *>(accountsTable->cellWidget(sourceRow, 3))->setChecked(destReadOnly);
    qobject_cast<QCheckBox *>(accountsTable->cellWidget(sourceRow, 4))->setChecked(destQuickPost);

    qobject_cast<QCheckBox *>(accountsTable->cellWidget(destRow, 2))->setChecked(sourceEnabled);
    qobject_cast<QCheckBox *>(accountsTable->cellWidget(destRow, 3))->setChecked(sourceReadOnly);
    qobject_cast<QCheckBox *>(accountsTable->cellWidget(destRow, 4))->setChecked(sourceQuickPost);

    accountsTable->setCurrentCell(destRow, 0);
    KMessageBox::information(this, i18n("You need to restart Choqok for the accounts priority changes to take effect."),
                             QString(), QLatin1String("ChangeAccountsPriority"));
}

// takes and returns the whole row
QList<QTableWidgetItem *> AccountsWidget::takeRow(int row)
{
    QList<QTableWidgetItem *> rowItems;
    for (int col = 0; col < accountsTable->columnCount(); ++col) {
        rowItems << accountsTable->takeItem(row, col);
    }
    return rowItems;
}

// sets the whole row
void AccountsWidget::setRow(int row, const QList<QTableWidgetItem *> &rowItems)
{
    for (int col = 0; col < accountsTable->columnCount(); ++col) {
        accountsTable->setItem(row, col, rowItems.at(col));
    }
}

void AccountsWidget::emitChanged()
{
    Q_EMIT changed(true);
}

void AccountsWidget::accountsTableCellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    editAccount();
}

void AccountsWidget::accountsTableCellClicked(int row, int column)
{
    Q_UNUSED(column);
    accountsTable->selectRow(row);
    accountsTablestateChanged();
}

#include "accountswidget.moc"
