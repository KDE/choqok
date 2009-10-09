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
#include <KDebug>
#include <klocalizedstring.h>
#include <twitterapihelper/twitterapiaccount.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include "choqokbehaviorsettings.h"

const QRegExp TwitterSearch::m_rId("tag:search.twitter.com,[0-9]+:([0-9]+)");

TwitterSearch::TwitterSearch(QObject* parent): TwitterApiSearch(parent)
{
    kDebug();
    mSearchCode[CustomSearch] = QString();
    mSearchCode[ToUser] = "to:";
    mSearchCode[FromUser] = "from:";
    mSearchCode[ReferenceUser] = '@';
    mSearchCode[ReferenceHashtag] = '#';

    mSearchTypes[CustomSearch].first = i18n( "Custom Search" );
    mSearchTypes[CustomSearch].second = true;

    mSearchTypes[ToUser].first = i18nc( "Tweets are Twitter posts",  "Tweets To This User" );
    mSearchTypes[ToUser].second = true;

    mSearchTypes[FromUser].first = i18nc( "Tweets are Twitter posts", "Tweets From This User" );
    mSearchTypes[FromUser].second = true;

    mSearchTypes[ReferenceUser].first = i18nc( "Tweets are Twitter posts", "Tweets Including This Username" );
    mSearchTypes[ReferenceUser].second = true;

    mSearchTypes[ReferenceHashtag].first = i18nc( "Tweets are Twitter posts", "Tweets Including This Hashtag" );
    mSearchTypes[ReferenceHashtag].second = true;
}

void TwitterSearch::requestSearchResults(const SearchInfo &searchInfo,
                                         const ChoqokId& sinceStatusId,
                                         uint count, uint page)
{
    kDebug();

    KUrl url = buildUrl( searchInfo.query, searchInfo.option, sinceStatusId, count, page );
    kDebug()<<url;
    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    if( !job ) {
        kError() << "Cannot create an http GET request!";
        return;
    }
    mSearchJobs[job] = searchInfo;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResultsReturned( KJob* ) ) );
    job->start();
}

void TwitterSearch::searchResultsReturned(KJob* job)
{
    kDebug();
    if( job == 0 ) {
        kDebug() << "job is a null pointer";
        emit error( i18n( "Unable to fetch search results." ) );
        return;
    }

    SearchInfo info = mSearchJobs.take(job);

    if( job->error() ) {
        kError() << "Error: " << job->errorString();
        emit error( i18n( "Unable to fetch search results: %1", job->errorString() ) );
        return;
    }
    KIO::StoredTransferJob *jj = qobject_cast<KIO::StoredTransferJob *>( job );
    QList<Choqok::Post*> postsList = parseAtom( jj->data() );


    emit searchResultsReceived( info, postsList );
}

QList< Choqok::Post* > TwitterSearch::parseAtom(const QByteArray& buffer)
{
    kDebug();
    QDomDocument document;
    QList<Choqok::Post*> statusList;

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
        Choqok::Post *status = new Choqok::Post;
        status->isPrivate = false;

        while ( !entryNode.isNull() ) {
            QDomElement elm = entryNode.toElement();
            if ( elm.tagName() == "id" ) {
                // Fomatting example: "tag:search.twitter.com,2005:1235016836"
                ChoqokId id;
                if(m_rId.exactMatch(elm.text())) {
                    id = m_rId.cap(1);
                }
                /*                sscanf( qPrintable( elm.text() ),
                "tag:search.twitter.com,%*d:%d", &id);*/
                status->postId = id;
            } else if ( elm.tagName() == "published" ) {
                // Formatting example: "2009-02-21T19:42:39Z"
                // Need to extract date in similar fashion to dateFromString
                int year, month, day, hour, minute, second;
                sscanf( qPrintable( elm.text() ),
                        "%d-%d-%dT%d:%d:%d%*s", &year, &month, &day, &hour, &minute, &second);
                        QDateTime recognized( QDate( year, month, day), QTime( hour, minute, second ) );
                        recognized.setTimeSpec( Qt::UTC );
                        status->creationDateTime = recognized;
            } else if ( elm.tagName() == "title" ) {
                status->content = elm.text();
            } else if ( elm.tagName() == "twitter:source" ) {
                status->source = elm.text();
            } else if ( elm.tagName() == "link") {
                if(elm.attributeNode( "rel" ).value() == "image") {
                status->author.profileImageUrl = elm.attribute( "href" );
                } else if(elm.attributeNode( "rel" ).value() == "alternate") {
                    status->link = elm.attribute( "href" );
                }
            } else if ( elm.tagName() == "author") {
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

                        status->author.realName = name;
                        status->author.userName = screenName;
                    }
                    userNode = userNode.nextSibling();
                }
            }
            entryNode = entryNode.nextSibling();
        }
        status->isFavorited = false;
        statusList.insert( 0, status );
        node = node.nextSibling();
    }

    return statusList;
}

QString TwitterSearch::optionCode(int option)
{
    return mSearchCode[option];
}

TwitterSearch::~TwitterSearch()
{
}

KUrl TwitterSearch::buildUrl( QString query, int option, ChoqokId sinceStatusId,
                              uint count, uint page)
{
    kDebug();
    QString formattedQuery = mSearchCode[option] + query;
    KUrl url( "http://search.twitter.com/search.atom" );
    url.addQueryItem("q", formattedQuery);
    if( !sinceStatusId.isEmpty() )
        url.addQueryItem( "since_id", sinceStatusId );
    int cntStr = Choqok::BehaviorSettings::countOfPosts();
    if( count && count <= 100 )
        cntStr =  count;
    url.addQueryItem( "rpp", QString::number(cntStr) );
    if( page > 1 )
        url.addQueryItem( "page", QString::number( page ) );

    return url;
}

#include "twittersearch.moc"
