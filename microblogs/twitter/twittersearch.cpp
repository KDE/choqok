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

#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include "twitterapimicroblog.h"

#include "twitteraccount.h"
#include "twitterdebug.h"

const QRegExp TwitterSearch::m_rId(QLatin1String("tag:search.twitter.com,[0-9]+:([0-9]+)"));

TwitterSearch::TwitterSearch(QObject *parent): TwitterApiSearch(parent)
{
    qCDebug(CHOQOK);
    mSearchCode[CustomSearch].clear();
    mSearchCode[ToUser] = QLatin1String("to:");
    mSearchCode[FromUser] = QLatin1String("from:");
    mSearchCode[ReferenceUser] = QLatin1Char('@');
    mSearchCode[ReferenceHashtag] = QLatin1Char('#');

    mI18nSearchCode[CustomSearch].clear();
    mI18nSearchCode[ReferenceUser] = QLatin1Char('@');
    mI18nSearchCode[ReferenceHashtag] = QLatin1Char('#');
    mI18nSearchCode[ToUser] = i18nc("Posts sent to user", "To:");
    mI18nSearchCode[FromUser] = i18nc("Posts from user, Sent by user", "From:");

    mSearchTypes[CustomSearch].first = i18n("Custom Search");
    mSearchTypes[CustomSearch].second = true;

    mSearchTypes[ToUser].first = i18nc("Tweets are Twitter posts",  "Tweets To This User");
    mSearchTypes[ToUser].second = true;

    mSearchTypes[FromUser].first = i18nc("Tweets are Twitter posts", "Tweets From This User");
    mSearchTypes[FromUser].second = true;

    mSearchTypes[ReferenceUser].first = i18nc("Tweets are Twitter posts", "Tweets Including This Username");
    mSearchTypes[ReferenceUser].second = true;

    mSearchTypes[ReferenceHashtag].first = i18nc("Tweets are Twitter posts", "Tweets Including This Hashtag");
    mSearchTypes[ReferenceHashtag].second = true;
}

void TwitterSearch::requestSearchResults(const SearchInfo &searchInfo,
        const QString &sinceStatusId, uint count, uint page)
{
    Q_UNUSED(page)
    qCDebug(CHOQOK);

    TwitterAccount *account = qobject_cast< TwitterAccount * >(searchInfo.account);
    QUrl url = account->apiUrl();

    QUrlQuery urlQuery;

    const QString query = searchInfo.query;
    if (searchInfo.option == TwitterSearch::FromUser) {
        url.setPath(url.path() + QLatin1String("/statuses/user_timeline.json"));

        urlQuery.addQueryItem(QLatin1String("screen_name"), query);
    } else {
        url.setPath(url.path() + QLatin1String("/search/tweets.json"));

        const QByteArray formattedQuery(QUrl::toPercentEncoding(mSearchCode[searchInfo.option] + query));
        urlQuery.addQueryItem(QLatin1String("q"), QString::fromLatin1(formattedQuery));
    }

    if (!sinceStatusId.isEmpty()) {
        urlQuery.addQueryItem(QLatin1String("since_id"), sinceStatusId);
    }

    int cntStr;
    if (count && count <= 100) { // Twitter API specifies a max count of 100
        cntStr = count;
    } else {
        cntStr = 100;
    }
    urlQuery.addQueryItem(QLatin1String("tweet_mode"), QLatin1String("extended"));
    urlQuery.addQueryItem(QLatin1String("count"), QString::number(cntStr));

    url.setQuery(urlQuery);

    qCDebug(CHOQOK) << url;
    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if (!job) {
        qCCritical(CHOQOK) << "Cannot create an http GET request!";
        return;
    }

    TwitterApiMicroBlog *microblog = qobject_cast<TwitterApiMicroBlog *>(account->microblog());

    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(microblog->authorizationHeader(account, url, QNetworkAccessManager::GetOperation)));

    mSearchJobs[job] = searchInfo;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(searchResultsReturned(KJob*)));
    job->start();
}

void TwitterSearch::searchResultsReturned(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "job is a null pointer";
        Q_EMIT error(i18n("Unable to fetch search results."));
        return;
    }

    const SearchInfo info = mSearchJobs.take(job);
    QList<Choqok::Post *> postsList;
    if (job->error()) {
        qCCritical(CHOQOK) << "Error:" << job->errorString();
        Q_EMIT error(i18n("Unable to fetch search results: %1", job->errorString()));
    } else {
        KIO::StoredTransferJob *jj = qobject_cast<KIO::StoredTransferJob *>(job);
        const QJsonDocument json = QJsonDocument::fromJson(jj->data());

        if (!json.isNull()) {
            if (info.option == TwitterSearch::FromUser) {
                for (const QVariant elem: json.toVariant().toList()) {
                    postsList.prepend(readStatusesFromJsonMap(elem.toMap()));
                }
            } else {
                const QVariantMap map = json.toVariant().toMap();

                if (map.contains(QLatin1String("statuses"))) {
                    for (const QVariant elem: map[QLatin1String("statuses")].toList()) {
                        postsList.prepend(readStatusesFromJsonMap(elem.toMap()));
                    }
                }
            }
        }
    }

    Q_EMIT searchResultsReceived(info, postsList);
}

Choqok::Post *TwitterSearch::readStatusesFromJsonMap(const QVariantMap &var)
{
    Choqok::Post *post = new Choqok::Post;

    post->content = var[QLatin1String("text")].toString();

    // Support for extended tweet_mode
    if (var.contains(QLatin1String("full_text"))) {
        post->content = var[QLatin1String("full_text")].toString();
    }

    //%*s %s %d %d:%d:%d %d %d
    post->creationDateTime = dateFromString(var[QLatin1String("created_at")].toString());
    post->postId = var[QLatin1String("id")].toString();
    post->source = var[QLatin1String("source")].toString();
    QVariantMap userMap = var[QLatin1String("user")].toMap();
    post->author.realName = userMap[QLatin1String("name")].toString();
    post->author.userName = userMap[QLatin1String("screen_name")].toString();
    post->author.profileImageUrl = userMap[QLatin1String("profile_image_url")].toUrl();
    post->isPrivate = false;
    post->isFavorited = false;
    post->replyToPostId = var[QLatin1String("in_reply_to_status_id_str")].toString();
    post->replyToUser.userName = var[QLatin1String("in_reply_to_screen_name")].toString();

    post->link =  QUrl::fromUserInput(QStringLiteral("https://twitter.com/%1/status/%2").arg(post->author.userName).arg(post->postId));

    return post;
}

QString TwitterSearch::optionCode(int option)
{
    return mI18nSearchCode[option];
}

TwitterSearch::~TwitterSearch()
{
}

