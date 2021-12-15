/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pumpiooauth.h"

#include <QNetworkReply>
#include <QOAuth1Signature>
#include <QUrlQuery>

#include <KIO/AccessManager>

#include "pumpioaccount.h"
#include "pumpiodebug.h"
#include "pumpiooauthreplyhandler.h"

PumpIOOAuth::PumpIOOAuth(PumpIOAccount *account)
    : QOAuth1(account), m_replyHandler(nullptr), m_networkAccessManager(nullptr)
{
    qCDebug(CHOQOK);

    m_replyHandler = new PumpIOOAuthReplyHandler(this);
    setReplyHandler(m_replyHandler);

    m_networkAccessManager = new KIO::AccessManager(this);
    setNetworkAccessManager(m_networkAccessManager);

    setClientIdentifier(account->consumerKey());
    setClientSharedSecret(account->consumerSecret());

    setSignatureMethod(SignatureMethod::Hmac_Sha1);

    setTemporaryCredentialsUrl(QUrl(account->host() + QLatin1String("/oauth/request_token")));
    setAuthorizationUrl(QUrl(account->host() + QLatin1String("/oauth/authorize")));
    setTokenCredentialsUrl(QUrl(account->host() + QLatin1String("/oauth/access_token")));
}

PumpIOOAuth::~PumpIOOAuth()
{
    m_replyHandler->deleteLater();
    m_networkAccessManager->deleteLater();
}

QString PumpIOOAuth::authorizationHeader(const QUrl &requestUrl, QNetworkAccessManager::Operation operation,
                                         const QVariantMap &signingParameters)
{
    QVariantMap oauthParams;

    const auto currentDateTime = QDateTime::currentDateTimeUtc();

    oauthParams.insert(QStringLiteral("oauth_consumer_key"), clientIdentifier());
    oauthParams.insert(QStringLiteral("oauth_version"), QStringLiteral("1.0"));
    oauthParams.insert(QStringLiteral("oauth_token"), token());
    oauthParams.insert(QStringLiteral("oauth_signature_method"), QStringLiteral("HMAC-SHA1"));
    oauthParams.insert(QStringLiteral("oauth_nonce"), QOAuth1::nonce());
    oauthParams.insert(QStringLiteral("oauth_timestamp"), QString::number(currentDateTime.toTime_t()));

    // Add signature parameter
    {
        const auto parameters = QVariantMap(oauthParams).unite(signingParameters);

        const QOAuth1Signature signature(requestUrl, clientSharedSecret(), tokenSecret(),
                                         static_cast<QOAuth1Signature::HttpRequestMethod>(operation),
                                         parameters);

        oauthParams.insert(QStringLiteral("oauth_signature"), signature.hmacSha1().toBase64());
    }

    const QByteArray authorization = generateAuthorizationHeader(oauthParams);

    return QStringLiteral("Authorization: ") + QLatin1String(authorization);
}
