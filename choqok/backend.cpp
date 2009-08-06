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
#include "backend.h"

#include <KDE/KLocale>
#include <QDomDocument>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurl.h>
#include "settings.h"
#include <kio/netaccess.h>
#include <kmimetype.h>

Backend::Backend( Account *account, QObject* parent )
: QObject( parent ), mCurrentAccount(account), mScheme("http")
{
    kDebug();
    settingsChanged();
//  login();
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

Backend::~Backend()
{
    kDebug();
}

void Backend::postNewStatus( const QString & statusMessage, qulonglong replyToStatusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/statuses/update.xml" );
    setDefaultArgs( url );
    QByteArray data = "status=";
    data += QUrl::toPercentEncoding( prepareStatus( statusMessage ) );
    if ( replyToStatusId != 0 && statusMessage.indexOf( '@' ) > -1 )
    {
        data += "&in_reply_to_status_id=";
        data += QString::number( replyToStatusId ).toLocal8Bit();
    }

    data += "&source=choqok";
    KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    mPostNewStatusBuffer[ job ] = QByteArray();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotPostNewStatusFinished( KJob* ) ) );
    connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
             this, SLOT(slotPostNewStatusData(KIO::Job*, const QByteArray&)));
    job->start();
    jobList<< job;
}

void Backend::twitPicCreatePost(const KUrl &picUrl, const QString &message)
{
    kDebug();
    QByteArray picData;
    QString tmp;
    KIO::TransferJob *picJob = KIO::get( picUrl, KIO::Reload, KIO::HideProgressInfo);
    if( !KIO::NetAccess::synchronousRun(picJob, 0, &picData) ){
        kError()<<"Job error: " << picJob->errorString();
        tmp = i18n( "Uploading media failed: cannot read the media file. "
        "Please check whether it exists. Path: %1", picUrl.prettyUrl() );
        kError() << "Emitting sigError...";
        emit sigError( tmp );
    }
    if ( picData.count() == 0 ) {
        kError() << "Cannot read the media file, please check if it exists.";
        tmp = i18n( "Uploading media failed: cannot read the media file. "
        "Please check whether it exists. Path: %1", picUrl.prettyUrl() );
        kError() << "Emitting sigError...";
        Q_EMIT sigError( tmp );
        return;
    }
    ///Documentation: http://twitpic.com/api.do
    KUrl url( "http://twitpic.com/api/uploadAndPost" );
    QByteArray newLine("\r\n");
    QString formHeader( newLine + "Content-Disposition: form-data; name=\"%1\"" );
    QByteArray header(newLine + "--AaB03x");
    QByteArray footer(newLine + "--AaB03x--");
    QByteArray fileContentType = KMimeType::findByUrl( picUrl, 0, true )->name().toAscii();
    QByteArray fileHeader(newLine + "Content-Disposition: file; name=\"media\"; filename=\"" +
                          picUrl.fileName().toAscii()+"\"");
    QByteArray data;
    data.append(header);

    data.append(fileHeader);
    data.append(newLine + "Content-Type: " + fileContentType);
    data.append(newLine);
    data.append(newLine + picData);

    data.append(header);
    data.append(formHeader.arg("username").toLatin1());
    data.append(newLine);
    data.append(newLine + mCurrentAccount->username().toLatin1());

    data.append(header);
    data.append(formHeader.arg("password").toLatin1());
    data.append(newLine);
    data.append(newLine + mCurrentAccount->password().toLatin1());

    data.append(header);
    data.append(formHeader.arg("message").toLatin1());
    data.append(newLine);
    data.append(newLine + message.toLatin1());

    data.append(footer);

    KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    job->addMetaData( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
    mPostNewStatusBuffer[ job ] = QByteArray();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotTwitPicCreatePost(KJob*) ) );
    connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
             this, SLOT(slotPostNewStatusData(KIO::Job*, const QByteArray&)));
    job->start();
    jobList<<job;
}

