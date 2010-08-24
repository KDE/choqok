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

#include "three_ly.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <notifymanager.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Three_ly > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_three_ly" ) )

Three_ly::Three_ly( QObject* parent, const QVariantList& )
    : Choqok::Shortener( MyPluginFactory::componentData(), parent )
{
}

Three_ly::~Three_ly()
{
}

QString Three_ly::shorten( const QString& url )
{
    kDebug() << "Using 3.ly";
    QByteArray data;
    QString apiKey = "ae2499582394";          // Custom API key by Andrey Esin
    KUrl reqUrl( "http://3.ly" );
    reqUrl.addQueryItem( "api", apiKey.toUtf8() );
    reqUrl.addQueryItem( "u", KUrl( url ).url() );

    KIO::Job* job = KIO::get( reqUrl, KIO::Reload, KIO::HideProgressInfo );

    if( KIO::NetAccess::synchronousRun( job, 0, &data ) ) {
        QString output( data );
        kDebug() << "Short url is: " << output;
        QRegExp rx( QString( "(http://3.ly/([a-zA-Z0-9])+)" ) );
        rx.indexIn( output );
        output = rx.cap( 1 );
        if( !output.isEmpty() ) {
            return output;
        }
        Choqok::NotifyManager::error( QString( data ), i18n("3.ly error") );
    }
    else {
        Choqok::NotifyManager::error( i18n("Cannot create a short url.\n%1", job->errorString()) );
        kDebug() << "Cannot create a shorten url.\t" << "KJob ERROR";
    }
    return url;
}
