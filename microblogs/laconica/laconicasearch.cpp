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

#include "laconicasearch.h"
#include <KDebug>
#include <klocalizedstring.h>
#include <twitterapihelper/twitterapiaccount.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <choqokbehaviorsettings.h>

const QRegExp LaconicaSearch::m_rId("tag:.+,[\\d-]+:(\\d+)");
const QRegExp LaconicaSearch::mIdRegExp("(?:user|(?:.*notice))/([0-9]+)");

LaconicaSearch::LaconicaSearch(QObject* parent): TwitterApiSearch(parent)
{
    kDebug();
    mSearchCode[ReferenceGroup] = '!';
    mSearchCode[ToUser] = '@';
    mSearchCode[FromUser] = QString();
    mSearchCode[ReferenceHashtag] = '#';

    mSearchTypes[ReferenceHashtag].first = i18nc( "Dents are Identica posts", "Dents Including This Hashtag" );
    mSearchTypes[ReferenceHashtag].second = true;

    mSearchTypes[ReferenceGroup].first = i18nc( "Dents are Identica posts", "Dents Including This Group" );
    mSearchTypes[ReferenceGroup].second = false;

    mSearchTypes[FromUser].first = i18nc( "Dents are Identica posts", "Dents From This User" );
    mSearchTypes[FromUser].second = false;

    mSearchTypes[ToUser].first = i18nc( "Dents are Identica posts", "Dents To This User" );
    mSearchTypes[ToUser].second = false;


}

LaconicaSearch::~LaconicaSearch()
{

}

KUrl LaconicaSearch::buildUrl(const SearchInfo &searchInfo,
                              ChoqokId sinceStatusId, uint count, uint page)
{
    kDebug();

    QString formattedQuery;
    switch ( searchInfo.option ) {
        case ToUser:
            formattedQuery = searchInfo.query + "/replies/rss";
            break;
        case FromUser:
            formattedQuery = searchInfo.query + "/rss";
            break;
        case ReferenceGroup:
            formattedQuery = "group/" + searchInfo.query + "/rss";
            break;
        case ReferenceHashtag:
            formattedQuery = searchInfo.query;
            break;
        default:
            formattedQuery = searchInfo.query + "/rss";
            break;
    };

    KUrl url;
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount*>(searchInfo.account);
    Q_ASSERT(theAccount);
    if( searchInfo.option == ReferenceHashtag ) {
        url = theAccount->apiUrl();
        url.addPath("/search.atom");
        url.addQueryItem("q", formattedQuery);
        if( !sinceStatusId.isEmpty() )
            url.addQueryItem( "since_id", sinceStatusId );
        int cntStr = Choqok::BehaviorSettings::countOfPosts();
        if( count && count <= 100 )
            cntStr =  count;
        url.addQueryItem( "rpp", QString::number(cntStr) );
        if( page > 1 )
            url.addQueryItem( "page", QString::number( page ) );
    } else {
        url = theAccount->apiUrl().url(KUrl::AddTrailingSlash).remove("api/", Qt::CaseInsensitive);
        url.addPath( formattedQuery );
    }
    return url;
}

void LaconicaSearch::requestSearchResults(const SearchInfo &searchInfo,
                                          const ChoqokId& sinceStatusId,
                                          uint count, uint page)
{
    kDebug();
    KUrl url = buildUrl( searchInfo, sinceStatusId, count, page );
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

void LaconicaSearch::searchResultsReturned(KJob* job)
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
    QList<Choqok::Post*> postsList;
    if(info.option == ReferenceHashtag)
        postsList = parseAtom( jj->data() );
    else
        postsList = parseRss( jj->data() );

    kDebug()<<"Emiting searchResultsReceived()";
    emit searchResultsReceived( info, postsList );
}

QString LaconicaSearch::optionCode(int option)
{
    return mSearchCode[option];
}

QList< Choqok::Post* > LaconicaSearch::parseAtom(const QByteArray& buffer)
{
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
            } else if ( elm.tagName() == "link") {
                if(elm.attribute( "rel" ) == "related") {
                    status->author.profileImageUrl = elm.attribute( "href" );
                } else if(elm.attribute( "rel" ) == "alternate") {
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

QList< Choqok::Post* > LaconicaSearch::parseRss(const QByteArray& buffer)
{
    kDebug();
    QDomDocument document;
    QList<Choqok::Post*> statusList;

    document.setContent( buffer );

    QDomElement root = document.documentElement();

    if ( root.tagName() != "rdf:RDF" ) {
        kDebug() << "There is no rdf:RDF element in RSS feed " << buffer.data();
        return statusList;
    }

    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "item" ) {
            node = node.nextSibling();
            continue;
        }

        Choqok::Post *status = new Choqok::Post;

        QDomAttr statusIdAttr = node.toElement().attributeNode( "rdf:about" );
        ChoqokId statusId;
    if(mIdRegExp.exactMatch(statusIdAttr.value())) {
      statusId = mIdRegExp.cap(1);
    }

//         if( statusId <= mSinceStatusId )
//         {
//             node = node.nextSibling();
//             continue;
//         }

        status->postId = statusId;

        QDomNode itemNode = node.firstChild();

        while( !itemNode.isNull() )
        {
            if( itemNode.toElement().tagName() == "title" )
            {
                QString content = itemNode.toElement().text();

                int nameSep = content.indexOf( ':', 0 );
                QString screenName = content.left( nameSep );
                QString statusText = content.right( content.size() - nameSep - 2 );

                status->author.userName = screenName;
                status->content = statusText;
            } else if ( itemNode.toElement().tagName() == "dc:date" ) {
                int year, month, day, hour, minute, second;
                sscanf( qPrintable( itemNode.toElement().text() ),
                        "%d-%d-%dT%d:%d:%d%*s", &year, &month, &day, &hour, &minute, &second);
                QDateTime recognized( QDate( year, month, day), QTime( hour, minute, second ) );
                recognized.setTimeSpec( Qt::UTC );
                status->creationDateTime = recognized;
            } else if ( itemNode.toElement().tagName() == "dc:creator" ) {
                status->author.realName = itemNode.toElement().text();
            } else if ( itemNode.toElement().tagName() == "sioc:reply_of" ) {
                QDomAttr userIdAttr = itemNode.toElement().attributeNode( "rdf:resource" );
                ChoqokId id;
                if(mIdRegExp.exactMatch(userIdAttr.value())) {
                    id = mIdRegExp.cap(1);
                }
                status->replyToPostId = id;
            } else if ( itemNode.toElement().tagName() == "statusnet:postIcon" ) {
                QDomAttr imageAttr = itemNode.toElement().attributeNode( "rdf:resource" );
                status->author.profileImageUrl = imageAttr.value();
            } else if ( itemNode.toElement().tagName() == "link" ) {
                QDomAttr imageAttr = itemNode.toElement().attributeNode( "rdf:resource" );
                status->link = itemNode.toElement().text();
            }

            itemNode = itemNode.nextSibling();
        }

        status->isPrivate = false;
        status->isFavorited = false;
        statusList.insert( 0, status );
        node = node.nextSibling();
    }

    return statusList;
}


#include "laconicasearch.moc"
