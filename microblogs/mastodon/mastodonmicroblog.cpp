/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017 Andrea Scarpino <scarpino@kde.org>

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

#include "mastodonmicroblog.h"

#include <QAction>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QMimeDatabase>
#include <QTextDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

#include "accountmanager.h"
#include "application.h"
#include "choqokappearancesettings.h"
#include "choqokbehaviorsettings.h"
#include "notifymanager.h"
#include "postwidget.h"

#include "mastodonaccount.h"
#include "mastodoncomposerwidget.h"
#include "mastodondebug.h"
#include "mastodondmessagedialog.h"
#include "mastodoneditaccountwidget.h"
#include "mastodonpost.h"
#include "mastodonpostwidget.h"

class MastodonMicroBlog::Private
{
public:
    Private(): countOfTimelinesToSave(0)
    {}
    int countOfTimelinesToSave;
};

K_PLUGIN_FACTORY_WITH_JSON(MastodonMicroBlogFactory, "choqok_mastodon.json",
                           registerPlugin < MastodonMicroBlog > ();)

const QString MastodonMicroBlog::homeTimeline(QLatin1String("/api/v1/timelines/home"));
const QString MastodonMicroBlog::publicTimeline(QLatin1String("/api/v1/timelines/public"));
const QString MastodonMicroBlog::favouritesTimeline(QLatin1String("/api/v1/favourites"));

MastodonMicroBlog::MastodonMicroBlog(QObject *parent, const QVariantList &args):
    MicroBlog(QStringLiteral("Mastodon") , parent), d(new Private)
{
    Q_UNUSED(args)
    setServiceName(QLatin1String("Mastodon"));
    setServiceHomepageUrl(QLatin1String("https://mastodon.social"));
    QStringList timelineNames;
    timelineNames << QLatin1String("Home") << QLatin1String("Local") << QLatin1String("Federated") << QLatin1String("Favourites");
    setTimelineNames(timelineNames);
    setTimelinesInfo();
}

MastodonMicroBlog::~MastodonMicroBlog()
{
    qDeleteAll(m_timelinesInfos);
    delete d;
}

void MastodonMicroBlog::aboutToUnload()
{
    for (Choqok::Account *acc: Choqok::AccountManager::self()->accounts()) {
        if (acc->microblog() == this) {
            d->countOfTimelinesToSave += acc->timelineNames().count();
        }
    }
    Q_EMIT saveTimelines();
}

ChoqokEditAccountWidget *MastodonMicroBlog::createEditAccountWidget(Choqok::Account *account,
        QWidget *parent)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount * >(account);
    if (acc || !account) {
        return new MastodonEditAccountWidget(this, acc, parent);
    } else {
        qCDebug(CHOQOK) << "Account passed here was not a valid MastodonAccount!";
        return nullptr;
    }
}

Choqok::UI::ComposerWidget *MastodonMicroBlog::createComposerWidget(Choqok::Account *account, QWidget *parent)
{
    return new MastodonComposerWidget(account, parent);
}

void MastodonMicroBlog::createPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    if (!post || post->content.isEmpty()) {
        qCDebug(CHOQOK) << "ERROR: Status text is empty!";
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::OtherError,
                         i18n("Creating the new post failed. Text is empty."), MicroBlog::Critical);
        return;
    }

    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert(QLatin1String("status"), post->content);

        const QByteArray data = QJsonDocument::fromVariant(object).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1String("/api/v1/statuses"));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_createPostJobs[job] = post;
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotCreatePost);
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

Choqok::Account *MastodonMicroBlog::createNewAccount(const QString &alias)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(
                             Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return new MastodonAccount(this, alias);
    } else {
        qCDebug(CHOQOK) << "Cannot create a new MastodonAccount!";
        return nullptr;
    }
}

QString MastodonMicroBlog::lastTimelineId(Choqok::Account *theAccount,
                                          const QString &timeline) const
{
    qCDebug(CHOQOK) << "Latest ID for timeline " << timeline << m_timelinesLatestIds[theAccount][timeline];
    return m_timelinesLatestIds[theAccount][timeline];
}

