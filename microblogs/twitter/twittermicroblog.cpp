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

#include "twittermicroblog.h"

#include <QAction>
#include <QJsonDocument>
#include <QMenu>
#include <QMimeDatabase>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

#include "account.h"
#include "accountmanager.h"
#include "choqokappearancesettings.h"
#include "choqoktypes.h"
#include "composerwidget.h"
#include "editaccountwidget.h"
#include "mediamanager.h"
#include "postwidget.h"
#include "timelinewidget.h"

#include "twitterapimicroblogwidget.h"

#include "twitteraccount.h"
#include "twittercomposerwidget.h"
#include "twitterdebug.h"
#include "twittereditaccount.h"
#include "twitterlistdialog.h"
#include "twitterpostwidget.h"
#include "twittersearch.h"
#include "twittertimelinewidget.h"

K_PLUGIN_FACTORY_WITH_JSON(TwitterMicroBlogFactory, "choqok_twitter.json",
                           registerPlugin < TwitterMicroBlog > ();)

TwitterMicroBlog::TwitterMicroBlog(QObject *parent, const QVariantList &)
    : TwitterApiMicroBlog(QLatin1String("choqok_twitter"), parent)
{
    qCDebug(CHOQOK);
    setServiceName(QLatin1String("Twitter"));
    setServiceHomepageUrl(QLatin1String("https://twitter.com/"));
    timelineApiPath[QLatin1String("Reply")] = QLatin1String("/statuses/mentions_timeline.%1");
    setTimelineInfos();
}
void TwitterMicroBlog::setTimelineInfos()
{
//   hange description of replies to mentions
    Choqok::TimelineInfo *t = mTimelineInfos[QLatin1String("Reply")];
    t->name = i18nc("Timeline Name", "Mentions");
    t->description = i18nc("Timeline description", "Mentions of you");
}

TwitterMicroBlog::~TwitterMicroBlog()
{
    qCDebug(CHOQOK);
}

Choqok::Account *TwitterMicroBlog::createNewAccount(const QString &alias)
{
    TwitterAccount *acc = qobject_cast<TwitterAccount *>(Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return new TwitterAccount(this, alias);
    } else {
        return 0;
    }
}

