/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "tinyarro_ws.h"

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

#include "tinyarro_ws_settings.h"

K_PLUGIN_CLASS_WITH_JSON(Tinyarro_ws, "choqok_tinyarro_ws.json")

Tinyarro_ws::Tinyarro_ws(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_tinyarro_ws"), parent)
{
}

Tinyarro_ws::~Tinyarro_ws()
{
}

QString Tinyarro_ws::shorten(const QString &url)
{
    QUrl reqUrl(QLatin1String("http://tinyarro.ws/api-create.php"));
    QUrlQuery reqQuery;

    Tinyarro_ws_Settings::self()->load();

    if (!Tinyarro_ws_Settings::tinyarro_ws_host_punny().isEmpty() ||
            Tinyarro_ws_Settings::tinyarro_ws_host_punny() != QLatin1String("Random")) {
        reqQuery.addQueryItem(QLatin1String("host"), Tinyarro_ws_Settings::tinyarro_ws_host_punny());
    }
    reqQuery.addQueryItem(QLatin1String("utfpure"), QLatin1String("1"));
    reqQuery.addQueryItem(QLatin1String("url"), QUrl(url).url());
    reqUrl.setQuery(reqQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        QString output = QString::fromUtf8(job->data());

        if (!output.isEmpty()) {
            if (output.startsWith(QLatin1String("http://"))) {
                return output;
            }
        }
        Choqok::NotifyManager::error(output, i18n("Tinyarro.ws error"));
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()));
    }
    return url;
}

#include "moc_tinyarro_ws.cpp"
#include "tinyarro_ws.moc"
