/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapioauthreplyhandler.h"

TwitterApiOAuthReplyHandler::TwitterApiOAuthReplyHandler(QObject *parent)
    : QOAuthHttpServerReplyHandler(parent)
{
}

TwitterApiOAuthReplyHandler::~TwitterApiOAuthReplyHandler()
{
}

QString TwitterApiOAuthReplyHandler::callback() const
{
    return QLatin1String("oob");
}

#include "moc_twitterapioauthreplyhandler.cpp"
