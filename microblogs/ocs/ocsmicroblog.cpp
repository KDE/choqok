/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "ocsmicroblog.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KConfig>

#include <Attica/ProviderManager>

#include "application.h"
#include "accountmanager.h"
#include "editaccountwidget.h"
#include "postwidget.h"

#include "ocsaccount.h"
#include "ocsdebug.h"
#include "ocsconfigurewidget.h"

K_PLUGIN_FACTORY_WITH_JSON(OCSMicroblogFactory, "choqok_ocs.json",
                           registerPlugin < OCSMicroblog > ();)

OCSMicroblog::OCSMicroblog(QObject *parent, const QVariantList &)
    : MicroBlog(QLatin1String("choqok_ocs"), parent)
    , mProviderManager(new Attica::ProviderManager)
    , mIsOperational(false)
{
    connect(mProviderManager, &Attica::ProviderManager::defaultProvidersLoaded,
            this, &OCSMicroblog::slotDefaultProvidersLoaded);
    mProviderManager->loadDefaultProviders();
    setServiceName(QLatin1String("Social Desktop Activities"));
}

OCSMicroblog::~OCSMicroblog()
{
    delete mProviderManager;
}

void OCSMicroblog::saveTimeline(Choqok::Account *account, const QString &timelineName,
                                const QList< Choqok::UI::PostWidget * > &timeline)
{
    qCDebug(CHOQOK);
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);

    ///Clear previous data:
    for (const QString &group: postsBackup.groupList()) {
        postsBackup.deleteGroup(group);
    }

    for (Choqok::UI::PostWidget *wd: timeline) {
        const Choqok::Post *post = wd->currentPost();
        KConfigGroup grp(&postsBackup, post->creationDateTime.toString());
        grp.writeEntry("creationDateTime", post->creationDateTime);
        grp.writeEntry("postId", post->postId);
        grp.writeEntry("text", post->content);
        grp.writeEntry("authorId", post->author.userId);
        grp.writeEntry("authorUserName", post->author.userName);
        grp.writeEntry("authorRealName", post->author.realName);
        grp.writeEntry("authorProfileImageUrl", post->author.profileImageUrl);
        grp.writeEntry("authorDescription" , post->author.description);
        grp.writeEntry("authorLocation" , post->author.location);
        grp.writeEntry("authorUrl" , post->author.homePageUrl);
        grp.writeEntry("link", post->link);
        grp.writeEntry("isRead" , post->isRead);
    }
    postsBackup.sync();
    if (Choqok::Application::isShuttingDown()) {
        Q_EMIT readyForUnload();
    }
}

QList< Choqok::Post * > OCSMicroblog::loadTimeline(Choqok::Account *account, const QString &timelineName)
{
    qCDebug(CHOQOK) << timelineName;
    QList< Choqok::Post * > list;
    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup(fileName, KConfig::NoGlobals, QStandardPaths::DataLocation);
    QStringList tmpList = postsBackup.groupList();

    QList<QDateTime> groupList;
    for (const QString &str: tmpList) {
        groupList.append(QDateTime::fromString(str));
    }
    qSort(groupList);
    int count = groupList.count();
    if (count) {
        Choqok::Post *st = nullptr;
        for (int i = 0; i < count; ++i) {
            st = new Choqok::Post;
            KConfigGroup grp(&postsBackup, groupList[i].toString());
            st->creationDateTime = grp.readEntry("creationDateTime", QDateTime::currentDateTime());
            st->postId = grp.readEntry("postId", QString());
            st->content = grp.readEntry("text", QString());
            st->author.userId = grp.readEntry("authorId", QString());
            st->author.userName = grp.readEntry("authorUserName", QString());
            st->author.realName = grp.readEntry("authorRealName", QString());
            st->author.profileImageUrl = grp.readEntry("authorProfileImageUrl", QUrl());
            st->author.description = grp.readEntry("authorDescription" , QString());
            st->author.location = grp.readEntry("authorLocation", QString());
            st->author.homePageUrl = grp.readEntry("authorUrl", QUrl());
            st->link = grp.readEntry("link", QUrl());
            st->isRead = grp.readEntry("isRead", true);

            list.append(st);
        }
    }
    return list;
}

Choqok::Account *OCSMicroblog::createNewAccount(const QString &alias)
{
    OCSAccount *acc = qobject_cast<OCSAccount *>(Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return new OCSAccount(this, alias);
    } else {
        return nullptr;//If there's an account with this alias, So We can't create a new one
    }
}

ChoqokEditAccountWidget *OCSMicroblog::createEditAccountWidget(Choqok::Account *account, QWidget *parent)
{
    qCDebug(CHOQOK);
    OCSAccount *acc = qobject_cast<OCSAccount *>(account);
    if (acc || !account) {
        return new OCSConfigureWidget(this, acc, parent);
    } else {
        qCDebug(CHOQOK) << "Account passed here was not a valid OCSAccount!";
        return nullptr;
    }
}

void OCSMicroblog::createPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    if (!mIsOperational) {
        Q_EMIT errorPost(theAccount, post, OtherError, i18n("OCS plugin is not initialized yet. Try again later."));
        return;
    }
    qCDebug(CHOQOK);
    OCSAccount *acc = qobject_cast<OCSAccount *>(theAccount);
    Attica::PostJob *job = acc->provider().postActivity(post->content);
    mJobsAccount.insert(job, acc);
    mJobsPost.insert(job, post);
    connect(job, &Attica::PostJob::finished, this,
            &OCSMicroblog::slotCreatePost);
    job->start();
}