QList< Choqok::Post * > MastodonMicroBlog::readTimeline(const QByteArray &buffer)
{
    QList<Choqok::Post * > posts;
    const QJsonDocument json = QJsonDocument::fromJson(buffer);
    if (!json.isNull()) {
        const QVariantList list = json.array().toVariantList();
        for (const QVariant &element: list) {
            posts.prepend(readPost(element.toMap(), new MastodonPost));
        }
    } else {
        qCDebug(CHOQOK) << "Cannot parse JSON reply";
    }

    return posts;
}

Choqok::Post *MastodonMicroBlog::readPost(const QVariantMap &var, Choqok::Post *post)
{
    MastodonPost *p = dynamic_cast< MastodonPost * >(post);
    if (p) {
        QVariantMap reblog = var[QLatin1String("reblog")].toMap();
        QVariantMap status;
        if (reblog.isEmpty()) {
            status = var;
        } else {
            status = reblog;
        }

        QTextDocument content;
        content.setHtml(status[QLatin1String("spoiler_text")].toString() + QLatin1String("<br />") + status[QLatin1String("content")].toString());
        p->content += content.toPlainText().trimmed();

        p->creationDateTime = QDateTime::fromString(var[QLatin1String("created_at")].toString(),
                              Qt::ISODate);
        p->creationDateTime.setTimeSpec(Qt::UTC);

        p->link = status[QLatin1String("url")].toUrl();
        p->isFavorited = var[QLatin1String("favourited")].toBool();
        if (p->isFavorited) {
            p->isRead = true;
        }
        p->postId = var[QLatin1String("id")].toString();

        p->conversationId = var[QLatin1String("id")].toString();

        QVariantMap application = var[QLatin1String("application")].toMap();
        if (!application.isEmpty()) {
            const QString client = application[QLatin1String("name")].toString();
            if (application[QLatin1String("website")].toString().isEmpty()) {
                p->source = client;
            } else {
                p->source = QStringLiteral("<a href=\"%1\" rel=\"nofollow\">%2</a>").arg(application[QLatin1String("website")].toString()).arg(client);
            }
        }

        if (var[QLatin1String("visibility")].toString().compare(QLatin1String("direct")) == 0) {
            p->isPrivate = true;
        }

        QVariantMap account = status[QLatin1Literal("account")].toMap();

        p->author.userId = account[QLatin1String("id")].toString();
        p->author.userName = account[QLatin1String("acct")].toString();
        p->author.realName = account[QLatin1String("display_name")].toString();
        p->author.homePageUrl = account[QLatin1String("url")].toUrl();

        QTextDocument description;
        description.setHtml(account[QLatin1String("note")].toString());
        p->author.description = description.toPlainText().trimmed();

        p->author.profileImageUrl = account[QLatin1String("avatar")].toUrl();

        p->replyToPostId = var[QLatin1String("in_reply_to_id")].toString();
        p->replyToUser.userId = var[QLatin1String("in_reply_to_account_id")].toString();

        if (!reblog.isEmpty()) {
            p->repeatedDateTime = QDateTime::fromString(var[QLatin1String("created_at")].toString(),
                              Qt::ISODate);
            p->repeatedDateTime.setTimeSpec(Qt::UTC);

            p->repeatedPostId = var[QLatin1String("id")].toString();
            const QVariantMap repeatedFrom = var[QLatin1Literal("account")].toMap();
            p->repeatedFromUser.userId = repeatedFrom[QLatin1String("id")].toString();
            p->repeatedFromUser.userName = repeatedFrom[QLatin1String("acct")].toString();
            p->repeatedFromUser.homePageUrl = repeatedFrom[QLatin1String("url")].toUrl();
        }

        return p;
    } else {
        qCDebug(CHOQOK) << "post is not a MastodonPost!";
        return post;
    }
}

void MastodonMicroBlog::createReply(Choqok::Account *theAccount, MastodonPost *post)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert(QLatin1String("status"), post->content);

        if (!post->replyToPostId.isEmpty()) {
            object.insert(QLatin1String("in_reply_to_id"), post->replyToPostId);
        }

        const QByteArray data = QJsonDocument::fromVariant(object).toJson();

        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1String("/api/v1/statuses"));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_createPostJobs[job] = post;
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotCreatePost);
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

