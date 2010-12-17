/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Andrey Esin <gmlastik@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or ( at your option ) version 3 or any later version
    accepted by the membership of KDE e.V. ( or its successor approved
    by the membership of KDE e.V. ), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "goo_gl.h"
#include <QtCore/QCoreApplication>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include <kio/job.h>
#include <math.h>
#include <notifymanager.h>
#include <qjson/parser.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Goo_gl > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_goo_gl" ) )

Goo_gl::Goo_gl( QObject* parent, const QVariantList& )
        : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

Goo_gl::~Goo_gl()
{
}

QString Goo_gl::shorten( const QString& url )
{
    kDebug() << "Using goo.gl";

    QByteArray req;
    req = "url=" + QUrl::toPercentEncoding( KUrl( url ).url() );// +
          "&user=toolbar";

    QMap<QString, QString> metaData;
    metaData.insert("accept","*/*");
    metaData.insert("content-type", "Content-Type: application/x-www-form-urlencoded" );

    KIO::StoredTransferJob *job = KIO::storedHttpPost ( req, KUrl("http://goo.gl/api/url"), KIO::HideProgressInfo ) ;
    job->setMetaData(KIO::MetaData(metaData));

    QByteArray data;
    if ( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
        QString output( data );
        QJson::Parser parser;
        bool ok;
        QVariantMap map = parser.parse( data , &ok ).toMap();

        if ( ok ) {
            if ( !map[ "error" ].toString().isEmpty() ) {
                Choqok::NotifyManager::error( map[ "error" ].toString(), i18n("Goo.gl error") );
                return url;
            }
            return map[ "short_url" ].toString();
        }
        Choqok::NotifyManager::error( i18n("Malformed response\n"), i18n("Goo.gl error")  );
    } else {
        Choqok::NotifyManager::error( i18n("Cannot create a short url.\n%1", job->errorString()), i18n("Goo.gl error") );
    }
    return url;
}

#include "goo_gl.moc"
