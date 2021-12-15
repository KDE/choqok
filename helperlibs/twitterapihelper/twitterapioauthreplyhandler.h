/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIOAUTHREPLYHANDLER_H
#define TWITTERAPIOAUTHREPLYHANDLER_H

#include <QOAuthHttpServerReplyHandler>

#include "choqok_export.h"

class CHOQOK_HELPER_EXPORT TwitterApiOAuthReplyHandler : public QOAuthHttpServerReplyHandler
{
    Q_OBJECT
public:
    explicit TwitterApiOAuthReplyHandler(QObject *parent = nullptr);
    ~TwitterApiOAuthReplyHandler();

    QString callback() const override;
};

#endif // TWITTERAPIOAUTHREPLYHANDLER_H
