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
#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QtCore/QObject>
#include "account.h"

#include <Accounts/Account>
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
    static AccountManager* self();

    /**
     * \brief Retrieve the list of accounts
     * \return a list of all the accounts
     */
    const QList<Account *> & accounts() const;

    /**
     * \brief Return the account asked
     * \param alias is the alias of account
     * \return the Account object found
     */
    Account * findAccount( const QString &alias );

    /**
     * @brief Add the account.
     *
     * This adds the account to the manager's account list.
     * It will check no accounts already exist with the same Alias, if any, the account is deleted. and not added
     *
     * @return @p account, or 0L if the account was deleted because alias collision
     */
    Account * registerAccount( Account * account );

    /**
     * \brief Delete the account and clean the config data
     *
     * This is praticaly called by the account config page when you remove the account.
     */
    bool removeAccount( const QString &alias );

    QString lastError() const;
    static QString generatePostBackupFileName( const QString &alias, const QString &name );

public Q_SLOTS:
    void ssoAccountCreated(const Accounts::AccountId &id);
    void loadAllAccounts();

Q_SIGNALS:
    void accountAdded( Choqok::Account *account );
    void accountRemoved( const QString &alias );
    void allAccountsLoaded();

private:
    AccountManager();
    class Private;
    Private * const d;
    static AccountManager *mSelf;
};
}
#endif
