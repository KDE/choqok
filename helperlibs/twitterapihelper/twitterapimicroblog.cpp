/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <qjson/parser.h>
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
#include "twitterapicomposerwidget.h"
#include <QtOAuth/QtOAuth>
#include <choqokappearancesettings.h>
#include "application.h"

class TwitterApiMicroBlog::Private
{
public:
    Private():countOfTimelinesToSave(0), friendsCursor("-1")
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
    QString friendsCursor;
    QMap<QString, int> monthes;
    QJson::Parser parser;
};

TwitterApiMicroBlog::TwitterApiMicroBlog ( const KComponentData &instance, QObject *parent )
: MicroBlog( instance, parent), d(new Private)
{
    kDebug();
    KConfigGroup grp(KGlobal::config(), "TwitterApi");
    format = grp.readEntry("format", "xml");

    setCharLimit(140);
    QStringList timelineTypes;
    timelineTypes<< "Home" << "Reply" << "Inbox" << "Outbox" << "Favorite" << "ReTweets" << "Public";
    setTimelineNames(timelineTypes);
    timelineApiPath["Home"] = "/statuses/home_timeline.%1";
    timelineApiPath["Reply"] = "/statuses/replies.%1";
    timelineApiPath["Inbox"] = "/direct_messages.%1";
    timelineApiPath["Outbox"] = "/direct_messages/sent.%1";
    timelineApiPath["Favorite"] = "/favorites.%1";
    timelineApiPath["ReTweets"] = "/statuses/retweets_of_me.%1";
    timelineApiPath["Public"] = "/statuses/public_timeline.%1";
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

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "ReTweets");
    t->description = i18nc("Timeline description", "Your posts that were ReTweeted by others");
    t->icon = "folder-red";
    mTimelineInfos["ReTweets"] = t;
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

    KAction *updateFriendsList = new KAction(KIcon("arrow-down"), i18n("Update Friends List"), menu);
    updateFriendsList->setData( theAccount->alias() );
    connect( updateFriendsList, SIGNAL(triggered(bool)), SLOT(slotUpdateFriendsList()) );
    menu->addAction(updateFriendsList);

    return menu;
}

