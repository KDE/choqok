/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MASTODONOAUTH_H
#define MASTODONOAUTH_H

#include <QOAuth2AuthorizationCodeFlow>
#include <QNetworkAccessManager>

class MastodonAccount;

namespace KIO {
class AccessManager;
}

class MastodonOAuthReplyHandler;

class MastodonOAuth : public QOAuth2AuthorizationCodeFlow
{
    Q_OBJECT
public:
    explicit MastodonOAuth(MastodonAccount *account);
    ~MastodonOAuth();

    QByteArray authorizationHeader(const QUrl &requestUrl, QNetworkAccessManager::Operation method,
                                   const QVariantMap &params = QVariantMap());

    void getToken(const QString &code);

private:
    MastodonOAuthReplyHandler *m_replyHandler;
    KIO::AccessManager *m_networkAccessManager;
};

#endif // MASTODONOAUTH_H
