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

#include "twitterapimicroblog.h"

#include <KLocale>
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <KAboutData>
#include <KGenericFactory>
#include "account.h"
#include "microblogwidget.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "postwidget.h"
#include "twitterapiaccount.h"
#include "twitterapipostwidget.h"
#include <KMenu>
#include <KAction>
#include <choqokuiglobal.h>
#include <accountmanager.h>
#include "twitterapidmessagedialog.h"
#include "choqokbehaviorsettings.h"
#include "choqokid.h"
#include "twitterapisearch.h"
#include "twitterapisearchdialog.h"
#include "twitterapisearchtimelinewidget.h"
#include <notifymanager.h>

class TwitterApiMicroBlog::Private
{
public:
    Private():countOfTimelinesToSave(0), friendsPage(1)
    {
        monthes["Jan"] = 1;
        monthes["Feb"] = 2;
        monthes["Mar"] = 3;
        monthes["Apr"] = 4;
        monthes["May"] = 5;
        monthes["Jun"] = 6;
        monthes["Jul"] = 7;
        monthes["Aug"] = 8;
        monthes["Sep"] = 9;
        monthes["Oct"] = 10;
        monthes["Nov"] = 11;
        monthes["Dec"] = 12;
    }
    int countOfTimelinesToSave;
    int friendsPage;
    QMap<QString, int> monthes;
    QStringList friendsList;
};

TwitterApiMicroBlog::TwitterApiMicroBlog ( const KComponentData &instance, QObject *parent )
: MicroBlog( instance, parent), d(new Private)
{
    kDebug();
    setCharLimit(140);
    QStringList timelineTypes;
    timelineTypes<< "Home" << "Reply" << "Inbox" << "Outbox" << "Favorite" << "Public";
    setTimelineNames(timelineTypes);
    timelineApiPath["Home"] = "/statuses/home_timeline.xml";
    timelineApiPath["Reply"] = "/statuses/replies.xml";
    timelineApiPath["Inbox"] = "/direct_messages.xml";
    timelineApiPath["Outbox"] = "/direct_messages/sent.xml";
    timelineApiPath["Favorite"] = "/favorites.xml";
    timelineApiPath["Public"] = "/statuses/public_timeline.xml";
    setTimelineInfos();
}

