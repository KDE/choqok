/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "ur1_ca.h"

#include <QEventLoop>
#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_FACTORY_WITH_JSON(Ur1_caFactory, "choqok_ur1_ca.json",
                           registerPlugin < Ur1_ca> ();)

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
    connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
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