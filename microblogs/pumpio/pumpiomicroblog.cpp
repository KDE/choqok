/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013-2014 Andrea Scarpino <scarpino@kde.org>
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

#include "pumpiomicroblog.h"

#include <QAction>
#include <QFile>
#include <QJsonDocument>
#include <QMenu>
#include <QMimeDatabase>
#include <QTextDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

#include "accountmanager.h"
#include "application.h"
#include "choqokbehaviorsettings.h"
#include "notifymanager.h"

#include "pumpioaccount.h"
#include "pumpiocomposerwidget.h"
#include "pumpiodebug.h"
#include "pumpioeditaccountwidget.h"
#include "pumpiomessagedialog.h"
#include "pumpiomicroblogwidget.h"
#include "pumpiopost.h"
#include "pumpiopostwidget.h"

class PumpIOMicroBlog::Private
{
public:
    Private(): countOfTimelinesToSave(0)
    {}
    int countOfTimelinesToSave;
};

K_PLUGIN_FACTORY_WITH_JSON(PumpIOMicroBlogFactory, "choqok_pumpio.json",
                           registerPlugin < PumpIOMicroBlog > ();)

const QString PumpIOMicroBlog::inboxActivity(QLatin1String("/api/user/%1/inbox"));
const QString PumpIOMicroBlog::outboxActivity(QLatin1String("/api/user/%1/feed"));

const QString PumpIOMicroBlog::PublicCollection(QLatin1String("http://activityschema.org/collection/public"));

PumpIOMicroBlog::PumpIOMicroBlog(QObject *parent, const QVariantList &args):
    MicroBlog(QStringLiteral("Pump.IO") , parent), d(new Private)
{
    Q_UNUSED(args)
    setServiceName(QLatin1String("Pump.io"));
    setServiceHomepageUrl(QLatin1String("http://pump.io"));
    QStringList timelineNames;
    timelineNames << QLatin1String("Activity") << QLatin1String("Favorites") << QLatin1String("Inbox") << QLatin1String("Outbox");
    setTimelineNames(timelineNames);
    setTimelinesInfo();
}

PumpIOMicroBlog::~PumpIOMicroBlog()
{
    qDeleteAll(m_timelinesInfos);
    delete d;
}

void PumpIOMicroBlog::abortAllJobs(Choqok::Account *theAccount)
{
    Q_FOREACH (KJob *job, m_accountJobs.keys(theAccount)) {
        job->kill(KJob::EmitResult);
    }
}

void PumpIOMicroBlog::abortCreatePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    if (m_createPostJobs.isEmpty()) {
        return;
    }
    if (post) {
        m_createPostJobs.key(post)->kill(KJob::EmitResult);
        return;
    }

    Q_FOREACH (KJob *job, m_createPostJobs.keys()) {
        if (m_accountJobs[job] == theAccount) {
            job->kill(KJob::EmitResult);
        }
    }
}

void PumpIOMicroBlog::aboutToUnload()
{
    Q_FOREACH (Choqok::Account *acc, Choqok::AccountManager::self()->accounts()) {
        if (acc->microblog() == this) {
            d->countOfTimelinesToSave += acc->timelineNames().count();
        }
    }
    Q_EMIT saveTimelines();
}

QMenu *PumpIOMicroBlog::createActionsMenu(Choqok::Account *theAccount, QWidget *parent)
{
    QMenu *menu = MicroBlog::createActionsMenu(theAccount, parent);

    QAction *directMessge = new QAction(QIcon::fromTheme(QLatin1String("mail-message-new")), i18n("Send Private Message..."), menu);
    directMessge->setData(theAccount->alias());
    connect(directMessge, SIGNAL(triggered(bool)), this, SLOT(showDirectMessageDialog()));
    menu->addAction(directMessge);

    return menu;
}

Choqok::UI::MicroBlogWidget *PumpIOMicroBlog::createMicroBlogWidget(Choqok::Account *account, QWidget *parent)
{
    return new PumpIOMicroBlogWidget(account, parent);
}

Choqok::UI::ComposerWidget *PumpIOMicroBlog::createComposerWidget(Choqok::Account *account, QWidget *parent)
{
    return new PumpIOComposerWidget(account, parent);
}

