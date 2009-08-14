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
#include "accountmanager.h"
#include "microblogwidget.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "postwidget.h"
#include "twitterapiaccount.h"
#include "twitterapipostwidget.h"

TwitterApiMicroBlog::TwitterApiMicroBlog ( const KComponentData &instance, QObject *parent )
: MicroBlog( instance, parent), countOfPost(20), countOfTimelinesToSave(0)
{
    kDebug();
    setCharLimit(140);
    QStringList timelineTypes;
    timelineTypes<< "Home" << "Reply" << "Inbox" << "Outbox";
    setTimelineTypes(timelineTypes);
    setServiceName("Twitter");
    setServiceHomepageUrl("http://twitterapi.com/");
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
    timelineApiPath["Home"] = "/statuses/friends_timeline.xml";
    timelineApiPath["Reply"] = "/statuses/replies.xml";
    timelineApiPath["Inbox"] = "/direct_messages.xml";
    timelineApiPath["Outbox"] = "/direct_messages/sent.xml";
    foreach(QString tm, this->timelineTypes()) {
        mTimelineLatestId.insert(tm, QString());
    }
}

TwitterApiMicroBlog::~TwitterApiMicroBlog()
{
    kDebug();
}

QList< Choqok::Post* > TwitterApiMicroBlog::loadTimeline(const QString& accountAlias, const QString& timelineName)
{
    kDebug();
    QString fileName = accountAlias + '_' + timelineName + "_backuprc";
    KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
    QStringList groupList = postsBackup.groupList();
    groupList.sort();
    int count = groupList.count();
    QList< Choqok::Post* > list;
    if(count) {
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
            st->link = postUrl(st->postId, st->author.userName);
            //Sorting The new statuses:
            int j = 0;
            int count = list.count();
            while (( j < count ) && ( st->postId > list[ j ]->postId ) ) {
                ++j;
            }
            list.insert( j, st );
        }
        mTimelineLatestId[timelineName] = st->postId;
    }
    return list;
}

void TwitterApiMicroBlog::saveTimeline(const QString& accountAlias, const QString& timelineName, QList< Choqok::UI::PostWidget* > timeline)
{
    kDebug();
    QString fileName = accountAlias + '_' + timelineName + "_backuprc";
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
    --countOfTimelinesToSave;
    if(countOfTimelinesToSave < 1)
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
        kError() << "ERROR: Status text is empty!";
        return;
    }
    if ( !post->isPrivate ) {///Status Update
        KUrl url = mApiUrl;
        url.addPath ( "/statuses/update.xml" );
        setDefaultArgs ( theAccount, url );
        QByteArray data = "status=";
        data += QUrl::toPercentEncoding (  post->content );
        if ( !post->replyToPostId.isEmpty() && post->content.indexOf ( '@' ) > -1 ) {
            data += "&in_reply_to_status_id=";
            data += post->replyToPostId.toLocal8Bit();
        }
        data += "&source=choqok";
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kError() << "Cannot create a http POST request!";
            QString errMsg = i18n ( "Cannot create an http POST request." );
            emit errorPost ( theAccount, Choqok::MicroBlog::OtherError, errMsg, post );
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        mCreatePostMap[ job ] = post;
        mAccountJobs[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
    } else {///Direct message
        QString recipientScreenName = post->replyToUserName;
        KUrl url = mApiUrl;
        url.addPath ( "/direct_messages/new.xml" );
        setDefaultArgs ( theAccount, url );
        QByteArray data = "user=";
        data += recipientScreenName.toLocal8Bit();
        data += "&text=";
        data += QUrl::toPercentEncoding ( post->content );
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kError() << "Cannot create an http POST request!";
            QString errMsg = i18n ( "Cannot create an http POST request." );
            emit errorPost ( theAccount, Choqok::MicroBlog::OtherError, errMsg, post );
            return;
        }
        job->addMetaData ( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        mCreatePostMap[ job ] = post;
        mAccountJobs[job] = theAccount;
        connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotCreatePost ( KJob* ) ) );
        job->start();
    }
}

