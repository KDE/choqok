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

#include "twitterapimicroblog.h"

#include <QAction>
#include <QJsonDocument>
#include <QMenu>
#include <QStandardPaths>

#include <KIO/Job>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QtOAuth/QtOAuth>

#include "account.h"
#include "accountmanager.h"
#include "application.h"
#include "choqokappearancesettings.h"
#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "editaccountwidget.h"
#include "microblogwidget.h"
#include "notifymanager.h"
#include "postwidget.h"
#include "timelinewidget.h"

#include "twitterapiaccount.h"
#include "twitterapicomposerwidget.h"
#include "twitterapidebug.h"
#include "twitterapidmessagedialog.h"
#include "twitterapipostwidget.h"
#include "twitterapisearch.h"
#include "twitterapisearchdialog.h"
#include "twitterapisearchtimelinewidget.h"

class TwitterApiMicroBlog::Private
{
public:
    Private(): countOfTimelinesToSave(0), friendsCursor(QLatin1String("-1"))
    {
        monthes[QLatin1String("Jan")] = 1;
        monthes[QLatin1String("Feb")] = 2;
        monthes[QLatin1String("Mar")] = 3;
        monthes[QLatin1String("Apr")] = 4;
        monthes[QLatin1String("May")] = 5;
        monthes[QLatin1String("Jun")] = 6;
        monthes[QLatin1String("Jul")] = 7;
        monthes[QLatin1String("Aug")] = 8;
        monthes[QLatin1String("Sep")] = 9;
        monthes[QLatin1String("Oct")] = 10;
        monthes[QLatin1String("Nov")] = 11;
        monthes[QLatin1String("Dec")] = 12;
    }
    int countOfTimelinesToSave;
    QString friendsCursor;
    QMap<QString, int> monthes;
};

TwitterApiMicroBlog::TwitterApiMicroBlog(const QString &componentName, QObject *parent)
    : MicroBlog(componentName, parent), d(new Private)
{
    qCDebug(CHOQOK);
    KConfigGroup grp(KSharedConfig::openConfig(), "TwitterApi");
    format = grp.readEntry("format", "json");

    QStringList timelineTypes;
    timelineTypes << QLatin1String("Home") << QLatin1String("Reply") << QLatin1String("Inbox") << QLatin1String("Outbox") << QLatin1String("Favorite") << QLatin1String("ReTweets") << QLatin1String("Public");
    setTimelineNames(timelineTypes);
    timelineApiPath[QLatin1String("Home")] = QLatin1String("/statuses/home_timeline.%1");
    timelineApiPath[QLatin1String("Reply")] = QLatin1String("/statuses/replies.%1");
    timelineApiPath[QLatin1String("Inbox")] = QLatin1String("/direct_messages.%1");
    timelineApiPath[QLatin1String("Outbox")] = QLatin1String("/direct_messages/sent.%1");
    timelineApiPath[QLatin1String("Favorite")] = QLatin1String("/favorites/list.%1");
    timelineApiPath[QLatin1String("ReTweets")] = QLatin1String("/statuses/retweets_of_me.%1");
    timelineApiPath[QLatin1String("Public")] = QLatin1String("/statuses/public_timeline.%1");
    setTimelineInfos();
}

void TwitterApiMicroBlog::setTimelineInfos()
{
    Choqok::TimelineInfo *t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Home");
    t->description = i18nc("Timeline description", "You and your friends");
    t->icon = QLatin1String("user-home");
    mTimelineInfos[QLatin1String("Home")] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Reply");
    t->description = i18nc("Timeline description", "Replies to you");
    t->icon = QLatin1String("edit-undo");
    mTimelineInfos[QLatin1String("Reply")] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Inbox");
    t->description = i18nc("Timeline description", "Your incoming private messages");
    t->icon = QLatin1String("mail-folder-inbox");
    mTimelineInfos[QLatin1String("Inbox")] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Outbox");
    t->description = i18nc("Timeline description", "Private messages you have sent");
    t->icon = QLatin1String("mail-folder-outbox");
    mTimelineInfos[QLatin1String("Outbox")] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Favorite");
    t->description = i18nc("Timeline description", "Your favorites");
    t->icon = QLatin1String("favorites");
    mTimelineInfos[QLatin1String("Favorite")] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Public");
    t->description = i18nc("Timeline description", "Public timeline");
    t->icon = QLatin1String("folder-green");
    mTimelineInfos[QLatin1String("Public")] = t;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "ReTweets");
    t->description = i18nc("Timeline description", "Your posts that were ReTweeted by others");
    t->icon = QLatin1String("folder-red");
    mTimelineInfos[QLatin1String("ReTweets")] = t;
}

TwitterApiMicroBlog::~TwitterApiMicroBlog()
{
    qDeleteAll(mTimelineInfos);
    delete d;
}

QMenu *TwitterApiMicroBlog::createActionsMenu(Choqok::Account *theAccount, QWidget *parent)
{
    QMenu *menu = MicroBlog::createActionsMenu(theAccount, parent);

    QAction *directMessge = new QAction(QIcon::fromTheme(QLatin1String("mail-message-new")), i18n("Send Private Message..."), menu);
    directMessge->setData(theAccount->alias());
    connect(directMessge, SIGNAL(triggered(bool)), SLOT(showDirectMessageDialog()));
    menu->addAction(directMessge);

    QAction *search = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Search..."), menu);
    search->setData(theAccount->alias());
    connect(search, SIGNAL(triggered(bool)), SLOT(showSearchDialog()));
    menu->addAction(search);

    QAction *updateFriendsList = new QAction(QIcon::fromTheme(QLatin1String("arrow-down")), i18n("Update Friends List"), menu);
    updateFriendsList->setData(theAccount->alias());
    connect(updateFriendsList, SIGNAL(triggered(bool)), SLOT(slotUpdateFriendsList()));
    menu->addAction(updateFriendsList);

    return menu;
}

QList< Choqok::Post * > TwitterApiMicroBlog::loadTimeline(Choqok::Account *account,
        const QString &timelineName)
{
    QList< Choqok::Post * > list;
    if (timelineName.compare(QLatin1String("Favorite")) == 0) {
        return list;    //NOTE Won't cache favorites, and this is for compatibility with older versions!
    }
    qCDebug(CHOQOK) << timelineName;
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);
    QStringList tmpList = postsBackup.groupList();

