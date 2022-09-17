/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "tighturl.h"

#include <QEventLoop>
#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_CLASS_WITH_JSON(TightUrl, "choqok_tighturl.json")

TightUrl::TightUrl(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_tighturl"), parent)
{
}

QString TightUrl::shorten(const QString &url)
{
    QUrl reqUrl(QLatin1String("http://2tu.us/"));
    QUrlQuery reqQuery;
    reqQuery.addQueryItem(QLatin1String("save"), QLatin1String("y"));
    reqQuery.addQueryItem(QLatin1String("url"), QUrl(url).url());
    reqUrl.setQuery(reqQuery);

    QEventLoop loop;
    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::result, &loop, &QEventLoop::quit);
    job->start();
    loop.exec();

    if (job->error() == KJob::NoError) {
        QString output(QLatin1String(job->data()));
        QRegExp rx(QLatin1String("<code>(.+)</code>"));
        rx.setMinimal(true);
        rx.indexIn(output);
        output = rx.cap(1);
        rx.setPattern(QLatin1String("href=[\'\"](.+)[\'\"]"));
        rx.indexIn(output);
        output = rx.cap(1);
        if (!output.isEmpty()) {
            return output;
        }
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1",
                                          job->errorString()), i18n("TightUrl Error"));
    }
    return url;
}

TightUrl::~TightUrl()
{
}

#include "tighturl.moc"