void MastodonMicroBlog::toggleReblog(Choqok::Account *theAccount, Choqok::Post *post)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        if (acc->username().compare(post->repeatedFromUser.userName) == 0) {
            url.setPath(url.path() + QStringLiteral("/api/v1/statuses/%1/unreblog").arg(post->postId));
        } else {
            url.setPath(url.path() + QStringLiteral("/api/v1/statuses/%1/reblog").arg(post->postId));
        }
        KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_shareJobs[job] = post;
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotReblog);
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

void MastodonMicroBlog::slotReblog(KJob *job)
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
        Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("The post has been shared."));
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
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

void MastodonMicroBlog::toggleFavorite(Choqok::Account *theAccount, Choqok::Post *post)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);

        if (post->isFavorited) {
            url.setPath(url.path() + QStringLiteral("/api/v1/statuses/%1/unfavourite").arg(post->postId));
        } else {
            url.setPath(url.path() + QStringLiteral("/api/v1/statuses/%1/favourite").arg(post->postId));
        }

        KIO::StoredTransferJob *job = KIO::storedHttpPost(QByteArray(), url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_favoriteJobs[job] = post;
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotFavorite);
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

void MastodonMicroBlog::slotFavorite(KJob *job)
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

void MastodonMicroBlog::setLastTimelineId(Choqok::Account *theAccount,
                                          const QString &timeline,
                                          const QString &id)
{
    m_timelinesLatestIds[theAccount][timeline] = id;
}

void MastodonMicroBlog::setTimelinesInfo()
{
    Choqok::TimelineInfo *t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Home");
    t->description = i18nc("Timeline description", "You and people you follow");
    t->icon = QLatin1String("user-home");
    m_timelinesInfos[QLatin1String("Home")] = t;
    m_timelinesPaths[QLatin1String("Home")] = homeTimeline;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Local");
    t->description = i18nc("Timeline description", "Local timeline");
    t->icon = QLatin1String("folder-public");
    m_timelinesInfos[QLatin1String("Local")] = t;
    m_timelinesPaths[QLatin1String("Local")] = publicTimeline;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Federated");
    t->description = i18nc("Timeline description", "Federated timeline");
    t->icon = QLatin1String("folder-remote");
    m_timelinesInfos[QLatin1String("Federated")] = t;
    m_timelinesPaths[QLatin1String("Federated")] = publicTimeline;

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Favourites");
    t->description = i18nc("Timeline description", "Favourites");
    t->icon = QLatin1String("favorites");
    m_timelinesInfos[QLatin1String("Favourites")] = t;
    m_timelinesPaths[QLatin1String("Favourites")] = favouritesTimeline;
}

void MastodonMicroBlog::removePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QStringLiteral("/api/v1/statuses/%1").arg(post->postId));
        KIO::TransferJob *job = KIO::http_delete(url, KIO::HideProgressInfo);
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_removePostJobs[job] = post;
        connect(job, &KIO::TransferJob::result, this, &MastodonMicroBlog::slotRemovePost);
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

