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
#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include "datacontainers.h"
#include "account.h"
#include "backend.h"
class KConfig;
namespace KWallet
{
    class Wallet;
}

/**
Account manager class

    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class AccountManager : public QObject
{
    Q_OBJECT
public:
    AccountManager( QObject* parent = 0 );

    ~AccountManager();

    static QString generateStatusBackupFileName( const QString &name, Backend::TimeLineType type );

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
    const QList<Account> & accounts() const;

    /**
     * \brief Return the account asked
     * \param alias is the alias of account
     * \return the Account object found
     */
    Account findAccount( const QString &alias );

    /**
     * \brief Delete the account and clean the config data
     *
     * This is praticaly called by the account config page when you remove the account.
     */
    bool removeAccount( const QString &alias );

    /**
     * @brief Add the account.
     *
     * This adds the account to the manager's account list.
     * It will check no accounts already exist with the same Alias, if any, the account is deleted. and not added
     *
     * @return @p account, or 0L if the account was deleted because alias collision
     */
    Account & addAccount( Account &account );

    /**
     * @brief Modify the account.
     *
     * This modifies the account in the manager's account list.
     * It will check that no accounts already exist with the same Alias, if any, the account will not be modified
     *
     * @return @p account, or 0L if the account was not modified because alias collision
     */
    Account & modifyAccount( Account &account, const QString &previousAlias );

    /**
     * Get the friends list of @p alias
     * @param alias Account alias
     * @return List of friends
     */
    QStringList listFriends( const QString &alias );

    /**
     * Save the @p list as friends of @p alias
     * @param alias Account alias
     * @param list list of new friends
     */
    void saveFriendsList( const QString &alias, const QStringList &list );

    QString lastError() const;

signals:
    void accountAdded( const Account &account );
    void accountRemoved( const QString &alias );

protected slots:
    void userVerified( Account *userAccount );
private:
    void loadAccounts();

    static AccountManager *mSelf;
    QList<Account> mAccounts;
    KConfig *conf;
    KWallet::Wallet* mWallet;
    QString mLastError;
};

#endif
