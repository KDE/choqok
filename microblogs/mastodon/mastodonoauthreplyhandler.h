/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MASTODONOAUTHREPLYHANDLER_H
#define MASTODONOAUTHREPLYHANDLER_H

#include <QOAuthOobReplyHandler>

class MastodonOAuthReplyHandler : public QOAuthOobReplyHandler
{
    Q_OBJECT
public:
    explicit MastodonOAuthReplyHandler(QObject *parent = nullptr);
    ~MastodonOAuthReplyHandler();

    QString callback() const override;
};

#endif // MASTODONOAUTHREPLYHANDLER_H
