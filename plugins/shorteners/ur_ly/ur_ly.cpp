/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010 Scott Banwart <sbanwart@rogue-technology.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ur_ly.h"

#include <QDebug>
#include <QJsonDocument>
#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_FACTORY_WITH_JSON(Ur_lyFactory, "choqok_ur_ly.json",
                           registerPlugin < Ur_ly> ();)

Ur_ly::Ur_ly(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_ur_ly"), parent)
{
}

Ur_ly::~Ur_ly()
{
}

QString Ur_ly::shorten(const QString &url)
{
    QUrl reqUrl(QLatin1String("http://ur.ly/new.json"));
    QUrlQuery reqQuery;
    reqQuery.addQueryItem(QLatin1String("href"), QUrl(url).url());
    reqUrl.setQuery(reqQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        const QByteArray data = job->data();
        const QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isNull()) {
            const QVariantMap result = json.toVariant().toMap();

            if (result.contains(QLatin1String("code"))) {
                return QStringLiteral("http://ur.ly/%1").arg(result.value(QLatin1String("code")).toString());
            }
        } else {
            qCritical() << "Ur_ly::shorten: Parse error, Job error:" << job->errorString();
            qCritical() << "Data:" << data;
            Choqok::NotifyManager::error(i18n("Malformed response"), i18n("Ur.ly Error"));
        }
    } else {
        qCritical() << "Cannot create a shortened url:" << job->errorString();
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1",
                                          job->errorString()), i18n("Ur.ly Error"));
    }
    return url;
}

#include "ur_ly.moc"
