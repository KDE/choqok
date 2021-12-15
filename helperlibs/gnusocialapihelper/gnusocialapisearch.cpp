/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gnusocialapisearch.h"

#include <QDomElement>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include "twitterapiaccount.h"

#include "gnusocialapidebug.h"

const QRegExp GNUSocialApiSearch::m_rId(QLatin1String("tag:.+,[\\d-]+:(\\d+)"));
const QRegExp GNUSocialApiSearch::mIdRegExp(QLatin1String("(?:user|(?:.*notice))/([0-9]+)"));

GNUSocialApiSearch::GNUSocialApiSearch(QObject *parent): TwitterApiSearch(parent)
{
    qCDebug(CHOQOK);
    mSearchCode[ReferenceGroup] = QLatin1Char('!');
    mSearchCode[ToUser] = QLatin1Char('@');
    mSearchCode[FromUser].clear();
    mSearchCode[ReferenceHashtag] = QLatin1Char('#');

    mSearchTypes[ReferenceHashtag].first = i18nc("Dents are Identica posts", "Dents Including This Hashtag");
    mSearchTypes[ReferenceHashtag].second = true;

    mSearchTypes[ReferenceGroup].first = i18nc("Dents are Identica posts", "Dents Including This Group");
    mSearchTypes[ReferenceGroup].second = false;

    mSearchTypes[FromUser].first = i18nc("Dents are Identica posts", "Dents From This User");
    mSearchTypes[FromUser].second = false;

    mSearchTypes[ToUser].first = i18nc("Dents are Identica posts", "Dents To This User");
    mSearchTypes[ToUser].second = false;

}

GNUSocialApiSearch::~GNUSocialApiSearch()
{

}

QUrl GNUSocialApiSearch::buildUrl(const SearchInfo &searchInfo,
                              QString sinceStatusId, uint count, uint page)
{
    qCDebug(CHOQOK);

    QString formattedQuery;
    switch (searchInfo.option) {
    case ToUser:
        formattedQuery = searchInfo.query + QLatin1String("/replies/rss");
        break;
    case FromUser:
        formattedQuery = searchInfo.query + QLatin1String("/rss");
        break;
    case ReferenceGroup:
        formattedQuery = QLatin1String("group/") + searchInfo.query + QLatin1String("/rss");
        break;
    case ReferenceHashtag:
        formattedQuery = searchInfo.query;
        break;
    default:
        formattedQuery = searchInfo.query + QLatin1String("/rss");
        break;
    };

    QUrl url;
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>(searchInfo.account);
    Q_ASSERT(theAccount);
    if (searchInfo.option == ReferenceHashtag) {
        url = theAccount->apiUrl();
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1String("/search.atom"));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QLatin1String("q"), formattedQuery);
        if (!sinceStatusId.isEmpty()) {
            urlQuery.addQueryItem(QLatin1String("since_id"), sinceStatusId);
        }
        int cntStr;
        if (count && count <= 100) { // GNU Social allows max 100 notices
            cntStr = count;
        } else {
            cntStr = 100;
        }
        urlQuery.addQueryItem(QLatin1String("rpp"), QString::number(cntStr));
        if (page > 1) {
            urlQuery.addQueryItem(QLatin1String("page"), QString::number(page));
        }
        url.setQuery(urlQuery);
    } else {
        url = QUrl(theAccount->apiUrl().url().remove(QLatin1String("/api"), Qt::CaseInsensitive));
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (formattedQuery));
    }
    return url;
}