QList<Choqok::Post * > MastodonMicroBlog::loadTimeline(Choqok::Account *account,
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
    for (const QString &str: tmpList) {
        groupList.append(QDateTime::fromString(str));
    }
    std::sort(groupList.begin(), groupList.end());
    MastodonPost *st;
    for (const QDateTime &datetime: groupList) {
        st = new MastodonPost;
        KConfigGroup grp(&postsBackup, datetime.toString());
        st->creationDateTime = grp.readEntry("creationDateTime", QDateTime::currentDateTime());
        st->postId = grp.readEntry("postId", QString());
        st->link = grp.readEntry("link", QUrl());
        st->content = grp.readEntry("content", QString());
        st->source = grp.readEntry("source", QString());
        st->isFavorited = grp.readEntry("favorited", false);
        st->author.userId = grp.readEntry("authorId", QString());
        st->author.userName = grp.readEntry("authorUserName", QString());
        st->author.realName = grp.readEntry("authorRealName", QString());
        st->author.description = grp.readEntry("authorDescription" , QString());
        st->author.profileImageUrl = grp.readEntry("authorProfileImageUrl", QUrl());
        st->author.homePageUrl = grp.readEntry("authorHomePageUrl", QUrl());
        st->isRead = grp.readEntry("isRead", true);
        st->conversationId = grp.readEntry("conversationId", QString());
        st->replyToPostId = grp.readEntry("replyToPostId", QString());
        st->replyToUser.userId = grp.readEntry("replyToUserId", QString());
        st->repeatedFromUser.userId = grp.readEntry("repeatedFromUserId", QString());
        st->repeatedFromUser.userName = grp.readEntry("repeatedFromUserName", QString());
        st->repeatedFromUser.homePageUrl = grp.readEntry("repeatedFromUserHomePage", QUrl());
        st->repeatedPostId = grp.readEntry("repeatedPostId", QString());
        st->repeatedDateTime = grp.readEntry("repeatedDateTime", QDateTime());

        list.append(st);
    }

    if (!list.isEmpty()) {
        setLastTimelineId(account, timelineName, list.last()->conversationId);
    }

    return list;
}

QUrl MastodonMicroBlog::profileUrl(Choqok::Account *account, const QString &username) const
{
    if (username.contains(QLatin1Char('@'))) {
        return QUrl::fromUserInput(QStringLiteral("https://%1/@%2").arg(hostFromAcct(username)).arg(userNameFromAcct(username)));
    } else {
        MastodonAccount *acc = qobject_cast<MastodonAccount *>(account);
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(QLatin1String("/@") + username);

        return url;
    }
}

QString MastodonMicroBlog::generateRepeatedByUserTooltip(const QString &username) const
{
    if (Choqok::AppearanceSettings::showRetweetsInChoqokWay()) {
        return i18n("Boost of %1", username);
    } else {
        return i18n("Boosted by %1", username);
    }
}

void MastodonMicroBlog::showDirectMessageDialog(MastodonAccount *theAccount, const QString &toUsername)
{
    qCDebug(CHOQOK);
    if (!theAccount) {
        QAction *act = qobject_cast<QAction *>(sender());
        theAccount = qobject_cast<MastodonAccount *>(
                         Choqok::AccountManager::self()->findAccount(act->data().toString()));
    }
    MastodonDMessageDialog *dmsg = new MastodonDMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    if (!toUsername.isEmpty()) {
        dmsg->setTo(toUsername);
    }
    dmsg->show();
}

QString MastodonMicroBlog::hostFromAcct(const QString &acct)
{
    if (acct.contains(QLatin1Char('@'))) {
        return acct.split(QLatin1Char('@'))[1];
    } else {
        return acct;
    }
}

QString MastodonMicroBlog::userNameFromAcct(const QString &acct)
{
    if (acct.contains(QLatin1Char('@'))) {
        return acct.split(QLatin1Char('@'))[0];
    } else {
        return acct;
    }
}

void MastodonMicroBlog::saveTimeline(Choqok::Account *account, const QString &timelineName,
                                     const QList< Choqok::UI::PostWidget * > &timeline)
{
    const QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(),
                             timelineName);
    KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);

    ///Clear previous data:
    for (const QString &group: postsBackup.groupList()) {
        postsBackup.deleteGroup(group);
    }

    for (Choqok::UI::PostWidget *wd: timeline) {
        MastodonPost *post = dynamic_cast<MastodonPost * >(wd->currentPost());
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
        grp.writeEntry("authorDescription", post->author.description);
        grp.writeEntry("authorProfileImageUrl", post->author.profileImageUrl);
        grp.writeEntry("authorHomePageUrl", post->author.homePageUrl);
        grp.writeEntry("isRead", post->isRead);
        grp.writeEntry("conversationId", post->conversationId);
        grp.writeEntry("replyToPostId", post->replyToPostId);
        grp.writeEntry("replyToUserId", post->replyToUser.userId);
        grp.writeEntry("repeatedFromUserId", post->repeatedFromUser.userId);
        grp.writeEntry("repeatedFromUserName", post->repeatedFromUser.userName);
        grp.writeEntry("repeatedFromUserHomePage", post->repeatedFromUser.homePageUrl);
        grp.writeEntry("repeatedPostId", post->repeatedPostId);
        grp.writeEntry("repeatedDateTime", post->repeatedDateTime);
    }
    postsBackup.sync();

    if (Choqok::Application::isShuttingDown()) {
        --d->countOfTimelinesToSave;
        if (d->countOfTimelinesToSave < 1) {
            Q_EMIT readyForUnload();
        }
    }
}