ChoqokEditAccountWidget *PumpIOMicroBlog::createEditAccountWidget(Choqok::Account *account,
        QWidget *parent)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount * >(account);
    if (acc || !account) {
        return new PumpIOEditAccountWidget(this, acc, parent);
    } else {
        qCDebug(CHOQOK) << "Account passed here was not a valid PumpIOAccount!";
        return 0;
    }
}

Choqok::Account *PumpIOMicroBlog::createNewAccount(const QString &alias)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(
                             Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return new PumpIOAccount(this, alias);
    } else {
        qCDebug(CHOQOK) << "Cannot create a new PumpIOAccount!";
        return 0;
    }
}

void PumpIOMicroBlog::createPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    QVariantList to;
    QVariantMap thePublic;
    thePublic.insert(QLatin1String("objectType"), QLatin1String("collection"));
    thePublic.insert(QLatin1String("id"), PumpIOMicroBlog::PublicCollection);
    to.append(thePublic);

    createPost(theAccount, post, to);
}

void PumpIOMicroBlog::createPost(Choqok::Account *theAccount, Choqok::Post *post,
                                 const QVariantList &to, const QVariantList &cc)
{
    if (!post || post->content.isEmpty()) {
        qCDebug(CHOQOK) << "ERROR: Status text is empty!";
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::OtherError,
                         i18n("Creating the new post failed. Text is empty."), MicroBlog::Critical);
        return;
    }

    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        if (!post->postId.isEmpty()) {
            object.insert(QLatin1String("id"), post->postId);
        }
        if (post->type.isEmpty()) {
            post->type = QLatin1String("note");
        }
        object.insert(QLatin1String("objectType"), post->type);
        //Convert URLs to href form
        post->content.replace(QRegExp(QLatin1String("((?:https?|ftp)://\\S+)")), QLatin1String("<a href=\"\\1\">\\1</a>"));
        object.insert(QLatin1String("content"), QUrl::toPercentEncoding(post->content));

        QVariantMap item;
        item.insert(QLatin1String("verb"), QLatin1String("post"));
        item.insert(QLatin1String("object"), object);
        item.insert(QLatin1String("to"), to);
        item.insert(QLatin1String("cc"), cc);

        const QByteArray data = QJsonDocument::fromVariant(item).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (outboxActivity.arg(acc->username())));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_createPostJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::createReply(Choqok::Account *theAccount, PumpIOPost *post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        post->type = QLatin1String("comment");

        QVariantMap object;
        object.insert(QLatin1String("objectType"), post->type);
        //Convert URLs to href form
        post->content.replace(QRegExp(QLatin1String("((?:https?|ftp)://\\S+)")), QLatin1String("<a href=\"\\1\">\\1</a>"));
        object.insert(QLatin1String("content"), QUrl::toPercentEncoding(post->content));

        if (!post->replyToPostId.isEmpty()) {
            QVariantMap inReplyTo;
            inReplyTo.insert(QLatin1String("id"), post->replyToPostId);
            inReplyTo.insert(QLatin1String("objectType"), post->replyToObjectType);
            object.insert(QLatin1String("inReplyTo"), inReplyTo);
        }

        QVariantMap item;
        item.insert(QLatin1String("verb"), QLatin1String("post"));
        item.insert(QLatin1String("object"), object);

        const QByteArray data = QJsonDocument::fromVariant(item).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (outboxActivity.arg(acc->username())));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_createPostJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::createPostWithMedia(Choqok::Account *theAccount, Choqok::Post *post,
        const QString &filePath)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QFile media(filePath);
        QByteArray data;
        if (media.open(QIODevice::ReadOnly)) {
            data = media.readAll();
            media.close();
        } else {
            qCDebug(CHOQOK) << "Cannot read the file";
            return;
        }

        const QMimeDatabase db;
        const QMimeType mimetype = db.mimeTypeForFileNameAndData(filePath, data);
        const QString mime = mimetype.name();
        if (mime == QLatin1String("application/octet-stream")) {
            qCDebug(CHOQOK) << "Cannot retrieve file mimetype";
            return;
        }

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QStringLiteral("/api/user/%1/uploads").arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: ") + mime);
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_uploadJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotUpload(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

Choqok::UI::PostWidget *PumpIOMicroBlog::createPostWidget(Choqok::Account *account,
        Choqok::Post *post,
        QWidget *parent)
{
    return new PumpIOPostWidget(account, post, parent);
}

void PumpIOMicroBlog::fetchPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        if (!post->link.startsWith(acc->host())) {
            qCDebug(CHOQOK) << "You can only fetch posts from your host!";
            return;
        }
        QUrl url(post->link);

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::GET));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFetchPost(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::removePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert(QLatin1String("id"), post->postId);
        object.insert(QLatin1String("objectType"), post->type);

        QVariantMap item;
        item.insert(QLatin1String("verb"), QLatin1String("delete"));
        item.insert(QLatin1String("object"), object);

        const QByteArray data = QJsonDocument::fromVariant(item).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (outboxActivity.arg(acc->username())));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_removePostJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRemovePost(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

QList< Choqok::Post * > PumpIOMicroBlog::loadTimeline(Choqok::Account *account,
        const QString &timelineName)
{
    QList< Choqok::Post * > list;
    const QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(),
                             timelineName);
    const KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);
    const QStringList tmpList = postsBackup.groupList();

    // don't load old archives
    if (tmpList.isEmpty() || !(QDateTime::fromString(tmpList.first()).isValid())) {
        return list;
    }

    QList<QDateTime> groupList;
    Q_FOREACH (const QString &str, tmpList) {
        groupList.append(QDateTime::fromString(str));
    }
    qSort(groupList);
    PumpIOPost *st;
    Q_FOREACH (const QDateTime &datetime, groupList) {
        st = new PumpIOPost;
        KConfigGroup grp(&postsBackup, datetime.toString());
        st->creationDateTime = grp.readEntry("creationDateTime", QDateTime::currentDateTime());
        st->postId = grp.readEntry("postId", QString());
        st->link = grp.readEntry("link", QString());
        st->content = grp.readEntry("content", QString());
        st->source = grp.readEntry("source", QString());
        st->isFavorited = grp.readEntry("favorited", false);
        st->author.userId = grp.readEntry("authorId", QString());
        st->author.userName = grp.readEntry("authorUserName", QString());
        st->author.realName = grp.readEntry("authorRealName", QString());
        st->author.location = grp.readEntry("authorLocation", QString());
        st->author.description = grp.readEntry("authorDescription" , QString());
        st->author.profileImageUrl = grp.readEntry("authorProfileImageUrl", QString());
        st->author.homePageUrl = grp.readEntry("authorHomePageUrl", QString());
        st->type = grp.readEntry("type", QString());
        st->media = grp.readEntry("media"), QString();
        st->mediaSizeHeight = grp.readEntry("mediaHeight", 0);
        st->mediaSizeWidth = grp.readEntry("mediaWidth", 0);
        st->isRead = grp.readEntry("isRead", true);
        st->conversationId = grp.readEntry("conversationId", QString());
        st->to = grp.readEntry("to", QStringList());
        st->cc = grp.readEntry("cc", QStringList());
        st->shares = grp.readEntry("shares", QStringList());
        st->replies = grp.readEntry("replies", QString());
        st->replyToPostId = grp.readEntry("replyToPostId", QString());
        st->replyToUserName = grp.readEntry("replyToUserName", QString());
        st->replyToObjectType = grp.readEntry("replyToObjectType", QString());
        list.append(st);
    }

    if (!list.isEmpty()) {
        setLastTimelineId(account, timelineName, list.last()->conversationId);
    }

    return list;
}