QList< Choqok::Post* > TwitterApiMicroBlog::loadTimeline( Choqok::Account *account,
                                                          const QString& timelineName)
{
    QList< Choqok::Post* > list;
    if(timelineName.compare("Favorite") == 0)
        return list;//NOTE Won't cache favorites, and this is for compatibility with older versions!
    kDebug()<<timelineName;
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
    QStringList tmpList = postsBackup.groupList();

/// to don't load old archives
    if( tmpList.isEmpty() || !(QDateTime::fromString(tmpList.first()).isValid()) )
        return list;
///--------------

    QList<QDateTime> groupList;
    foreach(const QString &str, tmpList)
        groupList.append(QDateTime::fromString(str) );
    qSort(groupList);
    int count = groupList.count();
    if( count ) {
        Choqok::Post *st = 0;
        for ( int i = 0; i < count; ++i ) {
            st = new Choqok::Post;
            KConfigGroup grp( &postsBackup, groupList[i].toString() );
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
            st->author.isProtected = grp.readEntry("isProtected", false);
            st->isPrivate = grp.readEntry( "isPrivate" , false );
            st->author.location = grp.readEntry("authorLocation", QString());
            st->author.homePageUrl = grp.readEntry("authorUrl", QString());
            st->link = postUrl( account, st->author.userName, st->postId);
            st->isRead = grp.readEntry("isRead", true);
            st->repeatedFromUsername = grp.readEntry("repeatedFrom", QString());
            st->repeatedPostId = grp.readEntry("repeatedPostId", QString());
            st->conversationId = grp.readEntry("conversationId", QString());

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
    if(timelineName.compare("Favorite") != 0) {
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
            const Choqok::Post *post = ((*it)->currentPost());
            KConfigGroup grp( &postsBackup, post->creationDateTime.toString() );
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
            grp.writeEntry( "isProtected" , post->author.isProtected );
            grp.writeEntry( "authorUrl" , post->author.homePageUrl );
            grp.writeEntry( "isRead" , post->isRead );
            grp.writeEntry( "repeatedFrom", post->repeatedFromUsername);
            grp.writeEntry( "repeatedPostId", post->repeatedPostId.toString());
            grp.writeEntry( "conversationId", post->conversationId.toString() );
        }
        postsBackup.sync();
    }
    if(Choqok::Application::isShuttingDown()) {
        --d->countOfTimelinesToSave;
        if(d->countOfTimelinesToSave < 1)
            emit readyForUnload();
    }
}

Choqok::UI::ComposerWidget* TwitterApiMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new TwitterApiComposerWidget(account, parent);
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
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    QByteArray data;
    QOAuth::ParamMap params;
    if ( !post || post->content.isEmpty() ) {
        kDebug() << "ERROR: Status text is empty!";
        emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError,
                         i18n ( "Creating the new post failed. Text is empty." ), MicroBlog::Critical );
        return;
    }
    if ( !post->isPrivate ) {///Status Update
        KUrl url = account->apiUrl();
        url.addPath ( QString("/statuses/update.%1").arg(format) );
        params.insert("status", QUrl::toPercentEncoding (  post->content ));
        if(!post->replyToPostId.isEmpty())
            params.insert("in_reply_to_status_id", post->replyToPostId.toLocal8Bit());
        data = "status=";
        data += QUrl::toPercentEncoding (  post->content );
        if ( !post->replyToPostId.isEmpty() ) {
            data += "&in_reply_to_status_id=";
            data += post->replyToPostId.toLocal8Bit();
        }
        if( !account->usingOAuth() )
            data += "&source=Choqok";
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST, params));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
    } else {///Direct message
        QString recipientScreenName = post->replyToUserName;
        KUrl url = account->apiUrl();
        url.addPath ( QString("/direct_messages/new.%1").arg(format) );
        params.insert("user", recipientScreenName.toLocal8Bit());
        params.insert("text", QUrl::toPercentEncoding ( post->content ));
        data = "user=";
        data += recipientScreenName.toLocal8Bit();
        data += "&text=";
        data += QUrl::toPercentEncoding ( post->content );
        if( !account->usingOAuth() )
            data += "&source=Choqok";
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Creating the new post failed. Cannot create an http POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST, params));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
    }
}

void TwitterApiMicroBlog::repeatPost(Choqok::Account* theAccount, const ChoqokId& postId)
{
    kDebug();
    if ( postId.isEmpty() ) {
        kError() << "ERROR: PostId is empty!";
        return;
    }
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( QString("/statuses/retweet/%1.%2").arg(postId).arg(format) );
    QByteArray data;
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
    Choqok::Post *post = new Choqok::Post;
    post->postId = postId;
    mCreatePostMap[ job ] = post;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
    job->start();
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
            if(format=="xml"){
                readPostFromXml ( theAccount, stj->data(), post );
            } else {
                readPostFromJson ( theAccount, stj->data(), post );
            }
            if ( post->isError ) {
                QString errorMsg;
                if(format == "json")
                    errorMsg = checkJsonForError(stj->data());
                else
                    errorMsg = checkXmlForError(stj->data());
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
                Choqok::NotifyManager::success(i18n("New post submitted successfully"));
                emit postCreated ( theAccount, post );
            }
        } else {
            Choqok::NotifyManager::success(i18n("Private message sent successfully"));
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
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( QString("/statuses/show/%1.%2").arg(post->postId).arg(format) );

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Fetching the new post failed. Cannot create an HTTP GET request."
//                                 "Please check your KDE installation." );
//         emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, Low );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::GET));
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
        if(format=="json"){
                readPostFromJson ( theAccount, stj->data(), post );
            } else {
                readPostFromXml ( theAccount, stj->data(), post );
            }
        if ( post->isError ) {
                QString errorMsg;
                if(format == "json")
                    errorMsg = checkJsonForError(stj->data());
                else
                    errorMsg = checkXmlForError(stj->data());
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
        TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
        KUrl url = account->apiUrl();
        if ( !post->isPrivate ) {
            url.addPath ( "/statuses/destroy/" + post->postId + ".xml" );
        } else {
            url.addPath ( "/direct_messages/destroy/" + post->postId + ".xml" );
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Removing the post failed. Cannot create an HTTP POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
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
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( "/favorites/create/" + postId + ".xml" );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "The Favorite creation failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
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
        KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
        QString err = checkXmlForError(stJob->data());
        if( !err.isEmpty() ){
            emit error(theAccount, ServerError, err, Critical);
            return;
        } else {
            emit favoriteCreated ( theAccount, postId );
        }
    }
}

void TwitterApiMicroBlog::removeFavorite ( Choqok::Account* theAccount, const QString& postId )
{
    kDebug();
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( "/favorites/destroy/" + postId + ".xml" );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "Removing the favorite failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
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
        KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
        QString err = checkXmlForError(stJob->data());
        if( !err.isEmpty() ){
            emit error(theAccount, ServerError, err, Critical);
            return;
        } else {
            emit favoriteRemoved ( theAccount, id );
        }
    }
}

