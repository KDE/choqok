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

#include "quicktwit.h"
#include <QKeyEvent>
#include <KComboBox>
#include "statustextedit.h"
#include "backend.h"
#include "datacontainers.h"
#include "mainwindow.h"
#include "constants.h"
#include "settings.h"
#include "accountmanager.h"

QuickTwit::QuickTwit( QWidget* parent ): KDialog( parent )
{
    kDebug();
    QWidget *wdg = new QWidget( this );
    ui.setupUi( wdg );

    txtStatus = new StatusTextEdit( this );
    txtStatus->setTabChangesFocus( true );
    loadAccounts();

//     txtStatus->setDefaultDirection(accountsList[ui.comboAccounts->currentIndex()].direction);
    ui.layout->addWidget( txtStatus );
    this->setMainWidget( wdg );
    this->resize( Settings::quickTweetSize() );
    txtStatus->setFocus( Qt::OtherFocusReason );

    this->setCaption( i18n( "Quick Tweet" ) );
    ui.lblCounter->setText( QString::number( MAX_STATUS_SIZE ) );

    connect( txtStatus, SIGNAL( returnPressed( QString& ) ),
             this, SLOT( slotPostNewStatus( QString& ) ) );
    connect( txtStatus, SIGNAL( charsLeft( int ) ),
             this, SLOT( checkNewStatusCharactersCount( int ) ) );
    connect( this, SIGNAL( accepted() ), this, SLOT( sltAccepted() ) );
    connect( ui.checkAll, SIGNAL( toggled( bool ) ),
             this, SLOT( checkAll( bool ) ) );
    connect( AccountManager::self(), SIGNAL( accountAdded( const Account& ) ),
             this, SLOT( addAccount( const Account& ) ) );
    connect( AccountManager::self(), SIGNAL( accountRemoved( const QString& ) ),
             this, SLOT( removeAccount( const QString& ) ) );
}

QuickTwit::~QuickTwit()
{
    Settings::setQuickTweetSize( this->size() );
    Settings::self()->writeConfig();
    kDebug();
}

void QuickTwit::showFocusedOnNewStatusField()
{
    txtStatus->setFocus( Qt::OtherFocusReason );
    this->show();
}

void QuickTwit::checkNewStatusCharactersCount( int numOfChars )
{
    if ( numOfChars < 0 ) {
        ui.lblCounter->setStyleSheet( "QLabel {color: red}" );
    } else if ( numOfChars < 30 ) {
        ui.lblCounter->setStyleSheet( "QLabel {color: rgb(242, 179, 19);}" );
    } else {
        ui.lblCounter->setStyleSheet( "QLabel {color: green}" );
    }
    ui.lblCounter->setText( KGlobal::locale()->formatNumber( numOfChars, 0 ) );
}

void QuickTwit::slotPostNewStatusDone( bool isError )
{
    kDebug();
    emit sigStatusUpdated( isError );
    if ( isError ) {
        Backend * b = qobject_cast<Backend *>( sender() );
        QString name( APPNAME );
        emit sigNotify( i18n( "Failed!" ), i18n( "Posting new status failed. %1",
                                                 b->latestErrorString() ), name );
    } else {
        txtStatus->clearContentsAndSetDirection( accountsList[ui.comboAccounts->currentIndex()].direction() );
        QString name( APPNAME );
        emit sigNotify( i18n( "Success!" ), i18n( "New status posted successfully" ), name );
    }
    this->close();
    sender()->deleteLater();
}

void QuickTwit::slotPostNewStatus( QString & newStatus )
{
    kDebug();
    this->hide();
    if ( ui.checkAll->isChecked() ) {
        int count = accountsList.count();
        for ( int i = 0;i < count; ++i ) {
            Backend *twitter = new Backend( &accountsList[i] , this );
            connect( twitter, SIGNAL( sigPostNewStatusDone( bool ) ),
                     this, SLOT( slotPostNewStatusDone( bool ) ) );
            twitter->postNewStatus( newStatus );
        }
    } else {
        Backend *twitter = new Backend( &accountsList[ui.comboAccounts->currentIndex()] , this );
        connect( twitter, SIGNAL( sigPostNewStatusDone( bool ) ),
                 this, SLOT( slotPostNewStatusDone( bool ) ) );
        twitter->postNewStatus( newStatus );
    }
}

void QuickTwit::sltAccepted()
{
    kDebug();
    QString txt = txtStatus->toPlainText();
    slotPostNewStatus( txt );
}

void QuickTwit::loadAccounts()
{
    kDebug();
    QList<Account> ac = AccountManager::self()->accounts();
    QListIterator<Account> it( ac );
    while ( it.hasNext() ) {
        Account current = it.next();
        accountsList.append( current );
        ui.comboAccounts->addItem( current.alias() );
    }
}

void QuickTwit::addAccount( const Account & account )
{
    kDebug();
    accountsList.append( account );
    ui.comboAccounts->addItem( account.alias() );
}

void QuickTwit::removeAccount( const QString & alias )
{
    kDebug();
    int count = accountsList.count();
    for ( int i = 0; i < count; ++i ) {
        if ( accountsList[i].alias() == alias ) {
            accountsList.removeAt( i );
            ui.comboAccounts->removeItem( i );
            return;
        }
    }
}

void QuickTwit::checkAll( bool isAll )
{
    ui.comboAccounts->setEnabled( !isAll );
}

#include "quicktwit.moc"