/// to don't load old archives
    if (tmpList.isEmpty() || !(QDateTime::fromString(tmpList.first()).isValid())) {
        return list;
    }
///--------------

    QList<QDateTime> groupList;
    Q_FOREACH (const QString &str, tmpList) {
        groupList.append(QDateTime::fromString(str));
    }
    qSort(groupList);
    int count = groupList.count();
    if (count) {
        Choqok::Post *st = 0;
        for (int i = 0; i < count; ++i) {
            st = new Choqok::Post;
            KConfigGroup grp(&postsBackup, groupList[i].toString());
            st->creationDateTime = grp.readEntry("creationDateTime", QDateTime::currentDateTime());
            st->postId = grp.readEntry("postId", QString());
            st->content = grp.readEntry("text", QString());
            st->source = grp.readEntry("source", QString());
            st->replyToPostId = grp.readEntry("inReplyToPostId", QString());
            st->replyToUserId = grp.readEntry("inReplyToUserId", QString());
            st->isFavorited = grp.readEntry("favorited", false);
            st->replyToUserName = grp.readEntry("inReplyToUserName", QString());
            st->author.userId = grp.readEntry("authorId", QString());
            st->author.userName = grp.readEntry("authorUserName", QString());
            st->author.realName = grp.readEntry("authorRealName", QString());
            st->author.profileImageUrl = grp.readEntry("authorProfileImageUrl", QString());
            st->author.description = grp.readEntry("authorDescription" , QString());
            st->author.isProtected = grp.readEntry("isProtected", false);
            st->isPrivate = grp.readEntry("isPrivate" , false);
            st->author.location = grp.readEntry("authorLocation", QString());
            st->link = postUrl(account, st->author.userName, st->postId);
            st->isRead = grp.readEntry("isRead", true);
            st->repeatedFromUsername = grp.readEntry("repeatedFrom", QString());
            st->repeatedPostId = grp.readEntry("repeatedPostId", QString());
            st->conversationId = grp.readEntry("conversationId", QString());
            st->media = grp.readEntry("mediaUrl", QString());
            st->mediaSizeWidth = grp.readEntry("mediaWidth", 0);
            st->mediaSizeHeight = grp.readEntry("mediaHeight", 0);

            list.append(st);
        }
        mTimelineLatestId[account][timelineName] = st->postId;
    }
    return list;
}

void TwitterApiMicroBlog::saveTimeline(Choqok::Account *account,
                                       const QString &timelineName,
                                       const QList< Choqok::UI::PostWidget * > &timeline)
{
    if (timelineName.compare(QLatin1String("Favorite")) != 0) {
        qCDebug(CHOQOK);
        QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
        KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);

        ///Clear previous data:
        QStringList prevList = postsBackup.groupList();
        int c = prevList.count();
        if (c > 0) {
            for (int i = 0; i < c; ++i) {
                postsBackup.deleteGroup(prevList[i]);
            }
        }
        QList< Choqok::UI::PostWidget *>::const_iterator it, endIt = timeline.constEnd();
        for (it = timeline.constBegin(); it != endIt; ++it) {
            const Choqok::Post *post = ((*it)->currentPost());
            KConfigGroup grp(&postsBackup, post->creationDateTime.toString());
            grp.writeEntry("creationDateTime", post->creationDateTime);
            grp.writeEntry("postId", post->postId);
            grp.writeEntry("text", post->content);
            grp.writeEntry("source", post->source);
            grp.writeEntry("inReplyToPostId", post->replyToPostId);
            grp.writeEntry("inReplyToUserId", post->replyToUserId);
            grp.writeEntry("favorited", post->isFavorited);
            grp.writeEntry("inReplyToUserName", post->replyToUserName);
            grp.writeEntry("authorId", post->author.userId);
            grp.writeEntry("authorUserName", post->author.userName);
            grp.writeEntry("authorRealName", post->author.realName);
            grp.writeEntry("authorProfileImageUrl", post->author.profileImageUrl);
            grp.writeEntry("authorDescription" , post->author.description);
            grp.writeEntry("isPrivate" , post->isPrivate);
            grp.writeEntry("authorLocation" , post->author.location);
            grp.writeEntry("isProtected" , post->author.isProtected);
            grp.writeEntry("isRead" , post->isRead);
            grp.writeEntry("repeatedFrom", post->repeatedFromUsername);
            grp.writeEntry("repeatedPostId", post->repeatedPostId);
            grp.writeEntry("conversationId", post->conversationId);
            grp.writeEntry("mediaUrl", post->media);
            grp.writeEntry("mediaWidth", post->mediaSizeWidth);
            grp.writeEntry("mediaHeight", post->mediaSizeHeight);
        }
        postsBackup.sync();
    }
    if (Choqok::Application::isShuttingDown()) {
        --d->countOfTimelinesToSave;
        if (d->countOfTimelinesToSave < 1) {
            Q_EMIT readyForUnload();
        }
    }
}

Choqok::UI::ComposerWidget *TwitterApiMicroBlog::createComposerWidget(Choqok::Account *account, QWidget *parent)
{
    return new TwitterApiComposerWidget(account, parent);
}

TwitterApiSearchTimelineWidget *TwitterApiMicroBlog::createSearchTimelineWidget(Choqok::Account *theAccount,
        QString name,
        const SearchInfo &info,
        QWidget *parent)
{
    return new TwitterApiSearchTimelineWidget(theAccount, name, info, parent);
}

