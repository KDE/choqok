/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

typedef KGenericFactory<Is_gd> MyPluginFactory;
static const KAboutData aboutdata("choqok_isgd", 0, ki18n("Is.gd Shortener") , "0.1" );
K_EXPORT_COMPONENT_FACTORY( choqok_isgd, MyPluginFactory( &aboutdata )  )

Is_gd::Is_gd(QObject* parent, const QStringList&):
Choqok::Shortener( MyPluginFactory::componentData(), parent )
{

}

Is_gd::~Is_gd()
{

}

QString Is_gd::shorten(const QString& baseUrl)
{
    kDebug()<<"Using is.gd";
    QMap<QString, QString> metaData;
    QByteArray data;
    KUrl url( "http://is.gd/api.php" );
    url.addQueryItem( "longurl", KUrl( baseUrl ).url() );

    KIO::Job *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );

    metaData.insert( "PropagateHttpHeader", "true" );
    if ( KIO::NetAccess::synchronousRun( job, 0, &data, 0, &metaData ) ) {
        QString responseHeaders = metaData[ "HTTP-Headers" ];
        QString code = responseHeaders.split( ' ' )[1];
        if ( code == "200" ) {
            kDebug() << "Short url is: " << data;
            return QString( data );
        } else {
            kDebug() << "shortenning url faild HTTP response code is: " << code;
        }
    } else {
        QString responseHeaders = metaData[ "HTTP-Headers" ];
        kDebug() << "Cannot create a shorten url.\t" << "Response header = " << responseHeaders;
    }
    return baseUrl;
}

