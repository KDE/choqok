/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOOAUTH_H
#define PUMPIOOAUTH_H

#include <QOAuth1>

class PumpIOAccount;
class PumpIOOAuthReplyHandler;

namespace KIO {
class AccessManager;
}

class PumpIOOAuth : public QOAuth1
{
    Q_OBJECT
public:
    explicit PumpIOOAuth(PumpIOAccount *account);
    ~PumpIOOAuth();

    QString authorizationHeader(const QUrl &requestUrl, QNetworkAccessManager::Operation method,
                                const QVariantMap &params = QVariantMap());

private:
    PumpIOOAuthReplyHandler *m_replyHandler;
    KIO::AccessManager *m_networkAccessManager;
};

#endif // PUMPIOOAUTH_H
