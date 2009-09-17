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
#include <choqokbehaviorsettings.h>

class TwitterApiMicroBlog::Private
{
public:
    Private():countOfTimelinesToSave(0), friendsPage(1)
    {}
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
    timelineTypes<< "Home" << "Reply" << "Inbox" << "Outbox";
    setTimelineNames(timelineTypes);
    d->monthes["Jan"] = 1;
    d->monthes["Feb"] = 2;
    d->monthes["Mar"] = 3;
    d->monthes["Apr"] = 4;
    d->monthes["May"] = 5;
    d->monthes["Jun"] = 6;
    d->monthes["Jul"] = 7;
    d->monthes["Aug"] = 8;
    d->monthes["Sep"] = 9;
    d->monthes["Oct"] = 10;
    d->monthes["Nov"] = 11;
    d->monthes["Dec"] = 12;
    timelineApiPath["Home"] = "/statuses/friends_timeline.xml";
    timelineApiPath["Reply"] = "/statuses/replies.xml";
    timelineApiPath["Inbox"] = "/direct_messages.xml";
    timelineApiPath["Outbox"] = "/direct_messages/sent.xml";
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
    return menu;
}

QList< Choqok::Post* > TwitterApiMicroBlog::loadTimeline( Choqok::Account *account, const QString& timelineName)
{
    kDebug();
    QString fileName = account->alias() + '_' + timelineName + "_backuprc";
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
    QStringList groupList = postsBackup.groupList();
    groupList.sort();
    QList< Choqok::Post* > list;
    int count = groupList.count();
    if( count ) {
        /**
        Checking if QStringList::sort() failed on sorting numbers,
        This happends when one or more items char count is less/more than other ones
        */
        if(groupList.constBegin()->count() != (--(groupList.constEnd()))->count()) {
            int charCount = groupList[0].count();
            for(int i=1; i<count; ++i){
                int tmp = groupList[i].count();
                if( tmp < charCount ){
                    int k = 0;
                    for(int j = i; j<count; ++j){
                        int tmp2 = groupList[j].count();
                        if( tmp2 == tmp )
                            groupList.move(j, k);
                        else
                            charCount = tmp2;
                    }
                } else
                    charCount = tmp;
            }
        }
        ///END checking
        Choqok::Post *st;
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
    QString fileName = account->alias() + '_' + timelineName + "_backuprc";
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
        grp.writeEntry( "postId", post->postId );
        grp.writeEntry( "text", post->content );
        grp.writeEntry( "source", post->source );
        grp.writeEntry( "inReplyToPostId", post->replyToPostId );
        grp.writeEntry( "inReplyToUserId", post->replyToUserId );
        grp.writeEntry( "favorited", post->isFavorited );
        grp.writeEntry( "inReplyToUserName", post->replyToUserName );
        grp.writeEntry( "authorId", post->author.userId );
        grp.writeEntry( "authorUserName", post->author.userName );
        grp.writeEntry( "authorRealName", post->author.realName );
        grp.writeEntry( "authorProfileImageUrl", post->author.profileImageUrl );
        grp.writeEntry( "authorDescription" , post->author.description );
        grp.writeEntry( "isPrivate" , post->isPrivate );
        grp.writeEntry( "authorLocation" , post->author.location );
        grp.writeEntry( "authorUrl" , post->author.homePageUrl );
    }
    postsBackup.sync();
    --d->countOfTimelinesToSave;
    if(d->countOfTimelinesToSave < 1)
        emit readyForUnload();
}

/*
QMap<QString, QString> timelineInfo ( const QString &timeline )
{
    QMap<QString, QString> res;
    if ( timeline == "home" ) {
        res["title"] = i18n ( "Home" );
        res["description"] = i18n ( "You and your friends" );
    } else if ( timeline == "reply" ) {
        res["title"] = i18n ( "Reply" );
        res["description"] = i18n ( "Reply to you" );
    } else if ( timeline == "inbox" ) {
        res["title"] = i18n ( "Inbox" );
        res["description"] = i18n ( "Your incoming private messages" );
    } else if ( timeline == "outbox" ) {
        res["title"] = i18n ( "Outbox" );
        res["description"] = i18n ( "Private messages you have sent" );
    }
    return res;
}
*/

