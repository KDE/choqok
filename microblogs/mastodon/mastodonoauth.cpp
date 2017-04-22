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

#include "mastodonoauth.h"

#include <QNetworkReply>
#include <QUrlQuery>

#include <KIO/AccessManager>

#include "mastodonaccount.h"
#include "mastodondebug.h"
#include "mastodonoauthreplyhandler.h"

MastodonOAuth::MastodonOAuth(MastodonAccount *account)
    : QOAuth2AuthorizationCodeFlow(account),
      m_replyHandler(0), m_networkAccessManager(0)
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