void Backend::sendDMessage( const QString & screenName, const QString & message )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/direct_messages/new.xml" );
    setDefaultArgs( url );
    QByteArray data = "user=";
    data += screenName.toLocal8Bit();
    data += "&text=";
    data += QUrl::toPercentEncoding( prepareStatus( message ) );

    KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    mSendDMessageBuffer[ job ] = QByteArray();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotSendDMessageFinished( KJob* ) ) );
    connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
             this, SLOT(slotSendDMessageData(KIO::Job*, const QByteArray&)));
    job->start();
    jobList<<job;
}

void Backend::requestTimeLine( qulonglong latestStatusId, TimeLineType type, int page )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    if ( type == HomeTimeLine )
        url.addPath( "/statuses/friends_timeline.xml" );
    else
        url.addPath( "/statuses/replies.xml" );
    setDefaultArgs( url );
    if(latestStatusId) {
        url.addQueryItem( "since_id", QString::number( latestStatusId ) );
    }
    url.addQueryItem( "count", QString::number( Settings::countOfStatusesOnMain() ) );
    if(page) {
        url.addQueryItem( "page", QString::number( page ) );
    }
    kDebug() << "Latest status Id: " << latestStatusId;


    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    mRequestTimelineMap[job] = type;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestTimelineFinished( KJob* ) ) );
    job->start();
}

QDateTime Backend::dateFromString( const QString &date )
{
    char s[10];
    int year, day, hours, minutes, seconds;
    sscanf( qPrintable( date ), "%*s %s %d %d:%d:%d %*s %d", s, &day, &hours, &minutes, &seconds, &year );
    int month = monthes[s];
    QDateTime recognized( QDate( year, month, day ), QTime( hours, minutes, seconds ) );
    recognized.setTimeSpec( Qt::UTC );
    return recognized.toLocalTime();
    ///Changed to this^ with hope it solve problem on some situations :)
//     QDateTime datetime = QDateTime::fromString(date, "ddd MMM dd h:mm:ss '+0000' yyyy");
//     if(!datetime.isValid() || datetime.isNull())
//         kDebug()<<"Convertion failed for \""<< date <<"\" date time, fetched from server.";
//     datetime.setTimeSpec(Qt::UTC);
//     return datetime.toLocalTime();
}

///***********************************************
Status Backend::readStatusFromXml ( const QByteArray &buffer )
{
    kDebug();
    QDomDocument document;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( !root.isNull() ) {
        return readStatusFromDomElement ( root.toElement() );
    } else {
        Status post;
        post.isDMessage = false;
        post.isError = true;
        return post;
    }
}

Status Backend::readStatusFromDomElement ( const QDomElement &root )
{
    Status post;
    post.isDMessage = false;
    post.isError = false ;
    post.user.friendsCount = 0;

    if ( root.tagName() != "status" ) {
        kError() << "there's no status tag in XML, Error!! TagName: "<< root.tagName() << " Text: "<< root.text();
        post.isError = true ;
        return post;
    }
    QDomNode node2 = root.firstChild();
    QString timeStr;
    QDomElement element;
    while ( !node2.isNull() ) {
        element = node2.toElement();
        if ( element.tagName() == "created_at" )
            timeStr = element.text();
        else if ( element.tagName() == "text" )
            post.content = element.text();
        else if ( element.tagName() == "id" )
            post.statusId = element.text().toULongLong();
        else if ( element.tagName() == "in_reply_to_status_id" )
            post.replyToStatusId = element.text().toULongLong();
        else if ( element.tagName() == "in_reply_to_user_id" )
            post.replyToUserId = element.text().toULongLong();
        else if ( element.tagName() == "in_reply_to_screen_name" )
            post.replyToUserScreenName = element.text();
        else if ( element.tagName() == "source" )
            post.source = element.text();
        else if ( element.tagName() == "favorited" )
            post.isFavorited = ( element.text() == "true" ) ? true : false;
        else if ( element.tagName() == "user" ) {
            QDomNode node3 = node2.firstChild();
            QDomElement subElement;
            while ( !node3.isNull() ) {
                subElement = node3.toElement();
                if ( subElement.tagName() == "screen_name" ) {
                    post.user.screenName = subElement.text();
                } else if ( subElement.tagName() == "profile_image_url" ) {
                    post.user.profileImageUrl = subElement.text();
                } else if ( subElement.tagName() == "id" ) {
                    post.user.userId = subElement.text().toULongLong();
                } else if ( subElement.tagName() == "name" ) {
                    post.user.name = subElement.text();
                } else if ( subElement.tagName() == QString ( "description" ) ) {
                    post.user.description = subElement.text();
                } else if ( subElement.tagName() == "location" ) {
                    post.user.location = subElement.text();
                } else if (subElement.tagName() == "url") {
                    post.user.homePageUrl = subElement.text();
                } else if (subElement.tagName() == "followers_count") {
                    post.user.followersCount = subElement.text().toInt();
                } else if (subElement.tagName() == "friends_count") {
                    post.user.friendsCount = subElement.text().toInt();
                }
                node3 = node3.nextSibling();
            }
        }
        node2 = node2.nextSibling();
    }
    post.creationDateTime = dateFromString ( timeStr );

    return post;
}