void TwitterApiMicroBlog::createPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QByteArray data;
    QOAuth::ParamMap params;
    if (!post || post->content.isEmpty()) {
        qCDebug(CHOQOK) << "ERROR: Status text is empty!";
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::OtherError,
                         i18n("Creating the new post failed. Text is empty."), MicroBlog::Critical);
        return;
    }
    if (!post->isPrivate) {  ///Status Update
        QUrl url = account->apiUrl();
        url.setPath(url.path() + QString::fromLatin1("/statuses/update.%1").arg(format));
        params.insert("status", QUrl::toPercentEncoding(post->content));
        if (!post->replyToPostId.isEmpty()) {
            params.insert("in_reply_to_status_id", post->replyToPostId.toLocal8Bit());
        }
        data = "status=";
        data += QUrl::toPercentEncoding(post->content);
        if (!post->replyToPostId.isEmpty()) {
            data += "&in_reply_to_status_id=";
            data += post->replyToPostId.toLocal8Bit();
        }
        if (!account->usingOAuth()) {
            data += "&source=Choqok";
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        job->addMetaData(QStringLiteral("content-type"),
                         QStringLiteral("Content-Type: application/x-www-form-urlencoded"));
        job->addMetaData(QStringLiteral("customHTTPHeader"),
                         QStringLiteral("Authorization: ") +
                         QLatin1String(authorizationHeader(account, url, QOAuth::POST, params)));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
        job->start();
    } else {///Direct message
        QString recipientScreenName = post->replyToUserName;
        QUrl url = account->apiUrl();
        url.setPath(url.path() + QString::fromLatin1("/direct_messages/new.%1").arg(format));
        params.insert("user", recipientScreenName.toLocal8Bit());
        params.insert("text", QUrl::toPercentEncoding(post->content));
        data = "user=";
        data += recipientScreenName.toLocal8Bit();
        data += "&text=";
        data += QUrl::toPercentEncoding(post->content);
        if (!account->usingOAuth()) {
            data += "&source=Choqok";
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Creating the new post failed. Cannot create an http POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData(QStringLiteral("content-type"),
                         QStringLiteral("Content-Type: application/x-www-form-urlencoded"));
        job->addMetaData(QStringLiteral("customHTTPHeader"),
                         QStringLiteral("Authorization: ")
                         + QLatin1String(authorizationHeader(account, url, QOAuth::POST, params)));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
        job->start();
    }
}

void TwitterApiMicroBlog::repeatPost(Choqok::Account *theAccount, const QString &postId)
{
    qCDebug(CHOQOK);
    if (postId.isEmpty()) {
        qCCritical(CHOQOK) << "ERROR: PostId is empty!";
        return;
    }
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/statuses/retweet/%1.%2").arg(postId).arg(format));
    QByteArray data;
    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("content-type"),
                     QStringLiteral("Content-Type: application/x-www-form-urlencoded"));
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, url, QOAuth::POST)));
    Choqok::Post *post = new Choqok::Post;
    post->postId = postId;
    mCreatePostMap[ job ] = post;
    mJobsAccount[job] = theAccount;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotCreatePost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = mCreatePostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (!post || !theAccount) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error: " << job->errorString();
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::CommunicationError,
                         i18n("Creating the new post failed. %1", job->errorString()), MicroBlog::Critical);
    } else {
        KIO::StoredTransferJob *stj = qobject_cast< KIO::StoredTransferJob * > (job);
        if (!post->isPrivate) {
            readPost(theAccount, stj->data(), post);
            if (post->isError) {
                QString errorMsg;
                errorMsg = checkForError(stj->data());
                if (errorMsg.isEmpty()) {    // ???? If empty, why is there an error?
                    qCCritical(CHOQOK) << "Creating post: JSON parsing error: " << stj->data() ;
                    Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::ParsingError,
                                     i18n("Creating the new post failed. The result data could not be parsed."), MicroBlog::Critical);
                } else {
                    qCCritical(CHOQOK) << "Server Error:" << errorMsg ;
                    Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::ServerError,
                                     i18n("Creating the new post failed, with error: %1", errorMsg),
                                     MicroBlog::Critical);
                }
            } else {
                Choqok::NotifyManager::success(i18n("New post submitted successfully"));
                Q_EMIT postCreated(theAccount, post);
            }
        } else {
            Choqok::NotifyManager::success(i18n("Private message sent successfully"));
            Q_EMIT postCreated(theAccount, post);
        }
    }
}

void TwitterApiMicroBlog::abortAllJobs(Choqok::Account *theAccount)
{
    Q_FOREACH (KJob *job, mJobsAccount.keys(theAccount)) {
        job->kill(KJob::EmitResult);
    }
}

void TwitterApiMicroBlog::abortCreatePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    if (mCreatePostMap.isEmpty()) {
        return;
    }
    if (post) {
        mCreatePostMap.key(post)->kill(KJob::EmitResult);
    } else {
        Q_FOREACH (KJob *job, mCreatePostMap.keys()) {
            if (mJobsAccount[job] == theAccount) {
                job->kill(KJob::EmitResult);
            }
        }
    }
}

void TwitterApiMicroBlog::fetchPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (!post || post->postId.isEmpty()) {
        return;
    }
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/statuses/show/%1.%2").arg(post->postId).arg(format));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Fetching the new post failed. Cannot create an HTTP GET request."
//                                 "Please check your KDE installation." );
//         emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, Low );
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, url, QOAuth::GET)));
    mFetchPostMap[ job ] = post;
    mJobsAccount[ job ] = theAccount;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFetchPost(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotFetchPost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCWarning(CHOQOK) << "NULL Job returned";
        return;
    }
    Choqok::Post *post = mFetchPostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error: " << job->errorString();
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching the new post failed. %1", job->errorString()), Low);
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> (job);
        readPost(theAccount, stj->data(), post);
        if (post->isError) {
            QString errorMsg;
            errorMsg = checkForError(stj->data());
            if (errorMsg.isEmpty()) {
                qCDebug(CHOQOK) << "Parsing Error";
                Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::ParsingError,
                                 i18n("Fetching new post failed. The result data could not be parsed."),
                                 Low);
            } else {
                qCCritical(CHOQOK) << "Fetching post: Server Error: " << errorMsg;
                Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::ServerError,
                                 i18n("Fetching new post failed, with error: %1", errorMsg),
                                 Low);
            }
        } else {
            post->isError = true;
            Q_EMIT postFetched(theAccount, post);
        }
    }
}

void TwitterApiMicroBlog::removePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (!post->postId.isEmpty()) {
        TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
        QUrl url = account->apiUrl();
        if (!post->isPrivate) {
            url.setPath(url.path() + QString::fromLatin1("/statuses/destroy/%1.%2").arg(post->postId).arg(format));
        } else {
            url.setPath(url.path() + QString::fromLatin1("/direct_messages/destroy/%1.%2").arg(post->postId).arg(format));
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
//             QString errMsg = i18n ( "Removing the post failed. Cannot create an HTTP POST request. Please check your KDE installation." );
//             emit errorPost ( theAccount, post, Choqok::MicroBlog::OtherError, errMsg, MicroBlog::Critical );
            return;
        }
        job->addMetaData(QStringLiteral("customHTTPHeader"),
                         QStringLiteral("Authorization: ") +
                         QLatin1String(authorizationHeader(account, url, QOAuth::POST)));
        mRemovePostMap[job] = post;
        mJobsAccount[job] = theAccount;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRemovePost(KJob*)));
        job->start();
    }
}