void TwitterApiMicroBlog::setTimelineInfos()
{
    Choqok::TimelineInfo *t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Home");
    t->description = i18nc("Timeline description", "You and your friends");
    t->icon = "user-home";
    mTimelineInfos["Home"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Reply");
    t->description = i18nc("Timeline description", "Replies to you");
    t->icon = "edit-undo";
    mTimelineInfos["Reply"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Inbox");
    t->description = i18nc("Timeline description", "Your incoming private messages");
    t->icon = "mail-folder-inbox";
    mTimelineInfos["Inbox"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Outbox");
    t->description = i18nc("Timeline description", "Private messages you have sent");
    t->icon = "mail-folder-outbox";
    mTimelineInfos["Outbox"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Favorite");
    t->description = i18nc("Timeline description", "Your favorites");
    t->icon = "favorites";
    mTimelineInfos["Favorite"] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Public");
    t->description = i18nc("Timeline description", "Public timeline");
    t->icon = "folder-green";
    mTimelineInfos["Public"] = t;
}

TwitterApiMicroBlog::~TwitterApiMicroBlog()
{
    kDebug();
    delete d;
}

QMenu* TwitterApiMicroBlog::createActionsMenu(Choqok::Account* theAccount, QWidget* parent)
{
    QMenu * menu = MicroBlog::createActionsMenu(theAccount, parent);

    KAction *directMessge = new KAction( KIcon("mail-message-new"), i18n("Send Private Message..."), menu );
    directMessge->setData( theAccount->alias() );
    connect( directMessge, SIGNAL(triggered(bool)), SLOT(showDirectMessageDialog()) );
    menu->addAction(directMessge);

    KAction *search = new KAction( KIcon("edit-find"), i18n("Search..."), menu );
    search->setData( theAccount->alias() );
    connect( search, SIGNAL(triggered(bool)), SLOT(showSearchDialog()) );
    menu->addAction(search);

    return menu;
}

QList< Choqok::Post* > TwitterApiMicroBlog::loadTimeline( Choqok::Account *account, const QString& timelineName)
{
    kDebug()<<timelineName;
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
    QStringList tmpList = postsBackup.groupList();
    QList<ChoqokId> groupList;
    foreach(const QString &str, tmpList)
        groupList<<str;
    qSort(groupList);
    QList< Choqok::Post* > list;
    int count = groupList.count();
    if( count ) {
        /**
        Checking if QStringList::sort() failed on sorting numbers,
        This happends when one or more items char count is less/more than other ones
        */
//         if(groupList.constBegin()->count() != (--(groupList.constEnd()))->count()) {
//             int charCount = groupList[0].count();
//             for(int i=1; i<count; ++i){
//                 int tmp = groupList[i].count();
//                 if( tmp < charCount ){
//                     int k = 0;
//                     for(int j = i; j<count; ++j){
//                         int tmp2 = groupList[j].count();
//                         if( tmp2 == tmp )
//                             groupList.move(j, k);
//                         else
//                             charCount = tmp2;
//                     }
//                 } else
//                     charCount = tmp;
//             }
//         }
        ///END checking
        Choqok::Post *st = 0;
        for ( int i = 0; i < count; ++i ) {
            st = new Choqok::Post;
            KConfigGroup grp( &postsBackup, groupList[i] );
            st->creationDateTime = grp.readEntry( "creationDateTime", QDateTime::currentDateTime() );
            st->postId = grp.readEntry( "postId", QString() );
            st->content = grp.readEntry( "text", QString() );
            st->source = grp.readEntry( "source", QString() );
            st->replyToPostId = grp.readEntry( "inReplyToPostId", QString() );
            st->replyToUserId = grp.readEntry( "inReplyToUserId", QString() );
            st->isFavorited = grp.readEntry( "favorited", false );
            st->replyToUserName = grp.readEntry( "inReplyToUserName", QString() );
            st->author.userId = grp.readEntry( "authorId", QString() );
            st->author.userName = grp.readEntry( "authorUserName", QString() );
            st->author.realName = grp.readEntry( "authorRealName", QString() );
            st->author.profileImageUrl = grp.readEntry( "authorProfileImageUrl", QString() );
            st->author.description = grp.readEntry( "authorDescription" , QString() );
            st->isPrivate = grp.readEntry( "isPrivate" , false );
            st->author.location = grp.readEntry("authorLocation", QString());
            st->author.homePageUrl = grp.readEntry("authorUrl", QString());
            st->link = postUrl( account, st->author.userName, st->postId);
            st->isRead = grp.readEntry("isRead", true);
            //Sorting The new statuses:
//             int j = 0;
//             int count = list.count();
//             while (( j < count ) && ( st->postId > list[ j ]->postId ) ) {
//                 ++j;
//             }
            list.append( st );
        }
        mTimelineLatestId[account][timelineName] = st->postId;
    }
    return list;
}

void TwitterApiMicroBlog::saveTimeline(Choqok::Account *account,
                                       const QString& timelineName,
                                       const QList< Choqok::UI::PostWidget* > &timeline)
{
    kDebug();
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );

    ///Clear previous data:
    QStringList prevList = postsBackup.groupList();
    int c = prevList.count();
    if ( c > 0 ) {
        for ( int i = 0; i < c; ++i ) {
            postsBackup.deleteGroup( prevList[i] );
        }
    }
    QList< Choqok::UI::PostWidget *>::const_iterator it, endIt = timeline.constEnd();
    for ( it = timeline.constBegin(); it != endIt; ++it ) {
        const Choqok::Post *post = &((*it)->currentPost());
        KConfigGroup grp( &postsBackup, post->postId );
        grp.writeEntry( "creationDateTime", post->creationDateTime );
        grp.writeEntry( "postId", post->postId.toString() );
        grp.writeEntry( "text", post->content );
        grp.writeEntry( "source", post->source );
        grp.writeEntry( "inReplyToPostId", post->replyToPostId.toString() );
        grp.writeEntry( "inReplyToUserId", post->replyToUserId.toString() );
        grp.writeEntry( "favorited", post->isFavorited );
        grp.writeEntry( "inReplyToUserName", post->replyToUserName );
        grp.writeEntry( "authorId", post->author.userId.toString() );
        grp.writeEntry( "authorUserName", post->author.userName );
        grp.writeEntry( "authorRealName", post->author.realName );
        grp.writeEntry( "authorProfileImageUrl", post->author.profileImageUrl );
        grp.writeEntry( "authorDescription" , post->author.description );
        grp.writeEntry( "isPrivate" , post->isPrivate );
        grp.writeEntry( "authorLocation" , post->author.location );
        grp.writeEntry( "authorUrl" , post->author.homePageUrl );
        grp.writeEntry( "isRead" , post->isRead );
    }
    postsBackup.sync();
    --d->countOfTimelinesToSave;
    if(d->countOfTimelinesToSave < 1)
        emit readyForUnload();
}

TwitterApiSearchTimelineWidget * TwitterApiMicroBlog::createSearchTimelineWidget(Choqok::Account* theAccount,
                                                                                 QString name,
                                                                                 const SearchInfo &info,
                                                                                 QWidget* parent)
{
    return new TwitterApiSearchTimelineWidget(theAccount, name, info, parent);
}

void TwitterApiMicroBlog::createPost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post || post->content.isEmpty() ) {
        kDebug() << "ERROR: Status text is empty!";
        emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError,
                         i18n ( "Creating the new post failed. Text is empty." ), MicroBlog::Critical );
        return;
    }
    if ( !post->isPrivate ) {///Status Update
        KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
        url.addPath ( "/statuses/update.xml" );
        QByteArray data = "status=";
        data += QUrl::toPercentEncoding (  post->content );
        if ( !post->replyToPostId.isEmpty() ) {
            data += "&in_reply_to_status_id=";
            data += post->replyToPostId.toLocal8Bit();
        }
        data += "&source=choqok";
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Creating the new post failed. Cannot create an http POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
    } else {///Direct message
        QString recipientScreenName = post->replyToUserName;
        KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
        url.addPath ( "/direct_messages/new.xml" );
        QByteArray data = "user=";
        data += recipientScreenName.toLocal8Bit();
        data += "&text=";
        data += QUrl::toPercentEncoding ( post->content );
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Creating the new post failed. Cannot create an http POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
    }
}

