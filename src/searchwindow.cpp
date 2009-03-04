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

#include "searchwindow.h"
#include "statuswidget.h"
#include "settings.h"
#include <kdebug.h>
#include <QTimer>
#include <QMap>
#include <QString>
#include <QScrollBar>
#include <KDE/KLocale>
#include <KMessageBox>

#include "twittersearch.h"
#include "identicasearch.h"

SearchWindow::SearchWindow( const Account &account, QWidget* parent ) :
        QWidget( parent )
{
    kDebug();
    mAccount = account;

    setAttribute( Qt::WA_DeleteOnClose );

    switch( mAccount.serviceType() )
    {
        case Account::Twitter:
            mSearch = new TwitterSearch;
            break;
        case Account::Identica:
            mSearch = new IdenticaSearch;
            break;
        default:
            mSearch = 0;
            break;
    }

    ui.setupUi( this );
    resize( Settings::searchWindowSize() );
    move( Settings::searchWindowPosition() );

    setWindowTitle( i18nc( "Search in service", "%1 Search",
                           mAccount.serviceName() ) );
}

void SearchWindow::init()
{
    kDebug();

    if( mSearch )
    {
        connect( mSearch, SIGNAL( searchResultsReceived( QList< Status>& ) ),
                this, SLOT( searchResultsReceived ( QList< Status >& ) ) );
        connect( mSearch, SIGNAL( error( QString ) ), this, SLOT( error( QString ) ) );
        connect( ui.txtSearch, SIGNAL( returnPressed() ), this, SLOT( search() ) );

        show();
        resetSearchArea();
    }
    else
    {
        kDebug() << "Service has no search implementation";
        KMessageBox::error( this, i18n( "This service has no search feature." ) );
        close();
    }
}

SearchWindow::~SearchWindow()
{
    kDebug();
    Settings::setSearchWindowPosition(pos());
    Settings::setSearchWindowSize(size());

    if( mSearch )
        mSearch->deleteLater();
}

void SearchWindow::error( QString message )
{
    ui.lblStatus->setText( i18n( "Failed, %1", message ) );
    lastSearchQuery.clear();
}

void SearchWindow::searchResultsReceived(QList<Status> &statusList )
{
    kDebug();

    int count = statusList.count();
    if ( count == 0 ) {
        kDebug() << "Status list is empty";
        ui.lblStatus->setText( i18n( "No search results." ) );
    } else {
        ui.lblStatus->setText( i18n( "Search Results Received!" ) );
        addNewStatusesToUi( statusList );
        ui.searchScroll->verticalScrollBar()->setSliderPosition( 0 );
    }
    ui.txtSearch->setEnabled( true );
}

void SearchWindow::search()
{
    kDebug();

    if ( ui.txtSearch->text().size() > 140 )
    {
        ui.lblStatus->setText( i18n( "Search text size is more than 140 characters." ) );
        return;
    }

    ui.txtSearch->setEnabled( false );
    clearSearchResults();
    ui.lblStatus->setText( i18n( "Searching..." ) );
    mSearch->requestSearchResults( ui.txtSearch->text(),
                                   ui.comboSearchType->currentIndex(), 0 );

    lastSearchQuery = ui.txtSearch->text();
    lastSearchType = ui.comboSearchType->currentIndex();

    setWindowTitle( i18nc( "Search in service", "%1 Search (%2)",
                           mAccount.serviceName(), lastSearchQuery ) );
}

void SearchWindow::updateSearchResults()
{
    kDebug();
    if( isVisible() && !lastSearchQuery.isNull() )
    {
        uint sinceStatusId = 0;
        if( listResults.count() )
            sinceStatusId = listResults.last()->currentStatus().statusId;

        ui.lblStatus->setText( i18n( "Searching..." ) );
        mSearch->requestSearchResults( lastSearchQuery,
                                       lastSearchType,
                                       sinceStatusId );
    }
}

void SearchWindow::autoUpdateSearchResults()
{
    kDebug();
    if( ui.chkAutoUpdate->isChecked() )
        updateSearchResults();
}