void TwitterApiMicroBlog::listFriendsUsername(TwitterApiAccount* theAccount)
{
    friendsList.clear();
    if ( theAccount ) {
        requestFriendsScreenName(theAccount);
    }
}

void TwitterApiMicroBlog::requestFriendsScreenName(TwitterApiAccount* theAccount)
{
    kDebug();
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( QString("/statuses/friends.xml") );
    KUrl tmpUrl(url);
    url.addQueryItem( "cursor", d->friendsCursor );
    QOAuth::ParamMap params;
    params.insert("cursor", d->friendsCursor.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, tmpUrl,
                                                                                 QOAuth::GET, params));
    mJobsAccount[job] = theAccount;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestFriendsScreenName(KJob*) ) );
    job->start();
    Choqok::UI::Global::mainWindow()->showStatusMessage( i18n("Updating friends list for account %1 ...", theAccount->username()) );
}

void TwitterApiMicroBlog::slotRequestFriendsScreenName(KJob* job)
{
    kDebug();
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>( mJobsAccount.take(job) );
    KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
    if (stJob->error()) {
        emit error(theAccount, ServerError, i18n("Friends list for account %1 could not be updated:\n%2",
            theAccount->username(), stJob->errorString()), Critical);
        return;
    }
    QStringList newList;
    newList = readUsersScreenNameFromXml( theAccount, stJob->data() );
    friendsList << newList;
    if ( newList.count() == 100 ) {
        requestFriendsScreenName( theAccount );
    } else {
        friendsList.removeDuplicates();
        theAccount->setFriendsList(friendsList);
        Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Friends list for account %1 has been updated.",
            theAccount->username()) );
        emit friendsUsernameListed( theAccount, friendsList );
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
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( timelineApiPath[type].arg(format) );
    KUrl tmpUrl(url);
    int countOfPost = Choqok::BehaviorSettings::countOfPosts();

    QOAuth::ParamMap params;
    if( account->usingOAuth() ){
        if ( !latestStatusId.isEmpty() ) {
            params.insert ( "since_id", latestStatusId.toLatin1() );
            countOfPost = 200;
        }
        params.insert ( "count", QByteArray::number( countOfPost ) );
        if ( !maxId.isEmpty() ) {
            params.insert ( "max_id", maxId.toLatin1() );
        }
        if ( page ) {
            params.insert ( "page", QByteArray::number ( page ) );
        }
    }
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
    kDebug() << "Latest " << type << " Id: " << latestStatusId;// << " apiReq: " << url;

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Cannot create an http GET request. Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg, Low );
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, tmpUrl, QOAuth::GET, params));
    mRequestTimelineMap[job] = type;
    mJobsAccount[job] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotRequestTimeline ( KJob* ) ) );
    job->start();
}

