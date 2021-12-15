/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>

#include "account.h"

namespace Choqok
{
/**
Account manager class

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT AccountManager : public QObject
{
    Q_OBJECT
public:
    ~AccountManager();

    /**
     * \brief Retrieve the instance of AccountManager.
     *
     * The account manager is a singleton class of which only a single
     * instance will exist. If no manager exists yet this function will
     * create one for you.
     *
     * \return the instance of the AccountManager
     */
    static AccountManager *self();

    /**
     * \brief Retrieve the list of accounts
     * \return a list of all the accounts
     */
    const QList<Account *> &accounts() const;

    /**
     * \brief Return the account asked
     * \param alias is the alias of account
     * \return the Account object found
     */
    Account *findAccount(const QString &alias);

    /**
     * @brief Add the account.
     *
     * This adds the account to the manager's account list.
     * It will check no accounts already exist with the same Alias, if any, the account is deleted. and not added
     *
     * @return @p account, or nullptr if the account was deleted because alias collision
     */
    Account *registerAccount(Account *account);

    /**
     * \brief Delete the account and clean the config data
     *
     * This is praticaly called by the account config page when you remove the account.
     */
    bool removeAccount(const QString &alias);

    QString lastError() const;
    static QString generatePostBackupFileName(const QString &alias, const QString &name);

public Q_SLOTS:
    void loadAllAccounts();

Q_SIGNALS:
    void accountAdded(Choqok::Account *account);
    void accountRemoved(const QString &alias);
    void allAccountsLoaded();

private:
    AccountManager();
    class Private;
    Private *const d;
    static AccountManager *mSelf;
};
}
#endif
