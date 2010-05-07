/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
Copyright (C) 2010 Scott Banwart <sbanwart@rogue-technology.com>

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
#include "ur_ly.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>
#include <qjson/parser.h>

K_PLUGIN_FACTORY ( MyPluginFactory, registerPlugin < Ur_ly> (); )
K_EXPORT_PLUGIN ( MyPluginFactory ( "choqok_ur_ly" ) )

Ur_ly::Ur_ly ( QObject* parent, const QVariantList& )
    : Choqok::Shortener ( MyPluginFactory::componentData(), parent )
{
}

Ur_ly::~Ur_ly()
{
}


QString Ur_ly::shorten ( const QString& url )
{
    kDebug() << "Using ur.ly";
    QByteArray data;
    KUrl reqUrl ( "http://ur.ly/new.json" );
    reqUrl.addQueryItem( "href", KUrl( url ).url() );

    KIO::Job* job = KIO::get ( reqUrl, KIO::Reload, KIO::HideProgressInfo );

    if ( KIO::NetAccess::synchronousRun ( job, 0, &data ) ){
        QJson::Parser parser;
        bool ok;
        QVariantMap result = parser.parse(data, &ok).toMap();
        if ( ok && result.contains("code") ) {
            return QString("http://ur.ly/%1").arg(result.value("code").toString());
        } else{
            kError()<<"Ur_ly::shorten: Parse error, Job error: "<<job->errorString()<<"\n Data:"<<data;
        }
    } else {
        kDebug() << "Cannot create a shortened url.\t" << job->errorString();
    }
    return url;
}
