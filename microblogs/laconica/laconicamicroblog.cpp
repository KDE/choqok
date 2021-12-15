/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