void TwitterApiMicroBlog::slotRemovePost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer.";
        return;
    }
    Choqok::Post *post = mRemovePostMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error: " << job->errorString();
        Q_EMIT errorPost(theAccount, post, CommunicationError,
                         i18n("Removing the post failed. %1", job->errorString()), MicroBlog::Critical);
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
        QString errMsg = checkForError(stj->data());
        if (errMsg.isEmpty()) {
            Q_EMIT postRemoved(theAccount, post);
        } else {
            qCCritical(CHOQOK) << "Server error on removing post: " << errMsg;
            Q_EMIT errorPost(theAccount, post, ServerError,
                             i18n("Removing the post failed. %1", errMsg), MicroBlog::Critical);
        }
    }
}

void TwitterApiMicroBlog::createFavorite(Choqok::Account *theAccount, const QString &postId)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/favorites/create.%1").arg(format));
    QUrl tmp(url);

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("id"), postId);
    url.setQuery(urlQuery);

    QOAuth::ParamMap params;
    params.insert("id", postId.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "The Favorite creation failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmp, QOAuth::POST, params)));
    mFavoriteMap[job] = postId;
    mJobsAccount[job] = theAccount;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreateFavorite(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotCreateFavorite(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer.";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString postId = mFavoriteMap.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error: " << job->errorString();
        Q_EMIT error(theAccount, CommunicationError, i18n("Favorite creation failed. %1", job->errorString()));
    } else {
        KIO::StoredTransferJob *stJob = qobject_cast<KIO::StoredTransferJob *>(job);
        QString err = checkForError(stJob->data());
        if (!err.isEmpty()) {
            Q_EMIT error(theAccount, ServerError, err, Critical);
            return;
        } else {
            Q_EMIT favoriteCreated(theAccount, postId);
        }
    }
}

void TwitterApiMicroBlog::removeFavorite(Choqok::Account *theAccount, const QString &postId)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/favorites/destroy.%1").arg(format));

    QUrl tmp(url);
    QUrlQuery tmpUrlQuery;
    tmpUrlQuery.addQueryItem(QLatin1String("id"), postId);
    url.setQuery(tmpUrlQuery);

    QOAuth::ParamMap params;
    params.insert("id", postId.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http POST request!";
//         QString errMsg = i18n ( "Removing the favorite failed. Cannot create an http POST request. "
//                                 "Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg );
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmp, QOAuth::POST, params)));
    mFavoriteMap[job] = postId;
    mJobsAccount[job] = theAccount;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRemoveFavorite(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotRemoveFavorite(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer.";
        return;
    }
    QString id = mFavoriteMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error: " << job->errorString();
        Q_EMIT error(theAccount, CommunicationError, i18n("Removing the favorite failed. %1", job->errorString()));
    } else {
        KIO::StoredTransferJob *stJob = qobject_cast<KIO::StoredTransferJob *>(job);
        QString err = checkForError(stJob->data());
        if (!err.isEmpty()) {
            Q_EMIT error(theAccount, ServerError, err, Critical);
            return;
        } else {
            Q_EMIT favoriteRemoved(theAccount, id);
        }
    }
}

void TwitterApiMicroBlog::listFriendsUsername(TwitterApiAccount *theAccount, bool active)
{
    friendsList.clear();
    d->friendsCursor = QLatin1String("-1");
    if (theAccount) {
        requestFriendsScreenName(theAccount, active);
    }
}

void TwitterApiMicroBlog::requestFriendsScreenName(TwitterApiAccount *theAccount, bool active)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + (QString::fromLatin1("/friends/list.%1").arg(format)));
    QUrl tmpUrl(url);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("cursor"), d->friendsCursor);
    urlQuery.addQueryItem(QLatin1String("count"), QString::fromLatin1("200"));
    url.setQuery(urlQuery);
    QOAuth::ParamMap params;
    params.insert("cursor", d->friendsCursor.toLatin1());
    params.insert("count", QString::fromLatin1("200").toLatin1());

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmpUrl, QOAuth::GET, params)));
    mJobsAccount[job] = theAccount;
    if (active) {
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRequestFriendsScreenNameActive(KJob*)));
    } else {
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRequestFriendsScreenNamePassive(KJob*)));
    }
    job->start();
    Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Updating friends list for account %1...", theAccount->username()));
}

void TwitterApiMicroBlog::slotRequestFriendsScreenNameActive(KJob *job)
{
    finishRequestFriendsScreenName(job, true);
}

void TwitterApiMicroBlog::slotRequestFriendsScreenNamePassive(KJob *job)
{
    finishRequestFriendsScreenName(job, false);
}

void TwitterApiMicroBlog::finishRequestFriendsScreenName(KJob *job, bool active)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>(mJobsAccount.take(job));
    KIO::StoredTransferJob *stJob = qobject_cast<KIO::StoredTransferJob *>(job);
    Choqok::MicroBlog::ErrorLevel level = active ? Critical : Low;
    if (stJob->error()) {
        Q_EMIT error(theAccount, ServerError, i18n("Friends list for account %1 could not be updated:\n%2",
                     theAccount->username(), stJob->errorString()), level);
        return;
    }
    QStringList newList = readUsersScreenName(theAccount, stJob->data());
    newList.removeDuplicates();
    if (! checkForError(stJob->data()).isEmpty()) {           // if an error occurred, do not replace the friends list.
        theAccount->setFriendsList(friendsList);
        Q_EMIT friendsUsernameListed(theAccount, friendsList);
    } else if (QString::compare(d->friendsCursor, QLatin1String("0"))) {   // if the cursor is not "0", there is more friends data to be had
        friendsList << newList;
        requestFriendsScreenName(theAccount, active);
    } else {
        friendsList << newList;
        theAccount->setFriendsList(friendsList);
        Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Friends list for account %1 has been updated.",
                theAccount->username()));
        Q_EMIT friendsUsernameListed(theAccount, friendsList);
    }
}

