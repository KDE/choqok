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

#include "search.h"
//#include "account.h"
#include <QDomDocument>
#include <KDebug>

Search::Search( Account* account, const QString & rId, const QString & searchUrl, QObject *parent )
    : QObject( parent ),m_rId(rId)
{
    mAccount = account;
    mSearchUrl = searchUrl;
    /**
     * TODO Support multiple pages on search results! instead of just first 20 latest results!
     */
}

Search::~Search()
{
    mSinceStatusId = 0;
}

QMap<int, QPair<QString, bool> > Search::getSearchTypes()
{
    return mSearchTypes;
}

KUrl Search::buildUrl( QString query, int option, qulonglong sinceStatusId, qulonglong count, qulonglong page )
{
    Q_UNUSED(query);
    Q_UNUSED(option);
    Q_UNUSED(sinceStatusId);
    Q_UNUSED(count);
    Q_UNUSED(page);
    return KUrl();
}

void Search::requestSearchResults( QString query, int option, qulonglong sinceStatusId, qulonglong count, qulonglong page )
{
    Q_UNUSED(query);
    Q_UNUSED(option);
    Q_UNUSED(sinceStatusId);
    Q_UNUSED(count);
    Q_UNUSED(page);
}

void Search::searchResultsReturned( KJob* job )
{
    Q_UNUSED(job);
}

void Search::singleStatusReturned( KJob* job )
{
    Q_UNUSED(job);
}

QList<Status>* Search::parseAtom( const QByteArray &buffer )
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
                qulonglong id = 0;
                if(m_rId.exactMatch(entryNode.toElement().text())) {
                    id = m_rId.cap(1).toULongLong();
                }
                /*                sscanf( qPrintable( entryNode.toElement().text() ),
                "tag:search.twitter.com,%*d:%d", &id);*/
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
