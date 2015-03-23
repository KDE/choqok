/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2009-2010 Felix Rohrbach <fxrh@gmx.de>

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

#include "is_gd.h"

#include <QEventLoop>
#include <QJsonDocument>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

#include "is_gd_settings.h"

K_PLUGIN_FACTORY_WITH_JSON(Is_gdFactory, "choqok_is_gd.json", registerPlugin < Is_gd > ();)

Is_gd::Is_gd(QObject *parent, const QVariantList &)
    : Choqok::Shortener("choqok_is_gd", parent)
{
}

Is_gd::~Is_gd()
{
}

QString Is_gd::shorten(const QString &url)
{
    Is_gd_Settings::self()->load();

    QUrl reqUrl("http://is.gd/create.php");
    reqUrl.addQueryItem("format", "json");
    reqUrl.addQueryItem("url", QUrl(url).url());
    if (Is_gd_Settings::logstats()) {
        reqUrl.addQueryItem("logstats", "true");
    }

    QEventLoop loop;
    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();

    if (job->error() == KJob::NoError) {

        const QJsonDocument json = QJsonDocument::fromJson(job->data());
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();

            if (!map[ "errorcode" ].toString().isEmpty()) {
                Choqok::NotifyManager::error(map[ "errormessage" ].toString(), i18n("is.gd Error"));
                return url;
            }
            QString shorturl = map[ "shorturl" ].toString();
            if (!shorturl.isEmpty()) {
                return shorturl;
            }
        } else {
            Choqok::NotifyManager::error(i18n("Malformed response"), i18n("is.gd Error"));
        }
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()), i18n("is.gd Error"));
    }
    return url;
}

#include "is_gd.moc"
