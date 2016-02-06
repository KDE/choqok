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

#include "laconicamicroblog.h"

#include <KPluginFactory>

#include "gnusocialapiaccount.h"

#include "laconicadebug.h"
#include "laconicaeditaccount.h"

K_PLUGIN_FACTORY_WITH_JSON(LaconicaFactory, "choqok_laconica.json",
                           registerPlugin < LaconicaMicroBlog > ();)

LaconicaMicroBlog::LaconicaMicroBlog(QObject *parent, const QVariantList &)
    : GNUSocialApiMicroBlog(QLatin1String("choqok_laconica"), parent)
{
    qCDebug(CHOQOK);
    setServiceName(QLatin1String("GNU social"));
    mTimelineInfos[QLatin1String("ReTweets")]->name = i18nc("Timeline name", "Repeated");
    mTimelineInfos[QLatin1String("ReTweets")]->description = i18nc("Timeline description", "Your posts that were repeated by others");
}

LaconicaMicroBlog::~LaconicaMicroBlog()
{
    qCDebug(CHOQOK);
}

ChoqokEditAccountWidget *LaconicaMicroBlog::createEditAccountWidget(Choqok::Account *account, QWidget *parent)
{
    qCDebug(CHOQOK);
    GNUSocialApiAccount *acc = qobject_cast<GNUSocialApiAccount *>(account);
    if (acc || !account) {
        return new LaconicaEditAccountWidget(this, acc, parent);
    } else {
        qCDebug(CHOQOK) << "Account passed here is not a GNUSocialApiAccount!";
        return nullptr;
    }
}

#include "laconicamicroblog.moc"
