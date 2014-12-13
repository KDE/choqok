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

#include <QFile>
#include <QMenu>
#include <QTextDocument>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include <KAction>
#include <KDebug>
#include <KGenericFactory>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KMimeType>

#include "accountmanager.h"
#include "application.h"
#include "choqokbehaviorsettings.h"
#include "notifymanager.h"

#include "pumpioaccount.h"
#include "pumpiocomposerwidget.h"
#include "pumpioeditaccountwidget.h"
#include "pumpiomessagedialog.h"
#include "pumpiomicroblogwidget.h"
#include "pumpiopost.h"
#include "pumpiopostwidget.h"

class PumpIOMicroBlog::Private
{
public:
    Private():countOfTimelinesToSave(0)
    {}
    int countOfTimelinesToSave;
};

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < PumpIOMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_pumpio" ) )

const QString PumpIOMicroBlog::inboxActivity("/api/user/%1/inbox");
const QString PumpIOMicroBlog::outboxActivity("/api/user/%1/feed");

const QString PumpIOMicroBlog::PublicCollection("http://activityschema.org/collection/public");

PumpIOMicroBlog::PumpIOMicroBlog(QObject* parent, const QVariantList& args):
    MicroBlog(MyPluginFactory::componentData(), parent), d(new Private)
{
    Q_UNUSED(args)
    setServiceName("Pump.io");
    setServiceHomepageUrl("http://pump.io");
    QStringList timelineNames;
    timelineNames << "Activity" << "Favorites" << "Inbox" << "Outbox";
    setTimelineNames(timelineNames);
    setTimelinesInfo();
}

PumpIOMicroBlog::~PumpIOMicroBlog()
{
    delete d;
}

void PumpIOMicroBlog::abortAllJobs(Choqok::Account* theAccount)
{
    Q_FOREACH (KJob *job, m_accountJobs.keys(theAccount)) {
        job->kill(KJob::EmitResult);
    }
}

void PumpIOMicroBlog::abortCreatePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    if (m_createPostJobs.isEmpty()) {
        return;
    }
    if (post) {
        m_createPostJobs.key(post)->kill(KJob::EmitResult);
        return;
    }

    Q_FOREACH (KJob *job, m_createPostJobs.keys()){
        if( m_accountJobs[job] == theAccount) {
            job->kill(KJob::EmitResult);
        }
    }
}

void PumpIOMicroBlog::aboutToUnload()
{
    Q_FOREACH (Choqok::Account* acc, Choqok::AccountManager::self()->accounts()) {
        if (acc->microblog() == this){
            d->countOfTimelinesToSave += acc->timelineNames().count();
        }
    }
    emit saveTimelines();
}

QMenu* PumpIOMicroBlog::createActionsMenu(Choqok::Account* theAccount, QWidget* parent)
{
    QMenu *menu = MicroBlog::createActionsMenu(theAccount, parent);

    KAction *directMessge = new KAction(KIcon("mail-message-new"), i18n("Send Private Message..."), menu);
    directMessge->setData(theAccount->alias());
    connect(directMessge, SIGNAL(triggered(bool)), this, SLOT(showDirectMessageDialog()));
    menu->addAction(directMessge);

    return menu;
}

Choqok::UI::MicroBlogWidget* PumpIOMicroBlog::createMicroBlogWidget(Choqok::Account* account, QWidget* parent)
{
    return new PumpIOMicroBlogWidget(account, parent);
}

Choqok::UI::ComposerWidget* PumpIOMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new PumpIOComposerWidget(account, parent);
}

ChoqokEditAccountWidget* PumpIOMicroBlog::createEditAccountWidget(Choqok::Account* account,
                                                                  QWidget* parent)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount* >(account);
    if (acc || !account) {
        return new PumpIOEditAccountWidget(this, acc, parent);
    } else {
        kDebug() << "Account passed here was not a valid PumpIOAccount!";
        return 0;
    }
}

