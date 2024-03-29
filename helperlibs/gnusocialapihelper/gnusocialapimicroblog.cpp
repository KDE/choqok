/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gnusocialapimicroblog.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>

#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

#include "account.h"
#include "accountmanager.h"
#include "choqokappearancesettings.h"
#include "composerwidget.h"
#include "editaccountwidget.h"
#include "mediamanager.h"
#include "microblogwidget.h"
#include "postwidget.h"
#include "timelinewidget.h"

#include "twitterapimicroblogwidget.h"
#include "twitterapipostwidget.h"
#include "twitterapitimelinewidget.h"

#include "gnusocialapiaccount.h"
#include "gnusocialapicomposerwidget.h"
#include "gnusocialapidebug.h"
#include "gnusocialapidmessagedialog.h"
#include "gnusocialapipostwidget.h"
#include "gnusocialapisearch.h"

GNUSocialApiMicroBlog::GNUSocialApiMicroBlog(const QString &componentName, QObject *parent = nullptr)
    : TwitterApiMicroBlog(componentName, parent), friendsPage(1)
{
    qCDebug(CHOQOK);
    setServiceName(QLatin1String("GNU social"));
    mTimelineInfos[QLatin1String("ReTweets")]->name = i18nc("Timeline name", "Repeated");
    mTimelineInfos[QLatin1String("ReTweets")]->description = i18nc("Timeline description", "Your posts that were repeated by others");
}

GNUSocialApiMicroBlog::~GNUSocialApiMicroBlog()
{
    qCDebug(CHOQOK);
}

Choqok::Account *GNUSocialApiMicroBlog::createNewAccount(const QString &alias)
{
    GNUSocialApiAccount *acc = qobject_cast<GNUSocialApiAccount *>(Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return new GNUSocialApiAccount(this, alias);
    } else {
        return nullptr;
    }
}

Choqok::UI::MicroBlogWidget *GNUSocialApiMicroBlog::createMicroBlogWidget(Choqok::Account *account, QWidget *parent)
{
    return new TwitterApiMicroBlogWidget(account, parent);
}