void TwitterApiMicroBlog::updateTimelines(Choqok::Account *theAccount)
{
    qCDebug(CHOQOK);
    Q_FOREACH (const QString &tm, theAccount->timelineNames()) {
        requestTimeLine(theAccount, tm, mTimelineLatestId[theAccount][tm]);
    }
}

void TwitterApiMicroBlog::requestTimeLine(Choqok::Account *theAccount, QString type,
        QString latestStatusId, int page, QString maxId)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + timelineApiPath[type].arg(format));
    QUrl tmpUrl(url);
    int countOfPost = Choqok::BehaviorSettings::countOfPosts();

    QOAuth::ParamMap params;
    // needed because lists have different parameter names but
    // returned timelines have the same JSON format
    if (timelineApiPath[type].contains(QLatin1String("lists/statuses"))) {
        QUrlQuery urlQuery;

        // type contains @username/timelinename
        const QString slug = type.mid(type.indexOf(QLatin1String("/")) + 1);
        urlQuery.addQueryItem(QLatin1String("slug"), slug);
        params.insert("slug", slug.toLatin1());

        const QString owner = type.mid(1, type.indexOf(QLatin1String("/")) - 1);
        urlQuery.addQueryItem(QString::fromLatin1("owner_screen_name"), owner);
        params.insert("owner_screen_name", owner.toLatin1());

        url.setQuery(urlQuery);
    } else {
        QUrlQuery urlQuery;
        if (account->usingOAuth()) {    //TODO: Check if needed
            if (!latestStatusId.isEmpty()) {
                params.insert("since_id", latestStatusId.toLatin1());
                countOfPost = 200;
            }
            params.insert("count", QByteArray::number(countOfPost));
            if (!maxId.isEmpty()) {
                params.insert("max_id", maxId.toLatin1());
            }
            if (page) {
                params.insert("page", QByteArray::number(page));
            }
        }
        if (!latestStatusId.isEmpty()) {
            urlQuery.addQueryItem(QLatin1String("since_id"), latestStatusId);
            countOfPost = 200;
        }
        urlQuery.addQueryItem(QLatin1String("count"), QString::number(countOfPost));
        if (!maxId.isEmpty()) {
            urlQuery.addQueryItem(QLatin1String("max_id"), maxId);
        }
        if (page) {
            urlQuery.addQueryItem(QLatin1String("page"), QString::number(page));
        }
        url.setQuery(urlQuery);
    }
    qCDebug(CHOQOK) << "Latest " << type << " Id: " << latestStatusId;// << " apiReq: " << url;

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo) ;
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
//         QString errMsg = i18n ( "Cannot create an http GET request. Please check your KDE installation." );
//         emit error ( theAccount, OtherError, errMsg, Low );
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ")
                     + QLatin1String(authorizationHeader(account, tmpUrl, QOAuth::GET, params)));
    mRequestTimelineMap[job] = type;
    mJobsAccount[job] = theAccount;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRequestTimeline(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotRequestTimeline(KJob *job)
{
    qCDebug(CHOQOK);//TODO Add error detection for XML "checkXmlForError()" and JSON
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error: " << job->errorString();
        Q_EMIT error(theAccount, CommunicationError,
                     i18n("Timeline update failed: %1", job->errorString()), Low);
        return;
    }
    QString type = mRequestTimelineMap.take(job);
    if (isValidTimeline(type)) {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob *>(job);
        QList<Choqok::Post *> list;
        if (type == QLatin1String("Inbox") || type == QLatin1String("Outbox")) {
            list = readDirectMessages(theAccount, j->data());
        } else {
            list = readTimeline(theAccount, j->data());

        }
        if (!list.isEmpty()) {
            mTimelineLatestId[theAccount][type] = list.last()->postId;
            Q_EMIT timelineDataReceived(theAccount, type, list);
        }
    }
}

QByteArray TwitterApiMicroBlog::authorizationHeader(TwitterApiAccount *theAccount, const QUrl &requestUrl,
        QOAuth::HttpMethod method, QOAuth::ParamMap params)
{
    QByteArray auth;
    if (theAccount->usingOAuth()) {
        auth = theAccount->oauthInterface()->createParametersString(requestUrl.url(), method, theAccount->oauthToken(),
                theAccount->oauthTokenSecret(), QOAuth::HMAC_SHA1,
                params, QOAuth::ParseForHeaderArguments);
    } else {
        auth = theAccount->username().toUtf8() + ':' + theAccount->password().toUtf8();
        auth = auth.toBase64().prepend("Basic ");
    }
    return auth;
}

void TwitterApiMicroBlog::setRepeatedOfInfo(Choqok::Post *post, Choqok::Post *repeatedPost)
{
    if (Choqok::AppearanceSettings::showRetweetsInChoqokWay()) {
        post->content = repeatedPost->content;
        post->replyToPostId = repeatedPost->replyToPostId;
        post->replyToUserId = repeatedPost->replyToUserId;
        post->replyToUserName = repeatedPost->replyToUserName;
        post->repeatedFromUsername = repeatedPost->author.userName;
        post->repeatedPostId = repeatedPost->postId;
    } else {
        post->content = repeatedPost->content;
        post->replyToPostId = repeatedPost->replyToPostId;
        post->replyToUserId = repeatedPost->replyToUserId;
        post->replyToUserName = repeatedPost->replyToUserName;
        post->repeatedFromUsername = post->author.userName;
        post->author = repeatedPost->author;
        post->repeatedPostId = repeatedPost->postId;
    }
    //post->creationDateTime = repeatedPost->creationDateTime;
}

QDateTime TwitterApiMicroBlog::dateFromString(const QString &date)
{
    char s[10];
    int year, day, hours, minutes, seconds, tz;
    sscanf(qPrintable(date), "%*s %s %d %d:%d:%d %d %d", s, &day, &hours, &minutes, &seconds, &tz, &year);
    int month = d->monthes[QLatin1String(s)];
    QDateTime recognized(QDate(year, month, day), QTime(hours, minutes, seconds));
    if (tz == 0) { //tz is the timezone, in Twitter it's always UTC(0) in Identica it's local +/-NUMBER
        recognized.setTimeSpec(Qt::UTC);
    }
    return recognized.toLocalTime();
}