QString PumpIOMicroBlog::postUrl(Choqok::Account *account, const QString &username,
                                 const QString &postId) const
{
    Q_UNUSED(account);
    return QString(postId).replace(QLatin1String("/api/"), QLatin1Char('/') + username + QLatin1Char('/'));
}

QString PumpIOMicroBlog::profileUrl(Choqok::Account *account, const QString &username) const
{
    Q_UNUSED(account)
    if (username.contains(QLatin1String("acct:"))) {
        return QStringLiteral("https://%1/%2").arg(hostFromAcct(username)).arg(userNameFromAcct(username));
    } else {
        return username;
    }
}

void PumpIOMicroBlog::saveTimeline(Choqok::Account *account, const QString &timelineName,
                                   const QList< Choqok::UI::PostWidget * > &timeline)
{
    const QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(),
                             timelineName);
    KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);

    ///Clear previous data:
    Q_FOREACH (const QString &group, postsBackup.groupList()) {
        postsBackup.deleteGroup(group);
    }

    QList< Choqok::UI::PostWidget *>::const_iterator it, endIt = timeline.constEnd();
    for (it = timeline.constBegin(); it != endIt; ++it) {
        PumpIOPost *post = dynamic_cast<PumpIOPost * >((*it)->currentPost());
        KConfigGroup grp(&postsBackup, post->creationDateTime.toString());
        grp.writeEntry("creationDateTime", post->creationDateTime);
        grp.writeEntry("postId", post->postId);
        grp.writeEntry("link", post->link);
        grp.writeEntry("content", post->content);
        grp.writeEntry("source", post->source);
        grp.writeEntry("favorited", post->isFavorited);
        grp.writeEntry("authorId", post->author.userId);
        grp.writeEntry("authorRealName", post->author.realName);
        grp.writeEntry("authorUserName", post->author.userName);
        grp.writeEntry("authorLocation", post->author.location);
        grp.writeEntry("authorDescription", post->author.description);
        grp.writeEntry("authorProfileImageUrl", post->author.profileImageUrl);
        grp.writeEntry("authorHomePageUrl", post->author.homePageUrl);
        grp.writeEntry("type", post->type);
        grp.writeEntry("media", post->media);
        grp.writeEntry("mediaHeight", post->mediaSizeHeight);
        grp.writeEntry("mediaWidth", post->mediaSizeWidth);
        grp.writeEntry("isRead", post->isRead);
        grp.writeEntry("conversationId", post->conversationId);
        grp.writeEntry("to", post->to);
        grp.writeEntry("cc", post->cc);
        grp.writeEntry("shares", post->shares);
        grp.writeEntry("replies", post->replies);
        grp.writeEntry("replyToPostId", post->replyToPostId);
        grp.writeEntry("replyToUserName", post->replyToUserName);
        grp.writeEntry("replyToObjectType", post->replyToObjectType);
    }
    postsBackup.sync();

    if (Choqok::Application::isShuttingDown()) {
        --d->countOfTimelinesToSave;
        if (d->countOfTimelinesToSave < 1) {
            Q_EMIT readyForUnload();
        }
    }
}