void TwitterApiMicroBlog::slotCreatePost ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = mCreatePostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if(!post || !theAccount) {
        kDebug()<<"Account or Post is NULL pointer";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit errorPost ( theAccount, post, Choqok::MicroBlog::CommunicationError,
                         i18n("Creating the new post failed. %1", job->errorString()), MicroBlog::Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast< KIO::StoredTransferJob * > ( job );
        if ( !post->isPrivate ) {
            readPostFromXml ( theAccount, stj->data(), post );
            if ( post->isError ) {
                QString errorMsg = checkXmlForError(stj->data());
                if( errorMsg.isEmpty() ){
                    kError() << "Creating post: XML parsing error: "<< stj->data() ;
                    emit errorPost ( theAccount, post, Choqok::MicroBlog::ParsingError,
                                    i18n ( "Creating the new post failed. The result data could not be parsed." ), MicroBlog::Critical );
                } else {
                    kError() << "Server Error:" << errorMsg ;
                    emit errorPost ( theAccount, post, Choqok::MicroBlog::ServerError,
                                     i18n ( "Creating the new post failed, with error: %1", errorMsg ),
                                     MicroBlog::Critical );
                }
            } else {
                emit postCreated ( theAccount, post );
            }
        } else {
            emit postCreated ( theAccount, post );
        }
    }
}

void TwitterApiMicroBlog::abortAllJobs(Choqok::Account* theAccount)
{
    foreach ( KJob *job, mJobsAccount.keys(theAccount) ) {
        job->kill(KJob::EmitResult);
    }
}

void TwitterApiMicroBlog::abortCreatePost(Choqok::Account* theAccount, Choqok::Post *post)
{
    if( mCreatePostMap.isEmpty() )
        return;
    if( post ) {
        mCreatePostMap.key(post)->kill(KJob::EmitResult);
    } else {
        foreach( KJob *job, mCreatePostMap.keys() ){
            if(mJobsAccount[job] == theAccount)
                job->kill(KJob::EmitResult);
        }
    }
}

void TwitterApiMicroBlog::fetchPost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post || post->postId.isEmpty()) {
        return;
    }
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath ( QString("/statuses/show/%1.xml").arg(post->postId) );

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Fetching the new post failed. Cannot create an http GET request."
//                                 "Please check your KDE installation." );
//         emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, Low );
        return;
    }
    mFetchPostMap[ job ] = post;
    mJobsAccount[ job ] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotFetchPost ( KJob* ) ) );
    job->start();
}

void TwitterApiMicroBlog::slotFetchPost ( KJob *job )
{
    kDebug();
    if(!job) {
        kWarning()<<"NULL Job returned";
        return;
    }
    Choqok::Post *post = mFetchPostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching the new post failed. %1", job->errorString()), Low );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
        readPostFromXml ( theAccount, stj->data(), post );
        if ( post->isError ) {
            QString errorMsg = checkXmlForError(stj->data());
            if( errorMsg.isEmpty() ){
                kDebug() << "Parsing Error";
                emit errorPost ( theAccount, post, Choqok::MicroBlog::ParsingError,
                                i18n ( "Fetching new post failed. The result data could not be parsed." ),
                                 Low );
            } else {
                kError()<<"Fetching post: Server Error: "<<errorMsg;
                emit errorPost ( theAccount, post, Choqok::MicroBlog::ServerError,
                                 i18n ( "Fetching new post failed, with error: %1", errorMsg ),
                                 Low );
            }
        } else {
            post->isError = true;
            emit postFetched ( theAccount, post );
            //             mFetchPostMap.remove(job);
        }
    }
}

