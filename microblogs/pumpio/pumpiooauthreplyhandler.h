/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOOAUTHREPLYHANDLER_H
#define PUMPIOOAUTHREPLYHANDLER_H

#include <QOAuthHttpServerReplyHandler>

class PumpIOOAuthReplyHandler : public QOAuthHttpServerReplyHandler
{
    Q_OBJECT
public:
    explicit PumpIOOAuthReplyHandler(QObject *parent = nullptr);
    ~PumpIOOAuthReplyHandler();

    QString callback() const override;
};

#endif // PUMPIOOAUTHREPLYHANDLER_H