Choqok::TimelineInfo *MastodonMicroBlog::timelineInfo(const QString &timelineName)
{
    return m_timelinesInfos.value(timelineName);
}

void MastodonMicroBlog::updateTimelines(Choqok::Account *theAccount)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        for (const QString &timeline: acc->timelineNames()) {
            QUrl url(acc->host());
            url = url.adjusted(QUrl::StripTrailingSlash);
            url.setPath(url.path() + QLatin1Char('/') + m_timelinesPaths[timeline]);

            QUrlQuery query;
            if (timeline.compare(QLatin1String("Local")) == 0) {
                query.addQueryItem(QLatin1String("local"), QLatin1String("true"));
            }
            url.setQuery(query);

            KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
            if (!job) {
                qCDebug(CHOQOK) << "Cannot create an http GET request!";
                continue;
            }
            job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
            m_timelinesRequests[job] = timeline;
            m_accountJobs[job] = acc;
            connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotUpdateTimeline);
            job->start();
        }
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

QString MastodonMicroBlog::authorizationMetaData(MastodonAccount *account) const
{
    return QStringLiteral("Authorization: Bearer ") + account->oAuth()->token();
}

Choqok::UI::PostWidget *MastodonMicroBlog::createPostWidget(Choqok::Account *account,
        Choqok::Post *post,
        QWidget *parent)
{
    return new MastodonPostWidget(account, post, parent);
}

void MastodonMicroBlog::fetchPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    MastodonAccount *acc = qobject_cast<MastodonAccount *>(theAccount);
    if (acc) {
        if (!post->link.toDisplayString().startsWith(acc->host())) {
            qCDebug(CHOQOK) << "You can only fetch posts from your host!";
            return;
        }
        QUrl url(post->link);

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(acc));
        m_accountJobs[job] = acc;
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotFetchPost);
        job->start();
    } else {
        qCDebug(CHOQOK) << "theAccount is not a MastodonAccount!";
    }
}

void MastodonMicroBlog::slotCreatePost(KJob *job)
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
            if (!reply[QLatin1String("id")].toString().isEmpty()) {
                Choqok::NotifyManager::success(i18n("New post for account %1 submitted successfully.",
                                                    theAccount->alias()));
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

void MastodonMicroBlog::slotFetchPost(KJob *job)
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
            MastodonPost *post = new MastodonPost;
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

void MastodonMicroBlog::slotRemovePost(KJob *job)
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
        KIO::TransferJob *j = qobject_cast<KIO::TransferJob * >(job);

        if (j->metaData().contains(QStringLiteral("responsecode"))) {
            int responseCode = j->queryMetaData(QStringLiteral("responsecode")).toInt();

            if (responseCode == 200 || responseCode == 404) {
                ret = 0;
                Q_EMIT postRemoved(theAccount, post);
            }
        }
    }

    if (ret) {
        Q_EMIT errorPost(theAccount, post, Choqok::MicroBlog::CommunicationError,
                         i18n("Removing the post failed. %1", job->errorString()),
                         MicroBlog::Critical);
    }
}

void MastodonMicroBlog::slotUpdateTimeline(KJob *job)
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

void MastodonMicroBlog::fetchFollowers(MastodonAccount* theAccount, bool active)
{
    qCDebug(CHOQOK);
    QUrl url(theAccount->host());
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QStringLiteral("/api/v1/accounts/%1/followers").arg(theAccount->id()));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("limit"), QLatin1String("80"));
    url.setQuery(urlQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(theAccount));
    mJobsAccount[job] = theAccount;
    if (active) {
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotRequestFollowersScreenNameActive);
    } else {
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotRequestFollowersScreenNamePassive);
    }
    job->start();
    Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Updating followers list for account %1...",
                                                             theAccount->alias()));
}

