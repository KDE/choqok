/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Felix Rohrbach <fxrh@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "is_gd.h"

#include <QEventLoop>
#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

#include "is_gd_settings.h"

K_PLUGIN_FACTORY_WITH_JSON(Is_gdFactory, "choqok_is_gd.json", registerPlugin < Is_gd > ();)

Is_gd::Is_gd(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_is_gd"), parent)
{
}

Is_gd::~Is_gd()
{
}

QString Is_gd::shorten(const QString &url)
{
    Is_gd_Settings::self()->load();

    QUrl reqUrl(QLatin1String("https://is.gd/create.php"));
    QUrlQuery reqQuery;
    reqQuery.addQueryItem(QLatin1String("format"), QLatin1String("json"));
    reqQuery.addQueryItem(QLatin1String("url"), QUrl(url).url());
    if (Is_gd_Settings::logstats()) {
        reqQuery.addQueryItem(QLatin1String("logstats"), QLatin1String("true"));
    }

    reqUrl.setQuery(reqQuery);
    QEventLoop loop;
    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::result, &loop, &QEventLoop::quit);
    job->start();
    loop.exec();

    if (job->error() == KJob::NoError) {

        const QJsonDocument json = QJsonDocument::fromJson(job->data());
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();

            if (!map[ QLatin1String("errorcode") ].toString().isEmpty()) {
                Choqok::NotifyManager::error(map[ QLatin1String("errormessage") ].toString(), i18n("is.gd Error"));
                return url;
            }
            QString shorturl = map[ QLatin1String("shorturl") ].toString();
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