void TwitterApiMicroBlog::removePost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post->postId.isEmpty() ) {
        KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
        if ( !post->isPrivate ) {
            url.addPath ( "/statuses/destroy/" + post->postId + ".xml" );
        } else {
            url.addPath ( "/direct_messages/destroy/" + post->postId + ".xml" );
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Removing post failed, Cannot create an http POST request, Check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        mRemovePostMap[job] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRemovePost ( KJob* ) ) );
        job->start();
    }
}

void TwitterApiMicroBlog::slotRemovePost ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    Choqok::Post *post = mRemovePostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit errorPost ( theAccount, post, CommunicationError,
                         i18n("Removing the post failed. %1", job->errorString() ), MicroBlog::Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
        QString errMsg = checkXmlForError(stj->data());
        if( errMsg.isEmpty() ){
            emit postRemoved ( theAccount, post );
        } else {
            kError()<<"Server error on removing post: "<<errMsg;
            emit errorPost ( theAccount, post, ServerError,
                             i18n("Removing the post failed. %1", errMsg ), MicroBlog::Critical );
        }
    }
}

void TwitterApiMicroBlog::createFavorite ( Choqok::Account* theAccount, const QString &postId )
{
    kDebug();
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath ( "/favorites/create/" + postId + ".xml" );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "The Favorite creation failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    mFavoriteMap[job] = postId;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreateFavorite ( KJob* ) ) );
    job->start();
}

void TwitterApiMicroBlog::slotCreateFavorite ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString postId = mFavoriteMap.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, CommunicationError, i18n( "Favorite creation failed. %1", job->errorString() ) );
    } else {
        emit favoriteCreated ( theAccount, postId );
    }
}

void TwitterApiMicroBlog::removeFavorite ( Choqok::Account* theAccount, const QString& postId )
{
    kDebug();
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath ( "/favorites/destroy/" + postId + ".xml" );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "Removing the favorite failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    mFavoriteMap[job] = postId;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRemoveFavorite ( KJob* ) ) );
    job->start();
}

void TwitterApiMicroBlog::slotRemoveFavorite ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    QString id = mFavoriteMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, CommunicationError, i18n("Removing the favorite failed. %1", job->errorString() ) );
    } else {
        emit favoriteRemoved ( theAccount, id );
    }
}

void TwitterApiMicroBlog::listFriendsUsername(TwitterApiAccount* theAccount)
{
    d->friendsList.clear();
    requestFriendsScreenName(theAccount);
}

void TwitterApiMicroBlog::requestFriendsScreenName(TwitterApiAccount* theAccount, int page)
{
    kDebug();
    KUrl url = apiUrl( theAccount );
    url.addPath( "/statuses/friends/" + theAccount->username() + ".xml" );
    url.setQuery( "?page=" + QString::number( page ) );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
        return;
    }
    mJobsAccount[job] = theAccount;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestFriendsScreenName(KJob*) ) );
    job->start();
}

void TwitterApiMicroBlog::slotRequestFriendsScreenName(KJob* job)
{
    kDebug();
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>( mJobsAccount.take(job) );
    KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
    QStringList newList = readUsersScreenNameFromXml( theAccount, stJob->data() );
    d->friendsList << newList;
    if ( newList.count() == 100 ) {
        requestFriendsScreenName( theAccount, ++d->friendsPage );
    } else {
        d->friendsList.removeDuplicates();
        theAccount->setFriendsList(d->friendsList);
        emit friendsUsernameListed( theAccount, d->friendsList );
    }
}

void TwitterApiMicroBlog::updateTimelines (Choqok::Account* theAccount)
{
    kDebug();
    foreach ( const QString &tm, theAccount->timelineNames() ) {
        requestTimeLine ( theAccount, tm, mTimelineLatestId[theAccount][tm] );
    }
}

void TwitterApiMicroBlog::requestTimeLine ( Choqok::Account* theAccount, QString type,
                                            QString latestStatusId, int page, QString maxId )
{
    kDebug();
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath ( timelineApiPath[type] );
    int countOfPost = Choqok::BehaviorSettings::countOfPosts();
    if ( !latestStatusId.isEmpty() ) {
        url.addQueryItem ( "since_id", latestStatusId );
        countOfPost = 200;
    }

    url.addQueryItem ( "count", QString::number( countOfPost ) );
    if ( !maxId.isEmpty() ) {
        url.addQueryItem ( "max_id", maxId );
    }
    if ( page ) {
        url.addQueryItem ( "page", QString::number ( page ) );
    }
    kDebug() << "Latest " << type << " Id: " << latestStatusId;

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Cannot create an http GET request. Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg, Low );
        return;
    }
    mRequestTimelineMap[job] = type;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRequestTimeline ( KJob* ) ) );
    job->start();
}

