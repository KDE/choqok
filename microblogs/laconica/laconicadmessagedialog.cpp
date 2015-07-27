/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2015 Andrea Scarpino <scarpino@kde.org>

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

#include "laconicadmessagedialog.h"

#include <QStringList>

#include "twitterapiaccount.h"

#include "laconicamicroblog.h"

LaconicaDMessageDialog::LaconicaDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent, Qt::WindowFlags flags)
    : TwitterApiDMessageDialog(theAccount, parent, flags)
{
    const QStringList list = theAccount->friendsList();

    if (list.isEmpty()) {
        reloadFriendslist();
    } else {
        QStringList sameHost;

        Q_FOREACH (const QString &user, list) {
            // QUrl::host() is needed to skip scheme check
            if (LaconicaMicroBlog::hostFromProfileUrl(user).compare(QUrl(theAccount->host()).host()) == 0) {
                sameHost.append(LaconicaMicroBlog::usernameFromProfileUrl(user));
            }
        }

        sameHost.sort();
        setFriends(sameHost);
    }
}

LaconicaDMessageDialog::~LaconicaDMessageDialog()
{
}