QList<Status> Backend::readTimelineFromXml ( const QByteArray &buffer )
{
    kDebug();
    QDomDocument document;
    QList<Status> postList;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( root.tagName() != "statuses" ) {
        kError() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
        return postList;
    }
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
        postList.prepend( readStatusFromDomElement ( node.toElement() ) );
        node = node.nextSibling();
    }
    return postList;
}

Status Backend::readDMessageFromXml ( const QByteArray &buffer )
{
    kDebug();
    QDomDocument document;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();

    if ( !root.isNull() ) {
        return readDMessageFromDomElement ( root.toElement() );
    } else {
        Status post;
        post.isError = true;
        post.isDMessage = true;
        return post;
    }
}

Status Backend::readDMessageFromDomElement ( const QDomElement &root )
{
    Status msg;
    msg.isError = false ;
    msg.isDMessage = true;
    msg.user.friendsCount = 0;
    if ( root.tagName() != "direct_message" ) {
        kError() << "there's no status tag in XML, Error!! TagName: "<< root.tagName() << " Text: "<< root.text();
        msg.isError = true ;
        return msg;
    }
    QDomNode node2 = root.firstChild();
//     qulonglong senderId = 0, recipientId = 0;
    User sender, recipient;
    QString timeStr;//, senderScreenName, recipientScreenName, senderProfileImageUrl, senderName,
//     senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;
    while ( !node2.isNull() ) {
        QDomElement element = node2.toElement();
        if ( element.tagName() == "created_at" )
            timeStr = element.text();
        else if ( element.tagName() == "text" )
            msg.content = element.text();
        else if ( element.tagName() == "id" )
            msg.statusId = element.text().toULongLong();
        else if ( element.tagName() == "sender_id" )
            sender.userId = element.text().toULongLong();
        else if ( element.tagName() == "recipient_id" )
            recipient.userId = element.text().toULongLong();
        else if ( element.tagName() == "sender_screen_name" )
            sender.screenName = element.text();
        else if ( element.tagName() == "recipient_screen_name" )
            recipient.screenName = element.text();
        else if ( element.tagName() == "sender" ) {
            QDomNode node3 = node2.firstChild();
            QDomElement subElement;
            while ( !node3.isNull() ) {
                subElement = node3.toElement();
                if ( subElement.tagName() == "profile_image_url" ) {
                    sender.profileImageUrl = subElement.text();
                } else if ( subElement.tagName() == "name" ) {
                    sender.name = subElement.text();
                } else if ( subElement.tagName() == "description" ) {
                    sender.description = subElement.text();
                } else if ( subElement.tagName() == "location" ) {
                    sender.location = subElement.text();
                } else if (subElement.tagName() == "url") {
                    sender.homePageUrl = subElement.text();
                } else if (subElement.tagName() == "followers_count") {
                    sender.followersCount = subElement.text().toInt();
                } else if (subElement.tagName() == "friends_count") {
                    sender.friendsCount = subElement.text().toInt();
                }
                node3 = node3.nextSibling();
            }
        } else
            if ( element.tagName() == "recipient" ) {
                QDomNode node3 = node2.firstChild();
                QDomElement subElement;
                while ( !node3.isNull() ) {
                    subElement = node3.toElement();
                    if ( subElement.tagName() == "profile_image_url" ) {
                        recipient.profileImageUrl = subElement.text();
                    } else if ( subElement.tagName() == "name" ) {
                        recipient.name = subElement.text();
                    } else if ( subElement.tagName() == "description" ) {
                        recipient.description = subElement.text();
                    } else if ( subElement.tagName() == "location" ) {
                        recipient.location = subElement.text();
                    } else if (subElement.tagName() == "url") {
                        recipient.homePageUrl = subElement.text();
                    } else if (subElement.tagName() == "followers_count") {
                        recipient.followersCount = subElement.text().toInt();
                    } else if (subElement.tagName() == "friends_count") {
                        recipient.friendsCount = subElement.text().toInt();
                    }
                    node3 = node3.nextSibling();
                }
            }
            node2 = node2.nextSibling();
    }
    msg.creationDateTime = dateFromString ( timeStr );
    if ( sender.userId == mCurrentAccount->userId() ) {
        msg.user = recipient;
        msg.replyToUserId = recipient.userId;
    } else {
        msg.user = sender;
        msg.replyToUserId = recipient.userId;
    }
    return msg;
}

