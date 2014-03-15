/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2009-2010 Felix Rohrbach <fxrh@gmx.de>

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

#include "is_gd.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include <qeventloop.h>
#include <notifymanager.h>
#include <qjson/parser.h>
#include "is_gd_settings.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Is_gd > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_is_gd" ) )

Is_gd::Is_gd( QObject* parent, const QVariantList& )
    : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

Is_gd::~Is_gd()
{
}

QString Is_gd::shorten( const QString& url )
{
    kDebug() << "Using is.gd";

    Is_gd_Settings::self()->readConfig();

    KUrl reqUrl( "http://is.gd/create.php" );
    reqUrl.addQueryItem( "format", "json" );
    reqUrl.addQueryItem( "url", KUrl( url ).url() );
    if (Is_gd_Settings::logstats()) {
        reqUrl.addQueryItem( "logstats", "true");
    }

    QEventLoop loop;
    KIO::StoredTransferJob* job = KIO::storedGet( reqUrl, KIO::Reload, KIO::HideProgressInfo );
    connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();

    if( job->error() == KJob::NoError ) {
        QJson::Parser parser;
        bool ok;
        QVariantMap map = parser.parse( job->data() , &ok ).toMap();

        if ( ok ) {
            if ( !map[ "errorcode" ].toString().isEmpty() ) {
                Choqok::NotifyManager::error( map[ "errormessage" ].toString(), i18n("is.gd Error") );
                return url;
            }
            QString shorturl = map[ "shorturl" ].toString();
            if (!shorturl.isEmpty()) {
                return shorturl;
            }
        } else {
            Choqok::NotifyManager::error( i18n("Malformed response"), i18n("is.gd Error")  );
        }
    }
    else {
        Choqok::NotifyManager::error( i18n("Cannot create a short URL.\n%1", job->errorString()), i18n("is.gd Error") );
    }
    return url;
}

// #include "is_gd.moc"
