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
    KUrl reqUrl( "http://is.gd/api.php" );
    reqUrl.addQueryItem( "longurl", KUrl( url ).url() );

    QEventLoop loop;
    KIO::StoredTransferJob* job = KIO::storedGet( reqUrl, KIO::Reload, KIO::HideProgressInfo );
    connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
    job->start();
    loop.exec();

    if( job->error() == KJob::NoError ) {
        QString output(job->data());
        kDebug() << "Short url is: " << output;
        if( !output.isEmpty() ) {
            return output;
        }
    }
    else {
        kDebug() << "KJob ERROR" << job->errorString();
    }
    return url;
}

// #include "is_gd.moc"
