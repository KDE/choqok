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
    saveStatuses( generateStatusBackupFileName(Backend::HomeTimeLine), listHomeStatus );
    saveStatuses( generateStatusBackupFileName(Backend::ReplyTimeLine), listReplyStatus );
    
}

void TimeLineWidget::initObjects()
{
    kDebug();

    txtNewStatus->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Maximum );
//     txtNewStatus->setMaximumHeight ( 80 );
//     txtNewStatus->setMinimumHeight ( 30 );
    txtNewStatus->setFocus ( Qt::OtherFocusReason );
    txtNewStatus->setTabChangesFocus ( true );
    
    QFont counterF;
    counterF.setBold( true );
    counterF.setPointSize( 12 );
    lblCounter->setFont( counterF );
    checkNewStatusCharactersCount(140);

    twitter = new Backend ( &mCurrentAccount, this );

    connect ( twitter, SIGNAL ( homeTimeLineRecived ( QList< Status >& ) ), this, SLOT ( homeTimeLinesRecived ( QList< Status >& ) ) );
    connect ( twitter, SIGNAL ( replyTimeLineRecived ( QList< Status >& ) ), this, SLOT ( replyTimeLineRecived ( QList< Status >& ) ) );
    connect ( twitter, SIGNAL ( sigPostNewStatusDone ( bool ) ), this, SLOT ( postingNewStatusDone ( bool ) ) );
    connect ( twitter, SIGNAL ( sigFavoritedDone ( bool ) ), this, SLOT ( requestFavoritedDone ( bool ) ) );
    connect ( twitter, SIGNAL ( sigDestroyDone ( bool ) ), this, SLOT ( requestDestroyDone ( bool ) ) );
    connect ( twitter, SIGNAL ( sigError ( QString& ) ), this, SLOT ( error ( QString& ) ) );

    replyToStatusId = unreadStatusCount = unreadStatusInReply = unreadStatusInHome = 0;

    loadConfigurations();
}

void TimeLineWidget::checkNewStatusCharactersCount ( int numOfChars )
{
    if ( numOfChars < 0 ) {
        lblCounter->setStyleSheet ( "QLabel {color: red;}" );
    } else if ( numOfChars < 30 ) {
        lblCounter->setStyleSheet ( "QLabel {color: rgb(255, 255, 0);}" );
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

    if ( latestHomeStatusId == 0 || latestReplyStatusId == 0 )
        isStartMode = true;
    else
        isStartMode = false;

  emit notify(i18n("Loading timelines..."));
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

        int count = statusList.count();
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

    if ( statusList.count() == 0 ) {
        kDebug() << "Status list is empty";
        emit notify ( i18n ( "No new statuses received. The list is up to date." ) );
        return;
    } else {
        addNewStatusesToUi ( statusList, replyLayout, &listReplyStatus, Backend::ReplyTimeLine );
        replyScroll->verticalScrollBar()->setSliderPosition ( 0 );

        int count = statusList.count();
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
		notifyStr = (mCurrentAccount.direction==Qt::RightToLeft) ? "<div dir='rtl'>" : "<div dir='ltr'>";
    for ( ;it != endIt; ++it ) {
        if ( it->replyToUserScreenName.toLower() == mCurrentAccount.username.toLower() && type == Backend::HomeTimeLine ) {
            --numOfNewStatuses;
            --unreadStatusInHome;
            continue;
        }

        StatusWidget *wt = new StatusWidget ( &mCurrentAccount, this );

        wt->setAttribute ( Qt::WA_DeleteOnClose );
        wt->setCurrentStatus ( *it );
        connect ( wt, SIGNAL ( sigReply ( QString&, uint ) ), this, SLOT ( prepareReply ( QString&, uint ) ) );
        connect ( wt, SIGNAL ( sigFavorite ( uint, bool ) ), twitter, SLOT ( requestFavorited ( uint, bool ) ) );
        connect ( wt, SIGNAL ( sigDestroy ( uint ) ), this, SLOT ( requestDestroy ( uint ) ) );
        list->append ( wt );
        layoutToAddStatuses->insertWidget ( 0, wt );

        if ( !isStartMode ) {
            if ( it->user.screenName.toLower() == mCurrentAccount.username.toLower() ) {
                --numOfNewStatuses;
                wt->setUnread ( StatusWidget::WithoutNotify );
            } else {
                if ( allInOne ) {
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
//  this->setLayoutDirection((Qt::LayoutDirection)Settings::direction());
    tabs->widget ( 0 )->setLayoutDirection ( mCurrentAccount.direction );
    tabs->widget ( 1 )->setLayoutDirection ( mCurrentAccount.direction );
//  txtNewStatus->document()->firstBlock()->
//  inputLayout->setLayoutDirection((Qt::LayoutDirection)Settings::direction());
    txtNewStatus->setDefaultDirection ( mCurrentAccount.direction );
}

void TimeLineWidget::error ( QString & errMsg )
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

    emit notify ( i18n ( "Posting New status..." ) );

    txtNewStatus->setEnabled ( false );
    twitter->postNewStatus ( status, replyToStatusId );
}

void TimeLineWidget::postingNewStatusDone ( bool isError )
{
    if ( !isError ) {
        txtNewStatus->clearContentsAndSetDirection ( mCurrentAccount.direction );
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
        list.append ( st );
    }

    return list;
}

void TimeLineWidget::prepareReply ( QString &userName, uint statusId )
{
    kDebug();

    emit showMe();

    QString current = txtNewStatus->toPlainText();

    txtNewStatus->setText ( '@' + userName + ' ' + current );

    replyToStatusId = statusId;

    txtNewStatus->setDefaultDirection ( mCurrentAccount.direction );

    txtNewStatus->moveCursor ( QTextCursor::End );
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

    isStartMode = true;

    QList< Status > lstHome = loadStatuses ( generateStatusBackupFileName(Backend::HomeTimeLine) );

    if ( lstHome.count() > 0 )
        addNewStatusesToUi ( lstHome, homeLayout, &listHomeStatus );

    QList< Status > lstReply = loadStatuses ( generateStatusBackupFileName(Backend::ReplyTimeLine) );

    if ( lstReply.count() > 0 )
        addNewStatusesToUi ( lstReply, replyLayout, &listReplyStatus, Backend::ReplyTimeLine );
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
        twitter->requestDestroy ( statusId );
        toBeDestroied = qobject_cast<StatusWidget*> ( sender() );
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
    QString name = mCurrentAccount.alias;
    name += '_';
    
    switch(type){
        case Backend::HomeTimeLine:
            name += "home";
            break;
        case Backend::ReplyTimeLine:
            name += "reply";
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


#include "timelinewidget.moc"
