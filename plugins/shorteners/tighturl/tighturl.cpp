/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "tighturl.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include <qeventloop.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < TightUrl > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_tighturl" ) )

TightUrl::TightUrl( QObject *parent, const QVariantList & )
: Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

QString TightUrl::shorten( const QString &url )
{
    kDebug()<<"Using 2tu.us";
    KUrl reqUrl( "http://2tu.us/" );
    reqUrl.addQueryItem( "save", "y" );
    reqUrl.addQueryItem( "url", KUrl( url ).url() );

    QEventLoop loop;
    KIO::StoredTransferJob *job = KIO::storedGet( reqUrl, KIO::Reload, KIO::HideProgressInfo );
    connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();

    if ( job->error() == KJob::NoError ) {
        QString output(job->data());
        QRegExp rx( QString( "<code>(.+)</code>" ) );
        rx.setMinimal(true);
        rx.indexIn(output);
        output = rx.cap(1);
        kDebug()<<output;
        rx.setPattern( QString( "href=[\'\"](.+)[\'\"]" ) );
        rx.indexIn(output);
        output = rx.cap(1);
        kDebug() << "Short url is: " << output;
        if(!output.isEmpty()) {
            return output;
        }
    } else {
        kDebug() << "KJob ERROR: " << job->errorString() ;
    }
    return url;
}

TightUrl::~TightUrl()
{
}

// #include "tighturl.moc"