Choqok::UI::TimelineWidget *GNUSocialApiMicroBlog::createTimelineWidget(Choqok::Account *account,
        const QString &timelineName, QWidget *parent)
{
    return new TwitterApiTimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget *GNUSocialApiMicroBlog::createPostWidget(Choqok::Account *account,
        Choqok::Post *post, QWidget *parent)
{
    return new GNUSocialApiPostWidget(account, post, parent);
}

Choqok::UI::ComposerWidget *GNUSocialApiMicroBlog::createComposerWidget(Choqok::Account *account, QWidget *parent)
{
    return new GNUSocialApiComposerWidget(account, parent);
}

Choqok::Post *GNUSocialApiMicroBlog::readPost(Choqok::Account *account, const QVariantMap &var, Choqok::Post *post)
{
    if (!post) {
        qCCritical(CHOQOK) << "post is NULL!";
        return nullptr;
    }

    if (var[QLatin1String("source")].toString().compare(QLatin1String("linkback")) == 0) {
        // Skip linkback statuses
        return nullptr;
    }

    post = TwitterApiMicroBlog::readPost(account, var, post);

    QUrl profileUrl = var[QLatin1String("user")].toMap()[QLatin1String("statusnet_profile_url")].toUrl();
    post->author.homePageUrl = profileUrl;

    const QVariantMap retweeted = var[QLatin1String("retweeted_status")].toMap();

    if (!retweeted.isEmpty()) {
        profileUrl = retweeted[QLatin1String("user")].toMap()[QLatin1String("statusnet_profile_url")].toUrl();
        post->repeatedFromUser.homePageUrl = profileUrl;
    }

    if (var.contains(QLatin1String("uri"))) {
        post->link = var[QLatin1String("uri")].toUrl();
    } else if (var.contains(QLatin1String("external_url"))) {
        post->link = var[QLatin1String("external_url")].toUrl();
    } else {
        if (retweeted.contains(QLatin1String("uri"))) {
            post->link = var[QLatin1String("uri")].toUrl();
        } else {
            // Last try, compone the url. However this only works for GNU Social instances.
            post->link = QUrl::fromUserInput(QStringLiteral("%1://%2/notice/%3")
                                             .arg(profileUrl.scheme()).arg(profileUrl.host()).arg(post->postId));
        }
    }

    return post;
}

QUrl GNUSocialApiMicroBlog::profileUrl(Choqok::Account *account, const QString &username) const
{
    if (username.contains(QLatin1Char('@'))) {
        const QStringList lst = username.split(QLatin1Char('@'), QString::SkipEmptyParts);

        if (lst.count() == 2) {
            return QUrl::fromUserInput(QStringLiteral("https://%1/%2").arg(lst[1]).arg(lst[0]));
        } else {
            return QUrl();
        }
    } else {
        GNUSocialApiAccount *acc = qobject_cast<GNUSocialApiAccount *>(account);

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(QLatin1Char('/') + username);

        return url;
    }
}

QUrl GNUSocialApiMicroBlog::postUrl(Choqok::Account *account,  const QString &username,
                                   const QString &postId) const
{
    Q_UNUSED(username)
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *>(account);
    if (acc) {
        QUrl url(acc->homepageUrl());
        url.setPath(url.path() + QStringLiteral("/notice/%1").arg(postId));
        return url;
    } else {
        return QUrl();
    }
}

TwitterApiSearch *GNUSocialApiMicroBlog::searchBackend()
{
    if (!mSearchBackend) {
        mSearchBackend = new GNUSocialApiSearch(this);
    }
    return mSearchBackend;
}

void GNUSocialApiMicroBlog::createPostWithAttachment(Choqok::Account *theAccount, Choqok::Post *post,
        const QString &mediumToAttach)
{
    if (mediumToAttach.isEmpty()) {
        TwitterApiMicroBlog::createPost(theAccount, post);
    } else {
        const QUrl picUrl = QUrl::fromUserInput(mediumToAttach);
        KIO::StoredTransferJob *picJob = KIO::storedGet(picUrl, KIO::Reload, KIO::HideProgressInfo);
        picJob->exec();
        if (picJob->error()) {
            qCCritical(CHOQOK) << "Job error:" << picJob->errorString();
            KMessageBox::detailedError(Choqok::UI::Global::mainWindow(),
                                       i18n("Uploading medium failed: cannot read the medium file."),
                                       picJob->errorString());
            return;
        }
        const QByteArray picData = picJob->data();
        if (picData.count() == 0) {
            qCCritical(CHOQOK) << "Cannot read the media file, please check if it exists.";
            KMessageBox::error(Choqok::UI::Global::mainWindow(),
                               i18n("Uploading medium failed: cannot read the medium file."));
            return;
        }
        ///Documentation: http://identi.ca/notice/17779990
        TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
        QUrl url = account->apiUrl();
        url.setPath(url.path() + QLatin1String("/statuses/update.json"));
        const QMimeDatabase db;
        QByteArray fileContentType = db.mimeTypeForUrl(picUrl).name().toUtf8();

        QMap<QString, QByteArray> formdata;
        formdata[QLatin1String("status")] = post->content.toUtf8();
        formdata[QLatin1String("in_reply_to_status_id")] = post->replyToPostId.toLatin1();
        formdata[QLatin1String("source")] = QCoreApplication::applicationName().toLatin1();

        QMap<QString, QByteArray> mediafile;
        mediafile[QLatin1String("name")] = "media";
        mediafile[QLatin1String("filename")] = picUrl.fileName().toUtf8();
        mediafile[QLatin1String("mediumType")] = fileContentType;
        mediafile[QLatin1String("medium")] = picData;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);

        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        if (!job) {
            qCCritical(CHOQOK) << "Cannot create a http POST request!";
            return;
        }
        job->addMetaData(QStringLiteral("content-type"),
                         QStringLiteral("Content-Type: multipart/form-data; boundary=AaB03x"));
        job->addMetaData(QStringLiteral("customHTTPHeader"),
                         QStringLiteral("Authorization: ") +
                         QLatin1String(authorizationHeader(account, url, QNetworkAccessManager::PostOperation)));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect(job, &KIO::StoredTransferJob::result, this, &GNUSocialApiMicroBlog::slotCreatePost);
        job->start();
    }
}

QString GNUSocialApiMicroBlog::generateRepeatedByUserTooltip(const QString &username)
{
    if (Choqok::AppearanceSettings::showRetweetsInChoqokWay()) {
        return i18n("Repeat of %1", username);
    } else {
        return i18n("Repeated by %1", username);
    }
}

QString GNUSocialApiMicroBlog::repeatQuestion()
{
    return i18n("Repeat this notice?");
}

void GNUSocialApiMicroBlog::listFriendsUsername(TwitterApiAccount *theAccount, bool active)
{
    Q_UNUSED(active);
    friendsList.clear();
    if (theAccount) {
        doRequestFriendsScreenName(theAccount, 1);
    }
}

