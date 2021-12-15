/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2016 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "friendicamicroblog.h"

#include <KPluginFactory>

#include "gnusocialapiaccount.h"

#include "friendicadebug.h"
#include "friendicaeditaccount.h"

K_PLUGIN_FACTORY_WITH_JSON(FriendicaFactory, "choqok_friendica.json",
                           registerPlugin < FriendicaMicroBlog > ();)

FriendicaMicroBlog::FriendicaMicroBlog(QObject *parent, const QVariantList &)
    : GNUSocialApiMicroBlog(QLatin1String("choqok_friendica"), parent)
{
    qCDebug(CHOQOK);
    setServiceName(QLatin1String("Friendica"));
}

FriendicaMicroBlog::~FriendicaMicroBlog()
{
    qCDebug(CHOQOK);
}

ChoqokEditAccountWidget *FriendicaMicroBlog::createEditAccountWidget(Choqok::Account *account, QWidget *parent)
{
    qCDebug(CHOQOK);
    GNUSocialApiAccount *acc = qobject_cast<GNUSocialApiAccount *>(account);
    if (acc || !account) {
        return new FriendicaEditAccountWidget(this, acc, parent);
    } else {
        qCDebug(CHOQOK) << "Account passed here is not a GNUSocialApiAccount!";
        return nullptr;
    }
}

QUrl FriendicaMicroBlog::profileUrl(Choqok::Account *account, const QString &username) const
{
    GNUSocialApiAccount *acc = qobject_cast<GNUSocialApiAccount *>(account);
    if (acc) {
        QUrl url(acc->host());
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(QLatin1String("/profile/") + username);

        return url;
    } else {
        return QUrl();
    }
}

#include "friendicamicroblog.moc"
