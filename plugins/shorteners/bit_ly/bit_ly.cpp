/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "bit_ly.h"

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"
#include "passwordmanager.h"

#include "bit_ly_settings.h"

K_PLUGIN_CLASS_WITH_JSON(Bit_ly, "choqok_bit_ly.json")

Bit_ly::Bit_ly(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_bit_ly"), parent)
{
}

Bit_ly::~Bit_ly()
{
}

QString Bit_ly::shorten(const QString &url)
{
    QString login = QCoreApplication::applicationName();
    QString apiKey = QLatin1String("R_bdd1ae8b6191dd36e13fc77ca1d4f27f");
    QUrl reqUrl(QLatin1String("http://api.bit.ly/v3/shorten"));
    QUrlQuery reqQuery;
    Bit_ly_Settings::self()->load();
    QString userApiKey = Choqok::PasswordManager::self()->readPassword(QStringLiteral("bitly_%1")
                         .arg(Bit_ly_Settings::login()));
    if (!Bit_ly_Settings::login().isEmpty() && !userApiKey.isEmpty()) {
        reqQuery.addQueryItem(QLatin1String("x_login"), Bit_ly_Settings::login());
        reqQuery.addQueryItem(QLatin1String("x_apiKey"), userApiKey);
    }

    if (Bit_ly_Settings::domain() == QLatin1String("j.mp")) { //bit.ly is default domain
        reqQuery.addQueryItem(QLatin1String("domain"), QLatin1String("j.mp"));
    }

    reqQuery.addQueryItem(QLatin1String("login"), QLatin1String(login.toUtf8()));
    reqQuery.addQueryItem(QLatin1String("apiKey"), QLatin1String(apiKey.toUtf8()));
    reqQuery.addQueryItem(QLatin1String("longUrl"), QUrl(url).url());
    reqQuery.addQueryItem(QLatin1String("format"), QLatin1String("txt"));
    reqUrl.setQuery(reqQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        const QByteArray data = job->data();
        QString output = QLatin1String(data);
        QRegExp rx(QLatin1String("(http://((.*)+)/([a-zA-Z0-9])+)"));
        rx.indexIn(output);
        QString bitlyUrl = rx.cap(0);
        if (!bitlyUrl.isEmpty()) {
            return bitlyUrl;
        }

        QString err = QLatin1String(data);
        if (output.startsWith(QLatin1String("INVALID_X_APIKEY"))) {
            err = i18n("API key is invalid");
        }
        if (output.startsWith(QLatin1String("INVALID_X_LOGIN"))) {
            err = i18n("Login is invalid");
        }
        if (output.startsWith(QLatin1String("RATE_LIMIT_EXCEEDED"))) {
            err = i18n("Rate limit exceeded. Try another shortener.");
        }

        Choqok::NotifyManager::error(i18n("Malformed response"), i18n("Bit.ly error"));
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()),
                                     i18n("Bit.ly error"));
    }
    return url;
}

#include "bit_ly.moc"
#include "moc_bit_ly.cpp"
