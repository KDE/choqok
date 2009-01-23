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
#include "timelinewidget.h"
#include "statustextedit.h"
#include "settings.h"
#include "statuswidget.h"
#include "constants.h"
#include "accountmanager.h"
#include <kdebug.h>
#include <QTimer>
#include <QScrollBar>
#include <KDE/KLocale>
#include <KMessageBox>
#include <QProcess>

TimeLineWidget::TimeLineWidget ( const Account &userAccount, QWidget* parent ) :
        QWidget ( parent )
{
    kDebug();
    setupUi ( this );
    mCurrentAccount = userAccount;
    latestHomeStatusId = latestReplyStatusId = 0;

    homeLayout->setDirection ( QBoxLayout::TopToBottom );
    replyLayout->setDirection ( QBoxLayout::TopToBottom );
    txtNewStatus = new StatusTextEdit ( this );
    lblCounter = new QLabel(this);
    tabs->setCornerWidget(lblCounter, Qt::TopRightCorner);
    txtNewStatus->setObjectName ( "txtNewStatus" );
    inputFrame->layout()->addWidget ( txtNewStatus );

//     connect ( toggleArrow, SIGNAL ( clicked() ), this, SLOT ( toggleTwitFieldVisible() ) );
    connect ( txtNewStatus, SIGNAL ( charsLeft ( int ) ), this, SLOT ( checkNewStatusCharactersCount ( int ) ) );
    connect ( txtNewStatus, SIGNAL ( returnPressed ( QString& ) ), this, SLOT ( postStatus ( QString& ) ) );

    QTimer::singleShot ( 0, this, SLOT ( initObjects() ) );
}

TimeLineWidget::~TimeLineWidget()
{
    kDebug();
    AccountManager::self()->saveFriendsList( mCurrentAccount.alias(), txtNewStatus->friendsList() );
    saveStatuses( generateStatusBackupFileName(Backend::HomeTimeLine), listHomeStatus );
    saveStatuses( generateStatusBackupFileName(Backend::ReplyTimeLine), listReplyStatus );
    saveStatuses( generateStatusBackupFileName(Backend::InboxTimeLine), listInboxStatus );
    saveStatuses( generateStatusBackupFileName(Backend::SentTimeLine), listSentStatus );
}

void TimeLineWidget::initObjects()
{
    kDebug();

    txtNewStatus->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Maximum );
    txtNewStatus->setFocus ( Qt::OtherFocusReason );
    txtNewStatus->setTabChangesFocus ( true );
    btnReloadFriends->setIcon( KIcon("view-refresh") );
    connect( btnReloadFriends, SIGNAL(clicked( bool )), this, SLOT(reloadFriendsList()) );

    QFont counterF;
    counterF.setBold( true );
    counterF.setPointSize( 12 );
    lblCounter->setFont( counterF );
    checkNewStatusCharactersCount(140);

    twitter = new Backend ( &mCurrentAccount, this );

    connect ( twitter, SIGNAL ( homeTimeLineRecived ( QList< Status >& ) ),
              this, SLOT ( homeTimeLinesRecived ( QList< Status >& ) ) );
    connect ( twitter, SIGNAL ( replyTimeLineRecived ( QList< Status >& ) ),
              this, SLOT ( replyTimeLineRecived ( QList< Status >& ) ) );
    connect( twitter, SIGNAL(directMessagesRecieved( QList< Status >&)), 
             this, SLOT(directMessagesRecieved(QList< Status >&)) );
    connect( twitter, SIGNAL(sentMessagesRecieved(QList< Status >&)), 
             this, SLOT(sentMessagesRecieved(QList< Status >&)) );
    connect ( twitter, SIGNAL ( sigPostNewStatusDone ( bool ) ), this, SLOT ( postingNewStatusDone ( bool ) ) );
    connect ( twitter, SIGNAL ( sigFavoritedDone ( bool ) ), this, SLOT ( requestFavoritedDone ( bool ) ) );
    connect ( twitter, SIGNAL ( sigDestroyDone ( bool ) ), this, SLOT ( requestDestroyDone ( bool ) ) );
    connect ( twitter, SIGNAL ( sigError ( const QString& ) ), this, SLOT ( error ( const QString& ) ) );
    connect ( twitter, SIGNAL (friendsListed(const QStringList&)), this, SLOT(friendsListed(const QStringList&)));
    connect ( twitter, SIGNAL (followersListed(const QStringList&)), this, SLOT(friendsListed(const QStringList&)));

    replyToStatusId = unreadStatusCount = unreadStatusInReply = unreadStatusInHome = latestRecievedStatusId = latestSentStatusId = 0;

    loadConfigurations();
}

