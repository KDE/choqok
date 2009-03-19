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
#include "backend.h"

#include <KDE/KLocale>
#include <QDomDocument>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurl.h>
#include "settings.h"
#include <kio/netaccess.h>

Backend::Backend( Account *account, QObject* parent ): QObject( parent )
{
    kDebug();
    mScheme = "http";
    settingsChanged();
    mCurrentAccount = account;
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

void Backend::postNewStatus( const QString & statusMessage, uint replyToStatusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() + "/statuses/update.xml" );
    setDefaultArgs( url );
    QByteArray data = "status=";
    data += QUrl::toPercentEncoding( prepareStatus( statusMessage ) );
    if ( replyToStatusId != 0 && statusMessage.indexOf( '@' ) > -1 )
        data += "&in_reply_to_status_id=" + QString::number( replyToStatusId );
    data += "&source=choqok";
    KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create a http POST request, please check your internet connection." );
        emit sigError( errMsg );
        return;
    }
    job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    mPostNewStatusBuffer[ job ] = QByteArray();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotPostNewStatusFinished( KJob* ) ) );
    connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
             this, SLOT(slotPostNewStatusData(KIO::Job*, const QByteArray&)));
    job->start();
}

void Backend::sendDMessage( const QString & screenName, const QString & message )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() + "/direct_messages/new.xml" );
    setDefaultArgs( url );
    QByteArray data = "user=";
    data += screenName;
    data += "&text=";
    data += QUrl::toPercentEncoding( prepareStatus( message ) );

    KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create a http POST request, please check your internet connection." );
        emit sigError( errMsg );
        return;
    }
    job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
    mSendDMessageBuffer[ job ] = QByteArray();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotSendDMessageFinished( KJob* ) ) );
    connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)),
             this, SLOT(slotSendDMessageData(KIO::Job*, const QByteArray&)));
    job->start();
}

void Backend::login()
{

}

void Backend::logout()
{
}

void Backend::requestTimeLine( uint latestStatusId, TimeLineType type, int page )
{
    kDebug();
    KUrl url;
    if ( type == HomeTimeLine )
        url.setUrl( mCurrentAccount->apiPath() + "/statuses/friends_timeline.xml" );
    else
        url.setUrl( mCurrentAccount->apiPath() + "/statuses/replies.xml" );
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
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
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

QList<Status> * Backend::readTimeLineFromXml( const QByteArray & buffer )
{
    kDebug();
    QDomDocument document;
    QList<Status> *statusList = new QList<Status>;

    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "statuses" ) {
        QString err = i18n( "Data returned from server corrupted!" );
        kDebug() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
        mLatestErrorString = err;
        return 0;
    }
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "status" ) {
            kDebug() << "there's no status tag in XML, maybe there is no new status!";
            return statusList;
        }
        QDomNode node2 = node.firstChild();
        Status status;
        status.isDMessage = false;
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "created_at" )
                timeStr = node2.toElement().text();
            else
                if ( node2.toElement().tagName() == "text" )
                    status.content = node2.toElement().text();
                else
                    if ( node2.toElement().tagName() == "id" )
                        status.statusId = node2.toElement().text().toInt();
                    else
                        if ( node2.toElement().tagName() == "in_reply_to_status_id" )
                            status.replyToStatusId = node2.toElement().text().toULongLong();
                        else
                            if ( node2.toElement().tagName() == "in_reply_to_user_id" )
                                status.replyToUserId = node2.toElement().text().toULongLong();
                            else
                                if ( node2.toElement().tagName() == "in_reply_to_screen_name" )
                                    status.replyToUserScreenName = node2.toElement().text();
                                else
                                    if ( node2.toElement().tagName() == "source" )
                                        status.source = node2.toElement().text();
                                    else
                                        if ( node2.toElement().tagName() == "truncated" )
                                            status.isTruncated = ( node2.toElement().text() == "true" ) ? true : false;
                                        else
                                            if ( node2.toElement().tagName() == "favorited" )
                                                status.isFavorited = ( node2.toElement().text() == "true" ) ? true : false;
                                            else
                                                if ( node2.toElement().tagName() == "user" ) {
                                                    QDomNode node3 = node2.firstChild();
                                                    while ( !node3.isNull() ) {
                                                        if ( node3.toElement().tagName() == "screen_name" ) {
                                                            status.user.screenName = node3.toElement().text();
                                                        } else
                                                            if ( node3.toElement().tagName() == "profile_image_url" ) {
                                                                status.user.profileImageUrl = node3.toElement().text();
                                                            } else
                                                                if ( node3.toElement().tagName() == "id" ) {
                                                                    status.user.userId = node3.toElement().text().toUInt();
                                                                } else
                                                                    if ( node3.toElement().tagName() == "name" ) {
                                                                        status.user.name = node3.toElement().text();
                                                                    } else
                                                                        if ( node3.toElement().tagName() == "description" ) {
                                                                            status.user.description = node3.toElement().text();
                                                                        }
                                                        node3 = node3.nextSibling();
                                                    }
                                                }
            node2 = node2.nextSibling();
        }
        node = node.nextSibling();
        status.creationDateTime = dateFromString( timeStr );