void TwitterApiMicroBlog::slotRequestTimeline ( KJob *job )
{
    kDebug();//TODO Add error detection for XML "checkXmlForError()" and JSON
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
            if(format=="json"){
                list = readDMessagesFromJson( theAccount, j->data() );
            } else {
                list = readDMessagesFromXml( theAccount, j->data() );
            }
        } else {
            if(format=="json"){
                list = readTimelineFromJson( theAccount, j->data() );
            } else {
                list = readTimelineFromXml( theAccount, j->data() );
            }
        }
        if(!list.isEmpty()) {
            mTimelineLatestId[theAccount][type] = list.last()->postId;
            emit timelineDataReceived( theAccount, type, list );
        }
    }
}

QByteArray TwitterApiMicroBlog::authorizationHeader(TwitterApiAccount* theAccount, const KUrl &requestUrl,
                                                    QOAuth::HttpMethod method, QOAuth::ParamMap params)
{
    QByteArray auth;
    if(theAccount->usingOAuth()){
        auth = theAccount->oauthInterface()->createParametersString( requestUrl.url(), method, theAccount->oauthToken(),
                                                             theAccount->oauthTokenSecret(), QOAuth::HMAC_SHA1,
                                                             params, QOAuth::ParseForHeaderArguments );
    } else {
        auth = theAccount->username().toUtf8() + ':' + theAccount->password().toUtf8();
        auth = auth.toBase64().prepend( "Basic " );
    }
    return auth;
}

Choqok::Post * TwitterApiMicroBlog::readPostFromXml ( Choqok::Account* theAccount,
                                                      const QByteArray& buffer, Choqok::Post* post )
{
    QDomDocument document;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( !root.isNull() ) {
        return readPostFromDomElement ( theAccount, root.toElement(), post );
    } else {
        if(!post){
            kError()<<"TwitterApiMicroBlog::readPostFromXml: post is NULL!";
            post = new Choqok::Post;
        }
        QString err = checkXmlForError(buffer);
        if(!err.isEmpty())
            Q_EMIT error(theAccount, ServerError, err);
        post->isError = true;
        return post;
    }
}

Choqok::Post * TwitterApiMicroBlog::readPostFromDomElement ( Choqok::Account* theAccount,
                                                             const QDomElement &root, Choqok::Post* post )
{
    if(!post){
        kError()<<"TwitterApiMicroBlog::readPostFromDomElement: post is NULL!";
        return 0;
    }

    if ( root.tagName() != "status" ) {
        kDebug() << "there's no status tag in XML, Error!!\t"
        <<"Tag is: "<<root.tagName();
        post->isError = true ;
        return post;
    }
    QDomNode node2 = root.firstChild();

    return readPostFromDomNode(theAccount, node2, post);;
}