Choqok::Account* PumpIOMicroBlog::createNewAccount(const QString& alias)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(
        Choqok::AccountManager::self()->findAccount(alias) );
    if (!acc) {
        return new PumpIOAccount(this, alias);
    } else {
        kDebug() << "Cannot create a new PumpIOAccount!";
        return 0;
    }
}

void PumpIOMicroBlog::createPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    QVariantList to;
    QVariantMap thePublic;
    thePublic.insert("objectType", "collection");
    thePublic.insert("id", PumpIOMicroBlog::PublicCollection);
    to.append(thePublic);

    createPost(theAccount, post, to);
}

void PumpIOMicroBlog::createPost(Choqok::Account* theAccount, Choqok::Post* post,
                                 const QVariantList& to, const QVariantList& cc)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        QVariantMap object;
        if (!post->postId.isEmpty()) {
            object.insert("id", post->postId);
        }
        if (post->type.isEmpty()) {
            post->type = "note";
        }
        object.insert("objectType", post->type);
        //Convert URLs to href form
        post->content.replace(QRegExp("((?:https?|ftp)://\\S+)"), "<a href=\"\\1\">\\1</a>");
        object.insert("content", QUrl::toPercentEncoding(post->content));

        QVariantMap item;
        item.insert("verb", "post");
        item.insert("object", object);
        item.insert("to", to);
        item.insert("cc", cc);

        QJson::Serializer serializer;
        const QByteArray data = serializer.serialize(item);

        KUrl url(acc->host());
        url.addPath(outboxActivity.arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: application/json");
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_createPostJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::createReply(Choqok::Account* theAccount, PumpIOPost* post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        post->type = "comment";

        QVariantMap object;
        object.insert("objectType", post->type);
        //Convert URLs to href form
        post->content.replace(QRegExp("((?:https?|ftp)://\\S+)"), "<a href=\"\\1\">\\1</a>");
        object.insert("content", QUrl::toPercentEncoding(post->content));

        if (!post->replyToPostId.isEmpty()) {
            QVariantMap inReplyTo;
            inReplyTo.insert("id", post->replyToPostId);
            inReplyTo.insert("objectType", post->replyToObjectType);
            object.insert("inReplyTo", inReplyTo);
        }

        QVariantMap item;
        item.insert("verb", "post");
        item.insert("object", object);

        QJson::Serializer serializer;
        const QByteArray data = serializer.serialize(item);

        KUrl url(acc->host());
        url.addPath(outboxActivity.arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: application/json");
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_createPostJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::createPostWithMedia(Choqok::Account* theAccount, Choqok::Post* post,
                                          const QString& filePath)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        QFile media(filePath);
        QByteArray data;
        if (media.open(QIODevice::ReadOnly)) {
            data = media.readAll();
            media.close();
        } else {
            kDebug() << "Cannot read the file";
            return;
        }

        KMimeType::Ptr mimetype = KMimeType::findByNameAndContent(filePath, data);
        const QString mime = mimetype.data()->name();
        if (mime == "application/octet-stream") {
            kDebug() << "Cannot retrieve file mimetype";
            return;
        }

        KUrl url(acc->host());
        url.addPath(QString("/api/user/%1/uploads").arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: " + mime);
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_uploadJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotUpload(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

Choqok::UI::PostWidget* PumpIOMicroBlog::createPostWidget(Choqok::Account* account,
                                                          Choqok::Post* post,
                                                          QWidget* parent)
{
    return new PumpIOPostWidget(account, post, parent);
}

void PumpIOMicroBlog::fetchPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        if (!post->link.startsWith(acc->host())) {
            kDebug() << "You can only fetch posts from your host!";
            return;
        }
        KUrl url(post->link);

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            kDebug() << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::GET));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFetchPost(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::removePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert("id", post->postId);
        object.insert("objectType", post->type);

        QVariantMap item;
        item.insert("verb", "delete");
        item.insert("object", object);

        QJson::Serializer serializer;
        const QByteArray data = serializer.serialize(item);

        KUrl url(acc->host());
        url.addPath(outboxActivity.arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: application/json");
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_removePostJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRemovePost(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

QList< Choqok::Post* > PumpIOMicroBlog::loadTimeline(Choqok::Account* account,
                                                     const QString& timelineName)
{
    QList< Choqok::Post* > list;
    const QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(),
                                                                                timelineName);
    const KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );
    const QStringList tmpList = postsBackup.groupList();

    // don't load old archives
    if (tmpList.isEmpty() || !(QDateTime::fromString(tmpList.first()).isValid())) {
        return list;
    }

    QList<QDateTime> groupList;
    Q_FOREACH (const QString& str, tmpList) {
        groupList.append(QDateTime::fromString(str));
    }
    qSort(groupList);
    PumpIOPost *st;
    Q_FOREACH (const QDateTime& datetime, groupList) {
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

QString PumpIOMicroBlog::postUrl(Choqok::Account* account, const QString& username,
                                 const QString& postId) const
{
    Q_UNUSED(account);
    return QString(postId).replace("/api/", '/' + username + '/');
}

QString PumpIOMicroBlog::profileUrl(Choqok::Account* account, const QString& username) const
{
    Q_UNUSED(account)
    if (username.contains("acct:")) {
        return QString("https://%1/%2").arg(hostFromAcct(username)).arg(userNameFromAcct(username));
    } else {
        return username;
    }
}

void PumpIOMicroBlog::saveTimeline(Choqok::Account* account, const QString& timelineName,
                                   const QList< Choqok::UI::PostWidget* >& timeline)
{
    const QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(),
                                                                                timelineName);
    KConfig postsBackup("choqok/" + fileName, KConfig::NoGlobals, "data");

    ///Clear previous data:
    Q_FOREACH (const QString& group, postsBackup.groupList()) {
        postsBackup.deleteGroup(group);
    }

    QList< Choqok::UI::PostWidget *>::const_iterator it, endIt = timeline.constEnd();
    for (it = timeline.constBegin(); it != endIt; ++it) {
        PumpIOPost *post = dynamic_cast<PumpIOPost* >((*it)->currentPost());
        KConfigGroup grp(&postsBackup, post->creationDateTime.toString());
        grp.writeEntry("creationDateTime", post->creationDateTime);
        grp.writeEntry("postId", post->postId.toString());
        grp.writeEntry("link", post->link);
        grp.writeEntry("content", post->content);
        grp.writeEntry("source", post->source);
        grp.writeEntry("favorited", post->isFavorited);
        grp.writeEntry("authorId", post->author.userId.toString());
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
        grp.writeEntry("conversationId", post->conversationId.toString());
        grp.writeEntry("to", post->to);
        grp.writeEntry("cc", post->cc);
        grp.writeEntry("shares", post->shares);
        grp.writeEntry("replies", post->replies);
        grp.writeEntry("replyToPostId", post->replyToPostId.toString());
        grp.writeEntry("replyToUserName", post->replyToUserName);
        grp.writeEntry("replyToObjectType", post->replyToObjectType);
    }
    postsBackup.sync();

    if (Choqok::Application::isShuttingDown()) {
        --d->countOfTimelinesToSave;
        if (d->countOfTimelinesToSave < 1) {
            emit readyForUnload();
        }
    }
}

Choqok::TimelineInfo* PumpIOMicroBlog::timelineInfo(const QString& timelineName)
{
    return m_timelinesInfos.value(timelineName);
}

void PumpIOMicroBlog::updateTimelines(Choqok::Account* theAccount)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        Q_FOREACH (const QString& timeline, acc->timelineNames()) {
            KUrl url(acc->host());
            url.addPath(m_timelinesPaths[timeline].arg(acc->username()));

            QOAuth::ParamMap oAuthParams;
            const ChoqokId lastActivityId(lastTimelineId(theAccount, timeline));
            if (!lastActivityId.isEmpty()) {
                oAuthParams.insert("count", QByteArray::number(200));
                oAuthParams.insert("since", KUrl::toPercentEncoding(lastActivityId.toString()));
            } else {
                oAuthParams.insert("count", QByteArray::number(Choqok::BehaviorSettings::countOfPosts()));
            }

            KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
            if (!job) {
                kDebug() << "Cannot create an http GET request!";
                continue;
            }
            job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::GET,
                                                                       oAuthParams));
            m_timelinesRequests[job] = timeline;
            m_accountJobs[job] = acc;
            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotUpdateTimeline(KJob*)));
            job->start();
        }
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::fetchFollowing(Choqok::Account* theAccount)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        KUrl url(acc->host());
        url.addPath(QString("/api/user/%1/following").arg(acc->username()));

        QOAuth::ParamMap oAuthParams;
        oAuthParams.insert("count", QByteArray::number(200));
        if (!acc->following().isEmpty()) {
            oAuthParams.insert("since", KUrl::toPercentEncoding(acc->following().last()));
        }

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            kDebug() << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::GET,
                                                                   oAuthParams));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFollowing(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::fetchLists(Choqok::Account* theAccount)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        KUrl url(acc->host());
        url.addPath(QString("/api/user/%1/lists/person").arg(acc->username()));

        QOAuth::ParamMap oAuthParams;
        oAuthParams.insert("count", QByteArray::number(200));

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            kDebug() << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::GET,
                                                                   oAuthParams));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotLists(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}