void TwitterApiMicroBlog::slotRequestTimeline ( KJob *job )
{
    kDebug();//TODO Add error detection for XML "checkXmlForError()"
    if ( !job ) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error( theAccount, CommunicationError,
                    i18n("Timeline update failed, %1", job->errorString()), Low );
        return;
    }
    QString type = mRequestTimelineMap.take(job);
    if( isValidTimeline(type) ) {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob*>( job );
        QList<Choqok::Post*> list;
        if( type=="Inbox" || type=="Outbox" ) {
            list = readDMessagesFromXml( theAccount, j->data() );
        } else {
            list = readTimelineFromXml( theAccount, j->data() );
        }
        if(!list.isEmpty()) {
            mTimelineLatestId[theAccount][type] = list.last()->postId;
            emit timelineDataReceived( theAccount, type, list );
        }
    }
}

KUrl TwitterApiMicroBlog::apiUrl ( TwitterApiAccount* theAccount )
{
    if(theAccount) {
        KUrl url( theAccount->apiUrl() );
        url.setScheme ( theAccount->useSecureConnection() ? "https" : "http" );
        url.setUser ( theAccount->username() );
        url.setPass ( theAccount->password() );
        return url;
    }
    return KUrl();
}

Choqok::Post * TwitterApiMicroBlog::readPostFromXml ( Choqok::Account* theAccount,
                                                      const QByteArray& buffer, Choqok::Post* post /*= 0*/ )
{
    QDomDocument document;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( !root.isNull() ) {
        return readPostFromDomElement ( theAccount, root.toElement(), post );
    } else {
        if(!post)
            post = new Choqok::Post;
        post->isError = true;
        return post;
    }
}

Choqok::Post * TwitterApiMicroBlog::readPostFromDomElement ( Choqok::Account* theAccount,
                                                             const QDomElement &root, Choqok::Post* post/* = 0*/ )
{
    if(!post)
        post = new Choqok::Post;

    if ( root.tagName() != "status" ) {
        kDebug() << "there's no status tag in XML, Error!!\t"
        <<"Tag is: "<<root.tagName();
        post->isError = true ;
        return post;
    }
    QDomNode node2 = root.firstChild();
    QString timeStr;
    while ( !node2.isNull() ) {
        if ( node2.toElement().tagName() == "created_at" )
            timeStr = node2.toElement().text();
        else if ( node2.toElement().tagName() == "text" )
            post->content = node2.toElement().text();
        else if ( node2.toElement().tagName() == "id" )
            post->postId = node2.toElement().text();
        else if ( node2.toElement().tagName() == "in_reply_to_status_id" )
            post->replyToPostId = node2.toElement().text();
        else if ( node2.toElement().tagName() == "in_reply_to_user_id" )
            post->replyToUserId = node2.toElement().text();
        else if ( node2.toElement().tagName() == "in_reply_to_screen_name" )
            post->replyToUserName = node2.toElement().text();
        else if ( node2.toElement().tagName() == "source" )
            post->source = node2.toElement().text();
        else if ( node2.toElement().tagName() == "favorited" )
            post->isFavorited = ( node2.toElement().text() == "true" ) ? true : false;
        else if ( node2.toElement().tagName() == "user" ) {
            QDomNode node3 = node2.firstChild();
            while ( !node3.isNull() ) {
                if ( node3.toElement().tagName() == "screen_name" ) {
                    post->author.userName = node3.toElement().text();
                } else if ( node3.toElement().tagName() == "profile_image_url" ) {
                    post->author.profileImageUrl = node3.toElement().text();
                } else if ( node3.toElement().tagName() == "id" ) {
                    post->author.userId = node3.toElement().text();
                } else if ( node3.toElement().tagName() == "name" ) {
                    post->author.realName = node3.toElement().text();
                } else if ( node3.toElement().tagName() == QString ( "description" ) ) {
                    post->author.description = node3.toElement().text();
                }
                node3 = node3.nextSibling();
            }
        }
        node2 = node2.nextSibling();
    }
    post->link = postUrl(theAccount, post->author.userName, post->postId);
    post->creationDateTime = dateFromString ( timeStr );

    return post;
}