QList<Status> Backend::readDMessagesFromXml ( const QByteArray &buffer )
{
    kDebug();
    QDomDocument document;
    QList<Status> postList;
    document.setContent ( buffer );
    QDomElement root = document.documentElement();
    
    if ( root.tagName() != "direct-messages" ) {
        //         QString err = i18n( "Data returned from server is corrupted." );
        kError() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
        return postList;
    }
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
        postList.prepend( readDMessageFromDomElement ( node.toElement() ) );
        node = node.nextSibling();
    }
    return postList;
}
///***********************************************

void Backend::abortPostNewStatus()
{
    kDebug();
    foreach(KJob *job, jobList){
        job->kill();
        jobList.removeAll(job);
    }
}

QString& Backend::latestErrorString()
{
    return mLatestErrorString;
}

void Backend::requestFavorited( qulonglong statusId, bool isFavorite )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    if ( isFavorite ) {
        url.addPath( "/favorites/create/" + QString::number( statusId ) + ".xml" );
    } else {
        url.addPath( "/favorites/destroy/" + QString::number( statusId ) + ".xml" );
    }
    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestFavoritedFinished( KJob* ) ) );
    job->start();
    jobList<<job;
}

void Backend::requestDestroy( qulonglong statusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/statuses/destroy/" + QString::number( statusId ) + ".xml" );

    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestDestroyFinished( KJob* ) ) );

    job->start();
    jobList<<job;
}

void Backend::requestDestroyDMessage( qulonglong statusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/direct_messages/destroy/" + QString::number( statusId ) + ".xml" );

    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestDestroyFinished( KJob* ) ) );

    job->start();
    jobList<<job;
}

void Backend::slotPostNewStatusFinished( KJob * job )
{
    kDebug();
    jobList.removeOne(job);
    if ( job->error() ) {
        kError() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigPostNewStatusDone( true );
    } else {
//      kError()<<mPostNewStatusBuffer[job];
        Status st = readStatusFromXml( mPostNewStatusBuffer[job] );
        if ( st.isError ) {
            kError() << "Error: " << job->errorString();
            mLatestErrorString = job->errorString();
            emit sigPostNewStatusDone( true );
        } else {
//             QList<Status> newSt;
//             newSt.append( st );
            emit sigPostNewStatusDone( false );
//             emit homeTimeLineReceived( newSt );
        }
    }
}

void Backend::slotTwitPicCreatePost( KJob *job )
{
    kDebug();
    jobList.removeOne(job);
    if ( job->error() ) {
        kError() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigPostNewStatusDone( true );
        return;
    } else {
        QDomDocument doc;
        QByteArray buffer = mPostNewStatusBuffer[job];
        mPostNewStatusBuffer.remove(job);
        doc.setContent(buffer);
        QDomElement element = doc.documentElement();
            if( element.tagName() == "rsp" ) {
                QString result;
                if(element.hasAttribute("stat") )
                    result = element.attribute("stat" , "fail");
                else if(element.hasAttribute("status"))
                    result = element.attribute("status" , "fail");
                else {
                    kError()<<"There isn't any \"stat\" or \"status\" attribute. Buffer:\n"<<buffer;
                    mLatestErrorString = i18n("Unrecognised result.");
                    emit sigPostNewStatusDone(true);
                    return;
                }
                if( result == "ok" ) {
                    emit sigPostNewStatusDone(false);
                    return;
                } else {
                    QDomNode node = element.firstChild();
                    while( !node.isNull() ){
                        element = node.toElement();
                        if(element.tagName() == "err") {
                            mLatestErrorString = element.attribute( "msg", i18n("Unrecognised result.") );
                        }
                        node = node.nextSibling();
                    }
                    emit sigPostNewStatusDone(true);
                    return;
                }
            } else {
                kError()<<"There isn't any \"rsp\" tag. Buffer:\n"<<buffer;
                mLatestErrorString = i18n("Unrecognised result.");
                emit sigPostNewStatusDone(true);
            }
    }
}

