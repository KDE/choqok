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

#include "posterous.h"
#include <KAction>
#include <KActionCollection>
#include <KAboutData>
#include <KGenericFactory>
#include "posteroussettings.h"
#include <KIO/Job>
#include <kio/netaccess.h>
#include <passwordmanager.h>
#include <QDomDocument>
#include <mediamanager.h>
#include <twitterapihelper/twitterapiaccount.h>
#include <accountmanager.h>
#include <twitterapihelper/twitterapimicroblog.h>
#include <QtOAuth/QtOAuth>
#include <qjson/parser.h>

K_PLUGIN_FACTORY ( MyPluginFactory, registerPlugin < Posterous > (); )
K_EXPORT_PLUGIN ( MyPluginFactory ( "choqok_posterous" ) )

Posterous::Posterous ( QObject* parent, const QList<QVariant>& )
        : Choqok::Uploader ( MyPluginFactory::componentData(), parent )
{

}

Posterous::~Posterous()
{

}

void Posterous::upload ( const KUrl& localUrl, const QByteArray& medium, const QByteArray& mediumType )
{
    PosterousSettings::self()->readConfig();
    QString alias = PosterousSettings::alias();
    if ( alias.isEmpty() ) {
        kError() << "No account to use";
        emit uploadingFailed ( localUrl, i18n ( "There's no Twitter account configured to use." ) );
        return;
    }
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *> ( Choqok::AccountManager::self()->findAccount ( alias ) );
    if ( !acc )
        return;

    KUrl url ( "http://posterous.com/api2/upload.json" );

    QMap<QString, QByteArray> formdata;
    formdata["source"] = "Choqok";
    formdata["sourceLink"] = "http://choqok.gnufolks.org/";

    QMap<QString, QByteArray> mediafile;
    mediafile["name"] = "media";
    mediafile["filename"] = localUrl.fileName().toUtf8();
    mediafile["mediumType"] = mediumType;
    mediafile["medium"] = medium;
    QList< QMap<QString, QByteArray> > listMediafiles;
    listMediafiles.append ( mediafile );

    QByteArray data = Choqok::MediaManager::createMultipartFormData ( formdata, listMediafiles );

    KIO::StoredTransferJob *job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
    QOAuth::ParamMap params;
    QString requrl = "https://api.twitter.com/1/account/verify_credentials.json";
    QByteArray credentials = acc->oauthInterface()->createParametersString ( requrl,
                             QOAuth::GET, acc->oauthToken(),
                             acc->oauthTokenSecret(),
                             QOAuth::HMAC_SHA1,
                             params, QOAuth::ParseForHeaderArguments );

    QString cHeader = "X-Verify-Credentials-Authorization: " +  QString ( credentials ) + "\r\n";
    cHeader.append ( "X-Auth-Service-Provider: https://api.twitter.com/1/account/verify_credentials.json" );
    job->addMetaData ( "customHTTPHeader", cHeader );
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData ( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
    mUrlMap[job] = localUrl;
    connect ( job, SIGNAL ( result ( KJob* ) ),
              SLOT ( slotUpload ( KJob* ) ) );
    job->start();
}

void Posterous::slotUpload ( KJob* job )
{
    kDebug();
    KUrl localUrl = mUrlMap.take ( job );
    if ( job->error() ) {
        kError() << "Job Error: " << job->errorString();
        emit uploadingFailed ( localUrl, job->errorString() );
        return;
    } else {
        kDebug() << qobject_cast<KIO::StoredTransferJob*> ( job )->data();
        QJson::Parser p;
        bool ok;
        QVariant res = p.parse ( qobject_cast<KIO::StoredTransferJob*> ( job )->data(), &ok );
        if ( ok ) {
            QVariantMap map = res.toMap();
            if ( map.contains ( "error" ) ) {
                emit uploadingFailed ( localUrl, map.value ( "error" ).toString() );
            } else {
                emit mediumUploaded ( localUrl, map.value ( "url" ).toString() );
            }
        } else {
            kDebug() << "Parse error: " << qobject_cast<KIO::StoredTransferJob*> ( job )->data();
            emit uploadingFailed ( localUrl, i18n ( "Malformed response" ) );
        }
    }
}