void TwitterApiMicroBlog::createPost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post || post->content.isEmpty() ) {
        kDebug() << "ERROR: Status text is empty!";
        return;
    }
    if ( !post->isPrivate ) {///Status Update
        KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
        url.addPath ( "/statuses/update.xml" );
        QByteArray data = "status=";
        data += QUrl::toPercentEncoding (  post->content );
        if ( !post->replyToPostId.isEmpty() && post->content.indexOf ( '@' ) > -1 ) {
            data += "&in_reply_to_status_id=";
            data += post->replyToPostId.toLocal8Bit();
        }
        data += "&source=choqok";
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Creating new post failed, Cannot create an http POST request, Check your KDE installation." );
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
//             QString errMsg = i18n ( "Creating new post failed, Cannot create an http POST request, Check your KDE installation." );
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
                         i18n("Creating new post failed, %1", job->errorString()), MicroBlog::Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast< KIO::StoredTransferJob * > ( job );
        if ( !post->isPrivate ) {
            readPostFromXml ( theAccount, stj->data(), post );
            if ( post->isError ) {
                kDebug() << "XML parsing error" ;
                emit errorPost ( theAccount, post, Choqok::MicroBlog::ParsingError,
                                 i18n ( "Creating new post failed, Could not parse result data." ), MicroBlog::Critical );
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
//         QString errMsg = i18n ( "Fetching new post failed, Cannot create an http GET request,"
//                                 "Check your KDE installation." );
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
                     i18n("Fetching new post failed, %1", job->errorString()), Low );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
        readPostFromXml ( theAccount, stj->data(), post );
        if ( post->isError ) {
            kDebug() << "Parsing Error";
            emit errorPost ( theAccount, post, Choqok::MicroBlog::ParsingError,
                             i18n ( "Fetching new post failed, Could not parse result data." ), Low );
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
                         i18n("Removing post failed, %1", job->errorString() ), MicroBlog::Critical );
    } else {
        emit postRemoved ( theAccount, post );
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
//         QString errMsg = i18n ( "Favorite creation failed, Cannot create an http POST request, "
//                                 "Check your KDE installation." );
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
        emit error ( theAccount, CommunicationError, i18n( "Favorite creation failed, %1", job->errorString() ) );
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
//         QString errMsg = i18n ( "Favorite removing failed, Cannot create an http POST request,"
//                                 "Check your KDE installation." );
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
        emit error ( theAccount, CommunicationError, i18n("Favorite removing failed, %1", job->errorString() ) );
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
//         QString errMsg = i18n ( "Cannot create an http GET request, Check your KDE installation." );
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
    kDebug();
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
        if( type=="Home" || type=="Reply" ) {
            list = readTimelineFromXml( theAccount, j->data() );
        } else if( type=="Inbox" || type=="Outbox" ) {
            list = readDMessagesFromXml( theAccount, j->data() );
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
        kDebug() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
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
        QString err = i18n( "Retrieving friends list failed, Data returned from server is corrupted." );
        kDebug() << "there's no users tag in XML\t the XML is: \n" << buffer.data();
        emit error(theAccount, ParsingError, err, Critical);
        return list;
    }
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "user" ) {
            kDebug() << "there's no status tag in XML, maybe there is no new status!";
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
    foreach(const Choqok::Account* acc, Choqok::AccountManager::self()->accounts()){
        if(acc->microblog() == this)
            d->countOfTimelinesToSave += acc->timelineNames().count();
    }
    emit saveTimelines();
}

void TwitterApiMicroBlog::showDirectMessageDialog( TwitterApiAccount *theAccount/* = 0*/,
                                                   const QString &toUsername/* = QString()*/ )
{
    kDebug();
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        Q_ASSERT(act);
        theAccount = qobject_cast<TwitterApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
        Q_ASSERT(theAccount);
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

#include "twitterapimicroblog.moc"
