/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#include "accountswizard.h"
#include <kdebug.h>
AccountsWizard::AccountsWizard ( QString alias, QWidget *parent )
        : KDialog ( parent )
{
    kDebug();
    QWidget *dialog = new QWidget ( this );
    ui.setupUi ( dialog );
    dialog->setAttribute ( Qt::WA_DeleteOnClose );
    this->setMainWidget ( dialog );

    accountsGrp = new KConfig;
//     accountsGrp = new KConfigGroup ( conf, "Accounts" );

    if ( alias.isEmpty() ) {
        this->setCaption ( i18n ( "Add a new account" ) );
        isEdit = false;
    } else {
        this->setCaption ( i18n ( "Edit existing account" ) );
        isEdit = true;
        this->mAlias = alias;
        loadAccount ( alias );
    }

    connect ( this, SIGNAL ( accepted() ), this, SLOT ( slotAccepted() ) );
}

void AccountsWizard::slotAccepted()
{
    kDebug();

    if ( isEdit ) {
        accountsGrp->deleteGroup ( "Account " + mAlias );
    }
    Account a;
//     QString apiPath, service;

    if ( ui.kcfg_service->currentIndex() == 1 ) {
        a.apiPath = IDENTICA_API_PATH;
        a.serviceName = IDENTICA_SERVICE_TEXT;
    } else {
        a.apiPath = TWITTER_API_PATH;
        a.serviceName = TWITTER_SERVICE_TEXT;
    }
    a.username = ui.kcfg_username->text();
    a.password = ui.kcfg_password->text();
    a.direction = (Qt::LayoutDirection)ui.kcfg_direction->currentIndex();
    mAlias = ui.kcfg_alias->text();
    a.alias = mAlias;
    KConfigGroup account ( accountsGrp, "Account "+mAlias );
    account.writeEntry ( "alias", mAlias );
    account.writeEntry ( "username", a.username );
    account.writeEntry ( "password", a.password );
    account.writeEntry ( "direction", ( a.direction == Qt::RightToLeft ) ? "rtl" : "ltr" );
    account.writeEntry ( "service", a.serviceName );
    account.writeEntry ( "api_path", a.apiPath );
    account.writeEntry ( "enabled", true );
    accountsGrp->sync();

    if ( isEdit ) {
        emit accountEdited ( a );
    } else {
        emit accountAdded ( a );
    }
}

void AccountsWizard::loadAccount ( const QString &alias )
{
    kDebug();
    KConfigGroup account ( accountsGrp, "Account "+alias );
    ui.kcfg_alias->setText ( account.readEntry ( "alias", QString() ) );
    ui.kcfg_username->setText ( account.readEntry ( "username", QString() ) );
    ui.kcfg_password->setText ( account.readEntry ( "password", QString() ) );
    ui.kcfg_direction->setCurrentIndex ( ( account.readEntry ( "direction", "ltr" ) == "rtl" ) ? 1 : 0 );
    QString service = account.readEntry ( "service", QString() );
    ui.kcfg_service->setCurrentIndex ( ( service == IDENTICA_SERVICE_TEXT ) ? 1 : 0 );
}

#include "accountswizard.moc"
