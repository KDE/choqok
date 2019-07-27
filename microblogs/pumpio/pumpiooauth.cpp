/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017 Andrea Scarpino <scarpino@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

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