void GNUSocialApiSearch::requestSearchResults(const SearchInfo &searchInfo,
        const QString &sinceStatusId,
        uint count, uint page)
{
    qCDebug(CHOQOK);
    QUrl url = buildUrl(searchInfo, sinceStatusId, count, page);
    qCDebug(CHOQOK) << url;
    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if (!job) {
        qCCritical(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    mSearchJobs[job] = searchInfo;
    connect(job, &KIO::StoredTransferJob::result, this,
            (void (GNUSocialApiSearch::*)(KJob*))&GNUSocialApiSearch::searchResultsReturned);
    job->start();
}

void GNUSocialApiSearch::searchResultsReturned(KJob *job)
{
    qCDebug(CHOQOK);
    if (job == nullptr) {
        qCDebug(CHOQOK) << "job is a null pointer";
        Q_EMIT error(i18n("Unable to fetch search results."));
        return;
    }

    SearchInfo info = mSearchJobs.take(job);

    if (job->error()) {
        qCCritical(CHOQOK) << "Error:" << job->errorString();
        Q_EMIT error(i18n("Unable to fetch search results: %1", job->errorString()));
        return;
    }
    KIO::StoredTransferJob *jj = qobject_cast<KIO::StoredTransferJob *>(job);
    QList<Choqok::Post *> postsList;
    if (info.option == ReferenceHashtag) {
        postsList = parseAtom(jj->data());
    } else {
        postsList = parseRss(jj->data());
    }

    qCDebug(CHOQOK) << "Emiting searchResultsReceived()";
    Q_EMIT searchResultsReceived(info, postsList);
}

QString GNUSocialApiSearch::optionCode(int option)
{
    return mSearchCode[option];
}

QList< Choqok::Post * > GNUSocialApiSearch::parseAtom(const QByteArray &buffer)
{
    QDomDocument document;
    QList<Choqok::Post *> statusList;

    document.setContent(buffer);

    QDomElement root = document.documentElement();

    if (root.tagName() != QLatin1String("feed")) {
        qCDebug(CHOQOK) << "There is no feed element in Atom feed " << buffer.data();
        return statusList;
    }

    QDomNode node = root.firstChild();
    QString timeStr;
    while (!node.isNull()) {
        if (node.toElement().tagName() != QLatin1String("entry")) {
            node = node.nextSibling();
            continue;
        }

        QDomNode entryNode = node.firstChild();
        Choqok::Post *status = new Choqok::Post;
        status->isPrivate = false;

        while (!entryNode.isNull()) {
            QDomElement elm = entryNode.toElement();
            if (elm.tagName() == QLatin1String("id")) {
                // Fomatting example: "tag:search.twitter.com,2005:1235016836"
                QString id;
                if (m_rId.exactMatch(elm.text())) {
                    id = m_rId.cap(1);
                }
                /*                sscanf( qPrintable( elm.text() ),
                "tag:search.twitter.com,%*d:%d", &id);*/
                status->postId = id;
            } else if (elm.tagName() == QLatin1String("published")) {
                // Formatting example: "2009-02-21T19:42:39Z"
                // Need to extract date in similar fashion to dateFromString
                int year, month, day, hour, minute, second;
                sscanf(qPrintable(elm.text()),
                       "%d-%d-%dT%d:%d:%d%*s", &year, &month, &day, &hour, &minute, &second);
                QDateTime recognized(QDate(year, month, day), QTime(hour, minute, second));
                recognized.setTimeSpec(Qt::UTC);
                status->creationDateTime = recognized;
            } else if (elm.tagName() == QLatin1String("title")) {
                status->content = elm.text();
            } else if (elm.tagName() == QLatin1String("link")) {
                if (elm.attribute(QLatin1String("rel")) == QLatin1String("related")) {
                    status->author.profileImageUrl = QUrl::fromUserInput(elm.attribute(QLatin1String("href")));
                } else if (elm.attribute(QLatin1String("rel")) == QLatin1String("alternate")) {
                    status->link = QUrl::fromUserInput(elm.attribute(QLatin1String("href")));
                }
            } else if (elm.tagName() == QLatin1String("author")) {
                QDomNode userNode = entryNode.firstChild();
                while (!userNode.isNull()) {
                    if (userNode.toElement().tagName() == QLatin1String("name")) {
                        QString fullName = userNode.toElement().text();
                        int bracketPos = fullName.indexOf(QLatin1Char(' '), 0);
                        QString screenName = fullName.left(bracketPos);
                        QString name = fullName.right(fullName.size() - bracketPos - 2);
                        name.chop(1);
                        status->author.realName = name;
                        status->author.userName = screenName;
                    }
                    userNode = userNode.nextSibling();
                }
            } else if (elm.tagName() == QLatin1String("twitter:source")) {
                status->source = QUrl::fromPercentEncoding(elm.text().toLatin1());
            }
            entryNode = entryNode.nextSibling();
        }
        status->isFavorited = false;
        statusList.insert(0, status);
        node = node.nextSibling();
    }
    return statusList;
}

QList< Choqok::Post * > GNUSocialApiSearch::parseRss(const QByteArray &buffer)
{
    qCDebug(CHOQOK);
    QDomDocument document;
    QList<Choqok::Post *> statusList;

    document.setContent(buffer);

    QDomElement root = document.documentElement();

    if (root.tagName() != QLatin1String("rdf:RDF")) {
        qCDebug(CHOQOK) << "There is no rdf:RDF element in RSS feed " << buffer.data();
        return statusList;
    }

    QDomNode node = root.firstChild();
    QString timeStr;
    while (!node.isNull()) {
        if (node.toElement().tagName() != QLatin1String("item")) {
            node = node.nextSibling();
            continue;
        }

        Choqok::Post *status = new Choqok::Post;

        QDomAttr statusIdAttr = node.toElement().attributeNode(QLatin1String("rdf:about"));
        QString statusId;
        if (mIdRegExp.exactMatch(statusIdAttr.value())) {
            statusId = mIdRegExp.cap(1);
        }

        status->postId = statusId;

        QDomNode itemNode = node.firstChild();

        while (!itemNode.isNull()) {
            if (itemNode.toElement().tagName() == QLatin1String("title")) {
                QString content = itemNode.toElement().text();

                int nameSep = content.indexOf(QLatin1Char(':'), 0);
                QString screenName = content.left(nameSep);
                QString statusText = content.right(content.size() - nameSep - 2);

                status->author.userName = screenName;
                status->content = statusText;
            } else if (itemNode.toElement().tagName() == QLatin1String("dc:date")) {
                int year, month, day, hour, minute, second;
                sscanf(qPrintable(itemNode.toElement().text()),
                       "%d-%d-%dT%d:%d:%d%*s", &year, &month, &day, &hour, &minute, &second);
                QDateTime recognized(QDate(year, month, day), QTime(hour, minute, second));
                recognized.setTimeSpec(Qt::UTC);
                status->creationDateTime = recognized;
            } else if (itemNode.toElement().tagName() == QLatin1String("dc:creator")) {
                status->author.realName = itemNode.toElement().text();
            } else if (itemNode.toElement().tagName() == QLatin1String("sioc:reply_of")) {
                QDomAttr userIdAttr = itemNode.toElement().attributeNode(QLatin1String("rdf:resource"));
                QString id;
                if (mIdRegExp.exactMatch(userIdAttr.value())) {
                    id = mIdRegExp.cap(1);
                }
                status->replyToPostId = id;
            } else if (itemNode.toElement().tagName() == QLatin1String("statusnet:postIcon")) {
                QDomAttr imageAttr = itemNode.toElement().attributeNode(QLatin1String("rdf:resource"));
                status->author.profileImageUrl = QUrl::fromUserInput(imageAttr.value());
            } else if (itemNode.toElement().tagName() == QLatin1String("link")) {
//                 QDomAttr imageAttr = itemNode.toElement().attributeNode( "rdf:resource" );
                status->link = QUrl::fromUserInput(itemNode.toElement().text());
            } else if (itemNode.toElement().tagName() == QLatin1String("sioc:has_discussion")) {
                status->conversationId = itemNode.toElement().attributeNode(QLatin1String("rdf:resource")).value();
            }

            itemNode = itemNode.nextSibling();
        }

        status->isPrivate = false;
        status->isFavorited = false;
        statusList.insert(0, status);
        node = node.nextSibling();
    }

    return statusList;
}

