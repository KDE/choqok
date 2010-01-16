
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
      kDebug() << "Cannot create a shortened url.\t" << "KJob ERROR";
    }
  return url;
}