void MastodonMicroBlog::slotRequestFollowersScreenNameActive(KJob* job)
{
    finishRequestFollowersScreenName(job, true);
}

void MastodonMicroBlog::slotRequestFollowersScreenNamePassive(KJob* job)
{
    finishRequestFollowersScreenName(job, false);
}

void MastodonMicroBlog::finishRequestFollowersScreenName(KJob *job, bool active)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::MicroBlog::ErrorLevel level = active ? Critical : Low;
    MastodonAccount *account = qobject_cast<MastodonAccount *>(mJobsAccount.take(job));
    if (!account) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }

    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(account, ServerError, i18n("Followers list for account %1 could not be updated:\n%2",
            account->username(), job->errorString()), level);
        return;
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QByteArray buffer = j->data();
        const QJsonDocument json = QJsonDocument::fromJson(buffer);
        if (!json.isNull()) {
            QStringList followers;
            for (const QVariant &user: json.array().toVariantList()) {
                followers.append(user.toMap()[QLatin1String("acct")].toString());
            }

            account->setFollowers(followers);
        } else {
            QString err = i18n("Retrieving the followers list failed. The data returned from the server is corrupted.");
            qCDebug(CHOQOK) << "JSON parse error:the buffer is: \n" << buffer;
            Q_EMIT error(account, ParsingError, err, Critical);
        }
    }
}

void MastodonMicroBlog::fetchFollowing(MastodonAccount* theAccount, bool active)
{
    qCDebug(CHOQOK);
    QUrl url(theAccount->host());
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QStringLiteral("/api/v1/accounts/%1/following").arg(theAccount->id()));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("limit"), QLatin1String("80"));
    url.setQuery(urlQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData(QLatin1String("customHTTPHeader"), authorizationMetaData(theAccount));
    mJobsAccount[job] = theAccount;
    if (active) {
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotRequestFollowingScreenNameActive);
    } else {
        connect(job, &KIO::StoredTransferJob::result, this, &MastodonMicroBlog::slotRequestFollowingScreenNamePassive);
    }
    job->start();
    Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Updating following list for account %1...",
                                                             theAccount->alias()));
}

void MastodonMicroBlog::slotRequestFollowingScreenNameActive(KJob* job)
{
    finishRequestFollowingScreenName(job, true);
}

void MastodonMicroBlog::slotRequestFollowingScreenNamePassive(KJob* job)
{
    finishRequestFollowingScreenName(job, false);
}

void MastodonMicroBlog::finishRequestFollowingScreenName(KJob *job, bool active)
{
    qCDebug(CHOQOK);
    if (!job) {
        qCDebug(CHOQOK) << "Job is null pointer";
        return;
    }
    Choqok::MicroBlog::ErrorLevel level = active ? Critical : Low;
    MastodonAccount *account = qobject_cast<MastodonAccount *>(mJobsAccount.take(job));
    if (!account) {
        qCDebug(CHOQOK) << "Account or Post is NULL pointer";
        return;
    }

    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
        Q_EMIT error(account, ServerError, i18n("Following list for account %1 could not be updated:\n%2",
            account->username(), job->errorString()), level);
        return;
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);

        const QByteArray buffer = j->data();
        const QJsonDocument json = QJsonDocument::fromJson(buffer);
        if (!json.isNull()) {
            QStringList following;
            for (const QVariant &user: json.array().toVariantList()) {
                following.append(user.toMap()[QLatin1String("acct")].toString());
            }

            account->setFollowing(following);
        } else {
            QString err = i18n("Retrieving the following list failed. The data returned from the server is corrupted.");
            qCDebug(CHOQOK) << "JSON parse error:the buffer is: \n" << buffer;
            Q_EMIT error(account, ParsingError, err, Critical);
        }
    }
}

#include "mastodonmicroblog.moc"
