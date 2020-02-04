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