void SearchWindow::addNewStatusesToUi( QList<Status> &statusList )
{
    kDebug();

    // This will make all statuses prior to the update marked as read
    // and deleted if there are more than Settings::countOfStatusesOnMain.
    // The reasoning for this is that there's a distinct possibility of
    // a searching racking up thousands of unread messages depending on
    // the query which would go undeleted as unread messages. The other
    // option to avoid this would be to enforce a strict message limit
    // regardless of whether or not they were marked as read.
    markStatusesAsRead();

    QList<Status>::const_iterator it = statusList.constBegin();
    QList<Status>::const_iterator endIt = statusList.constEnd();

    for( ; it != endIt; ++it ) {
        StatusWidget *wt = new StatusWidget( &mAccount, this );

        connect( wt, SIGNAL( sigReply( const QString&, uint, bool ) ),
                 this, SIGNAL( forwardReply( const QString&, uint, bool ) ) );

        connect( wt, SIGNAL( sigFavorite( uint, bool ) ),
                 this, SIGNAL( forwardFavorited( uint, bool ) ) );

        wt->setAttribute( Qt::WA_DeleteOnClose );
        wt->setCurrentStatus( *it );
        wt->setUnread( StatusWidget::WithoutNotify );

        listResults.append( wt );
        ui.searchLayout->insertWidget( 0, wt );
    }
    updateStatusList();
}

void SearchWindow::updateStatusList()
{
    kDebug();
    int toBeDelete = listResults.count() - Settings::countOfStatusesOnMain();

    if ( toBeDelete > 0 ) {
        for ( int i = 0; i < toBeDelete; ++i ) {
            StatusWidget* wt = listResults.at( i );

            if ( !wt->isRead() )
                break;

            listResults.removeAt( i );

            --i;

            --toBeDelete;

            wt->close();
        }
    }
}

void SearchWindow::clearSearchResults()
{
    kDebug();
    int count = listResults.count();

    for ( int i = 0; i < count; ++i ) {
        StatusWidget* wt = listResults.first();
        listResults.removeFirst();
        wt->close();
    }
}

void SearchWindow::markStatusesAsRead()
{
    kDebug();
    int count = listResults.count();
    for ( int i = 0;i < count; ++i ) {
        listResults[i]->setRead();
    }
//     qApp->setStyleSheet( qApp->styleSheet() );
}

void SearchWindow::setAccount( const Account &account )
{
    disconnect( mSearch, SIGNAL( searchResultsReceived( QList< Status>& ) ),
                this, SLOT( searchResultsReceived ( QList< Status >& ) ) );
    disconnect( mSearch, SIGNAL( error( QString ) ), this, SLOT( error( QString ) ) );

    mAccount = account;
    resetSearchArea();

    connect( mSearch, SIGNAL( searchResultsReceived( QList< Status>& ) ),
             this, SLOT( searchResultsReceived ( QList< Status >& ) ) );
    connect( mSearch, SIGNAL( error( QString ) ), this, SLOT( error( QString ) ) );
}

void SearchWindow::resetSearchArea()
{
    kDebug();
    ui.txtSearch->setText( QString() );
    ui.txtSearch->setEnabled( true );
    ui.comboSearchType->clear();
    ui.chkAutoUpdate->setChecked( false );
    ui.lblStatus->setText( i18n( "No Search Results" ) );

    QMap<int, QString> searchTypes = mSearch->getSearchTypes();
    for( int i = 0; i < searchTypes.count(); ++i )
        ui.comboSearchType->insertItem( i, searchTypes[i] );
}

void SearchWindow::keyPressEvent( QKeyEvent* e )
{
    if ( e->key() == Qt::Key_F5 ) {
//         emit updateTimeLines();
        updateSearchResults();
        e->accept();
    } else if ( e->modifiers() == Qt::CTRL && e->key() == Qt::Key_R ) {
        markStatusesAsRead();
    } else {
        QWidget::keyPressEvent( e );
    }
}
