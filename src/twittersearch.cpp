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

#include "twittersearch.h"

#include <KDE/KLocale>
#include <QDomDocument>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kdebug.h>

#include "backend.h"

TwitterSearch::TwitterSearch( Account* account, const QString searchUrl, QObject *parent ) :
        Search(account, searchUrl, parent)
{
    kDebug();
    mSearchTypes[CustomSearch].first = i18n( "Custom Search" );
    mSearchTypes[CustomSearch].second = true;

    mSearchTypes[ToUser].first = i18nc( "Tweets are Twitter posts",  "Tweets To This User" );
    mSearchTypes[ToUser].second = true;

    mSearchTypes[FromUser].first = i18nc( "Tweets are Twitter posts", "Tweets From This User" );
    mSearchTypes[FromUser].second = true;

    mSearchTypes[ReferenceUser].first = i18nc( "Tweets are Twitter posts", "Tweets Including This User's Name" );
    mSearchTypes[ReferenceUser].second = true;

    mSearchTypes[ReferenceHashtag].first = i18nc( "Tweets are Twitter posts", "Tweets Including This Hashtag" );
    mSearchTypes[ReferenceHashtag].second = true;
}

TwitterSearch::~TwitterSearch()
{
    kDebug();
}

KUrl TwitterSearch::buildUrl( QString query, int option, uint sinceStatusId, uint count, uint page )
{
    kDebug();
    QString formattedQuery;
    switch ( option ) {
        case CustomSearch:
            formattedQuery = query;
            break;
        case ToUser:
            formattedQuery = "to:" + query;
            break;
        case FromUser:
            formattedQuery = "from:" + query;
            break;
        case ReferenceUser:
            formattedQuery = '@' + query;
            break;
        case ReferenceHashtag:
            formattedQuery = "#" + query;
            break;
        default:
            formattedQuery = query;
            break;
    };
    KUrl url( "http://search.twitter.com/search.atom" );
    url.addQueryItem("q", formattedQuery);
    if( sinceStatusId )
        url.addQueryItem( "since_id", QString::number( sinceStatusId ) );
    if( count && count <= 100 )
        url.addQueryItem( "rpp", QString::number( count ) );
    url.addQueryItem( "page", QString::number( page ) );

    return url;
}

void TwitterSearch::requestSearchResults( QString query, int option, uint sinceStatusId, uint count, uint page )
{
    kDebug();

    KUrl url = buildUrl( query, option, sinceStatusId, count, page );

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    if( !job ) {
        kDebug() << "Cannot create a http GET request!";
        emit error( i18n( "Unable to fetch search results." ) );
        return;
    }

    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResultsReturned( KJob* ) ) );
    job->start();
}

void TwitterSearch::searchResultsReturned( KJob* job )
{
    kDebug();
    if( job == 0 ) {
        kDebug() << "job is a null pointer";
        emit error( i18n( "Unable to fetch search results." ) );
        return;
    }

    if( job->error() ) {
        kError() << "Error: " << job->errorString();
        emit error( i18n( "Unable to fetch search results: %1", job->errorString() ) );
        return;
    }
    KIO::StoredTransferJob *jj = qobject_cast<KIO::StoredTransferJob *>( job );
    QList<Status>* statusList = parseAtom( jj->data() );

    emit searchResultsReceived( *statusList );
}

QList<Status>* TwitterSearch::parseAtom( const QByteArray &buffer )
{
    kDebug();
    QDomDocument document;
    QList<Status> *statusList = new QList<Status>;

    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "feed" ) {
        kDebug() << "There is no feed element in Atom feed " << buffer.data();
        return statusList;
    }

    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "entry" ) {
            node = node.nextSibling();
            continue;
        }

        QDomNode entryNode = node.firstChild();
        Status status;
        status.isDMessage = false;

        while ( !entryNode.isNull() ) {
            if ( entryNode.toElement().tagName() == "id" ) {
                // Fomatting example: "tag:search.twitter.com,2005:1235016836"
                int id = 0;
                sscanf( qPrintable( entryNode.toElement().text() ),
                        "tag:search.twitter.com,%*d:%d", &id);
                status.statusId = id;
            } else if ( entryNode.toElement().tagName() == "published" ) {
                // Formatting example: "2009-02-21T19:42:39Z"
                // Need to extract date in similar fashion to dateFromString
                int year, month, day, hour, minute, second;
                sscanf( qPrintable( entryNode.toElement().text() ),
                        "%d-%d-%dT%d:%d:%d%*s", &year, &month, &day, &hour, &minute, &second);
                QDateTime recognized( QDate( year, month, day), QTime( hour, minute, second ) );
                recognized.setTimeSpec( Qt::UTC );
                status.creationDateTime = recognized;
            } else if ( entryNode.toElement().tagName() == "title" ) {
                status.content = entryNode.toElement().text();
            } else if ( entryNode.toElement().tagName() == "twitter:source" ) {
                status.source = entryNode.toElement().text();
            } else if ( entryNode.toElement().tagName() == "link" &&
                        entryNode.toElement().attributeNode( "rel" ).value() == "image") {

                QDomAttr imageAttr = entryNode.toElement().attributeNode( "href" );
                status.user.profileImageUrl = imageAttr.value();

            } else if ( entryNode.toElement().tagName() == "author") {

                QDomNode userNode = entryNode.firstChild();
                while ( !userNode.isNull() )
                {
                    if ( userNode.toElement().tagName() == "name" )
                    {
                        QString fullName = userNode.toElement().text();
                        int bracketPos = fullName.indexOf( " ", 0 );

                        QString screenName = fullName.left( bracketPos );
                        QString name = fullName.right ( fullName.size() - bracketPos - 2 );
                        name.chop( 1 );

                        status.user.name = name;
                        status.user.screenName = screenName;
                    }
                    userNode = userNode.nextSibling();
                }

            }

            entryNode = entryNode.nextSibling();
        }
        status.isFavorited = false;
        status.isTruncated = false;
        status.replyToStatusId = 0;
        statusList->insert( 0, status );
        node = node.nextSibling();
    }

    return statusList;
}