Choqok::TimelineInfo *PumpIOMicroBlog::timelineInfo(const QString &timelineName)
{
    return m_timelinesInfos.value(timelineName);
}

void PumpIOMicroBlog::updateTimelines(Choqok::Account *theAccount)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        Q_FOREACH (const QString &timeline, acc->timelineNames()) {
            QUrl url(acc->host());
            url = url.adjusted(QUrl::StripTrailingSlash);
            url.setPath(url.path() + QLatin1Char('/') + (m_timelinesPaths[timeline].arg(acc->username())));

            QOAuth::ParamMap oAuthParams;
            const QString lastActivityId(lastTimelineId(theAccount, timeline));
            if (!lastActivityId.isEmpty()) {
                oAuthParams.insert("count", QByteArray::number(200));
                oAuthParams.insert("since", QUrl::toPercentEncoding(lastActivityId));
            } else {
                oAuthParams.insert("count", QByteArray::number(Choqok::BehaviorSettings::countOfPosts()));
            }

            KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
            if (!job) {
                qCDebug(CHOQOK) << "Cannot create an http GET request!";
                continue;
            }
            job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::GET,
                             oAuthParams));
            m_timelinesRequests[job] = timeline;
            m_accountJobs[job] = acc;
            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotUpdateTimeline(KJob*)));
            job->start();
        }
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::fetchFollowing(Choqok::Account *theAccount)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QStringLiteral("/api/user/%1/following").arg(acc->username()));

        QOAuth::ParamMap oAuthParams;
        oAuthParams.insert("count", QByteArray::number(200));
        if (!acc->following().isEmpty()) {
            oAuthParams.insert("since", QUrl::toPercentEncoding(acc->following().last()));
        }

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::GET,
                         oAuthParams));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFollowing(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::fetchLists(Choqok::Account *theAccount)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QStringLiteral("/api/user/%1/lists/person").arg(acc->username()));

        QOAuth::ParamMap oAuthParams;
        oAuthParams.insert("count", QByteArray::number(200));

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::GET,
                         oAuthParams));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotLists(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::share(Choqok::Account *theAccount, Choqok::Post *post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert(QLatin1String("objectType"), post->type);
        object.insert(QLatin1String("id"), post->postId);

        QVariantMap item;
        item.insert(QLatin1String("verb"), QLatin1String("share"));
        item.insert(QLatin1String("object"), object);

        const QByteArray data = QJsonDocument::fromVariant(item).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (outboxActivity.arg(acc->username())));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_shareJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotShare(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::toggleFavorite(Choqok::Account *theAccount, Choqok::Post *post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert(QLatin1String("objectType"), post->type);
        object.insert(QLatin1String("id"), post->postId);

        QVariantMap item;
        item.insert(QLatin1String("verb"), post->isFavorited ? QLatin1String("unfavorite") : QLatin1String("favorite"));
        item.insert(QLatin1String("object"), object);

        const QByteArray data = QJsonDocument::fromVariant(item).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (outboxActivity.arg(acc->username())));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_favoriteJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFavorite(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::showDirectMessageDialog()
{
    qCDebug(CHOQOK);
    const QString alias = qobject_cast<QAction *>(sender())->data().toString();
    PumpIOAccount *theAccount = qobject_cast<PumpIOAccount *>(Choqok::AccountManager::self()->findAccount(alias));
    PumpIOMessageDialog *msg = new PumpIOMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    msg->show();
}

void PumpIOMicroBlog::slotCreatePost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_createPostJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap reply = json.toVariant().toMap();
            if (!reply[QLatin1String("object")].toMap().value(QLatin1String("id")).toString().isEmpty()) {
                Choqok::NotifyManager::success(i18n("New post submitted successfully"));
                ret = 0;
                Q_EMIT postCreated(theAccount, post);
            }
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::CommunicationError,
                         i18n("Creating the new post failed. %1", job->errorString()),
                         MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotFavorite(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_favoriteJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Cannot set/unset the post as favorite. %1", job->errorString()));
    } else {
        post->isFavorited = !post->isFavorited;
        Q_EMIT favorite(theAccount, post);
    }
}

void PumpIOMicroBlog::slotFetchPost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        qCDebug(CHOQOK) << "Account or postId is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap reply = json.toVariant().toMap();
            PumpIOPost *post = new PumpIOPost;
            readPost(reply, post);
            ret = 0;
            Q_EMIT postFetched(theAccount, post);
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Cannot fetch post. %1", job->errorString()),
                     MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotFetchReplies(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        qCDebug(CHOQOK) << "Account or postId is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap reply = json.toVariant().toMap();
            const QVariantList items = reply[QLatin1String("items")].toList();
            for (int i = items.size() - 1; i >= 0; i--) {
                QVariantMap item = items.at(i).toMap();
                PumpIOPost *r = new PumpIOPost;
                readPost(item, r);
                r->replyToPostId = reply[QLatin1String("url")].toString().remove(QLatin1String("/replies"));
                Q_EMIT postFetched(theAccount, r);
            }
            ret = 0;
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Cannot fetch replies. %1", job->errorString()),
                     MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotFollowing(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        qCDebug(CHOQOK) << "Account is NULL pointer";
        return;
    }
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    }
    bool ret = 1;
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        Choqok::UI::Global::mainWindow()->showStatusMessage(
            i18n("Following list for account %1 has been updated.",
                 acc->username()));
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantList items = json.toVariant().toMap().value(QLatin1String("items")).toList();
            QStringList following;
            Q_FOREACH (const QVariant &element, items) {
                following.append(element.toMap().value(QLatin1String("id")).toString());
            }
            acc->setFollowing(following);
            ret = 0;
            Q_EMIT followingFetched(acc);
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }

    if (ret) {
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Cannot retrieve the following list. %1", job->errorString()));
    }
}

