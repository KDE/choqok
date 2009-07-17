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

#include "digg.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include <QDomDocument>

typedef KGenericFactory<Digg> MyPluginFactory;
static const KAboutData aboutdata("choqok_digg", 0, ki18n("Digg.com Shortener") , "0.1" );
K_EXPORT_COMPONENT_FACTORY( choqok_digg, MyPluginFactory( &aboutdata )  )

Digg::Digg( QObject *parent, const QStringList &  args  )
: Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
    Q_UNUSED(args)
}

QString Digg::shorten( const QString &baseUrl )
{
    kDebug()<<"Using digg.com";
    QMap<QString, QString> metaData;
    QByteArray data;
    KUrl url( "http://services.digg.com/url/short/create" );
    url.addQueryItem( "url", KUrl( baseUrl ).url() );
    url.addQueryItem( "appkey", "http://choqok.gnufolks.org" );

    KIO::Job *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );

    metaData.insert( "PropagateHttpHeader", "true" );
    if ( KIO::NetAccess::synchronousRun( job, 0, &data, 0, &metaData ) ) {
        QString responseHeaders = metaData[ "HTTP-Headers" ];
        QString code = responseHeaders.split( ' ' )[1];
        if ( code == "200" ) {
            kDebug() << "Short url is: " << data;
            QDomDocument doc;
            doc.setContent(data);
            if(doc.documentElement().tagName() == "shorturls") {
                QDomElement elm = doc.documentElement().firstChild().toElement();
                if(elm.tagName() == "shorturl"){
                    return elm.attribute("short_url", baseUrl);
                }
            }
            return QString(data);
        } else {
            kDebug() << "shortenning url faild HTTP response code is: " << code;
        }
    } else {
        QString responseHeaders = metaData[ "HTTP-Headers" ];
        kDebug() << "Cannot create a shorten url.\t" << "Response header = " << responseHeaders;
    }
    return baseUrl;
}

Digg::~Digg()
{
}

#include "digg.moc"
