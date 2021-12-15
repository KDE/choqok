/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "goo_gl.h"

#include <QJsonDocument>
#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_FACTORY_WITH_JSON(Goo_glFactory, "choqok_goo_gl.json", registerPlugin < Goo_gl > ();)

Goo_gl::Goo_gl(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_goo_gl"), parent)
{
}

Goo_gl::~Goo_gl()
{
}

QString Goo_gl::shorten(const QString &url)
{
    QVariantMap req;
    req.insert(QLatin1String("longUrl"), url);
    const QByteArray json = QJsonDocument::fromVariant(req).toJson();
    KIO::StoredTransferJob *job = KIO::storedHttpPost(json, QUrl::fromUserInput(QLatin1String("https://www.googleapis.com/urlshortener/v1/url")), KIO::HideProgressInfo) ;
    if (!job) {
        Choqok::NotifyManager::error(i18n("Error when creating job"), i18n("Goo.gl Error"));
        return url;
    }
    job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
    job->exec();

    if (!job->error()) {
        const QJsonDocument json = QJsonDocument::fromJson(job->data());
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();
            const QVariantMap error = map[QLatin1String("error")].toMap();
            if (!error.isEmpty()) {
                Choqok::NotifyManager::error(error[QLatin1String("message")].toString(), i18n("Goo.gl Error"));
                return url;
            }
            return map[ QLatin1String("id") ].toString();
        }
        Choqok::NotifyManager::error(i18n("Malformed response"), i18n("Goo.gl Error"));
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()), i18n("Goo.gl Error"));
    }
    return url;
}

#include "goo_gl.moc"
