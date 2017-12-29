/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2010-2011 Marcello Ceschia <marcelloceschia@users.sourceforge.net>
Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "yourls.h"

#include <QUrl>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"
#include "passwordmanager.h"

#include "yourlssettings.h"

K_PLUGIN_FACTORY_WITH_JSON(YourlsFactory, "choqok_yourls.json",
                           registerPlugin < Yourls > ();)

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