void TwitterApiMicroBlog::aboutToUnload()
{
    d->countOfTimelinesToSave = 0;
    Q_FOREACH (Choqok::Account *acc, Choqok::AccountManager::self()->accounts()) {
        if (acc->microblog() == this) {
            d->countOfTimelinesToSave += acc->timelineNames().count();
        }
    }
    Q_EMIT saveTimelines();
}

void TwitterApiMicroBlog::showDirectMessageDialog(TwitterApiAccount *theAccount/* = 0*/,
        const QString &toUsername/* = QString()*/)
{
    qCDebug(CHOQOK);
    if (!theAccount) {
        QAction *act = qobject_cast<QAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount *>(
                         Choqok::AccountManager::self()->findAccount(act->data().toString()));
    }
    TwitterApiDMessageDialog *dmsg = new TwitterApiDMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    if (!toUsername.isEmpty()) {
        dmsg->setTo(toUsername);
    }
    dmsg->show();
}

Choqok::TimelineInfo *TwitterApiMicroBlog::timelineInfo(const QString &timelineName)
{
    if (isValidTimeline(timelineName)) {
        return mTimelineInfos.value(timelineName);
    } else {
        return 0;
    }
}

void TwitterApiMicroBlog::showSearchDialog(TwitterApiAccount *theAccount)
{
    if (!theAccount) {
        QAction *act = qobject_cast<QAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount *>(
                         Choqok::AccountManager::self()->findAccount(act->data().toString()));
    }
    QPointer<TwitterApiSearchDialog> searchDlg = new TwitterApiSearchDialog(theAccount,
            Choqok::UI::Global::mainWindow());
    searchDlg->show();
}

void TwitterApiMicroBlog::slotUpdateFriendsList()
{
    QAction *act = qobject_cast<QAction *>(sender());
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>(
                                        Choqok::AccountManager::self()->findAccount(act->data().toString()));
    listFriendsUsername(theAccount, true);
}

void TwitterApiMicroBlog::createFriendship(Choqok::Account *theAccount, const QString &username)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/friendships/create.%1").arg(format));
    QUrl tmp(url);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("screen_name"), username);
    url.setQuery(urlQuery);

    QOAuth::ParamMap params;
    params.insert("screen_name", username.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    qCDebug(CHOQOK) << url;
    if (!job) {
        qCCritical(CHOQOK) << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmp, QOAuth::POST, params)));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreateFriendship(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotCreateFriendship(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCCritical(CHOQOK) << "Job is a null Pointer!";
        return;
    }
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>(mJobsAccount.take(job));
    QString username = mFriendshipMap.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, CommunicationError,
                     i18n("Creating friendship with %1 failed. %2", username, job->errorString()));
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    Choqok::User *user = readUserInfo(stj->data());
    if (user /*&& user->userName.compare(username, Qt::CaseInsensitive)*/) {
        Q_EMIT friendshipCreated(theAccount, username);
        Choqok::NotifyManager::success(i18n("You are now listening to %1's posts.", username));
        theAccount->setFriendsList(QStringList());
        listFriendsUsername(theAccount);
    } else {
        QString errorMsg = checkForError(stj->data());
        if (errorMsg.isEmpty()) {
            qCDebug(CHOQOK) << "Parse Error: " << stj->data();
            Q_EMIT error(theAccount, ParsingError,
                         i18n("Creating friendship with %1 failed: the server returned invalid data.",
                              username));
        } else {
            qCDebug(CHOQOK) << "Server error: " << errorMsg;
            Q_EMIT error(theAccount, ServerError,
                         i18n("Creating friendship with %1 failed: %2",
                              username, errorMsg));
        }
    }
}

void TwitterApiMicroBlog::destroyFriendship(Choqok::Account *theAccount, const QString &username)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/friendships/destroy.%1").arg(format));
    QUrl tmp(url);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("screen_name"), username);
    url.setQuery(urlQuery);

    QOAuth::ParamMap params;
    params.insert("screen_name", username.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if (!job) {
        qCCritical(CHOQOK) << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmp, QOAuth::POST, params)));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotDestroyFriendship(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::slotDestroyFriendship(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCCritical(CHOQOK) << "Job is a null Pointer!";
        return;
    }
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>(mJobsAccount.take(job));
    QString username = mFriendshipMap.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, CommunicationError,
                     i18n("Destroying friendship with %1 failed. %2", username, job->errorString()));
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    Choqok::User *user = readUserInfo(stj->data());
    if (user /*&& user->userName.compare( username, Qt::CaseInsensitive )*/) {
        Q_EMIT friendshipDestroyed(theAccount, username);
        Choqok::NotifyManager::success(i18n("You will not receive %1's updates.", username));
        theAccount->setFriendsList(QStringList());
        listFriendsUsername(theAccount);
    } else {
        QString errorMsg = checkForError(stj->data());
        if (errorMsg.isEmpty()) {
            qCDebug(CHOQOK) << "Parse Error: " << stj->data();
            Q_EMIT error(theAccount, ParsingError,
                         i18n("Destroying friendship with %1 failed: the server returned invalid data.",
                              username));
        } else {
            qCDebug(CHOQOK) << "Server error: " << errorMsg;
            Q_EMIT error(theAccount, ServerError,
                         i18n("Destroying friendship with %1 failed: %2",
                              username, errorMsg));
        }
    }
}

void TwitterApiMicroBlog::blockUser(Choqok::Account *theAccount, const QString &username)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url.setPath(url.path() + QString::fromLatin1("/blocks/create.%1").arg(format));
    QUrl tmp(url);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("screen_name"), username);
    url.setQuery(urlQuery);

    QOAuth::ParamMap params;
    params.insert("screen_name", username.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if (!job) {
        qCCritical(CHOQOK) << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmp, QOAuth::POST, params)));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotBlockUser(KJob*)));
    job->start();
}

void TwitterApiMicroBlog::reportUserAsSpam(Choqok::Account *theAccount, const QString &username)
{
    qCDebug(CHOQOK);
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(theAccount);
    QUrl url = account->apiUrl();
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QString::fromLatin1("/users/report_spam.%1").arg(format));
    QUrl tmp(url);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("screen_name"), username);
    url.setQuery(urlQuery);

    QOAuth::ParamMap params;
    params.insert("screen_name", username.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo) ;
    if (!job) {
        qCCritical(CHOQOK) << "Cannot create an http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(account, tmp, QOAuth::POST, params)));
    mJobsAccount[job] = theAccount;
    mFriendshipMap[ job ] = username;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotReportUser(KJob*)));
    job->start();

}

