/*
    This file is part of choqoK, the KDE Twitter client

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
#include "accountswizard.h"
#include "accountmanager.h"
#include <kdebug.h>
#include <QProgressBar>
#include <KMessageBox>
#include "backend.h"
AccountsWizard::AccountsWizard ( QString alias, QWidget *parent )
        : KDialog ( parent )
{
    kDebug();
    QWidget *dialog = new QWidget ( this );
    ui.setupUi ( dialog );
    dialog->setAttribute ( Qt::WA_DeleteOnClose );
    this->setMainWidget ( dialog );

    if ( alias.isEmpty() ) {
        this->setCaption ( i18n ( "Add a new account" ) );
        isEdit = false;
    } else {
        this->setCaption ( i18n ( "Modify existing account" ) );
        isEdit = true;
        this->mAlias = alias;
        loadAccount ( alias );
    }

}

void AccountsWizard::loadAccount ( QString &alias )
{
    kDebug()<<"Loading account "<<alias;
    mAccount = AccountManager::self()->findAccount(alias);
    if(mAccount.isError()){
        kDebug()<<"Error on Loading Account with alias "<<alias;
        return;
    }
    ui.kcfg_username->setText ( mAccount.username() );
    ui.kcfg_alias->setText ( mAccount.alias());
    ui.kcfg_password->setText ( mAccount.password() );
    ui.kcfg_direction->setCurrentIndex ( ( mAccount.direction() == Qt::RightToLeft ) ? 1 : 0 );
    ui.kcfg_service->setCurrentIndex ( ( mAccount.serviceName() == IDENTICA_SERVICE_TEXT ) ? 1 : 0 );
}

void AccountsWizard::slotButtonClicked(int button)
{
    kDebug();
    if ( button == KDialog::Ok ) {
        ///Show ProgressBar:
        QProgressBar *progress = new QProgressBar(this);
        progress->setMinimum( 0 );
        progress->setMaximum( 0 );
        QGridLayout* grid = qobject_cast<QGridLayout*>(this->mainWidget()->layout());
        grid->addWidget(progress, grid->rowCount(), 0, grid->rowCount(), 2);
        ///Check for account
//         Account a;
        if ( ui.kcfg_service->currentIndex() == 1 ) {
            mAccount.setApiPath( IDENTICA_API_PATH );
            mAccount.setServiceName( IDENTICA_SERVICE_TEXT );
        } else {
            mAccount.setApiPath( TWITTER_API_PATH );
            mAccount.setServiceName( TWITTER_SERVICE_TEXT );
        }
        mAccount.setUsername( ui.kcfg_username->text() );
        mAccount.setPassword( ui.kcfg_password->text() );
        mAccount.setDirection( (Qt::LayoutDirection)ui.kcfg_direction->currentIndex() );
        mAccount.setAlias( ui.kcfg_alias->text() );
        
        Backend *b = new Backend(&mAccount, this);
        connect(b, SIGNAL(userVerified(Account*)), this, SLOT(slotUserVerified(Account*)));
        connect(b, SIGNAL(sigError(QString&)), this, SLOT(slotError(QString&)));
        b->verifyCredential();
    } else
        KDialog::slotButtonClicked( button );
}

void AccountsWizard::slotUserVerified(Account * userAccount)
{
    kDebug();
    if(!userAccount){
        kDebug()<<"userAccount is NULL";
        return;
    }
    mAccount = *userAccount;
    if(isEdit){
        mAccount = AccountManager::self()->modifyAccount(mAccount, mAlias);
    } else {
        mAccount = AccountManager::self()->addAccount(mAccount);
    }
    if(mAccount.isError()){
        kDebug()<<"Cannot add or modify account with alias "<<mAccount.alias();
        return;
    }

    if ( isEdit ) {
        emit accountEdited ( mAccount );
    } else {
        emit accountAdded ( mAccount );
    }
    accept();
}

void AccountsWizard::slotError(QString & errMsg)
{
    kDebug();
    KMessageBox::detailedError(this, i18n("authentication failed, please check your credentials."), errMsg);
}

#include "accountswizard.moc"
