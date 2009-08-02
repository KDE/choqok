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
#include "timelinewidget.h"
#include "statustextedit.h"
#include "settings.h"
#include "statuswidget.h"
#include "constants.h"
#include "accountmanager.h"
#include <kdebug.h>
#include <QTimer>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <QProcess>
#include <KDE/KNotification>
#include <KFileDialog>
#include <KUrl>
#include <QKeyEvent>

TimeLineWidget::TimeLineWidget( const Account &userAccount, QWidget* parent ) :
        QWidget( parent )
{
    kDebug();
    setupUi( this );
    mCurrentAccount = userAccount;
    twitter = new Backend( &mCurrentAccount, this );
    latestHomeStatusId = latestReplyStatusId = 0;

    homeLayout->setDirection( QBoxLayout::TopToBottom );
    replyLayout->setDirection( QBoxLayout::TopToBottom );
    txtNewStatus = new StatusTextEdit( this );
    lblCounter = new QLabel( this );
    tabs->setCornerWidget( lblCounter, Qt::TopRightCorner );
    txtNewStatus->setObjectName( "txtNewStatus" );
    inputFrame->layout()->addWidget( txtNewStatus );

    btnAttachMedia->setIcon(KIcon("mail-attachment"));
    attachMediaFrame->hide();
    if(mCurrentAccount.serviceType() == Account::Twitter) {
        mediaToAttach.clear();
        btnClearMedia->setIcon(KIcon("edit-clear"));
        connect(btnAttachMedia, SIGNAL(clicked(bool)), SLOT(attachMedia()));
        connect(btnClearMedia, SIGNAL(clicked(bool)), SLOT(clearMedia()));
    } else {
        btnAttachMedia->setEnabled(false);
    }

//     connect ( toggleArrow, SIGNAL ( clicked() ), this, SLOT ( toggleTwitFieldVisible() ) );
    connect( txtNewStatus, SIGNAL( charsLeft( int ) ), this, SLOT( checkNewStatusCharactersCount( int ) ) );
    connect( txtNewStatus, SIGNAL( returnPressed( QString& ) ), this, SLOT( postStatus( QString& ) ) );
    connect( txtNewStatus, SIGNAL( cleared() ), this, SLOT( txtNewStatusCleared() ) );
    connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( aboutQuit() ) );
    QTimer::singleShot( 0, this, SLOT( initObjects() ) );
}

TimeLineWidget::~TimeLineWidget()
{
    kDebug();
}

void TimeLineWidget::aboutQuit()
{
    kDebug();
    AccountManager::self()->saveFriendsList( mCurrentAccount.alias(), friendsList );
    saveStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(), Backend::HomeTimeLine ),
                  listHomeStatus.values() );
    saveStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(), Backend::ReplyTimeLine ),
                  listReplyStatus.values() );
    saveStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(), Backend::InboxTimeLine ),
                  listInboxStatus.values() );
    saveStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(), Backend::OutboxTimeLine ),
                  listOutboxStatus.values() );
    deleteLater();
}

