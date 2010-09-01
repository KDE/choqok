/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Andrey Esin <gmlastik@gmail.com>

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

#include "tinyarro_ws.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include "notifymanager.h"
#include "tinyarro_ws_settings.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Tinyarro_ws > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_tinyarro_ws" ) )

Tinyarro_ws::Tinyarro_ws( QObject *parent, const QVariantList & )
    : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

Tinyarro_ws::~Tinyarro_ws()
{
}

QString Tinyarro_ws::shorten( const QString &url )
{
    kDebug();
    QByteArray data;

    KUrl reqUrl( "http://tinyarro.ws/api-create.php" );

    Tinyarro_ws_Settings::self()->readConfig();

    if( !Tinyarro_ws_Settings::tinyarro_ws_host_punny().isEmpty() || 
        Tinyarro_ws_Settings::tinyarro_ws_host_punny() != "Random" ) {
        reqUrl.addQueryItem( "host", Tinyarro_ws_Settings::tinyarro_ws_host_punny() );
    }
    reqUrl.addQueryItem( "utfpure", "1" );
    reqUrl.addQueryItem( "url", KUrl( url ).url() );

    KIO::Job *job = KIO::get( reqUrl, KIO::Reload, KIO::HideProgressInfo );

    if ( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
        QString output = QString::fromUtf8( data );

        if( !output.isEmpty() ) {
            kDebug() << "Short url is: " << output;
            if ( output.startsWith( QString( "http://" ) ) )
              return output;
        }
        Choqok::NotifyManager::error( output, i18n( "Tinyarro.ws error" ) );
    } else {
        Choqok::NotifyManager::error( i18n( "Cannot create a short url.\n%1", job->errorString() ) );
    }
    return url;
}
