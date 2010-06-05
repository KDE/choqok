/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    
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
/**
 * @ author Boris Tsirkin <bgdotmail+choqok@gmail.com>
 */

#include "urls_io.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Urls_io > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_urls_io" ) )

Urls_io::Urls_io( QObject* parent, const QVariantList& )
    : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

Urls_io::~Urls_io()
{
}

QString Urls_io::shorten( const QString& url )
{
    kDebug() << "Using urls.io";
    QByteArray data;
    KUrl reqUrl( "http://urls.io/api/get-short/" );
    reqUrl.addQueryItem( "full_url", KUrl( url ).url() );
    reqUrl.addQueryItem( "api_key", "50a311b108bab2e5e44dfac43d7185e1" );
    reqUrl.addQueryItem( "env", "5" );

    KIO::Job* job = KIO::get( reqUrl, KIO::Reload, KIO::HideProgressInfo );

    if( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
        QString output(data);
        QRegExp rx ( QString( "\"status\":\"(.+)\"" ) );
        rx.setMinimal(true);
        rx.indexIn(output);
        output = rx.cap(1);
        kDebug() << "Status: " <<output;
        if( "SUCCEED" == output ) {
            rx.setPattern( QString( "\"short_url\":\"(.+)\"" ) );
            rx.indexIn(output);
            output = rx.cap(1);
            kDebug() << "Short url is: " << output;
            if( !output.isEmpty() ) {
               return output;
            }
        }
        else {
            rx.setPattern( QString( "\"error_msg\":\"(.+)\"" ) );
            rx.indexIn(output);
            output = rx.cap(1);
            kDebug() << "Error: " << output;
        }
    }
    else {
        kDebug() << "Cannot create a shorten url.\t" << "KJob ERROR";
    }
    return url;
}

// #include "Urls_io.moc"
