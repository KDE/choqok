/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIOAUTH_H
#define TWITTERAPIOAUTH_H

#include <QOAuth1>

#include "twitterapihelper_export.h"

class TwitterApiAccount;
class TwitterApiOAuthReplyHandler;

namespace KIO {
class AccessManager;
}

class TWITTERAPIHELPER_EXPORT TwitterApiOAuth : public QOAuth1
{
    Q_OBJECT
public:
    explicit TwitterApiOAuth(TwitterApiAccount *account);
    ~TwitterApiOAuth();

    QByteArray authorizationHeader(const QUrl &requestUrl, QNetworkAccessManager::Operation method,
                                   const QVariantMap &params = QVariantMap());

private:
    TwitterApiOAuthReplyHandler *m_replyHandler;
    KIO::AccessManager *m_networkAccessManager;
};

#endif // TWITTERAPIOAUTH_H
