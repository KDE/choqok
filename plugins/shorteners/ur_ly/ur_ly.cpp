/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
Copyright (C) 2010 Scott Banwart <sbanwart@rogue-technology.com>

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
#include "ur_ly.h"

#include <QDebug>
#include <QJsonDocument>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KPluginFactory>

#include "notifymanager.h"

K_PLUGIN_FACTORY_WITH_JSON ( Ur_lyFactory, "choqok_ur_ly.json",
                             registerPlugin < Ur_ly> (); )

Ur_ly::Ur_ly ( QObject* parent, const QVariantList& )
    : Choqok::Shortener ( "choqok_ur_ly", parent )
{
}

Ur_ly::~Ur_ly()
{
}

QString Ur_ly::shorten ( const QString& url )
{
    QByteArray data;
    QUrl reqUrl ( "http://ur.ly/new.json" );
    reqUrl.addQueryItem( "href", QUrl( url ).url() );

    KIO::Job* job = KIO::get ( reqUrl, KIO::Reload, KIO::HideProgressInfo );

    if ( KIO::NetAccess::synchronousRun ( job, 0, &data ) ){
        const QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isNull()) {
            const QVariantMap result = json.toVariant().toMap();

            if ( result.contains("code") ) {
                return QString("http://ur.ly/%1").arg(result.value("code").toString());
            }
        } else{
            qCritical() << "Ur_ly::shorten: Parse error, Job error:" << job->errorString();
            qCritical() << "Data:" << data;
            Choqok::NotifyManager::error( i18n("Malformed response"), i18n("Ur.ly Error") );
        }
    } else {
        qCritical() << "Cannot create a shortened url:" << job->errorString();
        Choqok::NotifyManager::error( i18n("Cannot create a short URL.\n%1",
                                           job->errorString()), i18n("Ur.ly Error") );
    }
    return url;
}

#include "ur_ly.moc"