Choqok::Post* TwitterApiMicroBlog::readPostFromDomNode(Choqok::Account* theAccount,
                                                       QDomNode node, Choqok::Post* post)
{
    Choqok::Post* repeatedPost = 0;
    while ( !node.isNull() ) {
        QDomElement elm = node.toElement();
        if ( elm.tagName() == "created_at" )
            post->creationDateTime = dateFromString ( elm.text() );
        else if ( elm.tagName() == "text" )
            post->content = elm.text();
        else if ( elm.tagName() == "id" )
            post->postId = elm.text();
        else if ( elm.tagName() == "in_reply_to_status_id" )
            post->replyToPostId = elm.text();
        else if ( elm.tagName() == "in_reply_to_user_id" )
            post->replyToUserId = elm.text();
        else if ( elm.tagName() == "in_reply_to_screen_name" )
            post->replyToUserName = elm.text();
        else if ( elm.tagName() == "source" )
            post->source = elm.text();
        else if ( elm.tagName() == "favorited" )
            post->isFavorited = ( elm.text() == "true" ) ? true : false;
        else if ( elm.tagName() == "statusnet:conversation_id" )
            post->conversationId = elm.text();
        else if ( elm.tagName() == "user" ) {
            QDomNode node3 = node.firstChild();
            while ( !node3.isNull() ) {
                QDomElement elm3 = node3.toElement();
                if ( elm3.tagName() == "screen_name" ) {
                    post->author.userName = elm3.text();
                } else if ( elm3.tagName() == "profile_image_url" ) {
                    post->author.profileImageUrl = elm3.text();
                } else if ( elm3.tagName() == "id" ) {
                    post->author.userId = elm3.text();
                } else if ( elm3.tagName() == "name" ) {
                    post->author.realName = elm3.text();
                } else if ( elm3.tagName() == QString ( "description" ) ) {
                    post->author.description = elm3.text();
                } else if ( elm3.tagName() == "statusnet:profile_url" ) {
                    post->author.homePageUrl = elm3.text();
                } else if ( elm3.tagName() == "protected" ) {
                    post->author.isProtected = elm3.text().contains("true") ? true : false;
                }
                node3 = node3.nextSibling();
            }
        } else if ( elm.tagName() == "retweeted_status" )
            repeatedPost = readPostFromDomNode( theAccount, elm.firstChild(), new Choqok::Post);

        node = node.nextSibling();
    }
    if(repeatedPost){
        setRepeatedOfInfo(post,repeatedPost);
        delete repeatedPost;
    }
    post->link = postUrl(theAccount, post->author.userName, post->postId);
    post->isRead = post->isFavorited || (post->repeatedFromUsername.compare(theAccount->username(), Qt::CaseInsensitive) == 0);
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
        QString err = checkXmlForError(buffer);
        if(!err.isEmpty())
            Q_EMIT error(theAccount, ServerError, err);
        return postList;
    }
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
        postList.prepend( readPostFromDomElement ( theAccount, node.toElement(), new Choqok::Post ) );
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
        QString err = checkXmlForError(buffer);
        if(!err.isEmpty())
            Q_EMIT error(theAccount, ServerError, err);
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
        QDomElement elm = node2.toElement();
        if ( elm.tagName() == "created_at" )
            timeStr = elm.text();
        else if ( elm.tagName() == "text" )
            msg->content = elm.text();
        else if ( elm.tagName() == "id" )
            msg->postId = elm.text();
        else if ( elm.tagName() == "sender_id" )
            senderId = elm.text();
        else if ( elm.tagName() == "recipient_id" )
            recipientId = elm.text();
        else if ( elm.tagName() == "sender_screen_name" )
            senderScreenName = elm.text();
        else if ( elm.tagName() == "recipient_screen_name" )
            recipientScreenName = elm.text();
        else if ( elm.tagName() == "sender" ) {
            QDomNode node3 = node2.firstChild();
            while ( !node3.isNull() ) {
                QDomElement elm3 = node3.toElement();
                if ( elm3.tagName() == "profile_image_url" ) {
                    senderProfileImageUrl = elm3.text();
                } else if ( elm3.tagName() == "name" ) {
                    senderName = elm3.text();
                } else if ( elm3.tagName() == "description" ) {
                    senderDescription = elm3.text();
                }
                node3 = node3.nextSibling();
            }
        } else
            if ( elm.tagName() == "recipient" ) {
                QDomNode node3 = node2.firstChild();
                while ( !node3.isNull() ) {
                    QDomElement elm3 = node3.toElement();
                    if ( elm3.tagName() == "profile_image_url" ) {
                        recipientProfileImageUrl = elm3.text();
                    } else if ( elm3.tagName() == "name" ) {
                        recipientName = elm3.text();
                    } else if ( elm3.tagName() == "description" ) {
                        recipientDescription = elm3.text();
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
        msg->isRead = true;
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
        QString err = checkXmlForError(buffer);
        if(!err.isEmpty())
            Q_EMIT error(theAccount, ServerError, err);
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

    if ( root.tagName() != "users_list" ) {
        QString err = checkXmlForError(buffer);
        if(!err.isEmpty()){
            emit error(theAccount, ServerError, err, Critical);
        } else {
            err = i18n( "Retrieving the friends list failed. The data returned from the server is corrupted." );
            kDebug() << "there's no users tag in XML\t the XML is: \n" << buffer;
            emit error(theAccount, ParsingError, err, Critical);
            list<<QString(' ');
        }
        return list;
    }
    QDomNode section = root.firstChild();
    QDomNode node = section.firstChild();
    while ( !section.isNull() ) {
        if ( section.toElement().tagName() == "users" ) {
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
        } else if ( section.toElement().tagName() == "next_cursor" ) {
            d->friendsCursor = section.toElement().text();
        }
        section = section.nextSibling();
    }
    return list;
}

void TwitterApiMicroBlog::setRepeatedOfInfo(Choqok::Post* post, Choqok::Post* repeatedPost)
{
    if( Choqok::AppearanceSettings::showRetweetsInChoqokWay() ) {
        post->content = repeatedPost->content;
        post->replyToPostId = repeatedPost->replyToPostId;
        post->replyToUserId = repeatedPost->replyToUserId;
        post->replyToUserName = repeatedPost->replyToUserName;
        post->repeatedFromUsername = repeatedPost->author.userName;
        post->repeatedPostId = repeatedPost->postId;
    } else {
        post->content = repeatedPost->content;
        post->replyToPostId = repeatedPost->replyToPostId;
        post->replyToUserId = repeatedPost->replyToUserId;
        post->replyToUserName = repeatedPost->replyToUserName;
        post->repeatedFromUsername = post->author.userName;
        post->author = repeatedPost->author;
        post->repeatedPostId = repeatedPost->postId;
    }
    post->creationDateTime = repeatedPost->creationDateTime;
}

QDateTime TwitterApiMicroBlog::dateFromString ( const QString &date )
{
    char s[10];
    int year, day, hours, minutes, seconds, tz;
    sscanf( qPrintable ( date ), "%*s %s %d %d:%d:%d %d %d", s, &day, &hours, &minutes, &seconds, &tz, &year );
    int month = d->monthes[s];
    QDateTime recognized ( QDate ( year, month, day ), QTime ( hours, minutes, seconds ) );
    if(tz == 0)//tz is the timezone, in Twitter it's always UTC(0) in Identica it's local +/-NUMBER
        recognized.setTimeSpec( Qt::UTC );
    return recognized.toLocalTime();
}

void TwitterApiMicroBlog::aboutToUnload()
{
    d->countOfTimelinesToSave = 0;
    foreach(Choqok::Account* acc, Choqok::AccountManager::self()->accounts()){
        if(acc->microblog() == this){
//             acc->writeConfig();
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

void TwitterApiMicroBlog::slotUpdateFriendsList()
{
    KAction *act = qobject_cast<KAction *>(sender());
    TwitterApiAccount* theAccount = qobject_cast<TwitterApiAccount*>(
                                        Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    listFriendsUsername(theAccount);
}

void TwitterApiMicroBlog::createFriendship( Choqok::Account *theAccount, const QString& username )
{
    kDebug();
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( "/friendships/create/"+ username +".xml" );
    kDebug()<<url;

    KIO::StoredTransferJob *job = KIO::storedHttpPost( QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
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
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( "/friendships/destroy/" + username + ".xml" );
    kDebug()<<url;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
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
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( "/blocks/create/"+ username +".xml" );

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
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
        Choqok::NotifyManager::success( i18n("You will no longer be disturbed by %1.", username) );
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

///===================================================================

QJson::Parser* TwitterApiMicroBlog::jsonParser()
{
    return &d->parser;
}

QString TwitterApiMicroBlog::checkJsonForError(const QByteArray& buffer)
{
    bool ok;
    QVariantMap map = d->parser.parse(buffer, &ok).toMap();
    if(ok && map.contains("error")){
        kError()<<"Error at request "<<map.value("request").toString()<<" : "<<map.value("error").toString();
        return map.value("error").toString();
    }
    return QString();
}

QList< Choqok::Post* > TwitterApiMicroBlog::readTimelineFromJson(Choqok::Account* theAccount,
                                                                 const QByteArray& buffer)
{
    QList<Choqok::Post*> postList;
    bool ok;
    QVariantList list = d->parser.parse(buffer, &ok).toList();

    if ( ok ) {
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for(; it != endIt; ++it){
            postList.prepend(readPostFromJsonMap(theAccount, it->toMap(), new Choqok::Post));
        }
    } else {
        QString err = checkJsonForError(buffer);
        if(err.isEmpty()){
            kError() << "JSON parsing failed.\nBuffer was: \n" << buffer;
            emit error(theAccount, ParsingError, i18n("Could not parse the data that has been received from the server."));
        } else {
            Q_EMIT error(theAccount, ServerError, err);
        }
        return postList;
    }
    return postList;
}

Choqok::Post* TwitterApiMicroBlog::readPostFromJson(Choqok::Account* theAccount,
                                                    const QByteArray& buffer,
                                                    Choqok::Post* post)
{
    bool ok;
    QVariantMap map = d->parser.parse(buffer, &ok).toMap();

    if ( ok ) {
        return readPostFromJsonMap ( theAccount, map, post );
    } else {
        if(!post){
            kError()<<"TwitterApiMicroBlog::readPostFromXml: post is NULL!";
            post = new Choqok::Post;
        }
        emit errorPost(theAccount, post, ParsingError, i18n("Could not parse the data that has been received from the server."));
        kError()<<"JSon parsing failed. Buffer was:"<<buffer;
        post->isError = true;
        return post;
    }
}

Choqok::Post* TwitterApiMicroBlog::readPostFromJsonMap(Choqok::Account* theAccount,
                                                       const QVariantMap& var,
                                                       Choqok::Post* post)
{
    if(!post){
        kError()<<"TwitterApiMicroBlog::readPostFromJsonMap: post is NULL!";
        return 0;
    }
    post->content = var["text"].toString();
    post->creationDateTime = dateFromString(var["created_at"].toString());
    post->isFavorited = var["favorited"].toBool();
    post->postId = var["id"].toString();
    post->replyToPostId = var["in_reply_to_status_id"].toString();
    post->replyToUserId = var["in_reply_to_user_id"].toString();
    post->replyToUserName = var["in_reply_to_screen_name"].toString();
    post->source = var["source"].toString();
    QVariantMap userMap = var["user"].toMap();
    post->author.description = userMap["description"].toString();
    post->author.realName = userMap["name"].toString();
    post->author.userId = userMap["id"].toString();
    post->author.userName = userMap["screen_name"].toString();
    post->author.profileImageUrl = userMap["profile_image_url"].toString();
    post->author.homePageUrl = userMap["statusnet_profile_url"].toString();
    Choqok::Post* repeatedPost = 0;
    QVariantMap retweetedMap = var["retweeted_status"].toMap();
    if( !retweetedMap.isEmpty() ){
        repeatedPost = readPostFromJsonMap( theAccount, retweetedMap, new Choqok::Post);
        setRepeatedOfInfo(post, repeatedPost);
        delete repeatedPost;
    }
    post->link = postUrl(theAccount, post->author.userName, post->postId);
    post->isRead = post->isFavorited || (post->repeatedFromUsername.compare(theAccount->username(), Qt::CaseInsensitive) == 0);
    return post;
}

QList< Choqok::Post* > TwitterApiMicroBlog::readDMessagesFromJson(Choqok::Account* theAccount,
                                                                  const QByteArray& buffer)
{
    QList<Choqok::Post*> postList;
    bool ok;
    QVariantList list = d->parser.parse(buffer, &ok).toList();

    if ( ok ) {
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for(; it != endIt; ++it){
            postList.prepend(readDMessageFromJsonMap(theAccount, it->toMap()));
        }
    } else {
        QString err = checkJsonForError(buffer);
        if(err.isEmpty()){
            kError() << "JSON parsing failed.\nBuffer was: \n" << buffer;
            emit error(theAccount, ParsingError, i18n("Could not parse the data that has been received from the server."));
        } else {
            Q_EMIT error(theAccount, ServerError, err);
        }
        return postList;
    }
    return postList;
}

Choqok::Post* TwitterApiMicroBlog::readDMessageFromJson(Choqok::Account* theAccount,
                                                        const QByteArray& buffer)
{
    bool ok;
    QVariantMap map = d->parser.parse(buffer, &ok).toMap();

    if ( ok ) {
        return readDMessageFromJsonMap ( theAccount, map );
    } else {
        Choqok::Post *post = new Choqok::Post;
        post->isError = true;
        return post;
    }
}

Choqok::Post* TwitterApiMicroBlog::readDMessageFromJsonMap(Choqok::Account* theAccount,
                                                           const QVariantMap& var)
{
    Choqok::Post *msg = new Choqok::Post;

    msg->isPrivate = true;
    QString senderId, recipientId, timeStr, senderScreenName, recipientScreenName, senderProfileImageUrl,
    senderName, senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;

    msg->creationDateTime = dateFromString ( var["created_at"].toString() );
    msg->content = var["text"].toString();
    msg->postId = var["id"].toString();;
    senderId = var["sender_id"].toString();
    recipientId = var["recipient_id"].toString();
    senderScreenName = var["sender_screen_name"].toString();
    recipientScreenName = var["recipient_screen_name"].toString();
    QVariantMap sender = var["sender"].toMap();
    senderProfileImageUrl = sender["profile_image_url"].toString();
    senderName = sender["name"].toString();
    senderDescription = sender["description"].toString();
    QVariantMap recipient = var["recipient"].toMap();
    recipientProfileImageUrl = recipient["profile_image_url"].toString();
    recipientName = recipient["name"].toString();
    recipientDescription = recipient["description"].toString();
    if ( senderScreenName.compare( theAccount->username(), Qt::CaseInsensitive) == 0 ) {
        msg->author.description = recipientDescription;
        msg->author.userName = recipientScreenName;
        msg->author.profileImageUrl = recipientProfileImageUrl;
        msg->author.realName = recipientName;
        msg->author.userId = recipientId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
        msg->isRead = true;
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

Choqok::User* TwitterApiMicroBlog::readUserInfoFromJson(const QByteArray& buffer)
{
    kError()<<"TwitterApiMicroBlog::readUserInfoFromJson: NOT IMPLEMENTED YET!";
    Q_UNUSED(buffer);
    return 0;
}

QStringList TwitterApiMicroBlog::readUsersScreenNameFromJson(Choqok::Account* theAccount,
                                                             const QByteArray& buffer)
{
    QStringList list;
    bool ok;
    QVariantList jsonList = d->parser.parse(buffer, &ok).toList();

    if ( ok ) {
        QVariantList::const_iterator it = jsonList.constBegin();
        QVariantList::const_iterator endIt = jsonList.constEnd();
        for(; it!=endIt; ++it){
            list<<it->toMap()["screen_name"].toString();
        }
    } else {
        QString err = i18n( "Retrieving the friends list failed. The data returned from the server is corrupted." );
        kDebug() << "JSON parse error: the buffer is: \n" << buffer;
        emit error(theAccount, ParsingError, err, Critical);
    }
    return list;
}

Choqok::User TwitterApiMicroBlog::readUserFromJsonMap(Choqok::Account* theAccount, const QVariantMap& map)
{
    Q_UNUSED(theAccount);
    Choqok::User u;
    u.description = map["description"].toString();
    u.followersCount = map["followers_count"].toUInt();
    u.homePageUrl = map["url"].toString();
    u.isProtected = map["protected"].toBool();
    u.location = map["location"].toString();
    u.profileImageUrl = map["profile_image_url"].toString();
    u.realName = map["name"].toString();
    u.userId = map["id"].toString();
    u.userName = map["screen_name"].toString();
    return u;
}

#include "twitterapimicroblog.moc"