void PumpIOMicroBlog::slotLists(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        qCDebug(CHOQOK) << "Account is NULL pointer";
        return;
    }
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    }
    bool ret = 1;
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        Choqok::UI::Global::mainWindow()->showStatusMessage(
            i18n("Lists for account %1 has been updated.",
                 acc->username()));
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantList items = json.toVariant().toMap().value(QLatin1String("items")).toList();
            QVariantList lists;
            Q_FOREACH (const QVariant &element, items) {
                QVariantMap e = element.toMap();
                QVariantMap list;
                list.insert(QLatin1String("id"), e.value(QLatin1String("id")).toString());
                list.insert(QLatin1String("name"), e.value(QLatin1String("displayName")).toString());
                lists.append(list);
            }
            acc->setLists(lists);
            ret = 0;
            Q_EMIT listsFetched(acc);
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }

    if (ret) {
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Cannot retrieve the lists. %1", job->errorString()));
    }
}

void PumpIOMicroBlog::slotShare(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_shareJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        Choqok::UI::Global::mainWindow()->showStatusMessage(
            i18n("The post has been shared."));
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap object = json.toVariant().toMap().value(QLatin1String("object")).toMap();
            ret = 0;
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        Q_EMIT error(theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Cannot share the post. %1", job->errorString()));
    }
}

