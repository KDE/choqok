/*
    This file is part of choqoK, the KDE micro-blogging client

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
#include <QTimer>

AccountsWizard::AccountsWizard( QString alias, QWidget *parent )
        : KDialog( parent )
{
    kDebug();
    QWidget *dialog = new QWidget( this );
    ui.setupUi( dialog );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    this->setMainWidget( dialog );

    if ( alias.isEmpty() ) {
        this->setCaption( i18n( "Add a new account" ) );
        isEdit = false;
    } else {
        this->setCaption( i18n( "Modify existing account" ) );
        isEdit = true;
        this->mAlias = alias;
        loadAccount( alias );
    }

}

void AccountsWizard::loadAccount( QString &alias )
{
    kDebug() << "Loading account " << alias;
    mAccount = AccountManager::self()->findAccount( alias );
    if ( mAccount.isError() ) {
        kDebug() << "Error on Loading Account with alias " << alias;
        return;
    }
    ui.kcfg_username->setText( mAccount.username() );
    ui.kcfg_alias->setText( mAccount.alias() );
    ui.kcfg_password->setText( mAccount.password() );
    ui.kcfg_direction->setCurrentIndex(( mAccount.direction() == Qt::RightToLeft ) ? 1 : 0 );
    ui.kcfg_service->setCurrentIndex( mAccount.serviceType() );
}

void AccountsWizard::slotButtonClicked( int button )
{
    kDebug();
    if ( button == KDialog::Ok ) {
        ///Show ProgressBar:
        if ( progress )
            progress = 0L;
        progress = new QProgressBar( this );
        progress->setMinimum( 0 );
        progress->setMaximum( 0 );
        QGridLayout* grid = qobject_cast<QGridLayout*>( this->mainWidget()->layout() );
        if ( !grid )
            return;
        grid->addWidget( progress, grid->rowCount(), 0, grid->rowCount(), 2 );
        ///Check for account
        mAccount.setServiceType( (Account::Service) ui.kcfg_service->currentIndex() );
        mAccount.setUsername( ui.kcfg_username->text() );
        mAccount.setPassword( ui.kcfg_password->text() );
        mAccount.setDirection(( Qt::LayoutDirection )ui.kcfg_direction->currentIndex() );
        mAccount.setAlias( ui.kcfg_alias->text() );

        Backend *b = new Backend( &mAccount, this );
        connect( b, SIGNAL( userVerified( Account* ) ), this, SLOT( slotUserVerified( Account* ) ) );
        connect( b, SIGNAL( sigError( const QString& ) ), this, SLOT( slotError( const QString& ) ) );
        b->verifyCredential();
        QTimer::singleShot( 45000, this, SLOT( handleVerifyTimeout() ) );
    } else
        KDialog::slotButtonClicked( button );
}

void AccountsWizard::slotUserVerified( Account * userAccount )
{
    kDebug();
    if ( !userAccount ) {
        kDebug() << "userAccount is NULL";
        return;
    }
    mAccount = *userAccount;
    if ( isEdit ) {
        mAccount = AccountManager::self()->modifyAccount( mAccount, mAlias );
    } else {
        mAccount = AccountManager::self()->addAccount( mAccount );
    }
    if ( mAccount.isError() ) {
        if ( progress )
            progress->deleteLater();
        kDebug() << "Cannot add or modify account with alias " << mAccount.alias();
        KMessageBox::detailedError(this, i18n( "An error occurred when adding this account" ),
                                    AccountManager::self()->lastError());
        return;
    }

    if ( isEdit ) {
        emit accountEdited( mAccount );
    } else {
        emit accountAdded( mAccount );
    }
    accept();
}

void AccountsWizard::slotError( const QString & errMsg )
{
    kDebug();
    KMessageBox::detailedError( this, i18n( "authentication failed, please check your credentials." ), errMsg );
    if ( progress )
        progress->deleteLater();
}

void AccountsWizard::handleVerifyTimeout()
{
    KMessageBox::sorry(this, i18n( "Verification progress timed out.\
Check your Internet connection and credentials then try again..." ) , i18n( "Timeout" ) );
    if ( progress )
        progress->deleteLater();
}

#include "accountswizard.moc"