void PumpIOMicroBlog::share(Choqok::Account* theAccount, Choqok::Post* post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert("objectType", post->type);
        object.insert("id", post->postId);

        QVariantMap item;
        item.insert("verb", "share");
        item.insert("object", object);

        QJson::Serializer serializer;
        const QByteArray data = serializer.serialize(item);

        KUrl url(acc->host());
        url.addPath(outboxActivity.arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: application/json");
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_shareJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotShare(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::toggleFavorite(Choqok::Account* theAccount, Choqok::Post* post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert("objectType", post->type);
        object.insert("id", post->postId);

        QVariantMap item;
        item.insert("verb", post->isFavorited ? "unfavorite" : "favorite");
        item.insert("object", object);

        QJson::Serializer serializer;
        const QByteArray data = serializer.serialize(item);

        KUrl url(acc->host());
        url.addPath(outboxActivity.arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: application/json");
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_favoriteJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFavorite(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

void PumpIOMicroBlog::showDirectMessageDialog()
{
    kDebug();
    const QString alias = qobject_cast<KAction *>(sender())->data().toString();
    PumpIOAccount *theAccount = qobject_cast<PumpIOAccount*>(Choqok::AccountManager::self()->findAccount(alias));
    PumpIOMessageDialog *msg = new PumpIOMessageDialog(theAccount, Choqok::UI::Global::mainWindow());
    msg->show();
}

void PumpIOMicroBlog::slotCreatePost(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_createPostJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap reply = parser.parse(j->data(), &ok).toMap();
        if (ok) {
            if (!reply["object"].toMap().value("id").toString().isEmpty()) {
                Choqok::NotifyManager::success(i18n("New post submitted successfully"));
                ret = 0;
                emit postCreated(theAccount, post);
            }
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        emit errorPost(theAccount, post, Choqok::MicroBlog::CommunicationError,
                       i18n("Creating the new post failed. %1", job->errorString()),
                       MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotFavorite(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_favoriteJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
        emit error(theAccount, Choqok::MicroBlog::CommunicationError,
                   i18n("Cannot set/unset the post as favorite. %1", job->errorString()));
    } else {
        post->isFavorited = !post->isFavorited;
        emit favorite(theAccount, post);
    }
}

void PumpIOMicroBlog::slotFetchPost(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        kDebug() << "Account or postId is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap reply = parser.parse(j->data(), &ok).toMap();
        if (ok) {
            PumpIOPost* post = new PumpIOPost;
            readPost(reply, post);
            ret = 0;
            emit postFetched(theAccount, post);
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        emit error(theAccount, Choqok::MicroBlog::CommunicationError,
                   i18n("Cannot fetch post. %1", job->errorString()),
                   MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotFetchReplies(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        kDebug() << "Account or postId is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap reply = parser.parse(j->data(), &ok).toMap();
        if (ok) {
            const QVariantList items = reply["items"].toList();
            for (int i = items.size() - 1; i >= 0; i--) {
                QVariantMap item = items.at(i).toMap();
                PumpIOPost* r = new PumpIOPost;
                readPost(item, r);
                r->replyToPostId = reply["url"].toString().remove("/replies");
                emit postFetched(theAccount, r);
            }
            ret = 0;
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        emit error(theAccount, Choqok::MicroBlog::CommunicationError,
                   i18n("Cannot fetch replies. %1", job->errorString()),
                   MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotFollowing(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        kDebug() << "Account is NULL pointer";
        return;
    }
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    }
    bool ret = 1;
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        Choqok::UI::Global::mainWindow()->showStatusMessage(
            i18n("Following list for account %1 has been updated.",
            acc->username()));
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantList items = parser.parse(j->data(), &ok).toMap().value("items").toList();
        if (ok) {
            QStringList following;
            Q_FOREACH (const QVariant& element, items) {
                following.append(element.toMap().value("id").toString());
            }
            acc->setFollowing(following);
            ret = 0;
            emit followingFetched(acc);
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }

    if (ret) {
        emit error(theAccount, Choqok::MicroBlog::CommunicationError,
                   i18n("Cannot retrieve the following list. %1", job->errorString()));
    }
}

void PumpIOMicroBlog::slotLists(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!theAccount) {
        kDebug() << "Account is NULL pointer";
        return;
    }
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    }
    bool ret = 1;
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        Choqok::UI::Global::mainWindow()->showStatusMessage(
            i18n("Lists for account %1 has been updated.",
            acc->username()));
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantList items = parser.parse(j->data(), &ok).toMap().value("items").toList();
        if (ok) {
            QVariantList lists;
            Q_FOREACH (const QVariant& element, items) {
                QVariantMap e = element.toMap();
                QVariantMap list;
                list.insert("id", e.value("id").toString());
                list.insert("name", e.value("displayName").toString());
                lists.append(list);
            }
            acc->setLists(lists);
            ret = 0;
            emit listsFetched(acc);
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }

    if (ret) {
        emit error(theAccount, Choqok::MicroBlog::CommunicationError,
                   i18n("Cannot retrieve the lists. %1", job->errorString()));
    }
}

void PumpIOMicroBlog::slotShare(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_shareJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        Choqok::UI::Global::mainWindow()->showStatusMessage(
                            i18n("The post has been shared."));
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap object = parser.parse(j->data(), &ok).toMap().value("object").toMap();
        if (ok) {
            ret = 0;
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        emit error(theAccount, Choqok::MicroBlog::CommunicationError,
                   i18n("Cannot share the post. %1", job->errorString()));
    }
}

void PumpIOMicroBlog::slotRemovePost(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_removePostJobs.take(job);
    Choqok::Account *theAccount = m_accountJobs.take(job);
    if (!post || !theAccount) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap object = parser.parse(j->data(), &ok).toMap().value("object").toMap();
        if (ok) {
            if (!object["deleted"].toString().isEmpty()) {
                Choqok::NotifyManager::success(i18n("Post removed successfully"));
                ret = 0;
                emit postRemoved(theAccount, post);
            }
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        emit errorPost(theAccount, post, Choqok::MicroBlog::CommunicationError,
                       i18n("Removing the post failed. %1", job->errorString()),
                       MicroBlog::Critical);
    }
}

void PumpIOMicroBlog::slotUpdatePost(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_updateJobs.take(job);
    Choqok::Account *account = m_accountJobs.take(job);
    if (!post || !account) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap reply = parser.parse(j->data(), &ok).toMap();
        if (ok) {
            ret = 0;
            createPost(account, post);
        } else {
            kDebug() << "Cannot parse JSON reply";
        }
    }

    if (ret) {
        emit error(account, Choqok::MicroBlog::CommunicationError,
                   i18n("An error occurred when updating the post"));
    }
}

void PumpIOMicroBlog::slotUpdateTimeline(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Account *account = m_accountJobs.take(job);
    if (!account) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
        emit error(account, Choqok::MicroBlog::CommunicationError,
                   i18n("An error occurred when fetching the timeline"));
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        const QList<Choqok::Post* > list = readTimeline(j->data());
        const QString timeline(m_timelinesRequests.take(job));
        if (!list.isEmpty()) {
            setLastTimelineId(account, timeline, list.last()->conversationId);
        }

        emit timelineDataReceived(account, timeline, list);
    }
}

void PumpIOMicroBlog::slotUpload(KJob* job)
{
    kDebug();
    if (!job) {
        kDebug() << "Job is null pointer";
        return;
    }
    Choqok::Post *post = m_uploadJobs.take(job);
    Choqok::Account *account = m_accountJobs.take(job);
    if (!post || !account) {
        kDebug() << "Account or Post is NULL pointer";
        return;
    }
    int ret = 1;
    if (job->error()) {
        kDebug() << "Job Error: " << job->errorString();
    } else {
        KIO::StoredTransferJob* j = qobject_cast<KIO::StoredTransferJob* >(job);
        bool ok;
        QJson::Parser parser;
        const QVariantMap reply = parser.parse(j->data(), &ok).toMap();
        if (ok) {
            const QString id = reply["id"].toString();
            if (!id.isEmpty()) {
                post->postId = id;
                post->type = reply["objectType"].toString();
                ret = 0;
                updatePost(account, post);
            }
        } else {
            kDebug() << "Cannot parse JSON reply";
        }

    }

    if (ret) {
        emit error(account, Choqok::MicroBlog::CommunicationError,
                   i18n("An error occurred when uploading the media"));
    }
}

QString PumpIOMicroBlog::authorizationMetaData(PumpIOAccount* account, const KUrl& url,
                                               const QOAuth::HttpMethod& method,
                                               const QOAuth::ParamMap& paramMap) const
{
    const QByteArray authorization = account->oAuth()->createParametersString(url.url(),
                                                       method, account->token().toLocal8Bit(),
                                                       account->tokenSecret().toLocal8Bit(),
                                                       QOAuth::HMAC_SHA1, paramMap,
                                                       QOAuth::ParseForHeaderArguments);
    return "Authorization: " + authorization;
}

void PumpIOMicroBlog::fetchReplies(Choqok::Account* theAccount, const QString& url)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        if (!url.startsWith(acc->host())) {
            kDebug() << "You can only fetch replies from your host!";
            return;
        }
        KUrl u(url);

        KIO::StoredTransferJob *job = KIO::storedGet(u, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            kDebug() << "Cannot create an http GET request!";
            return;
        }
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, u, QOAuth::GET));
        m_accountJobs[job] = acc;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotFetchReplies(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

ChoqokId PumpIOMicroBlog::lastTimelineId(Choqok::Account* theAccount,
                                         const QString& timeline) const
{
    kDebug() << "Latest ID for timeline " << timeline << m_timelinesLatestIds[theAccount][timeline];
    return m_timelinesLatestIds[theAccount][timeline];
}

Choqok::Post* PumpIOMicroBlog::readPost(const QVariantMap& var, Choqok::Post* post)
{
    PumpIOPost *p = dynamic_cast< PumpIOPost* >(post);
    if (p) {
        QVariantMap object;
        if (var.value("verb").toString() == "post" ||
            var.value("verb").toString() == "share") {
            object = var["object"].toMap();
        } else {
            object = var;
        }

        QTextDocument content;
        if (!object["displayName"].isNull()) {
            content.setHtml(object["displayName"].toString());
            p->content = content.toPlainText().trimmed();
            p->content += '\n';
        }

        content.setHtml(object["content"].toString());
        p->content += content.toPlainText().trimmed();

        if (!object["fullImage"].isNull()) {
            const QVariantMap fullImage = object["fullImage"].toMap();
            if (!fullImage.isEmpty()) {
                p->media = fullImage["url"].toString();
                p->mediaSizeHeight = fullImage["height"].toInt();
                p->mediaSizeWidth = fullImage["width"].toInt();
            }
        }
        p->creationDateTime = QDateTime::fromString(var["published"].toString(),
                                                   Qt::ISODate);
        p->creationDateTime.setTimeSpec(Qt::UTC);
        if (object["pump_io"].isNull()) {
            p->link = object["id"].toString();
        } else {
            p->link = object["pump_io"].toMap().value("proxyURL").toString();
        }
        p->type = object["objectType"].toString();
        p->isFavorited = object["liked"].toBool();
        if (p->isFavorited) {
            p->isRead = true;
        }
        p->postId = object["id"].toString();
        p->conversationId = var["id"].toString();

        QString author;
        var["author"].isNull() ? author = "actor" : author = "author";
        QVariantMap actor;
        if (var.value("verb").toString() == "share") {
            actor = object["author"].toMap();
            const QVariantList shares = object["shares"].toMap().value("items").toList();
            Q_FOREACH (const QVariant& element, shares) {
                p->shares.append(element.toMap().value("id").toString());
            }
        } else {
            actor = var[author].toMap();
        }
        const QString userId = actor["id"].toString();
        const QString homePageUrl = actor["url"].toString();
        p->author.userId = userId;
        p->author.userName = actor["preferredUsername"].toString();
        p->author.realName = actor["displayName"].toString();
        p->author.homePageUrl = homePageUrl;
        p->author.location = actor["location"].toMap().value("displayName").toString();
        p->author.description = actor["summary"].toString();

        const QString profileImageUrl = actor["image"].toMap().value("url").toString();
        if (!profileImageUrl.isEmpty()) {
            p->author.profileImageUrl = profileImageUrl;
        } else if (actor["objectType"].toString() == "service") {
            p->author.profileImageUrl = homePageUrl + "images/default.png";
        } else {
            p->author.profileImageUrl = QString("https://%1/%2").arg(hostFromAcct(userId)).arg("images/default.png");
        }

        if (!var["generator"].isNull()) {
            p->source = var["generator"].toMap().value("displayName").toString();
        }

        const QVariantList to = var["to"].toList();
        Q_FOREACH (const QVariant& element, to) {
            QVariantMap toElementMap = element.toMap();
            QString toElementType = toElementMap.value("objectType").toString();
            if (toElementType == "person" || toElementType == "collection") {
                p->cc.append(toElementMap.value("id").toString());
            }
        }

        const QVariantList cc = var["cc"].toList();
        Q_FOREACH (const QVariant& element, cc) {
            QVariantMap ccElementMap = element.toMap();
            QString ccElementType = ccElementMap.value("objectType").toString();
            if (ccElementType == "person" || ccElementType == "collection") {
                p->to.append(ccElementMap.value("id").toString());
            }
        }

        const QVariantMap replies = object["replies"].toMap();
        if (replies.value("pump_io").isNull()) {
            p->replies = replies["url"].toString();
        } else {
            p->replies = replies["pump_io"].toMap().value("proxyURL").toString();
        }

        return p;
    } else {
        kDebug() << "post is not a PumpIOPost!";
        return post;
    }
}

QList< Choqok::Post* > PumpIOMicroBlog::readTimeline(const QByteArray& buffer)
{
    QList<Choqok::Post* > posts;
    bool ok;
    QJson::Parser parser;
    const QVariantList list = parser.parse(buffer, &ok).toMap().value("items").toList();
    if (ok) {
        Q_FOREACH (const QVariant& element, list) {
            const QVariantMap elementMap = element.toMap();
            if (!elementMap["object"].toMap().value("deleted").isNull()) {
                // Skip deleted posts
                continue;
            }
            posts.prepend(readPost(elementMap, new PumpIOPost));
        }
    } else {
        kDebug() << "Cannot parse JSON reply";
    }

    return posts;
}

void PumpIOMicroBlog::setLastTimelineId(Choqok::Account* theAccount,
                                        const QString& timeline,
                                        const ChoqokId& id)
{
    m_timelinesLatestIds[theAccount][timeline] = id;
}

void PumpIOMicroBlog::setTimelinesInfo()
{
    Choqok::TimelineInfo *t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Activity");
    t->description = i18nc("Timeline description", "You and people you follow");
    t->icon = "user-home";
    m_timelinesInfos["Activity"] = t;
    m_timelinesPaths["Activity"] = inboxActivity + "/major";

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Favorites");
    t->description = i18nc("Timeline description", "Posts you favorited");
    t->icon = "favorites";
    m_timelinesInfos["Favorites"] = t;
    m_timelinesPaths["Favorites"] = "/api/user/%1/favorites";

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Inbox");
    t->description = i18nc("Timeline description", "Posts sent to you");
    t->icon = "mail-folder-inbox";
    m_timelinesInfos["Inbox"] = t;
    m_timelinesPaths["Inbox"] = inboxActivity + "/direct/major/";

    t = new Choqok::TimelineInfo;
    t->name = i18nc("Timeline Name", "Outbox");
    t->description = i18nc("Timeline description", "Posts you sent");
    t->icon = "mail-folder-outbox";
    m_timelinesInfos["Outbox"] = t;
    m_timelinesPaths["Outbox"] = outboxActivity + "/major/";
}

void PumpIOMicroBlog::updatePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    PumpIOAccount *acc = qobject_cast<PumpIOAccount*>(theAccount);
    if (acc) {
        QVariantMap object;
        object.insert("id", post->postId);
        object.insert("objectType", post->type);
        object.insert("content", QUrl::toPercentEncoding(post->content));

        // https://github.com/e14n/pump.io/issues/885
        QVariantList to;
        QVariantMap thePublic;
        thePublic.insert("objectType", "collection");
        thePublic.insert("id", PumpIOMicroBlog::PublicCollection);
        to.append(thePublic);

        QVariantMap item;
        item.insert("verb", "update");
        item.insert("object", object);
        item.insert("to", to);

        QJson::Serializer serializer;
        const QByteArray data = serializer.serialize(item);

        KUrl url(acc->host());
        url.addPath(outboxActivity.arg(acc->username()));
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        job->addMetaData("content-type", "Content-Type: application/json");
        job->addMetaData("customHTTPHeader", authorizationMetaData(acc, url, QOAuth::POST));
        if (!job) {
            kDebug() << "Cannot create an http POST request!";
            return;
        }
        m_accountJobs[job] = acc;
        m_updateJobs[job] = post;
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotUpdatePost(KJob*)));
        job->start();
    } else {
        kDebug() << "theAccount is not a PumpIOAccount!";
    }
}

QString PumpIOMicroBlog::hostFromAcct(const QString& acct)
{
    if (acct.contains("acct:")) {
        return acct.split(':')[1].split('@')[1];
    }

    return acct;
}

QString PumpIOMicroBlog::userNameFromAcct(const QString& acct)
{
    if (acct.contains("acct:")) {
        return acct.split(':')[1].split('@')[0];
    }

    return acct;
}
