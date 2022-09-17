/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2011 Marcello Ceschia <marcelloceschia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "yourls.h"

#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"
#include "passwordmanager.h"

#include "yourlssettings.h"

K_PLUGIN_CLASS_WITH_JSON(Yourls, "choqok_yourls.json")

Yourls::Yourls(QObject *parent, const QVariantList &)
    : Choqok::Shortener(QLatin1String("choqok_yourls"), parent)
{
    connect(YourlsSettings::self(), &YourlsSettings::configChanged,
            this, &Yourls::reloadConfigs);
}

Yourls::~Yourls()
{}

QString Yourls::shorten(const QString &url)
{
    QUrl reqUrl(YourlsSettings::yourlsHost());
    QUrlQuery reqQuery;
    reqQuery.addQueryItem(QLatin1String("action"), QLatin1String("shorturl"));           /* set action to shorturl */
    reqQuery.addQueryItem(QLatin1String("format"), QLatin1String("xml"));                /* get result as xml */
    reqQuery.addQueryItem(QLatin1String("url"), QUrl(url).url());                        /* url to be shorted */
    password = QLatin1String(Choqok::PasswordManager::self()->readPassword(
                   QStringLiteral("yourls_%1").arg(YourlsSettings::username())).toUtf8());
    if (!YourlsSettings::username().isEmpty()) {
        reqQuery.addQueryItem(QLatin1String("username"), YourlsSettings::username());
        reqQuery.addQueryItem(QLatin1String("password"), password);
    }

    reqUrl.setQuery(reqQuery);
    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        const QByteArray data = job->data();                            /* output field */
        QString output = QLatin1String(data);

        QRegExp rx(QLatin1String("<shorturl>(.+)</shorturl>"));
        rx.setMinimal(true);
        rx.indexIn(output);
        output = rx.cap(1);

        if (!output.isEmpty()) {
            return output;
        } else {
            output = QLatin1String(data);
            QRegExp rx(QLatin1String("<message>(.+)</message>"));
            rx.setMinimal(true);
            rx.indexIn(output);
            output = rx.cap(1);

            Choqok::NotifyManager::error(output, i18n("Yourls Error"));
        }
    } else {
        Choqok::NotifyManager::error(i18n("Cannot create a short URL.\n%1",
                                          job->errorString()));
    }
    return url;
}

void Yourls::reloadConfigs()
{
    password = QLatin1String(Choqok::PasswordManager::self()->readPassword(
                   QStringLiteral("yourls_%1").arg(YourlsSettings::username())).toUtf8());
}

#include "yourls.moc"
