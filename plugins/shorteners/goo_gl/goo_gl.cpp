/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#include <KAboutData>
#include <KDebug>
#include <KGenericFactory>
#include <KGlobal>
#include <KIO/Job>
#include <KIO/NetAccess>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "notifymanager.h"

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

    QVariantMap req;
    req.insert("longUrl", url);
    QJson::Serializer serializer;
    QByteArray data = serializer.serialize(req);

    KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, KUrl("https://www.googleapis.com/urlshortener/v1/url"), KIO::HideProgressInfo ) ;
    if (!job){
      Choqok::NotifyManager::error( i18n("Error when creating job"), i18n("Goo.gl Error") );
      return url;
    }
    job->addMetaData("content-type", "Content-Type: application/json");

    if ( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
        QString output( data );
        QJson::Parser parser;
        bool ok;
        QVariantMap map = parser.parse( data , &ok ).toMap();
        if ( ok ) {
            QVariantMap error = map["error"].toMap();
            if ( !error.isEmpty() ) {
                Choqok::NotifyManager::error( error["message"].toString(), i18n("Goo.gl Error") );
                return url;
            }
            return map[ "id" ].toString();
        }
        Choqok::NotifyManager::error( i18n("Malformed response"), i18n("Goo.gl Error")  );
    } else {
        Choqok::NotifyManager::error( i18n("Cannot create a short URL.\n%1", job->errorString()), i18n("Goo.gl Error") );
    }
    return url;
}

#include "goo_gl.moc"
