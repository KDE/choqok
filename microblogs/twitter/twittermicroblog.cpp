/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "twittereditaccount.h"
#include "postwidget.h"
#include "twitteraccount.h"
#include <composerwidget.h>
#include "twitterpostwidget.h"
#include "twitterapihelper/twitterapimicroblogwidget.h"
#include "twittersearch.h"
#include <twitterapihelper/twitterapicomposerwidget.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < TwitterMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_twitter" ) )

TwitterMicroBlog::TwitterMicroBlog ( QObject* parent, const QVariantList&  )
: TwitterApiMicroBlog(MyPluginFactory::componentData(), parent)
{
    kDebug();
    setServiceName("Twitter");
    setServiceHomepageUrl("https://twitter.com/");
    timelineApiPath["Reply"] = "/statuses/mentions.%1";
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
        kDebug()<<"Account passed here is not a TwitterAccount!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget * TwitterMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    return new TwitterApiMicroBlogWidget(account, parent);
}

Choqok::UI::TimelineWidget * TwitterMicroBlog::createTimelineWidget( Choqok::Account *account,
                                                                 const QString &timelineName, QWidget *parent )
{
    return new Choqok::UI::TimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget* TwitterMicroBlog::createPostWidget(Choqok::Account* account,
                                                        const Choqok::Post &post, QWidget* parent)
{
    return new TwitterPostWidget(account, post, parent);
}

Choqok::UI::ComposerWidget* TwitterMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new TwitterApiComposerWidget(account, parent);
}

QString TwitterMicroBlog::profileUrl(Choqok::Account* account, const QString& username) const
{
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount*>(account);
    if(acc){
        return QString( acc->homepageUrl().prettyUrl(KUrl::AddTrailingSlash) + username) ;
    } else {
        return QString( "http://twitter.com/%1" ).arg( username );
    }
}

QString TwitterMicroBlog::postUrl(Choqok::Account* account, const QString& username,
                                  const QString& postId) const
{
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount*>(account);
    if(acc){
        KUrl url( acc->homepageUrl() );
        url.addPath ( QString("/%1/status/%2" ).arg ( username ).arg ( postId ) );
        return url.prettyUrl();
    } else
        return QString ( "http://twitter.com/%1/status/%2" ).arg ( username ).arg ( postId );
}

TwitterApiSearch* TwitterMicroBlog::searchBackend()
{
    if(!mSearchBackend)
        mSearchBackend = new TwitterSearch(this);
    return mSearchBackend;
}

QString TwitterMicroBlog::generateRepeatedByUserTooltip(const QString& username)
{
    return i18n("Retweeted by %1", username);
}


#include "twittermicroblog.moc"