void PumpIOMicroBlog::slotRemovePost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_removePostJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap object = json.toVariant().toMap().value(QLatin1String("object")).toMap();
            if (!object[QLatin1String("deleted")].toString().isEmpty()) {
                Choqok::NotifyManager::success(i18n("Post removed successfully"));
                ret = 0;
                Q_EMIT postRemoved(theAccount, post);
            }
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::CommunicationError,
                         i18n("Removing the post failed. %1", job->errorString()),
                         MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotUpdatePost(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_updateJobs.take(job);
    Choqok::Account *account = m_accountJobs.take(job);
    if (!post || !account) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            ret = 0;
            createPost(account, post);
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        Q_EMIT error(account, Choqok::MicroBlog::CommunicationError,
                     i18n("An error occurred when updating the post"));
    }
}

void PumpIOMicroBlog::slotUpdateTimeline(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Account *account = m_accountJobs.take(job);
    if (!account) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(account, Choqok::MicroBlog::CommunicationError,
                     i18n("An error occurred when fetching the timeline"));
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);
        const QList<Choqok::Post * > list = readTimeline(j->data());
        const QString timeline(m_timelinesRequests.take(job));
        if (!list.isEmpty()) {
            setLastTimelineId(account, timeline, list.last()->conversationId);
        }

        Q_EMIT timelineDataReceived(account, timeline, list);
    }
}

void PumpIOMicroBlog::slotUpload(KJob *job)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_uploadJobs.take(job);
    Choqok::Account *account = m_accountJobs.take(job);
    if (!post || !account) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap reply = json.toVariant().toMap();
            const QString id = reply[QLatin1String("id")].toString();
            if (!id.isEmpty()) {
                post->postId = id;
                post->type = reply[QLatin1String("objectType")].toString();
                ret = 0;
                updatePost(account, post);
            }
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }

    }

    if (ret) {
        Q_EMIT error(account, Choqok::MicroBlog::CommunicationError,
                     i18n("An error occurred when uploading the media"));
    }
}

QString PumpIOMicroBlog::authorizationMetaData(PumpIOAccount *account, const QUrl &url,
        const QOAuth::HttpMethod &method,
        const QOAuth::ParamMap &paramMap) const
{
    const QByteArray authorization = account->oAuth()->createParametersString(url.url(),
                                     method, account->token().toLocal8Bit(),
                                     account->tokenSecret().toLocal8Bit(),
                                     QOAuth::HMAC_SHA1, paramMap,
                                     QOAuth::ParseForHeaderArguments);
    return QStringLiteral("Authorization: ") + QLatin1String(authorization);
}

