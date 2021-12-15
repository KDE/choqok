/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2015 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gnusocialapidmessagedialog.h"

#include <QStringList>

#include "twitterapiaccount.h"

#include "gnusocialapimicroblog.h"

GNUSocialApiDMessageDialog::GNUSocialApiDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent, Qt::WindowFlags flags)
    : TwitterApiDMessageDialog(theAccount, parent, flags)
{
    const QStringList list = theAccount->friendsList();

    if (list.isEmpty()) {
        reloadFriendslist();
    } else {
        QStringList sameHost;

        for (const QString &user: list) {
            // QUrl::host() is needed to skip scheme check
            if (GNUSocialApiMicroBlog::hostFromProfileUrl(user).compare(QUrl(theAccount->host()).host()) == 0) {
                sameHost.append(GNUSocialApiMicroBlog::usernameFromProfileUrl(user));
            }
        }

        sameHost.sort(Qt::CaseInsensitive);
        setFriends(sameHost);
    }
}

GNUSocialApiDMessageDialog::~GNUSocialApiDMessageDialog()
{
}
