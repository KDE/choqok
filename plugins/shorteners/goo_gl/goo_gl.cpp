/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or ( at your option ) version 3 or any later version
    accepted by the membership of KDE e.V. ( or its successor approved
    by the membership of KDE e.V. ), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "goo_gl.h"

#include <QJsonDocument>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_FACTORY_WITH_JSON(Goo_glFactory, "choqok_goo_gl.json", registerPlugin < Goo_gl > ();)

Goo_gl::Goo_gl(QObject *parent, const QVariantList &)
    : Choqok::Shortener("choqok_goo_gl", parent)
{
}

Goo_gl::~Goo_gl()
{
}

QString Goo_gl::shorten(const QString &url)
{
    QVariantMap req;
    req.insert("longUrl", url);
    const QByteArray json = QJsonDocument::fromVariant(req).toJson();
    KIO::StoredTransferJob *job = KIO::storedHttpPost(json, QUrl("https://www.googleapis.com/urlshortener/v1/url"), KIO::HideProgressInfo) ;
    if (!job) {
        Choqok::NotifyManager::error(i18n("Error when creating job"), i18n("Goo.gl Error"));
        return url;
    }
    job->addMetaData("content-type", "Content-Type: application/json");

    QByteArray data;
    if (KIO::NetAccess::synchronousRun(job, 0, &data)) {
        const QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();
            const QVariantMap error = map["error"].toMap();
            if (!error.isEmpty()) {
                Choqok::NotifyManager::error(error["message"].toString(), i18n("Goo.gl Error"));
                return url;
            }
            return map[ "id" ].toString();
        }
        Choqok::NotifyManager::error(i18n("Malformed response"), i18n("Goo.gl Error"));
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()), i18n("Goo.gl Error"));
    }
    return url;
}

#include "goo_gl.moc"