void TwitterApiMicroBlog::slotCreatePost ( KJob *job )
{
    kDebug();
    if ( !job ) {
        kError() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = mCreatePostMap.take(job);
    Choqok::Account *theAccount = mAccountJobs.take(job);
    if(!post || !theAccount) {
        kError()<<"Account or Post is NULL pointer";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit errorPost ( theAccount, Choqok::MicroBlog::CommunicationError, job->errorString(), post );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast< KIO::StoredTransferJob * > ( job );
        if ( !post->isPrivate ) {
            readPostFromXml ( stj->data(), post );
            if ( post->isError ) {
                kDebug() << "XML parsing error" ;
                emit errorPost ( theAccount, Choqok::MicroBlog::ParsingError,
                                 i18n ( "Error: Could not parse result data." ), post );
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
    foreach ( KJob *job, mAccountJobs.keys(theAccount) ) {
        job->kill();
    }
}

void TwitterApiMicroBlog::fetchPost ( Choqok::Account* theAccount, Choqok::Post* post )
{
    kDebug();
    if ( !post || post->postId.isEmpty()) {
        return;
    }
    KUrl url = mApiUrl;
    url.addPath ( QString("/statuses/show/%1.xml").arg(post->postId) );
    setDefaultArgs ( theAccount, url );

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n ( "Cannot create an http GET request." );
        emit errorPost ( theAccount, Choqok::MicroBlog::OtherError, errMsg, post );
        return;
    }
    mFetchPostMap[ job ] = post;
    mAccountJobs[ job ] = theAccount;
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
    Choqok::Account *theAccount = mAccountJobs.take(job);
    if ( job->error() ) {
        kError() << "Job Error: " << job->errorString();
        emit error ( theAccount, Choqok::MicroBlog::CommunicationError, job->errorString() );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
        readPostFromXml ( stj->data(), post );
        if ( post->isError ) {
            kError() << "Parsing Error";
            emit errorPost ( theAccount, Choqok::MicroBlog::ParsingError,
                             i18n ( "Error: Could not parse result data." ), post );
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
        KUrl url = mApiUrl;
        if ( !post->isPrivate ) {
            url.addPath ( "/statuses/destroy/" + post->postId + ".xml" );
        } else {
            url.addPath ( "/direct_messages/destroy/" + post->postId + ".xml" );
        }
        setDefaultArgs ( theAccount, url );
        KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create a http POST request!";
            QString errMsg = i18n ( "Cannot create an http POST request." );
            emit errorPost ( theAccount, Choqok::MicroBlog::OtherError, errMsg, post );
            return;
        }
        mRemovePostMap[job] = post;
        mAccountJobs[job] = theAccount;
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
    Choqok::Account *theAccount = mAccountJobs.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit errorPost ( theAccount, CommunicationError, job->errorString(), post );
    } else {
        emit postRemoved ( theAccount, post );
    }
}

void TwitterApiMicroBlog::createFavorite ( Choqok::Account* theAccount, const QString &postId )
{
    kDebug();
    KUrl url = mApiUrl;
    url.addPath ( "/favorites/create/" + postId + ".xml" );
    setDefaultArgs ( theAccount, url );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n ( "Cannot create an http POST request." );
        emit error ( theAccount, OtherError, errMsg );
        return;
    }
    mFavoriteMap[job] = postId;
    mAccountJobs[job] = theAccount;
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
    Choqok::Account *theAccount = mAccountJobs.take(job);
    QString postId = mFavoriteMap.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, CommunicationError, job->errorString() );
    } else {
        emit favoriteCreated ( theAccount, postId );
    }
}

void TwitterApiMicroBlog::removeFavorite ( Choqok::Account* theAccount, const QString& postId )
{
    kDebug();
    KUrl url = mApiUrl;
    url.addPath ( "/favorites/destroy/" + postId + ".xml" );
    setDefaultArgs ( theAccount, url );
    KIO::StoredTransferJob *job = KIO::storedHttpPost ( QByteArray(), url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n ( "Cannot create an http POST request." );
        emit error ( theAccount, OtherError, errMsg );
        return;
    }
    mFavoriteMap[job] = postId;
    mAccountJobs[job] = theAccount;
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
    Choqok::Account *theAccount = mAccountJobs.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, CommunicationError, job->errorString() );
    } else {
        emit favoriteCreated ( theAccount, id );
    }
}

void TwitterApiMicroBlog::updateTimelines (Choqok::Account* theAccount)
{
    kDebug();
    foreach ( QString tm, timelineTypes() ) {
        requestTimeLine ( theAccount, tm, mTimelineLatestId.value(tm) );
    }
}

