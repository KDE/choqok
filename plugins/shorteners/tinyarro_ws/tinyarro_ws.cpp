/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#include "tinyarro_ws.h"

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

#include "tinyarro_ws_settings.h"

K_PLUGIN_FACTORY_WITH_JSON(Tinyarro_wsFactory, "choqok_tinyarro_ws.json",
                           registerPlugin < Tinyarro_ws > ();)

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

    Tinyarro_ws_Settings::self()->load();

    if (!Tinyarro_ws_Settings::tinyarro_ws_host_punny().isEmpty() ||
            Tinyarro_ws_Settings::tinyarro_ws_host_punny() != QLatin1String("Random")) {
        reqUrl.addQueryItem(QLatin1String("host"), Tinyarro_ws_Settings::tinyarro_ws_host_punny());
    }
    reqUrl.addQueryItem(QLatin1String("utfpure"), QLatin1String("1"));
    reqUrl.addQueryItem(QLatin1String("url"), QUrl(url).url());

    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        QString output = QString::fromUtf8(job->data());

        if (!output.isEmpty()) {
            if (output.startsWith(QString::fromLatin1("http://"))) {
                return output;
            }
        }
        Choqok::NotifyManager::error(output, i18n("Tinyarro.ws error"));
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()));
    }
    return url;
}

#include "tinyarro_ws.moc"