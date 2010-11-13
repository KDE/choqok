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
    : MicroBlog(MyPluginFactory::componentData(), parent), mProviderManager(new Attica::ProviderManager)
{
    setServiceName("Social Desktop Activities");
}

OCSMicroblog::~OCSMicroblog()
{
    delete mProviderManager;
}

void OCSMicroblog::saveTimeline(Choqok::Account* account, const QString& timelineName, const QList< Choqok::UI::PostWidget* >& timeline)
{
    Choqok::MicroBlog::saveTimeline(account, timelineName, timeline);
}

QList< Choqok::Post* > OCSMicroblog::loadTimeline(Choqok::Account* account, const QString& timelineName)
{
    return Choqok::MicroBlog::loadTimeline(account, timelineName);
}

Choqok::Account* OCSMicroblog::createNewAccount(const QString& alias)
{
    OCSAccount *acc = qobject_cast<OCSAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {//Why do we don't return the "acc"!? :/
        return new OCSAccount(this, alias);
    } else {
        return 0;
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
    kDebug();
    OCSAccount* acc = qobject_cast<OCSAccount*>(theAccount);
    acc->provider().postActivity(post->content);
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
    kDebug();
    OCSAccount* acc = qobject_cast<OCSAccount*>(theAccount);
    Attica::ListJob <Attica::Activity>* job = acc->provider().requestActivities();
    mJobsAccount.insert(job, acc);
    connect(job, SIGNAL(finished(Attica::BaseJob*)), SLOT(slotTimelineLoaded(Attica::BaseJob*)));
}

void OCSMicroblog::slotTimelineLoaded(Attica::BaseJob* job)
{
    kDebug();
    OCSAccount* acc = mJobsAccount.take(job);
    Attica::Activity::List actList = static_cast<Attica::ListJob<Attica::Activity>*>(job)->itemList();
    emit timelineDataReceived( acc, i18n("Activity"), parseActivityList(actList) );
}

QList< Choqok::Post* > OCSMicroblog::parseActivityList(const Attica::Activity::List& list)
{
    kDebug();
    QList< Choqok::Post* > resultList;
    Attica::Activity::List::const_iterator it = list.constBegin();
    Attica::Activity::List::const_iterator endIt = list.constEnd();
    Choqok::Post* pst;
    for(;it!=endIt; ++it){
        pst = new Choqok::Post;
        pst->content = it->message();
        pst->creationDateTime = it->timestamp();
        pst->link = it->link().toString();
        pst->isError = !it->isValid();
        pst->author.userName = it->associatedPerson().id();
        pst->author.homePageUrl = it->associatedPerson().homepage();
        pst->author.location = QString("%1(%2)").arg(it->associatedPerson().country()).arg(it->associatedPerson().city());
        pst->author.profileImageUrl = it->associatedPerson().avatarUrl().toString();
        pst->author.realName = QString("%1 %2").arg(it->associatedPerson().firstName()).arg(it->associatedPerson().lastName());
        resultList.append(pst);
    }
    return resultList;
}

#include "ocsmicroblog.moc"