QList<Choqok::Post*> TwitterApiMicroBlog::readTimelineFromXml ( Choqok::Account* theAccount,
                                                                const QByteArray &buffer )
{
    QDomDocument document;
    QList<Choqok::Post*> postList;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( root.tagName() != "statuses" ) {
        //         QString err = i18n( "Data returned from server is corrupted." );
        kDebug() << "there's no statuses tag in XML\t the XML is: \n" << buffer;
        return postList;
    }
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
        postList.prepend( readPostFromDomElement ( theAccount, node.toElement() ) );
        node = node.nextSibling();
    }
    return postList;
}

Choqok::Post * TwitterApiMicroBlog::readDMessageFromXml (Choqok::Account *theAccount, const QByteArray &buffer )
{
    QDomDocument document;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( !root.isNull() ) {
        return readDMessageFromDomElement ( theAccount, root.toElement() );
    } else {
        Choqok::Post *post = new Choqok::Post;
        post->isError = true;
        return post;
    }
}

Choqok::Post * TwitterApiMicroBlog::readDMessageFromDomElement ( Choqok::Account* theAccount,
                                                                 const QDomElement& root )
{
    Choqok::Post *msg = new Choqok::Post;

    if ( root.tagName() != "direct_message" ) {
        kDebug() << "there's no direct_message tag in XML, Error!!\t"
        <<"Tag is: "<<root.tagName();
        msg->isError = true;
        return msg;
    }
    QDomNode node2 = root.firstChild();
    msg->isPrivate = true;
    QString senderId, recipientId, timeStr, senderScreenName, recipientScreenName, senderProfileImageUrl,
    senderName, senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;
    while ( !node2.isNull() ) {
        if ( node2.toElement().tagName() == "created_at" )
            timeStr = node2.toElement().text();
        else if ( node2.toElement().tagName() == "text" )
            msg->content = node2.toElement().text();
        else if ( node2.toElement().tagName() == "id" )
            msg->postId = node2.toElement().text();
        else if ( node2.toElement().tagName() == "sender_id" )
            senderId = node2.toElement().text();
        else if ( node2.toElement().tagName() == "recipient_id" )
            recipientId = node2.toElement().text();
        else if ( node2.toElement().tagName() == "sender_screen_name" )
            senderScreenName = node2.toElement().text();
        else if ( node2.toElement().tagName() == "recipient_screen_name" )
            recipientScreenName = node2.toElement().text();
        else if ( node2.toElement().tagName() == "sender" ) {
            QDomNode node3 = node2.firstChild();
            while ( !node3.isNull() ) {
                if ( node3.toElement().tagName() == "profile_image_url" ) {
                    senderProfileImageUrl = node3.toElement().text();
                } else if ( node3.toElement().tagName() == "name" ) {
                    senderName = node3.toElement().text();
                } else if ( node3.toElement().tagName() == "description" ) {
                    senderDescription = node3.toElement().text();
                }
                node3 = node3.nextSibling();
            }
        } else
            if ( node2.toElement().tagName() == "recipient" ) {
                QDomNode node3 = node2.firstChild();
                while ( !node3.isNull() ) {
                    if ( node3.toElement().tagName() == "profile_image_url" ) {
                        recipientProfileImageUrl = node3.toElement().text();
                    } else if ( node3.toElement().tagName() == "name" ) {
                        recipientName = node3.toElement().text();
                    } else if ( node3.toElement().tagName() == "description" ) {
                        recipientDescription = node3.toElement().text();
                    }
                    node3 = node3.nextSibling();
                }
            }
            node2 = node2.nextSibling();
    }
    msg->creationDateTime = dateFromString ( timeStr );
    if ( senderScreenName.compare( theAccount->username(), Qt::CaseInsensitive) == 0 ) {
        msg->author.description = recipientDescription;
        msg->author.userName = recipientScreenName;
        msg->author.profileImageUrl = recipientProfileImageUrl;
        msg->author.realName = recipientName;
        msg->author.userId = recipientId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
    } else {
        msg->author.description = senderDescription;
        msg->author.userName = senderScreenName;
        msg->author.profileImageUrl = senderProfileImageUrl;
        msg->author.realName = senderName;
        msg->author.userId = senderId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
    }
    return msg;
}

QList<Choqok::Post*> TwitterApiMicroBlog::readDMessagesFromXml (Choqok::Account *theAccount,
                                                                const QByteArray &buffer )
{
    QDomDocument document;
    QList<Choqok::Post*> postList;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( root.tagName() != "direct-messages" ) {
        //         QString err = i18n( "Data returned from server is corrupted." );
        kDebug() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
        return postList;
    }
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
        postList.prepend( readDMessageFromDomElement ( theAccount, node.toElement() ) );
        node = node.nextSibling();
    }
    return postList;
}