//               = QDateTime(time.date(), time.time(), Qt::UTC);
        statusList->insert( 0, status );
    }
    return statusList;
}

Status Backend::readStatusFromXml( const QByteArray & buffer )
{
    QDomDocument document;
    Status status;
    status.isDMessage = false;
    status.isError = false ;
    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "status" ) {
        kDebug() << "there's no status tag in XML, Error!!";
        status.isError = true ;
        return status;
    }
    QDomNode node2 = root.firstChild();
    QString timeStr;
    while ( !node2.isNull() ) {
        if ( node2.toElement().tagName() == "created_at" )
            timeStr = node2.toElement().text();
        else
            if ( node2.toElement().tagName() == "text" )
                status.content = node2.toElement().text();
            else
                if ( node2.toElement().tagName() == "id" )
                    status.statusId = node2.toElement().text().toInt();
                else
                    if ( node2.toElement().tagName() == "in_reply_to_status_id" )
                        status.replyToStatusId = node2.toElement().text().toULongLong();
                    else
                        if ( node2.toElement().tagName() == "in_reply_to_user_id" )
                            status.replyToUserId = node2.toElement().text().toULongLong();
                        else
                            if ( node2.toElement().tagName() == "in_reply_to_screen_name" )
                                status.replyToUserScreenName = node2.toElement().text();
                            else
                                if ( node2.toElement().tagName() == "source" )
                                    status.source = node2.toElement().text();
                                else
                                    if ( node2.toElement().tagName() == "truncated" )
                                        status.isTruncated = ( node2.toElement().text() == "true" ) ? true : false;
                                    else
                                        if ( node2.toElement().tagName() == "favorited" )
                                            status.isFavorited = ( node2.toElement().text() == "true" ) ? true : false;
                                        else
                                            if ( node2.toElement().tagName() == "user" ) {
                                                QDomNode node3 = node2.firstChild();
                                                while ( !node3.isNull() ) {
                                                    if ( node3.toElement().tagName() == "screen_name" ) {
                                                        status.user.screenName = node3.toElement().text();
                                                    } else
                                                        if ( node3.toElement().tagName() == "profile_image_url" ) {
                                                            status.user.profileImageUrl = node3.toElement().text();
                                                        } else
                                                            if ( node3.toElement().tagName() == "id" ) {
                                                                status.user.userId = node3.toElement().text().toUInt();
                                                            } else
                                                                if ( node3.toElement().tagName() == "name" ) {
                                                                    status.user.name = node3.toElement().text();
                                                                } else
                                                                    if ( node3.toElement().tagName() == QString( "description" ) ) {
                                                                        status.user.description = node3.toElement().text();
                                                                    }
                                                    node3 = node3.nextSibling();
                                                }
                                            }
        node2 = node2.nextSibling();
    }
    status.creationDateTime = dateFromString( timeStr );

    return status;
}

void Backend::abortPostNewStatus()
{
    kDebug() << "Not implemented yet!";
//  statusHttp.abort();
}

QString& Backend::latestErrorString()
{
    return mLatestErrorString;
}

void Backend::requestFavorited( uint statusId, bool isFavorite )
{
    kDebug();
    KUrl url;
    if ( isFavorite ) {
        url.setUrl( mCurrentAccount->apiPath() + "/favorites/create/" + QString::number( statusId ) + ".xml" );
    } else {
        url.setUrl( mCurrentAccount->apiPath() + "/favorites/destroy/" + QString::number( statusId ) + ".xml" );
    }
    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create a http POST request, please check your internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestFavoritedFinished( KJob* ) ) );
    job->start();
}

void Backend::requestDestroy( uint statusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() + "/statuses/destroy/" + QString::number( statusId ) + ".xml" );

    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create a http POST request, please check your internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestDestroyFinished( KJob* ) ) );

    job->start();
}

