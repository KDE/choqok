/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef EDITACCOUNTWIDGET_H
#define EDITACCOUNTWIDGET_H

#include <QtGui/QWidget>
#include "choqok_export.h"
#include "account.h"

namespace Choqok
{
class Account;
}

class ChoqokEditAccountWidgetPrivate;

/**
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 *
 * This class is used by the microblog plugins to add specific microblog fields in the add account wizard,
 * or in the account preferences. If the given account is 0L, then you will have to create a new account
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
     * If 'account' is 0L we are in the 'add account wizard', otherwise
     * we are editing an existing account.
     */
    explicit ChoqokEditAccountWidget( Choqok::Account* account, QWidget* parent );

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
    Choqok::Account * account() const;

protected:

    /**
    * Set the account
    */
    void setAccount( Choqok::Account *account );

private:
    ChoqokEditAccountWidgetPrivate * const d;
};

#endif

