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

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < OCSMicroblog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_ocs" ) )

OCSMicroblog::OCSMicroblog( QObject* parent, const QVariantList&  )
    : MicroBlog(MyPluginFactory::componentData(), parent)
{

}

OCSMicroblog::~OCSMicroblog()
{

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
    return Choqok::MicroBlog::createNewAccount(alias);
}

ChoqokEditAccountWidget* OCSMicroblog::createEditAccountWidget(Choqok::Account* account, QWidget* parent)
{

}

void OCSMicroblog::createPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    Choqok::MicroBlog::createPost(theAccount, post);
}

void OCSMicroblog::abortCreatePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    Choqok::MicroBlog::abortCreatePost(theAccount, post);
}

void OCSMicroblog::fetchPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    Choqok::MicroBlog::fetchPost(theAccount, post);
}

void OCSMicroblog::removePost(Choqok::Account* theAccount, Choqok::Post* post)
{
    Choqok::MicroBlog::removePost(theAccount, post);
}


#include "ocsmicroblog.moc"