void PumpIOMicroBlog::fetchReplies(Choqok::Account *theAccount, const QString &url)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        if (!url.startsWith(acc->host())) {
            qCDebug(CHOQOK) << "You can only fetch replies from your host!";
            return;
        }
        QUrl u(url);

        KIO::StoredTransferJob *job = KIO::storedGet(u, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, u, QOAuth::GET));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFetchReplies(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

QString PumpIOMicroBlog::lastTimelineId(Choqok::Account *theAccount,
                                        const QString &timeline) const
{
    qCDebug(CHOQOK) << "Latest ID for timeline " << timeline << m_timelinesLatestIds[theAccount][timeline];
    return m_timelinesLatestIds[theAccount][timeline];
}

Choqok::Post *PumpIOMicroBlog::readPost(const QVariantMap &var, Choqok::Post *post)
{
    PumpIOPost *p = dynamic_cast< PumpIOPost * >(post);
    if (p) {
        QVariantMap object;
        if (var.value(QLatin1String("verb")).toString() == QLatin1String("post") ||
                var.value(QLatin1String("verb")).toString() == QLatin1String("share")) {
            object = var[QLatin1String("object")].toMap();
        } else {
            object = var;
        }

        QTextDocument content;
        if (!object[QLatin1String("displayName")].isNull()) {
            content.setHtml(object[QLatin1String("displayName")].toString());
            p->content = content.toPlainText().trimmed();
            p->content += QLatin1Char('\n');
        }

        content.setHtml(object[QLatin1String("content")].toString());
        p->content += content.toPlainText().trimmed();

        if (!object[QLatin1String("fullImage")].isNull()) {
            const QVariantMap fullImage = object[QLatin1String("fullImage")].toMap();
            if (!fullImage.isEmpty()) {
                p->media = fullImage[QLatin1String("url")].toString();
                p->mediaSizeHeight = fullImage[QLatin1String("height")].toInt();
                p->mediaSizeWidth = fullImage[QLatin1String("width")].toInt();
            }
        }
        p->creationDateTime = QDateTime::fromString(var[QLatin1String("published")].toString(),
                              Qt::ISODate);
        p->creationDateTime.setTimeSpec(Qt::UTC);
        if (object[QLatin1String("pump_io")].isNull()) {
            p->link = object[QLatin1String("id")].toString();
        } else {
            p->link = object[QLatin1String("pump_io")].toMap().value(QLatin1String("proxyURL")).toString();
        }
        p->type = object[QLatin1String("objectType")].toString();
        p->isFavorited = object[QLatin1String("liked")].toBool();
        if (p->isFavorited) {
            p->isRead = true;
        }
        p->postId = object[QLatin1String("id")].toString();
        p->conversationId = var[QLatin1String("id")].toString();

        QString author;
        var[QLatin1String("author")].isNull() ? author = QLatin1String("actor") : author = QLatin1String("author");
        QVariantMap actor;
        if (var.value(QLatin1String("verb")).toString() == QLatin1String("share")) {
            actor = object[QLatin1String("author")].toMap();
            const QVariantList shares = object[QLatin1String("shares")].toMap().value(QLatin1String("items")).toList();
            Q_FOREACH (const QVariant &element, shares) {
                p->shares.append(element.toMap().value(QLatin1String("id")).toString());
            }
        } else {
            actor = var[author].toMap();
        }
        const QString userId = actor[QLatin1String("id")].toString();
        const QString homePageUrl = actor[QLatin1String("url")].toString();
        p->author.userId = userId;
        p->author.userName = actor[QLatin1String("preferredUsername")].toString();
        p->author.realName = actor[QLatin1String("displayName")].toString();
        p->author.homePageUrl = homePageUrl;
        p->author.location = actor[QLatin1String("location")].toMap().value(QLatin1String("displayName")).toString();
        p->author.description = actor[QLatin1String("summary")].toString();

        const QString profileImageUrl = actor[QLatin1String("image")].toMap().value(QLatin1String("url")).toString();
        if (!profileImageUrl.isEmpty()) {
            p->author.profileImageUrl = profileImageUrl;
        } else if (actor[QLatin1String("objectType")].toString() == QLatin1String("service")) {
            p->author.profileImageUrl = homePageUrl + QLatin1String("images/default.png");
        } else {
            p->author.profileImageUrl = QStringLiteral("https://%1/images/default.png").arg(hostFromAcct(userId));
        }

        if (!var[QLatin1String("generator")].isNull()) {
            p->source = var[QLatin1String("generator")].toMap().value(QLatin1String("displayName")).toString();
        }

        const QVariantList to = var[QLatin1String("to")].toList();
        Q_FOREACH (const QVariant &element, to) {
            QVariantMap toElementMap = element.toMap();
            QString toElementType = toElementMap.value(QLatin1String("objectType")).toString();
            if (toElementType == QLatin1String("person") || toElementType == QLatin1String("collection")) {
                const QString toId = toElementMap.value(QLatin1String("id")).toString();

                if (toId.compare(QLatin1String("acct:")) != 0) {
                    p->to.append(toId);
                }
            }
        }

        const QVariantList cc = var[QLatin1String("cc")].toList();
        Q_FOREACH (const QVariant &element, cc) {
            QVariantMap ccElementMap = element.toMap();
            QString ccElementType = ccElementMap.value(QLatin1String("objectType")).toString();
            if (ccElementType == QLatin1String("person") || ccElementType == QLatin1String("collection")) {
                const QString ccId = ccElementMap.value(QLatin1String("id")).toString();

                if (ccId.compare(QLatin1String("acct:")) != 0) {
                    p->cc.append(ccId);
                }
            }
        }

        const QVariantMap replies = object[QLatin1String("replies")].toMap();
        if (replies.value(QLatin1String("pump_io")).isNull()) {
            p->replies = replies[QLatin1String("url")].toString();
        } else {
            p->replies = replies[QLatin1String("pump_io")].toMap().value(QLatin1String("proxyURL")).toString();
        }

        return p;
    } else {
        qCDebug(CHOQOK) << "post is not a PumpIOPost!";
        return post;
    }
}

QList< Choqok::Post * > PumpIOMicroBlog::readTimeline(const QByteArray &buffer)
{
    QList<Choqok::Post * > posts;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        const QVariantList list = json.toVariant().toMap().value(QLatin1String("items")).toList();
        Q_FOREACH (const QVariant &element, list) {
            const QVariantMap elementMap = element.toMap();
            if (!elementMap[QLatin1String("object")].toMap().value(QLatin1String("deleted")).isNull()) {
                // Skip deleted posts
                continue;
            }
            posts.prepend(readPost(elementMap, new PumpIOPost));
        }
    } else {
        qCDebug(CHOQOK) << "Cannot parse JSON reply";
    }

    return posts;
}