void TimeLineWidget::initObjects()
{
    kDebug();

    txtNewStatus->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
    txtNewStatus->setTabChangesFocus( true );
    btnReloadFriends->setIcon( KIcon( "view-refresh" ) );
    connect( btnReloadFriends, SIGNAL( clicked( bool ) ), this, SLOT( reloadFriendsList() ) );

    QFont counterF;
    counterF.setBold( true );
    counterF.setPointSize( 12 );
    lblCounter->setFont( counterF );
    checkNewStatusCharactersCount( 140 );

    connect( twitter, SIGNAL( homeTimeLineReceived( QList< Status >& ) ),
             this, SLOT( homeTimeLinesReceived( QList< Status >& ) ) );
    connect( twitter, SIGNAL( replyTimeLineReceived( QList< Status >& ) ),
             this, SLOT( replyTimeLineReceived( QList< Status >& ) ) );
    connect( twitter, SIGNAL( directMessagesReceived( QList< Status >& ) ),
             this, SLOT( directMessagesReceived( QList< Status >& ) ) );
    connect( twitter, SIGNAL( outboxMessagesReceived( QList< Status >& ) ),
             this, SLOT( outboxMessagesReceived( QList< Status >& ) ) );
    connect( twitter, SIGNAL( sigPostNewStatusDone( bool ) ),
             this, SLOT( postingNewStatusDone( bool ) ) );
    connect( twitter, SIGNAL( sigFavoritedDone( bool ) ),
             this, SLOT( requestFavoritedDone( bool ) ) );
    connect( twitter, SIGNAL( sigDestroyDone( bool ) ),
             this, SLOT( requestDestroyDone( bool ) ) );
    connect( twitter, SIGNAL( sigError( const QString& ) ),
             this, SLOT( error( const QString& ) ) );
    connect( twitter, SIGNAL( friendsListed( const QStringList& ) ),
             this, SLOT( friendsListed( const QStringList& ) ) );
    connect( twitter, SIGNAL( followersListed( const QStringList& ) ),
             this, SLOT( friendsListed( const QStringList& ) ) );

    replyToStatusId = unreadStatusCount = unreadStatusInReply = unreadStatusInHome =
                                              unreadStatusInInbox = unreadStatusInOutbox = latestInboxStatusId =
                                                                        latestOutboxStatusId = 0;

    setTabOrder( chkDMessage, comboFriendList );
    setTabOrder( comboFriendList, btnReloadFriends );
    setTabOrder( btnReloadFriends, txtNewStatus );
    setTabOrder( txtNewStatus, chkDMessage );

    loadConfigurations();
    txtNewStatus->setFocus( Qt::OtherFocusReason );
    settingsChanged();
}

void TimeLineWidget::checkNewStatusCharactersCount( int numOfChars )
{
    if ( numOfChars < 0 ) {
        lblCounter->setStyleSheet( "QLabel {color: red;}" );
    } else if ( numOfChars < 30 ) {
        lblCounter->setStyleSheet( "QLabel {color: rgb(242, 179, 19);}" );
    } else {
        lblCounter->setStyleSheet( "QLabel {color: green;}" );
    }

    if ( numOfChars == 140 ) {
        txtNewStatus->setMaximumHeight( 28 );
        lblCounter->setVisible(false);
    } else {
        txtNewStatus->setMaximumHeight( 80 );
        lblCounter->setVisible(true);
    }

    lblCounter->setText( KGlobal::locale()->formatNumber( numOfChars, 0 ) );
}

void TimeLineWidget::settingsChanged()
{
    kDebug();
    setDefaultDirection();
    twitter->settingsChanged();
    updateUi();
    foreach(StatusWidget * s,listHomeStatus)
      s->setUiStyle();
    foreach(StatusWidget * s,listInboxStatus)
      s->setUiStyle();
    foreach(StatusWidget * s,listOutboxStatus)
      s->setUiStyle();
    foreach(StatusWidget * s,listReplyStatus)
      s->setUiStyle();
}

void TimeLineWidget::updateTimeLines()
{
    kDebug();
    twitter->requestTimeLine( latestHomeStatusId, Backend::HomeTimeLine );
    twitter->requestTimeLine( latestReplyStatusId, Backend::ReplyTimeLine );
    twitter->requestDMessages( latestInboxStatusId, Backend::Inbox );
    twitter->requestDMessages( latestOutboxStatusId, Backend::Outbox );

    if ( latestHomeStatusId == 0 || latestReplyStatusId == 0 )
        isStartMode = true;
    else
        isStartMode = false;

    emit notify( i18n( "Loading timelines..." ), true );
}

