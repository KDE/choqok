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
#include "ur1_ca.h"
#include <QString>
#include <KIO/Job>
#include <KDebug>
#include <kio/netaccess.h>
#include <KAboutData>
#include <KGenericFactory>
#include <kglobal.h>

K_PLUGIN_FACTORY ( MyPluginFactory, registerPlugin < Ur1_ca> (); )
K_EXPORT_PLUGIN ( MyPluginFactory ( "choqok_ur1_ca" ) )

Ur1_ca::Ur1_ca ( QObject* parent, const QVariantList& )
    : Choqok::Shortener ( MyPluginFactory::componentData(), parent )
{
}

Ur1_ca::~Ur1_ca()
{
}


QString Ur1_ca::shorten ( const QString& url )
{
  kDebug() << "Using ur1.ca";
  QByteArray data;
  KUrl reqUrl ( "http://ur1.ca/" );
  QString temp;
  temp = KUrl::encode_string(url);

  QByteArray parg("longurl=");
  parg.append(temp.toAscii());

  KIO::Job* job = KIO::http_post ( reqUrl, parg, KIO::HideProgressInfo );
  job->addMetaData("content-type","Content-Type: application/x-www-form-urlencoded");

  if ( KIO::NetAccess::synchronousRun ( job, 0, &data ) )
    {
      QString output ( data );
      QRegExp rx(QString("<p class=[\'\"]success[\'\"]>(.*)</p>"));
      rx.setMinimal(true);
      rx.indexIn(output);
      output = rx.cap(1);
      rx.setPattern(QString("href=[\'\"](.*)[\'\"]"));
      rx.indexIn(output);
      output = rx.cap(1);
      kDebug() << "Short url is: " << output;
      if ( !output.isEmpty() )
        {
          return output;
        }
    }
  else
    {
      kDebug() << "Cannot create a shortened url.\t" << job->errorString();
    }
  return url;
}