void Backend::slotRequestTimelineFinished( KJob *job )
{
    kDebug();
    if ( !job ) {
        kError() << "Job is null pointer";
        return;
    }
    if ( job->error() ) {
        kError() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigError( mLatestErrorString );
        return;
    }
    KIO::StoredTransferJob *jj = qobject_cast<KIO::StoredTransferJob *>( job );
    QList<Status> ptr = readTimelineFromXml( jj->data() );
//     QList<Status> *ptr = readTimeLineFromXml(mRequestTimelineBuffer[ job ].data());
    switch ( mRequestTimelineMap.value( job ) ) {
    case HomeTimeLine:
        emit homeTimeLineReceived( ptr );
        break;
    case ReplyTimeLine:
        emit replyTimeLineReceived( ptr );
        break;
    default:
        kError() << "The returned job isn't in Map!";
        break;
    };
    mRequestTimelineMap.remove( job );
//  mRequestTimelineBuffer.remove(job);
}

void Backend::slotRequestFavoritedFinished( KJob * job )
{
    kDebug();
    if ( !job ) {
        kError() << "Job is null pointer.";
        return;
    }
    jobList.removeOne(job);
    if ( job->error() ) {
        kError() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigFavoritedDone( true );
        return;
    } else {
        emit sigFavoritedDone(false);
//         KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>( job );
//         Status st = readStatusFromXml( stj->data() );
//         if ( !st.isError && st.isFavorited )
//             emit sigFavoritedDone( false );
//         else
//             emit sigFavoritedDone( true );
    }
}

void Backend::slotRequestDestroyFinished( KJob * job )
{
    kDebug();
    if ( !job ) {
        kError() << "Job is null pointer.";
        return;
    }
    jobList.removeOne(job);
    if ( job->error() ) {
        kError() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigDestroyDone( true );
        return;
    } else {
        emit sigDestroyDone(false);
//         KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>( job );
//         Status st = readStatusFromXml( stj->data() );
//         if ( st.isError )
//             emit sigDestroyDone( true );
//         else
//             emit sigDestroyDone( false );
    }
}

