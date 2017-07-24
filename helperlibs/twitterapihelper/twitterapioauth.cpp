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

#include "twitterapioauth.h"

#include <QEventLoop>
#include <QNetworkReply>

#include <KIO/AccessManager>

#include "twitterapiaccount.h"
#include "twitterapidebug.h"
#include "twitterapioauthreplyhandler.h"

TwitterApiOAuth::TwitterApiOAuth(TwitterApiAccount *account)
    : QOAuth1(account), m_replyHandler(0), m_networkAccessManager(0)
{
    qCDebug(CHOQOK);

    m_replyHandler = new TwitterApiOAuthReplyHandler(this);
    setReplyHandler(m_replyHandler);

    m_networkAccessManager = new KIO::AccessManager(this);
    setNetworkAccessManager(m_networkAccessManager);

    setClientIdentifier(QLatin1String(account->oauthConsumerKey()));
    setClientSharedSecret(QLatin1String(account->oauthConsumerSecret()));

    setSignatureMethod(SignatureMethod::Hmac_Sha1);

    setTemporaryCredentialsUrl(QUrl(account->host() + QLatin1String("/oauth/request_token")));
    setAuthorizationUrl(QUrl(account->host() + QLatin1String("/oauth/authorize")));
    setTokenCredentialsUrl(QUrl(account->host() + QLatin1String("/oauth/access_token")));
}

TwitterApiOAuth::~TwitterApiOAuth()
{
    m_replyHandler->deleteLater();
    m_networkAccessManager->deleteLater();
}

QByteArray TwitterApiOAuth::authorizationHeader(const QUrl &requestUrl, QNetworkAccessManager::Operation operation,
                                              const QVariantMap &signingParameters)
{
    QVariantMap oauthParams;
    QVariantMap otherParams = signingParameters;
    // Adding parameters located in the query
    {
        auto queryItems = QUrlQuery(requestUrl.query()).queryItems();
        for (auto it = queryItems.begin(), end = queryItems.end(); it != end; ++it)
            otherParams.insert(it->first, it->second);
    }

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
        oauthParams.insert(QStringLiteral("oauth_signature"), signature(parameters, requestUrl,
                                                                        operation, clientSharedSecret(), tokenSecret()));
    }

    return generateAuthorizationHeader(oauthParams);
}