QStringList TwitterApiMicroBlog::readUsersScreenNameFromXml( Choqok::Account* theAccount, const QByteArray& buffer )
{
    kDebug();
    QStringList list;
    QDomDocument document;
    document.setContent( buffer );
    QDomElement root = document.documentElement();

    if ( root.tagName() != "users" ) {
        QString err = i18n( "Retrieving the friends list failed. The data returned from the server is corrupted." );
        kDebug() << "there's no users tag in XML\t the XML is: \n" << buffer;
        emit error(theAccount, ParsingError, err, Critical);
        return list;
    }
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "user" ) {
            kDebug() << "there's no user tag in XML!\n"<<buffer;
            return list;
        }
        QDomNode node2 = node.firstChild();
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "screen_name" ) {
                list.append( node2.toElement().text() );
                break;
            }
            node2 = node2.nextSibling();
        }
        node = node.nextSibling();
    }
    return list;
}

QDateTime TwitterApiMicroBlog::dateFromString ( const QString &date )
{
    char s[10];
    int year, day, hours, minutes, seconds;
    sscanf( qPrintable ( date ), "%*s %s %d %d:%d:%d %*s %d", s, &day, &hours, &minutes, &seconds, &year );
    int month = d->monthes[s];
    QDateTime recognized ( QDate ( year, month, day ), QTime ( hours, minutes, seconds ) );
    recognized.setTimeSpec( Qt::UTC );
    return recognized.toLocalTime();
}

void TwitterApiMicroBlog::aboutToUnload()
{
    d->countOfTimelinesToSave = 0;
    foreach(Choqok::Account* acc, Choqok::AccountManager::self()->accounts()){
        if(acc->microblog() == this){
            acc->writeConfig();
            d->countOfTimelinesToSave += acc->timelineNames().count();
        }
    }
    emit saveTimelines();
}

void TwitterApiMicroBlog::showDirectMessageDialog( TwitterApiAccount *theAccount/* = 0*/,
                                                   const QString &toUsername/* = QString()*/ )
{
    kDebug();
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    }
    TwitterApiDMessageDialog *dmsg = new TwitterApiDMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    if(!toUsername.isEmpty())
        dmsg->setTo(toUsername);
    dmsg->show();
}

Choqok::TimelineInfo * TwitterApiMicroBlog::timelineInfo(const QString& timelineName)
{
    if( isValidTimeline(timelineName) )
        return mTimelineInfos.value(timelineName);
    else
        return 0;
}

void TwitterApiMicroBlog::showSearchDialog(TwitterApiAccount* theAccount)
{
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    }
    QPointer<TwitterApiSearchDialog> searchDlg = new TwitterApiSearchDialog( theAccount,
                                                                             Choqok::UI::Global::mainWindow() );
    searchDlg->show();
}

void TwitterApiMicroBlog::createFriendship( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath( "/friendships/create/"+ username +".xml" );
    kDebug()<<url;

    KIO::StoredTransferJob *job = KIO::storedHttpPost( QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }

    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCreateFriendship(KJob*) ) );
    job->start();
}

void TwitterApiMicroBlog::slotCreateFriendship(KJob* job)
{
    kDebug();
    if(!job){
        kError()<<"Job is a null Pointer!";
        return;
    }
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount*>( mJobsAccount.take(job) );
    QString username = mFriendshipMap.take(job);
    if(job->error()){
        kDebug()<<"Job Error:"<<job->errorString();
        emit error ( theAccount, CommunicationError,
                     i18n("Creating friendship with %1 failed. %2", username, job->errorString() ) );
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
    Choqok::User *user = readUserInfoFromXml(stj->data());
    if( user /*&& user->userName.compare(username, Qt::CaseInsensitive)*/ ){
        emit friendshipCreated(theAccount, username);
        Choqok::NotifyManager::success( i18n("You are now listening to %1's posts.", username) );
        theAccount->setFriendsList(QStringList());
        listFriendsUsername(theAccount);
    } else {
        QString errorMsg = checkXmlForError(stj->data());
        if( errorMsg.isEmpty() ){
            kDebug()<<"Parse Error: "<<stj->data();
            emit error( theAccount, ParsingError,
                        i18n("Creating friendship with %1 failed: the server returned invalid data.",
                             username ) );
        } else {
            kDebug()<<"Server error: "<<errorMsg;
            emit error( theAccount, ServerError,
                        i18n("Creating friendship with %1 failed: %2",
                            username, errorMsg ) );
        }
    }
}

void TwitterApiMicroBlog::destroyFriendship( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath( "/friendships/destroy/" + username + ".xml" );
    kDebug()<<url;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }

    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotDestroyFriendship(KJob*) ) );
    job->start();
}

