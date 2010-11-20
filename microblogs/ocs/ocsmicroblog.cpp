/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

This program is free software;
you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation;
either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY;
without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program;
if not, see http://www.gnu.org/licenses/
*/

#include "ocsmicroblog.h"
#include <KAboutData>
#include <KGenericFactory>
#include <attica/providermanager.h>
#include <accountmanager.h>
#include "ocsaccount.h"
#include <editaccountwidget.h>
#include "ocsconfigurewidget.h"
#include <KMessageBox>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < OCSMicroblog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_ocs" ) )

OCSMicroblog::OCSMicroblog( QObject* parent, const QVariantList&  )
    : MicroBlog(MyPluginFactory::componentData(), parent), mProviderManager(new Attica::ProviderManager),
    mIsOperational(false)
{
    connect( mProviderManager, SIGNAL(defaultProvidersLoaded()),
             this, SLOT(slotDefaultProvidersLoaded()) );
    mProviderManager->loadDefaultProviders();
    setServiceName("Social Desktop Activities");
}

OCSMicroblog::~OCSMicroblog()
{
    delete mProviderManager;
}

void OCSMicroblog::saveTimeline(Choqok::Account* account, const QString& timelineName,
                                const QList< Choqok::UI::PostWidget* >& timeline)
{
    //TODO implement
}

QList< Choqok::Post* > OCSMicroblog::loadTimeline(Choqok::Account* account, const QString& timelineName)
{
    //TODO implement
    return QList< Choqok::Post* >();
}

Choqok::Account* OCSMicroblog::createNewAccount(const QString& alias)
{
    OCSAccount *acc = qobject_cast<OCSAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {
        return new OCSAccount(this, alias);
    } else {
        return 0;//If there's an account with this alias, So We can't create a new one
    }
}

ChoqokEditAccountWidget* OCSMicroblog::createEditAccountWidget(Choqok::Account* account, QWidget* parent)
{
    kDebug();
    OCSAccount *acc = qobject_cast<OCSAccount*>(account);
    if(acc || !account)
        return new OCSConfigureWidget(this, acc, parent);
    else{
        kDebug()<<"Account passed here was not a valid OCSAccount!";
        return 0L;
    }
}

void OCSMicroblog::createPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    if(!mIsOperational){
        emit errorPost(theAccount, post, OtherError, i18n("OCS plugin is not initialized yet, Try again later."));
        return;
    }
    kDebug();
    OCSAccount* acc = qobject_cast<OCSAccount*>(theAccount);
    Attica::PostJob* job = acc->provider().postActivity(post->content);
    mJobsAccount.insert(job, acc);
    mJobsPost.insert(job, post);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotCreatePost(Attica::BaseJob*)));
    job->start();
}

void OCSMicroblog::slotCreatePost(Attica::BaseJob* job)
{
    OCSAccount* acc = mJobsAccount.take(job);
    Choqok::Post* post = mJobsPost.take(job);
    emit postCreated ( acc, post );
}

void OCSMicroblog::abortCreatePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    OCSAccount* acc = qobject_cast<OCSAccount*>(theAccount);
    Attica::BaseJob* job = mJobsAccount.key(acc);
    if(job)
        job->abort();
}

void OCSMicroblog::fetchPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    Q_UNUSED(theAccount);
    Q_UNUSED(post);
    KMessageBox::sorry(choqokMainWindow, i18n("Not Supported"));
}

void OCSMicroblog::removePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    Q_UNUSED(theAccount);
    Q_UNUSED(post);
    KMessageBox::sorry(choqokMainWindow, i18n("Not Supported"));
}

Attica::ProviderManager* OCSMicroblog::providerManager()
{
    return mProviderManager;
}

void OCSMicroblog::updateTimelines(Choqok::Account* theAccount)
{
    if(!mIsOperational)
        return;//TODO schedule job for later
    kDebug();
    OCSAccount* acc = qobject_cast<OCSAccount*>(theAccount);
    if(!acc){
        kError()<<"OCSMicroblog::updateTimelines: acc is not an OCSAccount";
        return;
    }
    Attica::ListJob <Attica::Activity>* job = acc->provider().requestActivities();
    mJobsAccount.insert(job, acc);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotTimelineLoaded(Attica::BaseJob*)));
    job->start();
}

void OCSMicroblog::slotTimelineLoaded(Attica::BaseJob* job)
{
    kDebug();
    OCSAccount* acc = mJobsAccount.take(job);
    if(job->metadata().error() == Attica::Metadata::NoError) {
        Attica::Activity::List actList = static_cast< Attica::ListJob<Attica::Activity> * >(job)->itemList();
        emit timelineDataReceived( acc, "Activity", parseActivityList(actList) );
    } else {
        //TODO
    }
}

QList< Choqok::Post* > OCSMicroblog::parseActivityList(const Attica::Activity::List& list)
{
    kDebug()<<list.count();
    QList< Choqok::Post* > resultList;
    foreach(Attica::Activity act, list){
        kDebug();
        Choqok::Post* pst = new Choqok::Post;
        pst->postId = act.id();
        pst->content = act.message();
        pst->creationDateTime = act.timestamp();
        pst->link = act.link().toString();
        pst->isError = !act.isValid();
        pst->author.userName = act.associatedPerson().id();
        pst->author.homePageUrl = act.associatedPerson().homepage();
        pst->author.location = QString("%1(%2)").arg(act.associatedPerson().country())
                                                .arg(act.associatedPerson().city());
        pst->author.profileImageUrl = act.associatedPerson().avatarUrl().toString();
        pst->author.realName = QString("%1 %2").arg(act.associatedPerson().firstName())
                                               .arg(act.associatedPerson().lastName());
        resultList.insert(0, pst);
    }
    kDebug()<<resultList.count();
    return resultList;
}

Choqok::TimelineInfo* OCSMicroblog::timelineInfo(const QString& timelineName)
{
    if(timelineName == "Activity") {
        Choqok::TimelineInfo* info = new Choqok::TimelineInfo;
        info->name = i18nc("Timeline Name", "Activity");
        info->description = i18n("Social activities");
        info->icon = "user-home";
        return info;
    } else {
        kError()<<"timelineName is not valid!";
        return 0;
    }
}

bool OCSMicroblog::isOperational()
{
    return mIsOperational;
}

void OCSMicroblog::slotDefaultProvidersLoaded()
{
    kDebug();
    mIsOperational = true;
    emit initialized();
}

QString OCSMicroblog::profileUrl(Choqok::Account* account, const QString& username) const
{
    OCSAccount* acc = qobject_cast<OCSAccount*>(account);
    if(acc->providerUrl().host().contains("opendesktop.org")){
        return QString("http://opendesktop.org/usermanager/search.php?username=%1").arg(username);
    }
    return QString();
}

#include "ocsmicroblog.moc"
