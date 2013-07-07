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

#include "twittersearch.h"
#include <KDebug>
#include <klocalizedstring.h>
#include <twitterapihelper/twitterapiaccount.h>
#include <twitterapihelper/twitterapimicroblog.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include "choqokbehaviorsettings.h"
#include "twitteraccount.h"
#include <QtOAuth/QtOAuth>
#include <qjson/parser.h>

const QRegExp TwitterSearch::m_rId("tag:search.twitter.com,[0-9]+:([0-9]+)");

TwitterSearch::TwitterSearch(QObject* parent): TwitterApiSearch(parent)
{
    kDebug();
    mSearchCode[CustomSearch].clear();
    mSearchCode[ToUser] = "to:";
    mSearchCode[FromUser] = "from:";
    mSearchCode[ReferenceUser] = '@';
    mSearchCode[ReferenceHashtag] = '#';

    mI18nSearchCode[CustomSearch].clear();
    mI18nSearchCode[ReferenceUser] = '@';
    mI18nSearchCode[ReferenceHashtag] = '#';
    mI18nSearchCode[ToUser] = i18nc("Posts sent to user", "To:");
    mI18nSearchCode[FromUser] = i18nc("Posts from user, Sent by user", "From:");

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

    QOAuth::ParamMap param;
    QString query = searchInfo.query;
    int option = searchInfo.option;

    QString formattedQuery = mSearchCode[option] + query;
    KUrl url( "https://api.twitter.com/1.1/search/tweets.json" );
    KUrl tmpUrl(url);
    url.addQueryItem("q", formattedQuery);
    QString q = url.query();
    param.insert( "q", q.mid(q.indexOf('=') + 1).toLatin1() );
    if( !sinceStatusId.isEmpty() ) {
        url.addQueryItem( "since_id", sinceStatusId );
        param.insert( "since_id", sinceStatusId.toLatin1() );
    }
    int cntStr = Choqok::BehaviorSettings::countOfPosts();
    if( count && count <= 100 )	// Twitter API specifies a max count of 100
        cntStr =  count;
    else
      cntStr = 100;
    url.addQueryItem( "count", QString::number(cntStr) );
    param.insert( "count", QString::number(cntStr).toLatin1() );
    if( page > 1 ) {
        url.addQueryItem( "page", QString::number( page ) );
        param.insert( "page", QString::number( page ).toLatin1() );
    }

    kDebug()<<url;
    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    if( !job ) {
        kError() << "Cannot create an http GET request!";
        return;
    }

    TwitterAccount* account = qobject_cast< TwitterAccount* >(searchInfo.account);
    TwitterApiMicroBlog *microblog = qobject_cast<TwitterApiMicroBlog*>(account->microblog());

    job->addMetaData("customHTTPHeader", "Authorization: " + microblog->authorizationHeader(account, tmpUrl, QOAuth::GET, param));

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
    QList<Choqok::Post*> postsList = parseJson( jj->data() );


    emit searchResultsReceived( info, postsList );
}

QList< Choqok::Post* > TwitterSearch::parseJson(QByteArray buffer)
{
    bool ok;
    QList<Choqok::Post*> statusList;
    QJson::Parser parser;
    QVariantMap map = parser.parse(buffer, &ok).toMap();

    if ( ok && map.contains("statuses") ) {
        QVariantList list = map["statuses"].toList();
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for(; it != endIt; ++it){
            statusList.prepend(readStatusesFromJsonMap(it->toMap()));
        }
    }
    return statusList;
}

Choqok::Post* TwitterSearch::readStatusesFromJsonMap(const QVariantMap& var)
{
    Choqok::Post *post = new Choqok::Post;

    post->content = var["text"].toString();
    post->creationDateTime = dateFromString(var["created_at"].toString());
    post->postId = var["id"].toString();
    post->source = var["source"].toString();
    QVariantMap userMap = var["user"].toMap();
    post->author.realName = userMap["name"].toString();
    post->author.userName = userMap["screen_name"].toString();
    post->author.profileImageUrl = userMap["profile_image_url"].toString();
    post->isPrivate = false;
    post->isFavorited = false;
    post->replyToPostId = var["in_reply_to_status_id_str"].toString();
    post->replyToUserName = var["in_reply_to_screen_name"].toString();

    post->link = QString ( "https://twitter.com/%1/status/%2" ).arg ( post->author.userName ).arg ( post->postId );

    return post;
}

QString TwitterSearch::optionCode(int option)
{
    return mI18nSearchCode[option];
}

TwitterSearch::~TwitterSearch()
{
}

#include "twittersearch.moc"