void Backend::requestDestroyDMessage( uint statusId )
{
    kDebug();
    KUrl url( mCurrentAccount->apiPath() + "/direct_messages/destroy/" + QString::number( statusId ) + ".xml" );

    setDefaultArgs( url );

    KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
    if ( !job ) {
        kDebug() << "Cannot create a http POST request!";
        QString errMsg = i18n( "Cannot create a http POST request, please check your internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestDestroyFinished( KJob* ) ) );

    job->start();
}

void Backend::slotPostNewStatusFinished( KJob * job )
{
    kDebug();
    if ( job->error() ) {
        kDebug() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigPostNewStatusDone( true );
    } else {
//      kDebug()<<mPostNewStatusBuffer[job];
        Status st = readStatusFromXml( mPostNewStatusBuffer[job] );
        if ( st.isError ) {
            kDebug() << "Error: " << job->errorString();
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

void Backend::slotRequestTimelineFinished( KJob *job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigError( mLatestErrorString );
        return;
    }
    KIO::StoredTransferJob *jj = qobject_cast<KIO::StoredTransferJob *>( job );
    QList<Status> *ptr = readTimeLineFromXml( jj->data() );
//     QList<Status> *ptr = readTimeLineFromXml(mRequestTimelineBuffer[ job ].data());
    switch ( mRequestTimelineMap.value( job ) ) {
    case HomeTimeLine:
        if ( ptr ) {
            emit homeTimeLineReceived( *ptr );
        } else {
            kDebug() << "Null returned from Backend::readTimeLineFromXml()";
        }
        break;
    case ReplyTimeLine:
        if ( ptr )
            emit replyTimeLineReceived( *ptr );
        else
            kDebug() << "Null returned from Backend::readTimeLineFromXml()";
        break;
    default:
        kDebug() << "The returned job isn't in Map!";
        break;
    };
    mRequestTimelineMap.remove( job );
//  mRequestTimelineBuffer.remove(job);
}

void Backend::slotRequestFavoritedFinished( KJob * job )
{
    kDebug();
    if ( !job ) {
        kDebug() << "Job is null pointer.";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Error: " << job->errorString();
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
        kDebug() << "Job is null pointer.";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Error: " << job->errorString();
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
    while (( j = status.indexOf( QRegExp( "(https?://)" ), i ) ) != -1 ) {
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
    KUrl url;
    url.setUrl( mCurrentAccount->apiPath() + "/account/verify_credentials.xml" );
    url.setUser( mCurrentAccount->username() );
    url.setPass( mCurrentAccount->password() );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
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
        kDebug() << "Job error, " << job->errorString();
        QString err = i18n( "Authorization Failed, more info: %1", job->errorString() );
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
                mCurrentAccount->setUserId( node2.toElement().text().toUInt() );
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
                kDebug() << "Authorization result is not TRUE, is : " << root.toElement().text();
                QString err = i18n( "Authorization Failed, more info: %1", job->errorString() );
                emit sigError( err );
                return;
            }
        } else
            if ( root.tagName() == "hash" ) {
                QDomNode node2 = root.firstChild();
                while ( !node2.isNull() ) {
                    if ( node2.toElement().tagName() == "error" ) {
                        emit sigError( i18n( "Authentication failed with this result: %1", node2.toElement().text() ) );
                        return;
                    }
                    node2 = node2.nextSibling();
                }
            } else {
                kDebug() << "ERROR, unrecognized result, buffer is: " << buffer;
            }
}

void Backend::slotUserInfoReceived( KJob * job )
{
    kDebug();

    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        QString err = i18n( "Request for user information failed. More info: %1", job->errorString() );
    }
    QDomDocument document;
    QByteArray buffer = qobject_cast<KIO::StoredTransferJob *>( job )->data();
    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "statuses" ) {
        QString err = i18n( "Data returned from server corrupted." );
        kDebug() << "there's no statuses tag in XML\t the XML is: \n" << buffer.data();
        mLatestErrorString = err;
        return;
    }
    QDomNode node = root.firstChild();

    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "status" ) {
            QString err = i18n( "Data returned from server corrupted!" );
            kDebug() << "there's no status tag in XML\t the XML is: \n" << buffer.data();
            mLatestErrorString = err;
            return;
        }
        QDomNode node2 = node.firstChild();
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "user" ) {
                QDomNode node3 = node2.firstChild();
                while ( !node3.isNull() ) {
                    if ( node3.toElement().tagName() == "id" ) {
                        mCurrentAccount->setUserId( node3.toElement().text().toUInt() );
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
    KUrl url;
    url.setUrl( mCurrentAccount->apiPath() + "/statuses/user_timeline.xml" );
    setDefaultArgs( url );
    url.setQuery( "?count=1" );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
        emit sigError( errMsg );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotUserInfoReceived( KJob* ) ) );
    job->start();
}

void Backend::requestDMessages( uint latestStatusId, DMessageType type, int page )
{
    kDebug();
    KUrl url;
    if ( type == Inbox )
        url.setUrl( mCurrentAccount->apiPath() + "/direct_messages.xml" );
    else
        url.setUrl( mCurrentAccount->apiPath() + "/direct_messages/sent.xml" );
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
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
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
        kDebug() << "Job is null pointer";
        return;
    }
    if ( job->error() ) {
        kDebug() << "Error: " << job->errorString();
        mLatestErrorString = job->errorString();
        emit sigError( mLatestErrorString );
        return;
    }
    KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob*>( job );
    QList<Status> *ptr = readDMessagesFromXml( j->data() );
    switch ( mRequestDMessagesMap.value( job ) ) {
    case Inbox:
        if ( ptr ) {
            emit directMessagesReceived( *ptr );
        } else {
            kDebug() << "Null returned from Backend::readDMessagesFromXml()";
        }
        break;
    case Outbox:
        if ( ptr )
            emit outboxMessagesReceived( *ptr );
        else
            kDebug() << "Null returned from Backend::readDMessagesFromXml()";
        break;
    default:
        kDebug() << "The returned job isn't in Map! or type is Unknown";
        break;
    };
    mRequestDMessagesMap.remove( job );
}