void TwitterApiMicroBlog::slotBlockUser(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCCritical(CHOQOK) << "Job is a null Pointer!";
        return;
    }
    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString username = mFriendshipMap.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, CommunicationError,
                     i18n("Blocking %1 failed. %2", username, job->errorString()));
        return;
    }
    Choqok::User *user = readUserInfo(qobject_cast<KIO::StoredTransferJob *>(job)->data());
    if (user /*&& user->userName.compare( username, Qt::CaseInsensitive )*/) {
        Q_EMIT userBlocked(theAccount, username);
        Choqok::NotifyManager::success(i18n("You will no longer be disturbed by %1.", username));
    } else {
        qCDebug(CHOQOK) << "Parse Error: " << qobject_cast<KIO::StoredTransferJob *>(job)->data();
        Q_EMIT error(theAccount, ParsingError,
                     i18n("Blocking %1 failed: the server returned invalid data.",
                          username));
    }
    //TODO Check for failor!
}

void TwitterApiMicroBlog::slotReportUser(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCCritical(CHOQOK) << "Job is a null Pointer!";
        return;
    }

    Choqok::Account *theAccount = mJobsAccount.take(job);
    QString username = mFriendshipMap.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, CommunicationError,
                     i18n("Reporting %1 failed. %2", username, job->errorString()));
        return;
    }
    Choqok::User *user = readUserInfo(qobject_cast<KIO::StoredTransferJob *>(job)->data());
    if (user) {
        Choqok::NotifyManager::success(i18n("Report sent successfully"));
    } else {
        qCDebug(CHOQOK) << "Parse Error: " << qobject_cast<KIO::StoredTransferJob *>(job)->data();
        Q_EMIT error(theAccount, ParsingError,
                     i18n("Reporting %1 failed: the server returned invalid data.",
                          username));
    }
}

///===================================================================

QString TwitterApiMicroBlog::checkForError(const QByteArray &buffer)
{
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        const QVariantMap map = json.toVariant().toMap();
        QStringList errors;
        if (map.contains(QLatin1String("errors"))) {
            for (int i = 0; i < map[QLatin1String("errors")].toList().count(); i++) {
                errors.append(map[QLatin1String("errors")].toList()[i].toMap()[QLatin1String("message")].toString());
                qCCritical(CHOQOK) << "Error: " << errors.last();
            }

            return errors.join(QLatin1String(";"));
        }
    }
    return QString();
}

QList< Choqok::Post * > TwitterApiMicroBlog::readTimeline(Choqok::Account *theAccount,
        const QByteArray &buffer)
{
    QList<Choqok::Post *> postList;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        QVariantList list = json.toVariant().toList();
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for (; it != endIt; ++it) {
            postList.prepend(readPost(theAccount, it->toMap(), new Choqok::Post));
        }
    } else {
        QString err = checkForError(buffer);
        if (err.isEmpty()) {
            qCCritical(CHOQOK) << "JSON parsing failed.\nBuffer was: \n" << buffer;
            Q_EMIT error(theAccount, ParsingError, i18n("Could not parse the data that has been received from the server."));
        } else {
            Q_EMIT error(theAccount, ServerError, err);
        }
        return postList;
    }
    return postList;
}

Choqok::Post *TwitterApiMicroBlog::readPost(Choqok::Account *theAccount,
        const QByteArray &buffer, Choqok::Post *post)
{
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        return readPost(theAccount, json.toVariant().toMap(), post);
    } else {
        if (!post) {
            qCCritical(CHOQOK) << "TwitterApiMicroBlog::readPost: post is NULL!";
            post = new Choqok::Post;
        }
        Q_EMIT errorPost(theAccount, post, ParsingError, i18n("Could not parse the data that has been received from the server."));
        qCCritical(CHOQOK) << "JSon parsing failed. Buffer was:" << buffer;
        post->isError = true;
        return post;
    }
}

Choqok::Post *TwitterApiMicroBlog::readPost(Choqok::Account *theAccount,
        const QVariantMap &var, Choqok::Post *post)
{
    if (!post) {
        qCCritical(CHOQOK) << "TwitterApiMicroBlog::readPost: post is NULL!";
        return 0;
    }
    post->content = var[QLatin1String("text")].toString();
    post->creationDateTime = dateFromString(var[QLatin1String("created_at")].toString());
    post->isFavorited = var[QLatin1String("favorited")].toBool();
    post->postId = var[QLatin1String("id")].toString();
    post->replyToPostId = var[QLatin1String("in_reply_to_status_id")].toString();
    post->replyToUserId = var[QLatin1String("in_reply_to_user_id")].toString();
    post->replyToUserName = var[QLatin1String("in_reply_to_screen_name")].toString();
    post->source = var[QLatin1String("source")].toString();
    QVariantMap userMap = var[QLatin1String("user")].toMap();
    post->author.description = userMap[QLatin1String("description")].toString();
    post->author.location = userMap[QLatin1String("location")].toString();
    post->author.realName = userMap[QLatin1String("name")].toString();
    post->author.userId = userMap[QLatin1String("id")].toString();
    post->author.userName = userMap[QLatin1String("screen_name")].toString();
    post->author.profileImageUrl = userMap[QLatin1String("profile_image_url")].toString();
    QVariantMap entities = var[QLatin1String("entities")].toMap();
    QVariantMap mediaMap;
    QVariantList media = entities[QLatin1String("media")].toList();
    if (media.size() > 0) {
        mediaMap = media.at(0).toMap();
        post->media = mediaMap[QLatin1String("media_url")].toString() + QLatin1String(":small");
        QVariantMap sizes = mediaMap[QLatin1String("sizes")].toMap();
        QVariantMap w = sizes[QLatin1String("small")].toMap();
        post->mediaSizeWidth = w[QLatin1String("w")].toInt() != 0 ? w[QLatin1String("w")].toInt() : 0;
        post->mediaSizeHeight = w[QLatin1String("h")].toInt() != 0 ? w[QLatin1String("h")].toInt() : 0;
    } else {
        post->media = QString();
        post->mediaSizeHeight = 0;
        post->mediaSizeWidth = 0;
    }

    Choqok::Post *repeatedPost = 0;
    QVariantMap retweetedMap = var[QLatin1String("retweeted_status")].toMap();
    if (!retweetedMap.isEmpty()) {
        repeatedPost = readPost(theAccount, retweetedMap, new Choqok::Post);
        setRepeatedOfInfo(post, repeatedPost);
        delete repeatedPost;
    }
    post->link = postUrl(theAccount, post->author.userName, post->postId);
    post->isRead = post->isFavorited || (post->repeatedFromUsername.compare(theAccount->username(), Qt::CaseInsensitive) == 0);
    return post;
}