void TimeLineWidget::directMessagesReceived( QList< Status > & msgList )
{
    kDebug();
    emit notify( i18n( "Latest direct messages received." ) );
    int count = msgList.count();

    if ( count == 0 ) {
        kDebug() << "Message list is empty";
        emit notify( i18n( "No new messages received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi( msgList, inboxLayout, &listInboxStatus, Backend::InboxTimeLine );
//         inboxScroll->verticalScrollBar()->setSliderPosition( 0 );
        kDebug() << count << " Statuses received.";
        if ( !isStartMode ) {
            unreadStatusInInbox += count;
            if ( unreadStatusInInbox > 0 )
                tabs->setTabText( 2, i18n( "Inbox(%1)", unreadStatusInInbox ) );
        }
    }
}

void TimeLineWidget::outboxMessagesReceived( QList< Status > & msgList )
{
    kDebug();
    emit notify( i18n( "Latest sent messages received." ) );
    int count = msgList.count();

    if ( count == 0 ) {
        kDebug() << "Message list is empty";
        emit notify( i18n( "No new messages received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi( msgList, outboxLayout, &listOutboxStatus, Backend::OutboxTimeLine );
//         outboxScroll->verticalScrollBar()->setSliderPosition( 0 );

        kDebug() << count << " Statuses received.";

//         if ( !isStartMode ) {
//             unreadStatusInOutbox += count;
//             if( unreadStatusInOutbox > 0 )
//                 tabs->setTabText ( 3, i18n ( "Outbox(%1)", unreadStatusInOutbox ) );
//         }
    }
}

void TimeLineWidget::homeTimeLinesReceived( QList< Status > & statusList )
{
    kDebug();
    emit notify( i18n( "Latest friend timelines received." ) );
    int count = statusList.count();

    if ( count == 0 ) {
        kDebug() << "Status list is empty";
        emit notify( i18n( "No new statuses received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi( statusList, homeLayout, &listHomeStatus );
//         homeScroll->verticalScrollBar()->setSliderPosition( 0 );

//         int count = statusList.count();
        kDebug() << count << " Statuses received.";

        if ( !isStartMode ) {
            unreadStatusInHome += count;
            if ( unreadStatusInHome > 0 )
                tabs->setTabText( 0, i18n( "Home(%1)", unreadStatusInHome ) );
        }
    }
}

void TimeLineWidget::replyTimeLineReceived( QList< Status > & statusList )
{
    kDebug();
    emit notify( i18n( "Latest reply timelines received." ) );
    int count = statusList.count();
    if ( count == 0 ) {
        kDebug() << "Status list is empty";
        emit notify( i18n( "No new statuses received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi( statusList, replyLayout, &listReplyStatus, Backend::ReplyTimeLine );
//         replyScroll->verticalScrollBar()->setSliderPosition( 0 );

//         int count = statusList.count();
        kDebug() << count << " Statuses received.";

        if ( !isStartMode ) {
            unreadStatusInReply += count;
            if ( unreadStatusInReply > 0 )
                tabs->setTabText( 1, i18n( "Reply(%1)", unreadStatusInReply ) );
        }
    }
}

void TimeLineWidget::addNewStatusesToUi( QList< Status > & statusList, QBoxLayout * layoutToAddStatuses,
        QMap<qulonglong, StatusWidget*> *list, Backend::TimeLineType type )
{
    kDebug();
    bool allInOne = Settings::showAllNotifiesInOne();
    QString notifyStr;
    int numOfNewStatuses = statusList.count();
    QList<Status>::const_iterator it = statusList.constBegin();
    QList<Status>::const_iterator endIt = statusList.constEnd();
    bool isThereAnyNewStatusToNotify = false;
    if ( allInOne && Settings::notifyType() != SettingsBase::LibNotify )
        notifyStr = ( mCurrentAccount.direction() == Qt::RightToLeft ) ? "<div dir='rtl'>" : "<div dir='ltr'>";
    for ( ;it != endIt; ++it ) {
        if ( Settings::doNotShowRepliesOnHome() ) {
            if ( it->replyToUserId == mCurrentAccount.userId() && type == Backend::HomeTimeLine ) {
                --numOfNewStatuses;
                --unreadStatusInHome;
                continue;
            }
        }
        if(list->keys().contains(it->statusId)) 
            continue;
	

        StatusWidget *wt = new StatusWidget( &mCurrentAccount, this );
        wt->setAttribute( Qt::WA_DeleteOnClose );
        wt->setCurrentStatus( *it );
        connect( wt, SIGNAL( sigReply( const QString&, qulonglong, bool ) ),
                 this, SLOT( prepareReply( const QString&, qulonglong, bool ) ) );
        connect( wt, SIGNAL( sigFavorite( qulonglong, bool ) ),
                 twitter, SLOT( requestFavorited( qulonglong, bool ) ) );
        connect( wt, SIGNAL( sigDestroy( qulonglong ) ),
                 this, SLOT( requestDestroy( qulonglong ) ) );
        connect(wt,SIGNAL(sigSearch(int,QString)),this,SIGNAL(sigSearch(int,QString)));
        connect(wt, SIGNAL(sigReTweet(const QString&)), this, SLOT(reTweet(const QString&)));

        list->insert( it->statusId, wt );
        layoutToAddStatuses->insertWidget( 0, wt );

        if ( !isStartMode ) {
            if ( it->user.userId == mCurrentAccount.userId() ) {
                --numOfNewStatuses;
                wt->setUnread( StatusWidget::WithoutNotify );
            } else {
                if ( type == Backend::OutboxTimeLine ) {
                    wt->setUnread( StatusWidget::WithoutNotify );
                } else if ( allInOne ) {
                    notifyStr += "<b>" + it->user.screenName + " : </b>" + it->content + "<br/>";
                    isThereAnyNewStatusToNotify = true;
                    wt->setUnread( StatusWidget::WithoutNotify );
                } else {
                    wt->setUnread( StatusWidget::WithNotify );
                }
            }
            listUnreadStatuses.append( wt );
        }
    }
    if ( allInOne && Settings::notifyType() != SettingsBase::LibNotify )
        notifyStr += "</div>";
    qulonglong latestId = statusList.last().statusId;
    if ( type == Backend::HomeTimeLine && latestId > latestHomeStatusId ) {
        kDebug() << "Latest home statusId sets to: " << latestId;
        latestHomeStatusId = latestId;
    } else if ( type == Backend::ReplyTimeLine && latestId > latestReplyStatusId ) {
        kDebug() << "Latest reply statusId sets to: " << latestId;
        latestReplyStatusId = latestId;
    } else if ( type == Backend::InboxTimeLine && latestId > latestInboxStatusId ) {
        kDebug() << "Latest inbox statusId sets to: " << latestId;
        latestInboxStatusId = latestId;
    } else if ( type == Backend::OutboxTimeLine && latestId > latestOutboxStatusId ) {
        kDebug() << "Latest outbox statusId sets to: " << latestId;
        latestOutboxStatusId = latestId;
    }
    if ( !isStartMode && type != Backend::OutboxTimeLine )
        checkUnreadStatuses( numOfNewStatuses );

    if ( isThereAnyNewStatusToNotify ) {
        showNotify( i18n( "New statuses" ), notifyStr );
    }

    updateStatusList( list );
}

void TimeLineWidget::abortPostNewStatus()
{
    kDebug();
    twitter->abortPostNewStatus();
}

void TimeLineWidget::setDefaultDirection()
{
//     tabs->widget( 0 )->setLayoutDirection( mCurrentAccount.direction() );
//     tabs->widget( 1 )->setLayoutDirection( mCurrentAccount.direction() );
//     tabs->widget( 2 )->setLayoutDirection( mCurrentAccount.direction() );
//     tabs->widget( 3 )->setLayoutDirection( mCurrentAccount.direction() );
//     tabs->widget ( 4 )->setLayoutDirection ( mCurrentAccount.direction() );

    txtNewStatus->setDefaultDirection( mCurrentAccount.direction() );
}

void TimeLineWidget::error( const QString & errMsg )
{
    emit notify( i18n( "Failed: %1", errMsg ), true );
}

void TimeLineWidget::postStatus( QString & status )
{
    kDebug();

    if ( status.size() > MAX_STATUS_SIZE && status.indexOf( QRegExp( "https?://" ) ) == -1 ) {
        QString err = i18n( "Message text size is greater than the server limit: \
the server may truncate or drop it.\nAre you sure you want to post this message?" );
        if(KMessageBox::warningContinueCancel( this, err ) == KMessageBox::Cancel)
            return;
    }
    txtNewStatus->setEnabled( false );
    if( mediaToAttach.isEmpty() ) {
        if ( chkDMessage->isChecked() ) {
            emit notify( i18n( "Sending direct message...." ), true );
            twitter->sendDMessage( comboFriendList->currentText(), status );
            chkDMessage->setChecked( false );
        } else {
            emit notify( i18n( "Posting new status...." ), true );
            twitter->postNewStatus( status, replyToStatusId );
        }
    } else {
        emit notify( i18n( "Posting new status...." ), true );
        twitter->twitPicCreatePost( mediaToAttach, status );
        mediaToAttach.clear();
    }
}

void TimeLineWidget::postingNewStatusDone( bool isError )
{
    kDebug();
//     emit sigStatusUpdated( isError );
    if ( !isError ) {
        clearMedia();
        txtNewStatus->clearContentsAndSetDirection( mCurrentAccount.direction() );
        QString successMsg = i18n( "New status posted successfully" );
        emit systemNotify( i18n( "Success." ), successMsg, APPNAME );
        notify( successMsg );
        replyToStatusId = 0;
        lblCounter->setPixmap(KIcon("dialog-ok").pixmap(22));
        twitter->requestTimeLine(latestHomeStatusId, Backend::HomeTimeLine);
    } else {
        lblCounter->setPixmap(KIcon("dialog-cancel").pixmap(22));
        error( twitter->latestErrorString() );
    }
    QTimer::singleShot( 3000, this, SLOT( revertCounterLabelShape() ) );
    lblCounter->show();
    lblCounter->setText(QString());
    txtNewStatus->setFocus( Qt::OtherFocusReason );
    txtNewStatus->setEnabled( true );

}

bool TimeLineWidget::saveStatuses( QString fileName, QList<StatusWidget*> list )
{
    kDebug();
    KConfig statusesBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );

    ///Clear previous data:
    QStringList prevList = statusesBackup.groupList();
    int c = prevList.count();

    if ( c > 0 ) {
        for ( int i = 0; i < c; ++i ) {
            statusesBackup.deleteGroup( prevList[i] );
        }
    }

    int count = list.count();

    for ( int i = 0; i < count; ++i ) {
//   QString str = ;
        KConfigGroup grp( &statusesBackup, QString::number( list[i]->currentStatus().statusId ) );
        grp.writeEntry( "created_at", list[i]->currentStatus().creationDateTime );
        grp.writeEntry( "id", list[i]->currentStatus().statusId );
        grp.writeEntry( "text", list[i]->currentStatus().content );
        grp.writeEntry( "source", list[i]->currentStatus().source );
        grp.writeEntry( "truncated", list[i]->currentStatus().isTruncated );
        grp.writeEntry( "in_reply_to_status_id", list[i]->currentStatus().replyToStatusId );
        grp.writeEntry( "in_reply_to_user_id", list[i]->currentStatus().replyToUserId );
        grp.writeEntry( "favorited", list[i]->currentStatus().isFavorited );
        grp.writeEntry( "in_reply_to_screen_name", list[i]->currentStatus().replyToUserScreenName );
        grp.writeEntry( "userId", list[i]->currentStatus().user.userId );
        grp.writeEntry( "screen_name", list[i]->currentStatus().user.screenName );
        grp.writeEntry( "name", list[i]->currentStatus().user.name );
        grp.writeEntry( "profile_image_url", list[i]->currentStatus().user.profileImageUrl );
        grp.writeEntry( "description" , list[i]->currentStatus().user.description );
        grp.writeEntry( "isDMessage" , list[i]->currentStatus().isDMessage );
        grp.writeEntry( "location" , list[i]->currentStatus().user.location );
        grp.writeEntry( "url" , list[i]->currentStatus().user.homePageUrl );
        grp.writeEntry( "followers_count" , list[i]->currentStatus().user.followersCount );
        grp.writeEntry( "friends_count" , list[i]->currentStatus().user.friendsCount );
    }

    statusesBackup.sync();

    return true;
}

QList< Status > TimeLineWidget::loadStatuses( QString fileName )
{
    kDebug();
    KConfig statusesBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
//  KConfigGroup entries(&statusesBackup, "Statuses");
    QList< Status > list;
    QStringList groupList = statusesBackup.groupList();
//  kDebug()<<groupList;
    int count = groupList.count();

    for ( int i = 0; i < count; ++i ) {
        KConfigGroup grp( &statusesBackup, groupList[i] );
        Status st;
        st.creationDateTime = grp.readEntry( "created_at", QDateTime::currentDateTime() );
        st.statusId = grp.readEntry( "id", ( qulonglong ) 0 );
        st.content = grp.readEntry( "text", QString() );
        st.source = grp.readEntry( "source", QString() );
        st.isTruncated = grp.readEntry( "truncated", false );
        st.replyToStatusId = grp.readEntry( "in_reply_to_status_id", ( qulonglong ) 0 );
        st.replyToUserId = grp.readEntry( "in_reply_to_user_id", ( qulonglong ) 0 );
        st.isFavorited = grp.readEntry( "favorited", false );
        st.replyToUserScreenName = grp.readEntry( "in_reply_to_screen_name", QString() );
        st.user.userId = grp.readEntry( "userId", ( qulonglong ) 0 );
        st.user.screenName = grp.readEntry( "screen_name", QString() );
        st.user.name = grp.readEntry( "name", QString() );
        st.user.profileImageUrl = grp.readEntry( "profile_image_url", QString() );
        st.user.description = grp.readEntry( "description" , QString() );
        st.isDMessage = grp.readEntry( "isDMessage" , false );
        st.user.location = grp.readEntry("location", QString());
        st.user.homePageUrl = grp.readEntry("url", QString());
        st.user.followersCount = grp.readEntry("followers_count", 0);
        st.user.friendsCount = grp.readEntry("friends_count", 0);

        //Sorting The new statuses:
        int j = 0;
        int count = list.count();
        while (( j < count ) && ( st.statusId > list[ j ].statusId ) ) {
            ++j;
        }
        list.insert( j, st );
    }

    return list;
}

void TimeLineWidget::prepareReply( const QString &userName, qulonglong statusId, bool dMsg )
{
    kDebug();
    emit showMe();
    if ( dMsg ) {
        chkDMessage->setChecked( true );
        comboFriendList->setCurrentItem( userName, true, 0 );
    } else {
        QString current = txtNewStatus->toPlainText();
        txtNewStatus->setText( '@' + userName + ' ' + current );
        replyToStatusId = statusId;
        txtNewStatus->setDefaultDirection( mCurrentAccount.direction() );
    }
    txtNewStatus->moveCursor( QTextCursor::End );
    txtNewStatus->setFocus( Qt::OtherFocusReason );
}

void TimeLineWidget::updateStatusList( QMap<qulonglong, StatusWidget*> *list )
{
    kDebug();
    int toBeDelete = list->count() - Settings::countOfStatusesOnMain();

    if ( toBeDelete > 0 ) {
        QMap<qulonglong, StatusWidget*>::const_iterator it = list->constBegin();
        QMap<qulonglong, StatusWidget*>::const_iterator endIt = list->constEnd();
        for ( ; it != endIt && toBeDelete > 0; ++it ) {
            StatusWidget* wt = it.value();
            if( !wt )
                continue;
            if ( !wt->isRead() )
                break;
            wt->close();
            list->remove( it.key() );
            --toBeDelete;
        }
    }
}

void TimeLineWidget::clearTimeLineList( QMap<qulonglong, StatusWidget * > * list )
{
    kDebug();
    qDeleteAll(*list);
}

void TimeLineWidget::loadConfigurations()
{
    kDebug();
    setDefaultDirection();
    friendsList = AccountManager::self()->listFriends( mCurrentAccount.alias() );
    friendsList.sort();
//     txtNewStatus->setFriendsList( friendsList );
    comboFriendList->addItems( friendsList );
    KCompletion *c = comboFriendList->completionObject( true );;
    c->setItems( friendsList );
    c->setCompletionMode( KGlobalSettings::CompletionPopupAuto );

    isStartMode = true;

    QList< Status > lstHome =
        loadStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(),
                                                                    Backend::HomeTimeLine ) );
    if ( lstHome.count() > 0 )
        homeTimeLinesReceived( lstHome );

    QList< Status > lstReply =
        loadStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(),
                                                                    Backend::ReplyTimeLine ) );
    if ( lstReply.count() > 0 )
        replyTimeLineReceived( lstReply );

    QList< Status > lstInbox =
        loadStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(),
                                                                    Backend::InboxTimeLine ) );
    if ( lstInbox.count() > 0 )
        directMessagesReceived( lstInbox );

    QList< Status > lstOutbox =
        loadStatuses( AccountManager::generateStatusBackupFileName( mCurrentAccount.alias(),
                                                                    Backend::OutboxTimeLine ) );
    if ( lstOutbox.count() > 0 )
        outboxMessagesReceived( lstOutbox );

}

void TimeLineWidget::checkUnreadStatuses( int numOfNewStatusesReciened )
{
    kDebug();
//  if(this->isVisible()){
//   unreadStatusCount = 0;
//  } else {
    unreadStatusCount += numOfNewStatusesReciened;
//  }
    emit sigSetUnread( numOfNewStatusesReciened );
    emit sigSetUnreadOnMainWin( unreadStatusCount );
}

void TimeLineWidget::setUnreadStatusesToReadState()
{
    kDebug();
    tabs->setTabText( 0, i18nc( "@title:tab", "Home" ) );
    tabs->setTabText( 1, i18nc( "@title:tab", "Reply" ) );
    tabs->setTabText( 2, i18nc( "@title:tab", "Inbox" ) );
    tabs->setTabText( 3, i18nc( "@title:tab", "Outbox" ) );
    int count = listUnreadStatuses.count();

    for ( int i = 0;i < count; ++i ) {
        listUnreadStatuses[i]->setRead();
    }

    listUnreadStatuses.clear();
//     if(unreadStatusCount) {
//       QString s = qApp->styleSheet();
//       qApp->setStyleSheet(s);
//     }
//     qApp->setStyleSheet( StatusWidget::getColoredStyle() );

    emit sigSetUnread( -unreadStatusCount );
    emit sigSetUnreadOnMainWin( 0 );
    unreadStatusCount = unreadStatusInReply = unreadStatusInHome = 0;
}

void TimeLineWidget::requestFavoritedDone( bool isError )
{
    kDebug() << "is Error: " << isError;
    notify( "Done!" );
}

void TimeLineWidget::requestDestroyDone( bool isError )
{
    kDebug() << "is Error: " << isError;
    notify( "Done!" );
    toBeDestroied->close();
}

void TimeLineWidget::requestDestroy( qulonglong statusId )
{
    if ( KMessageBox::warningYesNo( this, i18n( "Are you sure you wish to destroy this status?" ) ) == KMessageBox::Yes ) {
        toBeDestroied = qobject_cast<StatusWidget*> ( sender() );
        if ( toBeDestroied->currentStatus().isDMessage ) {
            twitter->requestDestroyDMessage( statusId );
        } else {
            twitter->requestDestroy( statusId );
        }
        setUnreadStatusesToReadState();
    }
}

void TimeLineWidget::disableApp()
{
    kDebug();
    txtNewStatus->setEnabled( false );
}

void TimeLineWidget::enableApp()
{
    kDebug();
    txtNewStatus->setEnabled( true );
}

Account TimeLineWidget::currentAccount() const
{
    return mCurrentAccount;
}

void TimeLineWidget::setCurrentAccount( const Account & account )
{
    mCurrentAccount = account;
}

void TimeLineWidget::reloadFriendsList()
{
    kDebug();
    friendsList.clear();
    comboFriendList->setCurrentItem( i18n("Please wait...."), true, 0);
//     twitter->listFollowersScreenName();
    twitter->listFriendsScreenName();
}

void TimeLineWidget::friendsListed( const QStringList & list )
{
    friendsList<<list;
    friendsList.removeDuplicates();
    friendsList.sort();

    comboFriendList->clear();
    comboFriendList->addItems( friendsList );
    KCompletion *c = comboFriendList->completionObject( true );
    c->setItems( friendsList );
    c->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
}

void TimeLineWidget::showNotify( const QString &title, const QString &message )
{
    if ( Settings::notifyType() == SettingsBase::KNotify ) {//KNotify
        KNotification *notif = new KNotification( "new-status-arrived", parentWidget() );
        notif->setText( message );
        notif->setFlags( KNotification::RaiseWidgetOnActivation | KNotification::Persistent );
        notif->sendEvent();
        QTimer::singleShot( Settings::notifyInterval()*1000, notif, SLOT( close() ) );
    } else if ( Settings::notifyType() == SettingsBase::LibNotify ) {//Libnotify!
        QString msg = message;
        msg = msg.replace( "<br/>", "\n" );
        QString libnotifyCmd = QString( "notify-send -t " ) + QString::number( Settings::notifyInterval() * 1000 )
                               + QString( " -u low \"" ) + title + QString( "\" \"" ) + msg + QString( "\"" );
        QProcess::execute( libnotifyCmd );
    }
}

void TimeLineWidget::txtNewStatusCleared()
{
    replyToStatusId = 0;
}

void TimeLineWidget::updateUi()
{
    if(Settings::showIconsOnTimelineTabs()) {
        tabs->setTabIcon(0, KIcon( "user-home" ));
        tabs->setTabIcon(1, KIcon( "edit-undo" ));
        tabs->setTabIcon(2, KIcon( "mail-folder-inbox" ));
        tabs->setTabIcon(3, KIcon( "mail-folder-outbox" ));
    } else {
        tabs->setTabIcon(0, KIcon());
        tabs->setTabIcon(1, KIcon());
        tabs->setTabIcon(2, KIcon());
        tabs->setTabIcon(3, KIcon());
    }
}

void TimeLineWidget::revertCounterLabelShape()
{
    lblCounter->setPixmap( QPixmap() );
    checkNewStatusCharactersCount( txtNewStatus->countOfRemainsChar() );
}

Backend* TimeLineWidget::getBackend()
{
    return twitter;
}

void TimeLineWidget::reTweet( const QString &text )
{
    replyToStatusId = 0;
    txtNewStatus->setText( text );
    txtNewStatus->setFocus(Qt::OtherFocusReason);
}

void TimeLineWidget::attachMedia()
{
    kDebug();
    QString mediaPath = KFileDialog::getOpenFileName( KUrl("kfiledialog:///image?global"),
                                                      "image/png image/jpeg image/gif", this,
                                                      i18n("Select media to attach"));
    if( !mediaPath.isEmpty() ) {
        mediaToAttach.setPath(mediaPath);
        lblMediaName->setText( mediaToAttach.fileName() );
        lblMediaName->setToolTip(mediaPath);
        attachMediaFrame->show();
    }
    txtNewStatus->setFocus(Qt::OtherFocusReason);
}

void TimeLineWidget::clearMedia()
{
    lblMediaName->clear();
    mediaToAttach.clear();
    attachMediaFrame->hide();
}

void TimeLineWidget::keyPressEvent( QKeyEvent *event )
{
    if(event->key() == Qt::Key_Escape){
        twitter->abortPostNewStatus();
        txtNewStatus->setEnabled(true);
        txtNewStatus->setFocus();
    }
}

#include "timelinewidget.moc"