QString Backend::prepareStatus( QString status )
{
    kDebug();
    QString t = "";
    int i = 0, j = 0;
    QRegExp urlRegExp( "((ftps?|https?)://)" );
    while (( j = status.indexOf( urlRegExp, i ) ) != -1 ) {
        t += status.mid( i, j - i );
        int k = status.indexOf( ' ', j );
        if ( k == -1 )
            k = status.length();
        QString baseUrl = status.mid( j, k - j );
        if ( baseUrl.count() > 30 && Settings::shortenService() != SettingsBase::NoShorten ) {
            t += shortenUrl(baseUrl);
        } else {
            t += baseUrl;
        }
        i = k;
    }
    t += status.mid( i );
    return t;
}
QString Backend::shortenUrl(const QString &baseUrl)
{
    QMap<QString, QString> metaData;
    QByteArray data;
    if(Settings::shortenService() == SettingsBase::TightURL){
        kDebug()<<"Using 2tu.us";
        KUrl url( "http://2tu.us/" );
        url.addQueryItem( "save", "y" );
        url.addQueryItem( "url", KUrl( baseUrl ).url() );
        
        KIO::Job *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );
        
        if ( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
            QString output(data);
            QRegExp rx( QString( "<code>(.+)</code>" ) );
            rx.setMinimal(true);
            rx.indexIn(output);
            output = rx.cap(1);
            kDebug()<<output;
            rx.setPattern( QString( "href=[\'\"](.+)[\'\"]" ) );
            rx.indexIn(output);
            output = rx.cap(1);
            kDebug() << "Short url is: " << output;
            if(!output.isEmpty())
                return output;
        } else {
            kDebug() << "Cannot create a shorten url.\t" << "KJob ERROR";
        }
    } else if(Settings::shortenService() == SettingsBase::IS_GD) {
        kDebug()<<"Using is.gd";
        KUrl url( "http://is.gd/api.php" );
        url.addQueryItem( "longurl", KUrl( baseUrl ).url() );
        
        KIO::Job *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );
        
        metaData.insert( "PropagateHttpHeader", "true" );
        if ( KIO::NetAccess::synchronousRun( job, 0, &data, 0, &metaData ) ) {
            QString responseHeaders = metaData[ "HTTP-Headers" ];
            QString code = responseHeaders.split( ' ' )[1];
            if ( code == "200" ) {
                kDebug() << "Short url is: " << data;
                return QString( data );
            } else {
                kDebug() << "shortenning url faild HTTP response code is: " << code;
            }
        } else {
            QString responseHeaders = metaData[ "HTTP-Headers" ];
            kDebug() << "Cannot create a shorten url.\t" << "Response header = " << responseHeaders;
        }
    } else if( Settings::shortenService() == SettingsBase::DIGG ) {
        kDebug()<<"Using digg.com";
        KUrl url( "http://services.digg.com/url/short/create" );
        url.addQueryItem( "url", KUrl( baseUrl ).url() );
        url.addQueryItem( "appkey", "http://choqok.gnufolks.org" );
        
        KIO::Job *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );
        
        metaData.insert( "PropagateHttpHeader", "true" );
        if ( KIO::NetAccess::synchronousRun( job, 0, &data, 0, &metaData ) ) {
            QString responseHeaders = metaData[ "HTTP-Headers" ];
            QString code = responseHeaders.split( ' ' )[1];
            if ( code == "200" ) {
                kDebug() << "Short url is: " << data;
                QDomDocument doc;
                doc.setContent(data);
                if(doc.documentElement().tagName() == "shorturls") {
                    QDomElement elm = doc.documentElement().firstChild().toElement();
                    if(elm.tagName() == "shorturl"){
                        return elm.attribute("short_url", baseUrl);
                    }
                }
                return QString( data );
            } else {
                kDebug() << "shortenning url faild HTTP response code is: " << code;
            }
        } else {
            QString responseHeaders = metaData[ "HTTP-Headers" ];
            kDebug() << "Cannot create a shorten url.\t" << "Response header = " << responseHeaders;
        }
    }
    return baseUrl;
}

void Backend::settingsChanged()
{
    if ( Settings::useSecureConnection() )
        mScheme = "https";
    else
        mScheme = "http";
}

void Backend::verifyCredential()
{
    kDebug();
    KUrl url(mCurrentAccount->apiPath());
    url.addPath( "/account/verify_credentials.xml" );
    setDefaultArgs(url);

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCredentialsReceived( KJob* ) ) );
    job->start();
}

void Backend::slotCredentialsReceived( KJob * job )
{
    kDebug();
    if ( job->error() ) {
        kError() << "Job error, " << job->errorString();
        QString err = i18n( "Authorization failed: %1", job->errorString() );
        emit sigError( err );
        return;
    }
    QByteArray buffer = qobject_cast<KIO::StoredTransferJob *>( job )->data();
    ///Read response!
    QDomDocument document;
    Status status;
    status.isError = false ;
    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() == "user" ) {
        QDomNode node2 = root.firstChild();
        QString timeStr;
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "id" ) {
                mCurrentAccount->setUserId( node2.toElement().text().toULongLong() );
                break;
            }
            node2 = node2.nextSibling();
        }
        emit userVerified( mCurrentAccount );
    } else
        if ( root.tagName() == "authorized" ) {
            if ( root.toElement().text() == "true" ) {
                requestCurrentUser();
            } else {
                kError() << "Authorization result is not TRUE, is : " << root.toElement().text();
                QString err = i18n( "Authentication failed." );
                emit sigError( err );
                return;
            }
        } else
            if ( root.tagName() == "hash" ) {
                QDomNode node2 = root.firstChild();
                while ( !node2.isNull() ) {
                    if ( node2.toElement().tagName() == "error" ) {
                        emit sigError( i18n( "Authentication failed: %1", node2.toElement().text() ) );
                        return;
                    }
                    node2 = node2.nextSibling();
                }
            } else {
                kError() << "ERROR, unrecognized result, buffer is: " << buffer;
                emit sigError( i18n("Error, Unrecognized result.\nCannot parse result data back from server, maybe it's corrupted") );
            }
}

