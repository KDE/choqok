/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2016 Andrea Scarpino <scarpino@kde.org>

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