void TimeLineWidget::checkNewStatusCharactersCount ( int numOfChars )
{
    if ( numOfChars < 0 ) {
        lblCounter->setStyleSheet ( "QLabel {color: red;}" );
    } else if ( numOfChars < 30 ) {
        lblCounter->setStyleSheet ( "QLabel {color: rgb(242, 179, 19);}" );
    } else {
        lblCounter->setStyleSheet ( "QLabel {color: green;}" );
    }
//     kDebug()<<numOfChars;
    if(numOfChars == 140 ){
        txtNewStatus->setMaximumHeight( 28 );
    } else {
        txtNewStatus->setMaximumHeight( 80 );
    }

    lblCounter->setText ( i18n ( "%1", numOfChars ) );
}

void TimeLineWidget::settingsChanged()
{
    kDebug();
    setDefaultDirection();

}

void TimeLineWidget::updateTimeLines()
{
    kDebug();
    twitter->requestTimeLine ( latestHomeStatusId, Backend::HomeTimeLine );
    twitter->requestTimeLine ( latestReplyStatusId, Backend::ReplyTimeLine );
    twitter->requestDMessages( latestRecievedStatusId, Backend::Recieved );
    twitter->requestDMessages( latestSentStatusId, Backend::Sent );

    if ( latestHomeStatusId == 0 || latestReplyStatusId == 0 )
        isStartMode = true;
    else
        isStartMode = false;

  emit notify(i18n("Loading timelines..."));
}