void Backend::slotUserInfoReceived( KJob * job )
{
    kDebug();

    if ( job->error() ) {
        kError() << "Job Error: " << job->errorString();
        QString err = i18n( "Request for user information failed: %1", job->errorString() );
    }
    QDomDocument document;
    QByteArray buffer = qobject_cast<KIO::StoredTransferJob *>( job )->data();
    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "statuses" ) {
        QString err = i18n( "Data returned from server is corrupted." );
        kError() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
        mLatestErrorString = err;
        return;
    }
    QDomNode node = root.firstChild();

    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "status" ) {
            QString err = i18n( "Data returned from server is corrupted." );
            kError() << "there's no status tag in XML\t the XML is: \n" << buffer.data();
            mLatestErrorString = err;
            return;
        }
        QDomNode node2 = node.firstChild();
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "user" ) {
                QDomNode node3 = node2.firstChild();
                while ( !node3.isNull() ) {
                    if ( node3.toElement().tagName() == "id" ) {
                        mCurrentAccount->setUserId( node3.toElement().text().toULongLong() );
                        emit userVerified( mCurrentAccount );
                        return;
                    }
                    node3 = node3.nextSibling();
                }
            }
            node2 = node2.nextSibling();
        }
        node = node.nextSibling();
    }

}

void Backend::requestCurrentUser()
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/statuses/user_timeline.xml" );
    setDefaultArgs( url );
    url.setQuery( "?count=1" );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotUserInfoReceived( KJob* ) ) );
    job->start();
}

void Backend::requestDMessages( qulonglong latestStatusId, DMessageType type, int page )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    if ( type == Inbox )
        url.addPath( "/direct_messages.xml" );
    else
        url.addPath( "/direct_messages/sent.xml" );
    setDefaultArgs( url );
    if(latestStatusId) {
        url.addQueryItem( "since_id", QString::number( latestStatusId ) );
    }
    if(page) {
        url.addQueryItem( "page", QString::number( page ) );
    }
    kDebug() << "DMessage: Latest status Id: " << latestStatusId;


    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    mRequestDMessagesMap[job] = type;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestDMessagesFinished( KJob* ) ) );
    job->start();
}

void Backend::slotRequestDMessagesFinished( KJob *job )
{
    kDebug();
    if ( !job ) {
        kError() << "Job is null pointer";
        return;
    }
    if ( job->error() ) {
        kError() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigError( mLatestErrorString );
        return;
    }
    KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob*>( job );
    QList<Status> ptr = readDMessagesFromXml( j->data() );
    switch ( mRequestDMessagesMap.value( job ) ) {
    case Inbox:
        emit directMessagesReceived( ptr );
        break;
    case Outbox:
        emit outboxMessagesReceived( ptr );
        break;
    default:
        kError() << "The returned job isn't in Map! or type is Unknown";
        break;
    };
    mRequestDMessagesMap.remove( job );
}

void Backend::slotSendDMessageFinished( KJob *job )
{
    kDebug();
    jobList.removeOne(job);
    if ( job->error() ) {
        kError() << "Job Error: " << job->error() << " Text:" << job->errorString();
//         kDebug()<<mSendDMessageBuffer.value(job);
        mLatestErrorString = job->errorString();
        emit sigPostNewStatusDone( true );
    } else {
        Status st = readDMessageFromXml(mSendDMessageBuffer[job]);
        if ( st.isError ) {
            emit sigPostNewStatusDone( false );
        } else {
            QList<Status> newSt;
            newSt.append( st );
            emit sigPostNewStatusDone( false );
            emit outboxMessagesReceived( newSt );
        }
    }
}

