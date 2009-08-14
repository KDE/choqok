/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <KLocale>
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <KAboutData>
#include <KGenericFactory>
#include "account.h"
#include "accountmanager.h"
#include "microblogwidget.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "twittereditaccount.h"
#include "postwidget.h"
#include "twitteraccount.h"
#include <composerwidget.h>
#include <twitterapihelper/twitterapipostwidget.h>

typedef KGenericFactory<TwitterMicroBlog> TWPluginFactory;
static const KAboutData aboutdata("choqok_twitter", 0, ki18n("Twitter MicroBlog") , "0.1" );
K_EXPORT_COMPONENT_FACTORY( choqok_twitter, TWPluginFactory( &aboutdata )  )

TwitterMicroBlog::TwitterMicroBlog ( QObject* parent, const QStringList&  )
: TwitterApiMicroBlog(TWPluginFactory::componentData(), parent)
{
    kDebug();
    setServiceName("Twitter");
    setServiceHomepageUrl("http://twitter.com/");
    timelineApiPath["Reply"] = "/statuses/mentions.xml";
}

TwitterMicroBlog::~TwitterMicroBlog()
{
    kDebug();
}

Choqok::Account * TwitterMicroBlog::createNewAccount( const QString &alias )
{
    TwitterAccount *acc = qobject_cast<TwitterAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {
        return new TwitterAccount(this, alias);
    } else {
        return 0;
    }
}

ChoqokEditAccountWidget * TwitterMicroBlog::createEditAccountWidget( Choqok::Account *account, QWidget *parent )
{
    kDebug();
    TwitterAccount *acc = qobject_cast<TwitterAccount*>(account);
    if(acc || !account)
        return new TwitterEditAccountWidget(this, acc, parent);
    else{
        kError()<<"Account passed here is not a TwitterAccount!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget * TwitterMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    Choqok::UI::MicroBlogWidget *wd = new Choqok::UI::MicroBlogWidget(account, parent);
    if(!account->isReadOnly())
        wd->setComposerWidget(new Choqok::UI::ComposerWidget(account, wd));
    return wd;
}

Choqok::UI::TimelineWidget * TwitterMicroBlog::createTimelineWidget( Choqok::Account *account,
                                                                 const QString &timelineName, QWidget *parent )
{
    return new Choqok::UI::TimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget* TwitterMicroBlog::createPostWidget(Choqok::Account* account,
                                                        const Choqok::Post &post, QWidget* parent)
{
    return new TwitterApiPostWidget(account, post, parent);
}

QString TwitterMicroBlog::profileUrl(Choqok::Account* account, const QString& username) const
{
    Q_UNUSED(account)
    return QString( KUrl( homepageUrl() ).prettyUrl(KUrl::AddTrailingSlash) + username) ;
}

QString TwitterMicroBlog::postUrl(Choqok::Account* account, const QString& username,
                                  const QString& postId) const
{
    Q_UNUSED(account)
    return QString ( "http://twitter.com/%1/status/%2" ).arg ( username ).arg ( postId );
}


#include "twittermicroblog.moc"
