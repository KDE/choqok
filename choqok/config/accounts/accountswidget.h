/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include "ui_accountswidget_base.h"

#include <KCModule>

class QMenu;
namespace Choqok
{
class Account;
}

class AccountsWidget: public KCModule, public Ui_AccountsWidgetBase
{
    Q_OBJECT
public:
    AccountsWidget(QWidget *parent, const QVariantList &args);
    ~AccountsWidget();

public Q_SLOTS:
    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void addAccount();
    void editAccount(QString alias = QString());
    void removeAccount(QString alias = QString());

    void slotAccountAdded(Choqok::Account *account);
    void slotAccountRemoved(const QString alias);
    void accountsTablestateChanged();

    void moveCurrentRowUp();
    void moveCurrentRowDown();

    void emitChanged();

    void accountsTableCellDoubleClicked(int row, int column);
    void accountsTableCellClicked(int row, int column);
private:
    void move(bool up);
    QList<QTableWidgetItem *> takeRow(int row);
    void setRow(int row, const QList<QTableWidgetItem *> &rowItems);
    void addAccountToTable(Choqok::Account *account);
    QMenu *createAddAccountMenu();
    QMenu *mBlogMenu;
};

#endif