void PumpIOMicroBlog::setLastTimelineId(Choqok::Account *theAccount,
                                        const QString &timeline,
                                        const QString &id)
{
    m_timelinesLatestIds[theAccount][timeline] = id;
}

void PumpIOMicroBlog::setTimelinesInfo()
{
    Choqok::TimelineInfo *t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Activity");
    t->description = i18nc("Timeline description", "You and people you follow");
    t->icon = QLatin1String("user-home");
    m_timelinesInfos[QLatin1String("Activity")] = t;
    m_timelinesPaths[QLatin1String("Activity")] = inboxActivity + QLatin1String("/major");

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Favorites");
    t->description = i18nc("Timeline description", "Posts you favorited");
    t->icon = QLatin1String("favorites");
    m_timelinesInfos[QLatin1String("Favorites")] = t;
    m_timelinesPaths[QLatin1String("Favorites")] = QLatin1String("/api/user/%1/favorites");

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Inbox");
    t->description = i18nc("Timeline description", "Posts sent to you");
    t->icon = QLatin1String("mail-folder-inbox");
    m_timelinesInfos[QLatin1String("Inbox")] = t;
    m_timelinesPaths[QLatin1String("Inbox")] = inboxActivity + QLatin1String("/direct/major/");

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Outbox");
    t->description = i18nc("Timeline description", "Posts you sent");
    t->icon = QLatin1String("mail-folder-outbox");
    m_timelinesInfos[QLatin1String("Outbox")] = t;
    m_timelinesPaths[QLatin1String("Outbox")] = outboxActivity + QLatin1String("/major/");
}

void PumpIOMicroBlog::updatePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert(QLatin1String("id"), post->postId);
        object.insert(QLatin1String("objectType"), post->type);
        object.insert(QLatin1String("content"), QUrl::toPercentEncoding(post->content));

        // https://github.com/e14n/pump.io/issues/885
        QVariantList to;
        QVariantMap thePublic;
        thePublic.insert(QLatin1String("objectType"), QLatin1String("collection"));
        thePublic.insert(QLatin1String("id"), PumpIOMicroBlog::PublicCollection);
        to.append(thePublic);

        QVariantMap item;
        item.insert(QLatin1String("verb"), QLatin1String("update"));
        item.insert(QLatin1String("object"), object);
        item.insert(QLatin1String("to"), to);

        const QByteArray data = QJsonDocument::fromVariant(item).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (outboxActivity.arg(acc->username())));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_updateJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotUpdatePost(KJob*)));
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a PumpIOAccount!";
    }
}

QString PumpIOMicroBlog::hostFromAcct(const QString &acct)
{
    if (acct.contains(QLatin1String("acct:"))) {
        return acct.split(QLatin1Char(':'))[1].split(QLatin1Char('@'))[1];
    }

    return acct;
}

QString PumpIOMicroBlog::userNameFromAcct(const QString &acct)
{
    if (acct.contains(QLatin1String("acct:"))) {
        return acct.split(QLatin1Char(':'))[1].split(QLatin1Char('@'))[0];
    }

    return acct;
}

#include "pumpiomicroblog.moc"
