/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Timothy Redaelli <timothy@redaelli.eu>

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

#include "u_nu.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < U_nu > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_u_nu" ) )

U_nu::U_nu( QObject* parent, const QVariantList& )
    : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

U_nu::~U_nu()
{
}

QString U_nu::shorten( const QString& url )
{
    kDebug() << "Using u.nu";
    QByteArray data;
    KUrl reqUrl( "http://u.nu/unu-api-simple" );
    reqUrl.addQueryItem( "url", KUrl( url ).url() );

    KIO::Job* job = KIO::get( reqUrl, KIO::Reload, KIO::HideProgressInfo );

    if( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
        QString output(data);
        kDebug() << "Short url is: " << output;
        if( !output.isEmpty() ) {
            return output;
        }
    }
    else {
        kDebug() << "Cannot create a shorten url.\t" << "KJob ERROR";
    }
    return url;
}

// #include "u_nu.moc"