QList< Status > * Backend::readDMessagesFromXml( const QByteArray & buffer )
{
    kDebug();
    QDomDocument document;
    QList<Status> *statusList = new QList<Status>;

    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "direct-messages" ) {
        QString err = i18n( "Data returned from server corrupted!" );
        kDebug() << "there's no direct-messages tag in XML\t the XML is: \n" << buffer.data();
        mLatestErrorString = err;
        return 0;
    }
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "direct_message" ) {
            kDebug() << "there's no direct_message tag in XML, maybe there is no new status!";
            return statusList;
        }
        QDomNode node2 = node.firstChild();
        Status msg;
        msg.isDMessage = true;
        uint senderId = 0, recipientId = 0;
        QString senderScreenName, recipientScreenName, senderProfileImageUrl, senderName,
        senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "created_at" )
                timeStr = node2.toElement().text();
            else
                if ( node2.toElement().tagName() == "text" )
                    msg.content = node2.toElement().text();
                else
                    if ( node2.toElement().tagName() == "id" )
                        msg.statusId = node2.toElement().text().toInt();
                    else
                        if ( node2.toElement().tagName() == "sender_id" )
                            senderId = node2.toElement().text().toULongLong();
                        else
                            if ( node2.toElement().tagName() == "recipient_id" )
                                recipientId = node2.toElement().text().toULongLong();
                            else
                                if ( node2.toElement().tagName() == "sender_screen_name" )
                                    senderScreenName = node2.toElement().text();
                                else
                                    if ( node2.toElement().tagName() == "recipient_screen_name" )
                                        recipientScreenName = node2.toElement().text();
                                    else
                                        if ( node2.toElement().tagName() == "sender" ) {
                                            QDomNode node3 = node2.firstChild();
                                            while ( !node3.isNull() ) {
                                                if ( node3.toElement().tagName() == "profile_image_url" ) {
                                                    senderProfileImageUrl = node3.toElement().text();
                                                } else
                                                    if ( node3.toElement().tagName() == "name" ) {
                                                        senderName = node3.toElement().text();
                                                    } else
                                                        if ( node3.toElement().tagName() == "description" ) {
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
                                                    } else
                                                        if ( node3.toElement().tagName() == "name" ) {
                                                            recipientName = node3.toElement().text();
                                                        } else
                                                            if ( node3.toElement().tagName() == "description" ) {
                                                                recipientDescription = node3.toElement().text();
                                                            }
                                                    node3 = node3.nextSibling();
                                                }
                                            }
            node2 = node2.nextSibling();
        }
        node = node.nextSibling();
        msg.creationDateTime = dateFromString( timeStr );
        if ( senderId == mCurrentAccount->userId() ) {
            msg.user.description = recipientDescription;
            msg.user.screenName = recipientScreenName;
            msg.user.profileImageUrl = recipientProfileImageUrl;
            msg.user.name = recipientName;
            msg.user.userId = recipientId;
            msg.replyToUserId = recipientId;
        } else {
            msg.user.description = senderDescription;
            msg.user.screenName = senderScreenName;
            msg.user.profileImageUrl = senderProfileImageUrl;
            msg.user.name = senderName;
            msg.user.userId = senderId;
            msg.replyToUserId = recipientId;
        }
        statusList->insert( 0, msg );
    }
    return statusList;
}