void TimeLineWidget::directMessagesRecieved(QList< Status > & msgList)
{
    kDebug();
    emit notify ( i18n ( "Latest direct messages received!" ) );
    int count = msgList.count();

    if ( count == 0 ) {
        kDebug() << "Message list is empty";
        emit notify ( i18n ( "No new messages received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi ( msgList, inboxLayout, &listInboxStatus, Backend::InboxTimeLine );
        inboxScroll->verticalScrollBar()->setSliderPosition ( 0 );
        kDebug() << count << " Statuses received.";
        if ( !isStartMode ) {
            unreadStatusInInbox += count;
            if( unreadStatusInInbox > 0 )
                tabs->setTabText ( 2, i18n ( "Inbox(%1)", unreadStatusInInbox ) );
        }
    }
}

void TimeLineWidget::sentMessagesRecieved(QList< Status > & msgList)
{
    kDebug();
    emit notify ( i18n ( "Latest sent messages received!" ) );
    int count = msgList.count();

    if ( count == 0 ) {
        kDebug() << "Message list is empty";
        emit notify ( i18n ( "No new messages received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi ( msgList, sentLayout, &listSentStatus, Backend::SentTimeLine );
        sentScroll->verticalScrollBar()->setSliderPosition ( 0 );

        kDebug() << count << " Statuses received.";

//         if ( !isStartMode ) {
//             unreadStatusInSent += count;
//             if( unreadStatusInSent > 0 )
//                 tabs->setTabText ( 3, i18n ( "Sent(%1)", unreadStatusInSent ) );
//         }
    }
}

void TimeLineWidget::homeTimeLinesRecived ( QList< Status > & statusList )
{
    kDebug();
    emit notify ( i18n ( "Latest friends timeline received!" ) );
    int count = statusList.count();

    if ( count == 0 ) {
        kDebug() << "Status list is empty";
        emit notify ( i18n ( "No new statuses received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi ( statusList, homeLayout, &listHomeStatus );
        homeScroll->verticalScrollBar()->setSliderPosition ( 0 );

//         int count = statusList.count();
        kDebug() << count << " Statuses received.";

        if ( !isStartMode ) {
            unreadStatusInHome += count;
            if( unreadStatusInHome > 0 )
                tabs->setTabText ( 0, i18n ( "Home(%1)", unreadStatusInHome ) );
        }
    }
}

void TimeLineWidget::replyTimeLineRecived ( QList< Status > & statusList )
{
    kDebug();
    emit notify ( i18n ( "Latest replies timeline received!" ) );
    int count = statusList.count();
    if ( count == 0 ) {
        kDebug() << "Status list is empty";
        emit notify ( i18n ( "No new statuses received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi ( statusList, replyLayout, &listReplyStatus, Backend::ReplyTimeLine );
        replyScroll->verticalScrollBar()->setSliderPosition ( 0 );

//         int count = statusList.count();
        kDebug() << count << " Statuses received.";

        if ( !isStartMode ) {
            unreadStatusInReply += count;
            if( unreadStatusInReply > 0 )
                tabs->setTabText ( 1, i18n ( "Reply(%1)", unreadStatusInReply ) );
        }
    }
}

void TimeLineWidget::addNewStatusesToUi ( QList< Status > & statusList, QBoxLayout * layoutToAddStatuses,
        QList<StatusWidget*> *list, Backend::TimeLineType type )
{
    kDebug();
    bool allInOne = Settings::showAllNotifiesInOne();
    QString notifyStr;
    int numOfNewStatuses = statusList.count();
    QList<Status>::const_iterator it = statusList.constBegin();
    QList<Status>::const_iterator endIt = statusList.constEnd();
    bool isThereIsAnyNewStatusToNotify = false;
	if(Settings::notifyType() != 2)
		notifyStr = (mCurrentAccount.direction()==Qt::RightToLeft) ? "<div dir='rtl'>" : "<div dir='ltr'>";
    for ( ;it != endIt; ++it ) {
        if ( it->replyToUserId == mCurrentAccount.userId() && type == Backend::HomeTimeLine ) {
            --numOfNewStatuses;
            --unreadStatusInHome;
            continue;
        }

        StatusWidget *wt = new StatusWidget ( &mCurrentAccount, this );

        wt->setAttribute ( Qt::WA_DeleteOnClose );
        wt->setCurrentStatus ( *it );
        connect ( wt, SIGNAL ( sigReply ( const QString&, uint ) ),
                  this, SLOT ( prepareReply ( const QString&, uint ) ) );
        connect ( wt, SIGNAL ( sigFavorite ( uint, bool ) ),
                  twitter, SLOT ( requestFavorited ( uint, bool ) ) );
        connect ( wt, SIGNAL ( sigDestroy ( uint ) ),
                  this, SLOT ( requestDestroy ( uint ) ) );
        list->append ( wt );
        layoutToAddStatuses->insertWidget ( 0, wt );

        if ( !isStartMode ) {
            if ( it->user.userId == mCurrentAccount.userId() ) {
                --numOfNewStatuses;
                wt->setUnread ( StatusWidget::WithoutNotify );
            } else {
                if(type == Backend::SentTimeLine){
                    wt->setUnread ( StatusWidget::WithoutNotify );
                } else if ( allInOne ) {
                    notifyStr += "<b>" + it->user.screenName + " : </b>" + it->content + "<br/>";
                    isThereIsAnyNewStatusToNotify = true;
                    wt->setUnread ( StatusWidget::WithoutNotify );
                } else {
                    wt->setUnread ( StatusWidget::WithNotify );
                }
            }

            listUnreadStatuses.append ( wt );
        }
	}
	if(Settings::notifyType() != 2)
		notifyStr += "</div>";
    uint latestId = statusList.last().statusId;
    if( type == Backend::HomeTimeLine && latestId > latestHomeStatusId){
        kDebug()<<"Latest home statusId sets to: "<<latestId;
        latestHomeStatusId = latestId;
    } else if( type == Backend::ReplyTimeLine && latestId > latestReplyStatusId){
        kDebug()<<"Latest reply statusId sets to: "<<latestId;
        latestReplyStatusId = latestId;
    } else if( type == Backend::InboxTimeLine && latestId > latestRecievedStatusId ) {
        kDebug()<<"Latest recieved statusId sets to: "<<latestId;
        latestRecievedStatusId = latestId;
    } else if( type == Backend::SentTimeLine && latestId > latestSentStatusId ) {
        kDebug()<<"Latest sent statusId sets to: "<<latestId;
        latestSentStatusId = latestId;
    }
    if ( !isStartMode )
        checkUnreadStatuses ( numOfNewStatuses );

    if ( isThereIsAnyNewStatusToNotify ) {
        emit systemNotify ( i18n ( "New statuses" ), notifyStr, APPNAME );
    }

    updateStatusList ( list );
}

void TimeLineWidget::abortPostNewStatus()
{
    kDebug();
    twitter->abortPostNewStatus();
}

void TimeLineWidget::setDefaultDirection()
{
    tabs->widget ( 0 )->setLayoutDirection ( mCurrentAccount.direction() );
    tabs->widget ( 1 )->setLayoutDirection ( mCurrentAccount.direction() );
    tabs->widget ( 2 )->setLayoutDirection ( mCurrentAccount.direction() );
    tabs->widget ( 3 )->setLayoutDirection ( mCurrentAccount.direction() );
    tabs->widget ( 4 )->setLayoutDirection ( mCurrentAccount.direction() );

    txtNewStatus->setDefaultDirection ( mCurrentAccount.direction() );
}

void TimeLineWidget::error ( const QString & errMsg )
{
    emit notify ( i18n ( "Failed, %1", errMsg ) );
}

void TimeLineWidget::postStatus ( QString & status )
{
    kDebug();

    if ( status.size() > MAX_STATUS_SIZE && status.indexOf ( "http://" ) == -1 ) {
        QString err = i18n ( "Status text size is more than server limit size." );
        error ( err );
        return;
    }
    txtNewStatus->setEnabled ( false );
    if(chkDMessage->isChecked()){
        emit notify ( i18n ( "Sending direct message..." ) );
        twitter->sendDMessage( comboFriendList->currentText(), status );
    } else {
        emit notify ( i18n ( "Posting new status..." ) );
        twitter->postNewStatus ( status, replyToStatusId );
    }
}

void TimeLineWidget::postingNewStatusDone ( bool isError )
{
    if ( !isError ) {
        txtNewStatus->clearContentsAndSetDirection ( mCurrentAccount.direction() );
        QString successMsg = i18n ( "New status posted successfully" );
        emit systemNotify ( i18n ( "Success!" ), successMsg, APPNAME );
        notify ( successMsg );
        replyToStatusId = 0;
    } else {
        error ( twitter->latestErrorString() );
    }
    txtNewStatus->setFocus( Qt::OtherFocusReason );
    txtNewStatus->setEnabled ( true );

}

bool TimeLineWidget::saveStatuses ( QString fileName, QList<StatusWidget*> &list )
{
    kDebug();
    KConfig statusesBackup ( "choqok/"+fileName, KConfig::NoGlobals, "data" );

    ///Clear previous data:
    QStringList prevList = statusesBackup.groupList();
    int c = prevList.count();

    if ( c > 0 ) {
        for ( int i = 0; i < c; ++i ) {
            statusesBackup.deleteGroup ( prevList[i] );
        }
    }

    int count = list.count();

    for ( int i = 0; i < count; ++i ) {
//   QString str = ;
        KConfigGroup grp ( &statusesBackup, QString::number ( list[i]->currentStatus().statusId ) );
        grp.writeEntry ( "created_at", list[i]->currentStatus().creationDateTime );
        grp.writeEntry ( "id", list[i]->currentStatus().statusId );
        grp.writeEntry ( "text", list[i]->currentStatus().content );
        grp.writeEntry ( "source", list[i]->currentStatus().source );
        grp.writeEntry ( "truncated", list[i]->currentStatus().isTruncated );
        grp.writeEntry ( "in_reply_to_status_id", list[i]->currentStatus().replyToStatusId );
        grp.writeEntry ( "in_reply_to_user_id", list[i]->currentStatus().replyToUserId );
        grp.writeEntry ( "favorited", list[i]->currentStatus().isFavorited );
        grp.writeEntry ( "in_reply_to_screen_name", list[i]->currentStatus().replyToUserScreenName );
        grp.writeEntry ( "userId", list[i]->currentStatus().user.userId );
        grp.writeEntry ( "screen_name", list[i]->currentStatus().user.screenName );
        grp.writeEntry ( "name", list[i]->currentStatus().user.name );
        grp.writeEntry ( "profile_image_url", list[i]->currentStatus().user.profileImageUrl );
        grp.writeEntry ( "description" , list[i]->currentStatus().user.description );
    }

    statusesBackup.sync();

    return true;
}

QList< Status > TimeLineWidget::loadStatuses ( QString fileName )
{
    kDebug();
    KConfig statusesBackup ( "choqok/"+fileName, KConfig::NoGlobals, "data" );
//  KConfigGroup entries(&statusesBackup, "Statuses");
    QList< Status > list;
    QStringList groupList = statusesBackup.groupList();
//  kDebug()<<groupList;
    int count = groupList.count();

    for ( int i = 0; i < count; ++i ) {
        KConfigGroup grp ( &statusesBackup, groupList[i] );
        Status st;
        st.creationDateTime = grp.readEntry ( "created_at", QDateTime::currentDateTime() );
        st.statusId = grp.readEntry ( "id", ( uint ) 0 );
        st.content = grp.readEntry ( "text", QString() );
        st.source = grp.readEntry ( "source", QString() );
        st.isTruncated = grp.readEntry ( "truncated", false );
        st.replyToStatusId = grp.readEntry ( "in_reply_to_status_id", ( uint ) 0 );
        st.replyToUserId = grp.readEntry ( "in_reply_to_user_id", ( uint ) 0 );
        st.isFavorited = grp.readEntry ( "favorited", false );
        st.replyToUserScreenName = grp.readEntry ( "in_reply_to_screen_name", QString() );
        st.user.userId = grp.readEntry ( "userId", ( uint ) 0 );
        st.user.screenName = grp.readEntry ( "screen_name", QString() );
        st.user.name = grp.readEntry ( "name", QString() );
        st.user.profileImageUrl = grp.readEntry ( "profile_image_url", QString() );
        st.user.description = grp.readEntry( "description" , QString() );

        //Sorting The new statuses:
        int i=0;
        int count = list.count();
        while( (i < count) && (st.statusId > list[ i ].statusId) ){
            ++i;
        }
        list.insert( i, st );
    }

    return list;
}

void TimeLineWidget::prepareReply ( const QString &userName, uint statusId )
{
    kDebug();
    emit showMe();
    if (qobject_cast<StatusWidget*> ( sender() )->currentStatus().isDMessage ){
        chkDMessage->setChecked( true );
        comboFriendList->setCurrentItem( userName, true, 0 );
    } else {
        QString current = txtNewStatus->toPlainText();
        txtNewStatus->setText ( '@' + userName + ' ' + current );
        replyToStatusId = statusId;
        txtNewStatus->setDefaultDirection ( mCurrentAccount.direction() );
    }
    txtNewStatus->moveCursor ( QTextCursor::End );
    txtNewStatus->setFocus( Qt::OtherFocusReason );
}

void TimeLineWidget::updateStatusList ( QList<StatusWidget*> *list )
{
    kDebug();
    int toBeDelete = list->count() - Settings::countOfStatusesOnMain();

    if ( toBeDelete > 0 ) {
        for ( int i = 0; i < toBeDelete; ++i ) {
            StatusWidget* wt = list->at ( i );

            if ( !wt->isReaded() )
                break;

            list->removeAt ( i );

            --i;

            --toBeDelete;

            wt->close();
        }
    }
}

void TimeLineWidget::reloadTimeLineLists()
{
    kDebug();
    clearTimeLineList ( &listHomeStatus );
    clearTimeLineList ( &listReplyStatus );
//   Settings::setLatestStatusId(0);
    updateTimeLines();

}

void TimeLineWidget::clearTimeLineList ( QList< StatusWidget * > * list )
{
    kDebug();
    int count = list->count();

    for ( int i = 0; i < count; ++i ) {
        StatusWidget* wt = list->first();
        list->removeFirst();
        wt->close();
    }
}

void TimeLineWidget::loadConfigurations()
{
    kDebug();
    setDefaultDirection();
    QStringList fList = AccountManager::self()->listFriends( mCurrentAccount.alias() );
    txtNewStatus->setFriendsList( fList );
    comboFriendList->addItems( fList );

    isStartMode = true;

    QList< Status > lstHome = loadStatuses ( generateStatusBackupFileName(Backend::HomeTimeLine) );
    if ( lstHome.count() > 0 )
        addNewStatusesToUi ( lstHome, homeLayout, &listHomeStatus );

    QList< Status > lstReply = loadStatuses ( generateStatusBackupFileName(Backend::ReplyTimeLine) );
    if ( lstReply.count() > 0 )
        addNewStatusesToUi ( lstReply, replyLayout, &listReplyStatus, Backend::ReplyTimeLine );

    QList< Status > lstInbox = loadStatuses ( generateStatusBackupFileName(Backend::InboxTimeLine) );
    if ( lstInbox.count() > 0 )
        addNewStatusesToUi ( lstInbox, inboxLayout, &listInboxStatus, Backend::InboxTimeLine );

    QList< Status > lstSent = loadStatuses ( generateStatusBackupFileName(Backend::SentTimeLine) );
    if ( lstSent.count() > 0 )
        addNewStatusesToUi ( lstSent, sentLayout, &listSentStatus, Backend::SentTimeLine );
}

void TimeLineWidget::checkUnreadStatuses ( int numOfNewStatusesReciened )
{
    kDebug();
//  if(this->isVisible()){
//   unreadStatusCount = 0;
//  } else {
    unreadStatusCount += numOfNewStatusesReciened;
//  }
    emit sigSetUnread ( numOfNewStatusesReciened );
    emit sigSetUnreadOnMainWin( unreadStatusCount );
}

void TimeLineWidget::setUnreadStatusesToReadState()
{
    kDebug();
    tabs->setTabText ( 0, i18n ( "Home" ) );
    tabs->setTabText ( 1, i18n ( "Reply" ) );
    int count = listUnreadStatuses.count();

    for ( int i = 0;i < count; ++i ) {
        listUnreadStatuses[i]->setRead();
    }

    listUnreadStatuses.clear();

    emit sigSetUnread ( -unreadStatusCount );
    emit sigSetUnreadOnMainWin( 0 );
    unreadStatusCount = unreadStatusInReply = unreadStatusInHome = 0;
}

void TimeLineWidget::requestFavoritedDone ( bool isError )
{
    kDebug() << "is Error: " << isError;
    notify ( "Done!" );
}

void TimeLineWidget::requestDestroyDone ( bool isError )
{
    kDebug() << "is Error: " << isError;
    notify ( "Done!" );
    toBeDestroied->close();
}

void TimeLineWidget::requestDestroy ( uint statusId )
{
    if ( KMessageBox::warningYesNo ( this, i18n ( "Are you sure of destroying this status?" ) ) == KMessageBox::Yes ) {
        toBeDestroied = qobject_cast<StatusWidget*> ( sender() );
        if(toBeDestroied->currentStatus().isDMessage){
            twitter->requestDestroyDMessage( statusId );
        } else {
            twitter->requestDestroy ( statusId );
        }
            setUnreadStatusesToReadState();
    }
}

void TimeLineWidget::disableApp()
{
    kDebug();
    txtNewStatus->setEnabled ( false );
}

void TimeLineWidget::enableApp()
{
    kDebug();
    txtNewStatus->setEnabled ( true );
}

QString TimeLineWidget::generateStatusBackupFileName(Backend::TimeLineType type)
{
    QString name = mCurrentAccount.alias();
    name += '_';
    
    switch(type){
        case Backend::HomeTimeLine:
            name += "home";
            break;
        case Backend::ReplyTimeLine:
            name += "reply";
            break;
        case Backend::InboxTimeLine:
            name += "inbox";
            break;
        case Backend::SentTimeLine:
            name += "sent";
            break;
        default:
            name += QString::number(type);
            break;
    };
    name += "statuslistrc";
    return name;
}

Account TimeLineWidget::currentAccount() const
{
    return mCurrentAccount;
}

void TimeLineWidget::setCurrentAccount(const Account & account)
{
    mCurrentAccount = account;
}

void TimeLineWidget::reloadFriendsList()
{
    kDebug();
    comboFriendList->clear();
    txtNewStatus->clearFriendsList();
    twitter->listFollowersScreenName();
    twitter->listFriendsScreenName();
}

void TimeLineWidget::friendsListed(const QStringList & list)
{
    comboFriendList->addItems( list );
    txtNewStatus->appendToFriendsList( list );
}

#include "timelinewidget.moc"