ChoqokEditAccountWidget *TwitterMicroBlog::createEditAccountWidget(Choqok::Account *account, QWidget *parent)
{
    qCDebug(CHOQOK);
    TwitterAccount *acc = qobject_cast<TwitterAccount *>(account);
    if (acc || !account) {
        return new TwitterEditAccountWidget(this, acc, parent);
    } else {
        qCDebug(CHOQOK) << "Account passed here is not a TwitterAccount!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget *TwitterMicroBlog::createMicroBlogWidget(Choqok::Account *account, QWidget *parent)
{
    return new TwitterApiMicroBlogWidget(account, parent);
}

Choqok::UI::TimelineWidget *TwitterMicroBlog::createTimelineWidget(Choqok::Account *account,
        const QString &timelineName, QWidget *parent)
{
    return new TwitterTimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget *TwitterMicroBlog::createPostWidget(Choqok::Account *account,
        Choqok::Post *post, QWidget *parent)
{
    return new TwitterPostWidget(account, post, parent);
}

Choqok::UI::ComposerWidget *TwitterMicroBlog::createComposerWidget(Choqok::Account *account, QWidget *parent)
{
    return new TwitterComposerWidget(account, parent);
}

QString TwitterMicroBlog::profileUrl(Choqok::Account *, const QString &username) const
{
    return QStringLiteral("https://twitter.com/#!/%1").arg(username);
}

QString TwitterMicroBlog::postUrl(Choqok::Account *, const QString &username,
                                  const QString &postId) const
{
    return QStringLiteral("https://twitter.com/%1/status/%2").arg(username).arg(postId);
}

TwitterApiSearch *TwitterMicroBlog::searchBackend()
{
    if (!mSearchBackend) {
        mSearchBackend = new TwitterSearch(this);
    }
    return mSearchBackend;
}

void TwitterMicroBlog::createPostWithAttachment(Choqok::Account *theAccount, Choqok::Post *post,
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
        TwitterAccount *account = qobject_cast<TwitterAccount *>(theAccount);
        QUrl url = account->uploadUrl();
        url.setPath(url.path() + QStringLiteral("/statuses/update_with_media.%1").arg(format));
        const QMimeDatabase db;
        QByteArray fileContentType = db.mimeTypeForUrl(picUrl).name().toUtf8();

        QMap<QString, QByteArray> formdata;
        formdata[QLatin1String("status")] = post->content.toUtf8();
        if (!post->replyToPostId.isEmpty()) {
            formdata[QLatin1String("in_reply_to_status_id")] = post->replyToPostId.toLatin1();
        }
        formdata[QLatin1String("source")] = QCoreApplication::applicationName().toLatin1();

        QMap<QString, QByteArray> mediafile;
        mediafile[QLatin1String("name")] = "media[]";
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
                         QLatin1String(authorizationHeader(account, url, QOAuth::POST)));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect(job, SIGNAL(result(KJob*)),
                SLOT(slotCreatePost(KJob*)));
        job->start();
    }
}

QString TwitterMicroBlog::generateRepeatedByUserTooltip(const QString &username)
{
    if (Choqok::AppearanceSettings::showRetweetsInChoqokWay()) {
        return i18n("Retweet of %1", username);
    } else {
        return i18n("Retweeted by %1", username);
    }
}

QString TwitterMicroBlog::repeatQuestion()
{
    return i18n("Retweet to your followers?");
}

QMenu *TwitterMicroBlog::createActionsMenu(Choqok::Account *theAccount, QWidget *parent)
{
    QMenu *menu = TwitterApiMicroBlog::createActionsMenu(theAccount, parent);

    QAction *lists = new QAction(i18n("Add User List..."), menu);
    lists->setData(theAccount->alias());
    connect(lists, SIGNAL(triggered(bool)), SLOT(showListDialog()));
    menu->addAction(lists);

    return menu;
}

void TwitterMicroBlog::showListDialog(TwitterApiAccount *theAccount)
{
    if (!theAccount) {
        QAction *act = qobject_cast<QAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount *>(
                         Choqok::AccountManager::self()->findAccount(act->data().toString()));
    }
    QPointer<TwitterListDialog> listDlg = new TwitterListDialog(theAccount,
            Choqok::UI::Global::mainWindow());
    listDlg->show();
}

void TwitterMicroBlog::fetchUserLists(TwitterAccount *theAccount, const QString &username)
{
    qCDebug(CHOQOK);
    if (!theAccount) {
        return;
    }
    QUrl url = theAccount->apiUrl();
    url.setPath(url.path() + QStringLiteral("/lists/ownerships.%1").arg(format));
    QUrl url_for_oauth(url);//we need base URL (without params) to make OAuth signature with it!
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("screen_name"), username);
    url.setQuery(urlQuery);
    QOAuth::ParamMap params;
    params.insert("screen_name", username.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo) ;
    if (!job) {
        qCCritical(CHOQOK) << "TwitterMicroBlog::loadUserLists: Cannot create an http GET request!";
        return;
    }

    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(authorizationHeader(theAccount, url_for_oauth, QOAuth::GET, params)));
    mFetchUsersListMap[ job ] = username;
    mJobsAccount[ job ] = theAccount;
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFetchUserLists(KJob*)));
    job->start();
}

void TwitterMicroBlog::slotFetchUserLists(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCWarning(CHOQOK) << "NULL Job returned";
        return;
    }
    QString username = mFetchUsersListMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching %1's lists failed. %2", username, job->errorString()), Critical);
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> (job);
        QByteArray buffer = stj->data();
        QList<Twitter::List> list = readUserListsFromJson(theAccount, buffer);
        if (list.isEmpty()) {
            qCDebug(CHOQOK) << buffer;
            QString errorMsg;
            errorMsg = checkForError(buffer);
            if (errorMsg.isEmpty()) {
                KMessageBox::information(choqokMainWindow, i18n("There is no list record for user %1", username));
            } else {
                Q_EMIT error(theAccount, ServerError, errorMsg, Critical);
            }
        } else {
            Q_EMIT userLists(theAccount, username, list);
        }
    }
}