QList< Choqok::Post * > TwitterApiMicroBlog::readDirectMessages(Choqok::Account *theAccount,
        const QByteArray &buffer)
{
    QList<Choqok::Post *> postList;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        QVariantList list = json.toVariant().toList();
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for (; it != endIt; ++it) {
            postList.prepend(readDirectMessage(theAccount, it->toMap()));
        }
    } else {
        QString err = checkForError(buffer);
        if (err.isEmpty()) {
            qCCritical(CHOQOK) << "JSON parsing failed.\nBuffer was: \n" << buffer;
            Q_EMIT error(theAccount, ParsingError, i18n("Could not parse the data that has been received from the server."));
        } else {
            Q_EMIT error(theAccount, ServerError, err);
        }
        return postList;
    }
    return postList;
}

Choqok::Post *TwitterApiMicroBlog::readDirectMessage(Choqok::Account *theAccount,
        const QByteArray &buffer)
{
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        return readDirectMessage(theAccount, json.toVariant().toMap());
    } else {
        Choqok::Post *post = new Choqok::Post;
        post->isError = true;
        return post;
    }
}

Choqok::Post *TwitterApiMicroBlog::readDirectMessage(Choqok::Account *theAccount,
        const QVariantMap &var)
{
    Choqok::Post *msg = new Choqok::Post;

    msg->isPrivate = true;
    QString senderId, recipientId, timeStr, senderScreenName, recipientScreenName, senderProfileImageUrl,
            senderName, senderDescription, recipientProfileImageUrl, recipientName, recipientDescription;

    msg->creationDateTime = dateFromString(var[QLatin1String("created_at")].toString());
    msg->content = var[QLatin1String("text")].toString();
    msg->postId = var[QLatin1String("id")].toString();;
    senderId = var[QLatin1String("sender_id")].toString();
    recipientId = var[QLatin1String("recipient_id")].toString();
    senderScreenName = var[QLatin1String("sender_screen_name")].toString();
    recipientScreenName = var[QLatin1String("recipient_screen_name")].toString();
    QVariantMap sender = var[QLatin1String("sender")].toMap();
    senderProfileImageUrl = sender[QLatin1String("profile_image_url")].toString();
    senderName = sender[QLatin1String("name")].toString();
    senderDescription = sender[QLatin1String("description")].toString();
    QVariantMap recipient = var[QLatin1String("recipient")].toMap();
    recipientProfileImageUrl = recipient[QLatin1String("profile_image_url")].toString();
    recipientName = recipient[QLatin1String("name")].toString();
    recipientDescription = recipient[QLatin1String("description")].toString();
    if (senderScreenName.compare(theAccount->username(), Qt::CaseInsensitive) == 0) {
        msg->author.description = recipientDescription;
        msg->author.userName = recipientScreenName;
        msg->author.profileImageUrl = recipientProfileImageUrl;
        msg->author.realName = recipientName;
        msg->author.userId = recipientId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
        msg->isRead = true;
    } else {
        msg->author.description = senderDescription;
        msg->author.userName = senderScreenName;
        msg->author.profileImageUrl = senderProfileImageUrl;
        msg->author.realName = senderName;
        msg->author.userId = senderId;
        msg->replyToUserId = recipientId;
        msg->replyToUserName = recipientScreenName;
    }
    return msg;
}

Choqok::User *TwitterApiMicroBlog::readUserInfo(const QByteArray &buffer)
{
    Choqok::User *user;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        Choqok::User u(readUser(0, json.toVariant().toMap()));
        user = &u;
    } else {
        QString err = i18n("Retrieving the friends list failed. The data returned from the server is corrupted.");
        qCDebug(CHOQOK) << "JSON parse error: the buffer is: \n" << buffer;
        Q_EMIT error(0, ParsingError, err, Critical);
    }
    return user;
}

QStringList TwitterApiMicroBlog::readUsersScreenName(Choqok::Account *theAccount,
        const QByteArray &buffer)
{
    QStringList list;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        const QVariantMap map = json.toVariant().toMap();
        QVariantList jsonList = map[QLatin1String("users")].toList();
        QString nextCursor = map[QLatin1String("next_cursor_str")].toString();

        if (nextCursor.isEmpty()) {
            nextCursor = QLatin1String("0"); // we probably ran the rate limit; stop bugging the server already
        }

        QVariantList::const_iterator it = jsonList.constBegin();
        QVariantList::const_iterator endIt = jsonList.constEnd();
        for (; it != endIt; ++it) {
            list << it->toMap()[QLatin1String("screen_name")].toString();
        }
        d->friendsCursor = nextCursor;
    } else {
        QString err = i18n("Retrieving the friends list failed. The data returned from the server is corrupted.");
        qCDebug(CHOQOK) << "JSON parse error: the buffer is: \n" << buffer;
        Q_EMIT error(theAccount, ParsingError, err, Critical);
    }
    return list;
}

Choqok::User TwitterApiMicroBlog::readUser(Choqok::Account *theAccount, const QVariantMap &map)
{
    Q_UNUSED(theAccount);
    Choqok::User u;
    u.description = map[QLatin1String("description")].toString();
    u.followersCount = map[QLatin1String("followers_count")].toUInt();
    u.homePageUrl = map[QLatin1String("url")].toString();
    u.isProtected = map[QLatin1String("protected")].toBool();
    u.location = map[QLatin1String("location")].toString();
    u.profileImageUrl = map[QLatin1String("profile_image_url")].toString();
    u.realName = map[QLatin1String("name")].toString();
    u.userId = map[QLatin1String("id_str")].toString();
    u.userName = map[QLatin1String("screen_name")].toString();
    return u;
}