void TwitterApiMicroBlog::requestTimeLine ( Choqok::Account* theAccount, QString type, QString latestStatusId, int page, QString maxId )
{
    kDebug();
    KUrl url = mApiUrl;
    url.addPath ( timelineApiPath[type] );
    setDefaultArgs ( theAccount, url );
    if ( !latestStatusId.isEmpty() ) {
        url.addQueryItem ( "since_id", latestStatusId );
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
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n ( "Cannot create an http GET request." );
        emit error ( theAccount, OtherError, errMsg );
        return;
    }
    mRequestTimelineMap[job] = type;
    mAccountJobs[job] = theAccount;
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
    Choqok::Account *theAccount = mAccountJobs.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error( theAccount, CommunicationError, job->errorString() );
        return;
    }
    QString type = mRequestTimelineMap.take(job);
    if( isValidTimeline(type) ) {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob*>( job );
        QList<Choqok::Post*> list;
        if( type=="Home" || type=="Reply" ) {
            list = readTimelineFromXml( j->data() );
        } else if( type=="Inbox" || type=="Outbox" ) {
            list = readDMessagesFromXml( theAccount, j->data() );
        }
        if(!list.isEmpty()) {
            mTimelineLatestId[type] = list.last()->postId;
            emit timelineDataReceived( theAccount, type, list );
        }
    }
}

void TwitterApiMicroBlog::setDefaultArgs ( Choqok::Account* theAccount, KUrl& url )
{
    if(theAccount) {
        url.setScheme ( qobject_cast<TwitterApiAccount*>(theAccount)->useSecureConnection() ? "https" : "http" );
        url.setUser ( theAccount->username() );
        url.setPass ( theAccount->password() );
    }
}

Choqok::Post * TwitterApiMicroBlog::readPostFromXml ( const QByteArray& buffer, Choqok::Post* post /*= 0*/ )
{
    kDebug();
    QDomDocument document;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( !root.isNull() ) {
        return readPostFromDomElement ( root.toElement(), post );
    } else {
        if(!post)
            post = new Choqok::Post;
        post->isError = true;
        return post;
    }
}

Choqok::Post * TwitterApiMicroBlog::readPostFromDomElement ( const QDomElement &root, Choqok::Post* post/* = 0*/ )
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
    post->link = postUrl(post->postId, post->author.userName);
    post->creationDateTime = dateFromString ( timeStr );

    return post;
}

QList<Choqok::Post*> TwitterApiMicroBlog::readTimelineFromXml ( const QByteArray &buffer )
{
    kDebug();
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
        postList.prepend( readPostFromDomElement ( node.toElement() ) );
        node = node.nextSibling();
    }
    return postList;
}

Choqok::Post * TwitterApiMicroBlog::readDMessageFromXml (Choqok::Account *theAccount, const QByteArray &buffer )
{
    kDebug();
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

Choqok::Post * TwitterApiMicroBlog::readDMessageFromDomElement ( Choqok::Account* theAccount, const QDomElement& root )
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
    if ( senderId == qobject_cast<TwitterApiAccount*>( theAccount )->userId() ) {
        msg->author.description = recipientDescription;
        msg->author.userName = recipientScreenName;
        msg->author.profileImageUrl = recipientProfileImageUrl;
        msg->author.realName = recipientName;
        msg->author.userId = recipientId;
        msg->replyToUserId = recipientId;
    } else {
        msg->author.description = senderDescription;
        msg->author.userName = senderScreenName;
        msg->author.profileImageUrl = senderProfileImageUrl;
        msg->author.realName = senderName;
        msg->author.userId = senderId;
        msg->replyToUserId = recipientId;
    }
    return msg;
}

QList<Choqok::Post*> TwitterApiMicroBlog::readDMessagesFromXml (Choqok::Account *theAccount, const QByteArray &buffer )
{
    kDebug();
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

QDateTime TwitterApiMicroBlog::dateFromString ( const QString &date )
{
    char s[10];
    int year, day, hours, minutes, seconds;
    sscanf ( qPrintable ( date ), "%*s %s %d %d:%d:%d %*s %d", s, &day, &hours, &minutes, &seconds, &year );
    int month = monthes[s];
    QDateTime recognized ( QDate ( year, month, day ), QTime ( hours, minutes, seconds ) );
    recognized.setTimeSpec( Qt::UTC );
    return recognized.toLocalTime();
}

void TwitterApiMicroBlog::aboutToUnload()
{
    countOfTimelinesToSave = 0;
    foreach(Choqok::Account* acc, Choqok::AccountManager::self()->accounts()){
        if(acc->microblog() == this)
            countOfTimelinesToSave += this->timelineTypes().count();
    }
    emit saveTimelines();
}

#include "twitterapimicroblog.moc"