void TwitterMicroBlog::addListTimeline(TwitterAccount *theAccount, const QString &username,
                                       const QString &listname)
{
    qCDebug(CHOQOK);
    QStringList tms = theAccount->timelineNames();
    QString name = QStringLiteral("@%1/%2").arg(username).arg(listname);
    tms.append(name);
    addTimelineName(name);
    theAccount->setTimelineNames(tms);
    theAccount->writeConfig();
    QString url = QLatin1String("/lists/statuses");
    timelineApiPath[name] = url + QLatin1String(".%1");
    updateTimelines(theAccount);
}

// TODO: Change to new API
void TwitterMicroBlog::setListTimelines(TwitterAccount *theAccount, const QStringList &lists)
{
    qCDebug(CHOQOK) << lists;
    QStringList tms = theAccount->timelineNames();
    Q_FOREACH (const QString &name, lists) {
        tms.append(name);
        addTimelineName(name);
        QString url = QLatin1String("/lists/statuses");
        timelineApiPath[name] = url + QLatin1String(".%1");
    }
    tms.removeDuplicates();
    theAccount->setTimelineNames(tms);
}

Choqok::TimelineInfo *TwitterMicroBlog::timelineInfo(const QString &timelineName)
{
    if (timelineName.startsWith(QLatin1Char('@'))) {
        if (mListsInfo.contains(timelineName)) {
            return mListsInfo.value(timelineName);
        } else {
            Choqok::TimelineInfo *info = new Choqok::TimelineInfo;
            info->description = info->name = timelineName;
            info->icon = QLatin1String("format-list-unordered");
            mListsInfo.insert(timelineName, info);
            return info;
        }
    } else {
        return TwitterApiMicroBlog::timelineInfo(timelineName);
    }
}

QList< Twitter::List > TwitterMicroBlog::readUserListsFromJson(Choqok::Account *theAccount, QByteArray buffer)
{
    QList<Twitter::List> twitterList;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        const QVariantMap map = json.toVariant().toMap();
        if (map.contains(QLatin1String("lists"))) {
            Q_FOREACH (const QVariant &list, map[QLatin1String("lists")].toList()) {
                twitterList.append(readListFromJsonMap(theAccount, list.toMap()));
            }
        }
    }
    return twitterList;
}

Twitter::List TwitterMicroBlog::readListFromJsonMap(Choqok::Account *theAccount, QVariantMap map)
{
    Twitter::List l;
    l.author = readUser(theAccount, map[QLatin1String("user")].toMap());
    l.description = map[QLatin1String("description")].toString();
    l.fullname = map[QLatin1String("full_name")].toString();
    l.isFollowing = map[QLatin1String("following")].toBool();
    l.listId = map[QLatin1String("id")].toString();
    l.memberCount = map[QLatin1String("member_count")].toInt();
    l.mode = (map[QLatin1String("mode")].toString() == QLatin1String("public") ? Twitter::Public : Twitter::Private);
    l.name = map[QLatin1String("name")].toString();
    l.slug = map[QLatin1String("slug")].toString();
    l.subscriberCount = map[QLatin1String("subscriber_count")].toInt();
    l.uri = map[QLatin1String("uri")].toString();
    return l;
}

Choqok::Post *TwitterMicroBlog::readPost(Choqok::Account *account, const QVariantMap &var, Choqok::Post *post)
{
    if (!post) {
        qCCritical(CHOQOK) << "TwitterMicroBlog::readPost: post is NULL!";
        return 0;
    }

    post = TwitterApiMicroBlog::readPost(account, var, post);

    post->postId = var[QLatin1String("id_str")].toString();
    post->replyToPostId = var[QLatin1String("in_reply_to_status_id_str")].toString();
    post->replyToUserId = var[QLatin1String("in_reply_to_user_id_str")].toString();

    //postId is changed, regenerate link url
    post->link = postUrl(account, post->author.userName, post->postId);

    QVariantMap userMap = var[QLatin1String("user")].toMap();
    post->author.userId = userMap[QLatin1String("id_str")].toString();

    return post;
}

#include "twittermicroblog.moc"
