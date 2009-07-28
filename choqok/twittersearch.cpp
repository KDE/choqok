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

TwitterSearch::TwitterSearch( Account* account, const QString & searchUrl, QObject *parent ) :
        Search(account, "tag:search.twitter.com,[0-9]+:([0-9]+)", searchUrl, parent)
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

KUrl TwitterSearch::buildUrl( QString query, int option, qulonglong sinceStatusId, qulonglong count, qulonglong page )
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

void TwitterSearch::requestSearchResults( QString query, int option, qulonglong sinceStatusId, qulonglong count, qulonglong page )
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
