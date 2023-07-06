/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mastodonoauth.h"

#include <QNetworkReply>
#include <QUrlQuery>

#include <KIO/AccessManager>

#include "mastodonaccount.h"
#include "mastodondebug.h"
#include "mastodonoauthreplyhandler.h"

MastodonOAuth::MastodonOAuth(MastodonAccount *account)
    : QOAuth2AuthorizationCodeFlow(account),
      m_replyHandler(nullptr), m_networkAccessManager(nullptr)
{
    qCDebug(CHOQOK);

    m_replyHandler = new MastodonOAuthReplyHandler(this);
    setReplyHandler(m_replyHandler);

    m_networkAccessManager = new KIO::AccessManager(this);
    setNetworkAccessManager(m_networkAccessManager);

    setClientIdentifier(account->consumerKey());
    setClientIdentifierSharedKey(account->consumerSecret());

    setScope(QLatin1String("read write follow"));

    setAccessTokenUrl(QUrl(account->host() + QLatin1String("/oauth/token")));
    setAuthorizationUrl(QUrl(account->host() + QLatin1String("/oauth/authorize")));
}

MastodonOAuth::~MastodonOAuth()
{
    m_replyHandler->deleteLater();
    m_networkAccessManager->deleteLater();
}

void MastodonOAuth::getToken(const QString &code)
{
    requestAccessToken(code);
}

#include "moc_mastodonoauth.cpp"
