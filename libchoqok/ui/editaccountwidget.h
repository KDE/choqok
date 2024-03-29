/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef EDITACCOUNTWIDGET_H
#define EDITACCOUNTWIDGET_H

#include <QWidget>

#include "account.h"
#include "choqok_export.h"

namespace Choqok
{
class Account;
}

class ChoqokEditAccountWidgetPrivate;

/**
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 *
 * This class is used by the microblog plugins to add specific microblog fields in the add account wizard,
 * or in the account preferences. If the given account is nullptr, then you will have to create a new account
 * in @ref apply().
 *
 * Each microblog has to subclass this class.
 *
 * We suggest to put at least these fields in the page:
 *
 * - Alias or Account Label
 *
 * - The User login, or the accountId. you can retrieve it from @ref Choqok::Account::username(). This
 *   field has to be marked as ReadOnly or shown as a label if the account already exists. Remember
 *   that accountId should be constant after account creation!
 *
 * - The password.
 *
 *
 * You may add other custom fields, e.g. the nickname.
 */
class CHOQOK_EXPORT ChoqokEditAccountWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * If 'account' is nullptr we are in the 'add account wizard', otherwise
     * we are editing an existing account.
     */
    explicit ChoqokEditAccountWidget(Choqok::Account *account, QWidget *parent);

    /**
     * Destructor
     */
    virtual ~ChoqokEditAccountWidget();

    /**
    * Check if the inserted information is valid.
    * This method must be reimplemented.
    */
    virtual bool validateData();

    /**
    * Create a new account if we are in the 'add account wizard',
    * otherwise update the existing account.
    */
    virtual Choqok::Account *apply() = 0;

    /**
     * Get a pointer to the Choqok::Account passed to the constructor.
     * You can modify it any way you like, just don't delete the object.
     */
    Choqok::Account *account() const;

protected:

    /**
    * Set the account
    */
    void setAccount(Choqok::Account *account);

private:
    ChoqokEditAccountWidgetPrivate *const d;
};

#endif