QStringList GNUSocialApiMicroBlog::readFriendsScreenName(Choqok::Account *theAccount, const QByteArray &buffer)
{
    QStringList list;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        for (const QJsonValue &u: json.array()) {
            const QJsonObject user = u.toObject();

            if (user.contains(QStringLiteral("statusnet_profile_url"))) {
                list.append(user.value(QLatin1String("statusnet_profile_url")).toString());
            }
        }
    } else {
        QString err = i18n("Retrieving the friends list failed. The data returned from the server is corrupted.");
        qCDebug(CHOQOK) << "JSON parse error:the buffer is: \n" << buffer;
        Q_EMIT error(theAccount, ParsingError, err, Critical);
    }
    return list;
}

void GNUSocialApiMicroBlog::requestFriendsScreenName(TwitterApiAccount *theAccount, bool active)
{
    Q_UNUSED(active);
    doRequestFriendsScreenName(theAccount, 1);
}

void GNUSocialApiMicroBlog::showDirectMessageDialog(TwitterApiAccount *theAccount, const QString &toUsername)
{
    qCDebug(CHOQOK);
    if (!theAccount) {
        QAction *act = qobject_cast<QAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount *>(Choqok::AccountManager::self()->findAccount(act->data().toString()));
    }
    GNUSocialApiDMessageDialog *dmsg = new GNUSocialApiDMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    if (!toUsername.isEmpty()) {
        dmsg->setTo(toUsername);
    }
    dmsg->show();
}

void GNUSocialApiMicroBlog::doRequestFriendsScreenName(TwitterApiAccount *theAccount, int page)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1String("/statuses/friends.json"));

    if (page > 1) {
        QUrlQuery urlQuery;
        urlQuery.addQueryItem(QLatin1String("page"), QString::number(page));
        url.setQuery(urlQuery);
    }

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, url, QNetworkAccessManager::GetOperation)));
    mJobsAccount[job] = theAccount;
    connect(job, &KIO::StoredTransferJob::result, this, &GNUSocialApiMicroBlog::slotRequestFriendsScreenName);
    job->start();
}

void GNUSocialApiMicroBlog::slotRequestFriendsScreenName(KJob *job)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>(mJobsAccount.take(job));
    if (job->error()) {
        Q_EMIT error(theAccount, ServerError, i18n("Friends list for account %1 could not be updated:\n%2",
                     theAccount->username(), job->errorString()), Normal);
        return;
    }
    KIO::StoredTransferJob *stJob = qobject_cast<KIO::StoredTransferJob *>(job);
    QStringList newList = readFriendsScreenName(theAccount, stJob->data());
    friendsList << newList;
    if (newList.count() == 100) {
        doRequestFriendsScreenName(theAccount, ++friendsPage);
    } else {
        friendsList.removeDuplicates();
        theAccount->setFriendsList(friendsList);
        Q_EMIT friendsUsernameListed(theAccount, friendsList);
    }
}

void GNUSocialApiMicroBlog::fetchConversation(Choqok::Account *theAccount, const QString &conversationId)
{
    qCDebug(CHOQOK);
    if (conversationId.isEmpty()) {
        return;
    }
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(QStringLiteral("/statusnet/conversation/%1.json").arg(conversationId));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, url, QNetworkAccessManager::GetOperation)));
    mFetchConversationMap[ job ] = conversationId;
    mJobsAccount[ job ] = theAccount;
    connect(job, &KIO::StoredTransferJob::result, this, &GNUSocialApiMicroBlog::slotFetchConversation);
    job->start();
}

QString GNUSocialApiMicroBlog::usernameFromProfileUrl(const QString &profileUrl)
{
    // Remove the initial slash from path
    return QUrl(profileUrl).path().remove(0, 1);
}

QString GNUSocialApiMicroBlog::hostFromProfileUrl(const QString &profileUrl)
{
    return QUrl(profileUrl).host();
}

void GNUSocialApiMicroBlog::slotFetchConversation(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCWarning(CHOQOK) << "NULL Job returned";
        return;
    }
    QList<Choqok::Post *> posts;
    QString conversationId = mFetchConversationMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching conversation failed. %1", job->errorString()), Normal);
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> (job);
        //if(format=="json"){
        posts = readTimeline(theAccount, stj->data());
        //} else {
        //    posts = readTimelineFromXml ( theAccount, stj->data() );
        //}
        if (!posts.isEmpty()) {
            Q_EMIT conversationFetched(theAccount, conversationId, posts);
        }
    }
}

#include "moc_gnusocialapimicroblog.cpp"