void OCSMicroblog::slotCreatePost(Attica::BaseJob *job)
{
    OCSAccount *acc = mJobsAccount.take(job);
    Choqok::Post *post = mJobsPost.take(job);
    Q_EMIT postCreated(acc, post);
}

void OCSMicroblog::abortCreatePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    Q_UNUSED(post);
    OCSAccount *acc = qobject_cast<OCSAccount *>(theAccount);
    Attica::BaseJob *job = mJobsAccount.key(acc);
    if (job) {
        job->abort();
    }
}

void OCSMicroblog::fetchPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    Q_UNUSED(theAccount);
    Q_UNUSED(post);
    KMessageBox::sorry(choqokMainWindow, i18n("Not Supported"));
}

void OCSMicroblog::removePost(Choqok::Account *theAccount, Choqok::Post *post)
{
    Q_UNUSED(theAccount);
    Q_UNUSED(post);
    KMessageBox::sorry(choqokMainWindow, i18n("Not Supported"));
}

Attica::ProviderManager *OCSMicroblog::providerManager()
{
    return mProviderManager;
}

void OCSMicroblog::updateTimelines(Choqok::Account *theAccount)
{
    if (!mIsOperational) {
        scheduledTasks.insertMulti(theAccount, Update);
        return;
    }
    qCDebug(CHOQOK);
    OCSAccount *acc = qobject_cast<OCSAccount *>(theAccount);
    if (!acc) {
        qCCritical(CHOQOK) << "OCSMicroblog::updateTimelines: acc is not an OCSAccount";
        return;
    }
    Attica::ListJob <Attica::Activity> *job = acc->provider().requestActivities();
    mJobsAccount.insert(job, acc);
    connect(job, &Attica::BaseJob::finished, this, &OCSMicroblog::slotTimelineLoaded);
    job->start();
}

void OCSMicroblog::slotTimelineLoaded(Attica::BaseJob *job)
{
    qCDebug(CHOQOK);
    OCSAccount *acc = mJobsAccount.take(job);
    if (job->metadata().error() == Attica::Metadata::NoError) {
        Attica::Activity::List actList = static_cast< Attica::ListJob<Attica::Activity> * >(job)->itemList();
        Q_EMIT timelineDataReceived(acc, QLatin1String("Activity"), parseActivityList(actList));
    } else {
        Q_EMIT error(acc, ServerError, job->metadata().message(), Low);
    }
}

QList< Choqok::Post * > OCSMicroblog::parseActivityList(const Attica::Activity::List &list)
{
    qCDebug(CHOQOK) << list.count();
    QList< Choqok::Post * > resultList;
    for (const Attica::Activity &act: list) {
        Choqok::Post *pst = new Choqok::Post;
        pst->postId = act.id();
        pst->content = act.message();
        pst->creationDateTime = act.timestamp();
        pst->link = act.link();
        pst->isError = !act.isValid();
        pst->author.userId = act.associatedPerson().id();
        pst->author.userName = act.associatedPerson().id();
        pst->author.homePageUrl = QUrl::fromUserInput(act.associatedPerson().homepage());
        pst->author.location = QStringLiteral("%1(%2)").arg(act.associatedPerson().country())
                               .arg(act.associatedPerson().city());
        pst->author.profileImageUrl = act.associatedPerson().avatarUrl();
        pst->author.realName = QStringLiteral("%1 %2").arg(act.associatedPerson().firstName())
                               .arg(act.associatedPerson().lastName());
        resultList.insert(0, pst);
    }
    return resultList;
}

Choqok::TimelineInfo *OCSMicroblog::timelineInfo(const QString &timelineName)
{
    if (timelineName == QLatin1String("Activity")) {
        Choqok::TimelineInfo *info = new Choqok::TimelineInfo;
        info->name = i18nc("Timeline Name", "Activity");
        info->description = i18n("Social activities");
        info->icon = QLatin1String("user-home");
        return info;
    } else {
        qCCritical(CHOQOK) << "timelineName is not valid!";
        return nullptr;
    }
}

bool OCSMicroblog::isOperational()
{
    return mIsOperational;
}

void OCSMicroblog::slotDefaultProvidersLoaded()
{
    qCDebug(CHOQOK);
    mIsOperational = true;
    Q_EMIT initialized();

    for (Choqok::Account *acc: scheduledTasks.keys()) {
        switch (scheduledTasks.value(acc)) {
        case Update:
            updateTimelines(acc);
            break;
        default:
            break;
        };
    }
}

QUrl OCSMicroblog::profileUrl(Choqok::Account *account, const QString &username) const
{
    OCSAccount *acc = qobject_cast<OCSAccount *>(account);
    if (acc->providerUrl().host().contains(QLatin1String("opendesktop.org"))) {
        return QUrl::fromUserInput(QStringLiteral("https://opendesktop.org/usermanager/search.php?username=%1").arg(username));
    }
    return QUrl();
}

void OCSMicroblog::aboutToUnload()
{
    Q_EMIT saveTimelines();
}

#include "ocsmicroblog.moc"