void Backend::slotSendDMessageFinished( KJob *job )
{
    kDebug();
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->error() << " Text:" << job->errorString();
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

Status Backend::readDMessageFromXml( const QByteArray & buffer )
{
    QDomDocument document;
    Status status;
    status.isError = false ;
    document.setContent( buffer );
    Status msg;
    msg.isDMessage = true;
    QDomElement root = document.documentElement();
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "direct_message" ) {
            kDebug() << "there's no direct_message tag in XML, probably an error occurred!";
            status.isError = true;
            return status;
        }
        QDomNode node2 = node.firstChild();
        uint senderId = 0, recipientId = 0;
        QString senderScreenName, recipientScreenName, senderProfileImageUrl, senderName,
        senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "created_at" )
                timeStr = node2.toElement().text();
            else
                if ( node2.toElement().tagName() == "text" )
                    msg.content = node2.toElement().text();
                else
                    if ( node2.toElement().tagName() == "id" )
                        msg.statusId = node2.toElement().text().toInt();
                    else
                        if ( node2.toElement().tagName() == "sender_id" )
                            senderId = node2.toElement().text().toULongLong();
                        else
                            if ( node2.toElement().tagName() == "recipient_id" )
                                recipientId = node2.toElement().text().toULongLong();
                            else
                                if ( node2.toElement().tagName() == "sender_screen_name" )
                                    senderScreenName = node2.toElement().text();
                                else
                                    if ( node2.toElement().tagName() == "recipient_screen_name" )
                                        recipientScreenName = node2.toElement().text();
                                    else
                                        if ( node2.toElement().tagName() == "sender" ) {
                                            QDomNode node3 = node2.firstChild();
                                            while ( !node3.isNull() ) {
                                                if ( node3.toElement().tagName() == "profile_image_url" ) {
                                                    senderProfileImageUrl = node3.toElement().text();
                                                } else
                                                    if ( node3.toElement().tagName() == "name" ) {
                                                        senderName = node3.toElement().text();
                                                    } else
                                                        if ( node3.toElement().tagName() == "description" ) {
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
                                                    } else
                                                        if ( node3.toElement().tagName() == "name" ) {
                                                            recipientName = node3.toElement().text();
                                                        } else
                                                            if ( node3.toElement().tagName() == "description" ) {
                                                                recipientDescription = node3.toElement().text();
                                                            }
                                                    node3 = node3.nextSibling();
                                                }
                                            }
            node2 = node2.nextSibling();
        }
        node = node.nextSibling();
        msg.creationDateTime = dateFromString( timeStr );
        if ( senderId == mCurrentAccount->userId() ) {
            msg.user.description = recipientDescription;
            msg.user.screenName = recipientScreenName;
            msg.user.profileImageUrl = recipientProfileImageUrl;
            msg.user.name = recipientName;
            msg.user.userId = recipientId;
        } else {
            msg.user.description = senderDescription;
            msg.user.screenName = senderScreenName;
            msg.user.profileImageUrl = senderProfileImageUrl;
            msg.user.name = senderName;
            msg.user.userId = senderId;
        }
    }
    return msg;
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
    KUrl url;
    url.setUrl( mCurrentAccount->apiPath() + "/statuses/followers.xml" );
    setDefaultArgs( url );
    url.setQuery( "?page=" + QString::number( page ) );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
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
    KUrl url;
    url.setUrl( mCurrentAccount->apiPath() + "/statuses/friends/" + mCurrentAccount->username() + ".xml" );
    setDefaultArgs( url );
    url.setQuery( "?page=" + QString::number( page ) );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
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
        QString err = i18n( "Data returned from server corrupted!" );
        kDebug() << "there's no users tag in XML\t the XML is: \n" << buffer.data();
        mLatestErrorString = err;
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

void Backend::setDefaultArgs( KUrl & url )
{
    url.setScheme( mScheme );
    url.setUser( mCurrentAccount->username() );
    url.setPass( mCurrentAccount->password() );
}

void Backend::requestSingleStatus( uint statusId )
{
    kDebug();
    KUrl url;
    url.setUrl( mCurrentAccount->apiPath() + "/statuses/show/" + QString::number(statusId) + ".xml" );
    setDefaultArgs( url );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create a http GET request!";
        QString errMsg = i18n( "Cannot create a http GET request, please check your internet connection." );
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
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>( job );
        Status st = readStatusFromXml( stj->data() );
        if ( st.isError ) {
            kDebug() << "Parsing Error";
        } else {
            emit singleStatusReceived( st );
            mRequestSingleStatusMap.remove(job);
        }
    }
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
    }
    return baseUrl;
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

#include "backend.moc"
