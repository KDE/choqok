/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ur1_ca.h"

#include <QEventLoop>
#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_CLASS_WITH_JSON(Ur1_ca, "choqok_ur1_ca.json")

Ur1_ca::Ur1_ca(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_ur1_ca"), parent)
{
}

Ur1_ca::~Ur1_ca()
{
}

QString Ur1_ca::shorten(const QString &url)
{
    QUrl reqUrl(QLatin1String("http://ur1.ca/"));
    QString temp;
    temp = QLatin1String(QUrl::toPercentEncoding(url));

    QByteArray parg("longurl=");
    parg.append(temp.toLatin1());

    QEventLoop loop;
    KIO::StoredTransferJob *job = KIO::storedHttpPost(parg, reqUrl, KIO::HideProgressInfo);
    job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/x-www-form-urlencoded"));
    connect(job, &KIO::StoredTransferJob::result, &loop, &QEventLoop::quit);
    job->start();
    loop.exec();

    if (job->error() == KJob::NoError) {
        QString output(QLatin1String(job->data()));
        QRegExp rx(QLatin1String("<p class=[\'\"]success[\'\"]>(.*)</p>"));
        rx.setMinimal(true);
        rx.indexIn(output);
        output = rx.cap(1);
        rx.setPattern(QLatin1String("href=[\'\"](.*)[\'\"]"));
        rx.indexIn(output);
        output = rx.cap(1);
        if (!output.isEmpty()) {
            return output;
        }
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1", job->errorString()), i18n("ur1.ca Error"));
    }
    return url;
}

#include "ur1_ca.moc"
