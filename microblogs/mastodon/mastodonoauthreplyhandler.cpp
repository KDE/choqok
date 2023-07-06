/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mastodonoauthreplyhandler.h"

MastodonOAuthReplyHandler::MastodonOAuthReplyHandler(QObject *parent)
    : QOAuthOobReplyHandler(parent)
{
}

MastodonOAuthReplyHandler::~MastodonOAuthReplyHandler()
{
}

QString MastodonOAuthReplyHandler::callback() const
{
    return QLatin1String("urn:ietf:wg:oauth:2.0:oob");
}

#include "moc_mastodonoauthreplyhandler.cpp"