void Backend::listFollowersScreenName()
{
    kDebug();
    requestFollowers();
    followersPage = 1;
}

void Backend::requestFollowers( int page )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/statuses/followers.xml" );
    setDefaultArgs( url );
    url.setQuery( "?page=" + QString::number( page ) );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotListFollowersScreenName( KJob* ) ) );
    job->start();
}

void Backend::slotListFollowersScreenName( KJob * job )
{
    kDebug();
    KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
    QStringList newList = readUsersNameFromXml( stJob->data() );
    followersList << newList;
    if ( newList.count() == 100 ) {
        requestFollowers( ++followersPage );
    } else {
        emit followersListed( followersList );
    }
}

void Backend::listFriendsScreenName()
{
    kDebug();
    requestFriends();
    friendsPage = 1;
}

void Backend::requestFriends( int page )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/statuses/friends/" + mCurrentAccount->username() + ".xml" );
    setDefaultArgs( url );
    url.setQuery( "?page=" + QString::number( page ) );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotListFriendsScreenName( KJob* ) ) );
    job->start();
}

void Backend::slotListFriendsScreenName( KJob * job )
{
    kDebug();
    KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
    QStringList newList = readUsersNameFromXml( stJob->data() );
    friendsList << newList;
    if ( newList.count() == 100 ) {
        requestFriends( ++friendsPage );
    } else {
        emit friendsListed( friendsList );
    }
}

QStringList Backend::readUsersNameFromXml( const QByteArray & buffer )
{
    kDebug();
    QStringList list;
    QDomDocument document;
    document.setContent( buffer );
    QDomElement root = document.documentElement();

    if ( root.tagName() != "users" ) {
        QString err = i18n( "Data returned from server is corrupted." );
        kError() << "there's no users tag in XML\t the XML is: \n" << buffer.data();
        mLatestErrorString = err;
        return list;
    }
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "user" ) {
            kError() << "there's no status tag in XML, maybe there is no new status!";
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

void Backend::setDefaultArgs( KUrl & url )
{
    url.setScheme( mScheme );
    url.setUser( mCurrentAccount->username() );
    url.setPass( mCurrentAccount->password() );
}

void Backend::requestSingleStatus( qulonglong statusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/statuses/show/" + QString::number(statusId) + ".xml" );
    setDefaultArgs( url );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create an http GET request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }
    mRequestSingleStatusMap[ job ] = statusId;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestSingleStatusFinished ( KJob* ) ) );
    job->start();
}

void Backend::slotRequestSingleStatusFinished( KJob* job )
{
    kDebug();
    if ( job->error() ) {
        kError() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>( job );
        Status st = readStatusFromXml( stj->data() );
        if ( st.isError ) {
            kError() << "Parsing Error";
        } else {
            emit singleStatusReceived( st );
            mRequestSingleStatusMap.remove(job);
        }
    }
}

void Backend::slotPostNewStatusData(KIO::Job * job, const QByteArray & data)
{
    kDebug();
    if( !job ) {
        kError() << "Job is a null pointer.";
        return;
    }
    mPostNewStatusBuffer[ job ].append(data);
}

void Backend::slotSendDMessageData(KIO::Job *job, const QByteArray &data)
{
    kDebug();
    if( !job ) {
        kError() << "Job is a null pointer.";
        return;
    }
    mPostNewStatusBuffer[ job ].append( data );
}

void Backend::slotAddFriend(const QString &screenName)
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() );
    url.addPath( "/friendships/create/" + screenName + ".xml" );
    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create an http POST request, please check your Internet connection." );
        emit sigError( errMsg );
        return;
    }

    mRequestFriendMap[ job ] = screenName;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestNewFriendFinished ( KJob* ) ) );
    job->start();
}

void Backend::slotRequestNewFriendFinished(KJob *job)
{
    // If we are already friends, then HTTP 403 is returned
    kDebug();
    if ( job->error() ) {
        kError() << "Job Error: " << job->errorString();
    } else {
        emit friendAdded( mRequestFriendMap[job] );
    }
    mRequestFriendMap.remove(job);
}

#include "backend.moc"