void TwitterApiMicroBlog::slotDestroyFriendship(KJob* job)
{
    kDebug();
    if(!job){
        kError()<<"Job is a null Pointer!";
        return;
    }
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount*>( mJobsAccount.take(job) );
    QString username = mFriendshipMap.take(job);
    if(job->error()){
        kDebug()<<"Job Error:"<<job->errorString();
        emit error ( theAccount, CommunicationError,
                     i18n("Destroying friendship with %1 failed. %2", username, job->errorString() ) );
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
    Choqok::User *user = readUserInfoFromXml(stj->data());
    if( user /*&& user->userName.compare( username, Qt::CaseInsensitive )*/ ){
        emit friendshipDestroyed(theAccount, username);
        Choqok::NotifyManager::success( i18n("You will not receive %1's updates.", username) );
        theAccount->setFriendsList(QStringList());
        listFriendsUsername(theAccount);
    } else {
        QString errorMsg = checkXmlForError(stj->data());
        if( errorMsg.isEmpty() ){
            kDebug()<<"Parse Error: "<<stj->data();
            emit error( theAccount, ParsingError,
                        i18n("Destroying friendship with %1 failed: the server returned invalid data.",
                            username ) );
        } else {
            kDebug()<<"Server error: "<<errorMsg;
            emit error( theAccount, ServerError,
                        i18n("Destroying friendship with %1 failed: %2",
                             username, errorMsg ) );
        }
    }
}

void TwitterApiMicroBlog::blockUser( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
    url.addPath( "/blocks/create/"+ username +".xml" );

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }

    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotBlockUser(KJob*) ) );
    job->start();
}

void TwitterApiMicroBlog::slotBlockUser(KJob* job)
{
    kDebug();
    if(!job){
        kError()<<"Job is a null Pointer!";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString username = mFriendshipMap.take(job);
    if(job->error()){
        kDebug()<<"Job Error:"<<job->errorString();
        emit error ( theAccount, CommunicationError,
                     i18n("Blocking %1 failed. %2", username, job->errorString() ) );
        return;
    }
    Choqok::User *user = readUserInfoFromXml(qobject_cast<KIO::StoredTransferJob*>(job)->data());
    if( user /*&& user->userName.compare( username, Qt::CaseInsensitive )*/ ){
        emit userBlocked(theAccount, username);
        Choqok::NotifyManager::success( i18n("Your posts are blocked for %1.", username) );
    } else {
        kDebug()<<"Parse Error: "<<qobject_cast<KIO::StoredTransferJob*>(job)->data();
        emit error( theAccount, ParsingError,
                     i18n("Blocking %1 failed: the server returned invalid data.",
                          username ) );
    }
//     Choqok::User *user = readUserInfoFromXml(); TODO Check for failor!
}

Choqok::User* TwitterApiMicroBlog::readUserInfoFromXml(const QByteArray& buffer)
{
    QDomDocument doc;
    doc.setContent(buffer);

    QDomElement root = doc.documentElement();
    if ( root.tagName() != "user" ) {
        kDebug()<<"There's no user tag in returned document from server! Data is:\n\t"<<buffer;
        return 0;
    }
    QDomNode node = root.firstChild();
    Choqok::User *user = new Choqok::User;
    QString timeStr;
    while( !node.isNull() ){
        QDomElement elm = node.toElement();
        if(elm.tagName() == "name"){
            user->realName = elm.text();
        } else if(elm.tagName() == "screen_name"){
            user->userName = elm.text();
        } else if(elm.tagName() == "location"){
            user->location = elm.text();
        } else if(elm.tagName() == "description"){
            user->description = elm.text();
        } else if(elm.tagName() == "profile_image_url"){
            user->profileImageUrl = elm.text();
        } else if(elm.tagName() == "url") {
            user->homePageUrl = elm.text();
        } else if(elm.tagName() == "followers_count") {
            user->followersCount = elm.text().toUInt();
        } else if( elm.tagName() == "protected" ){
            if(elm.text() == "true"){
                user->isProtected = true;
            }
        }
        node = node.nextSibling();
    }
    return user;
}

QString TwitterApiMicroBlog::checkXmlForError(const QByteArray& buffer)
{
    QDomDocument doc;
    doc.setContent(buffer);
    QDomElement root = doc.documentElement();
    if( root.tagName() == "hash" ){
        QDomNode node = root.firstChild();
        QString errorMessage;
        QString request;
        while( !node.isNull() ){
            QDomElement elm = node.toElement();
            if(elm.tagName() == "error"){
                errorMessage = elm.text();
            } else if(elm.tagName() == "request"){
                request = elm.text();
            }
            node = node.nextSibling();
        }
        kError()<<"Error at request "<<request<<" : "<<errorMessage;
        return errorMessage;
    } else {
        return QString();
    }
}

#include "twitterapimicroblog.moc"
