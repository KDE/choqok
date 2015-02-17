/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#include "mobypicture.h"

#include <QDomDocument>

#include <KAboutData>
#include <KAction>
#include <KActionCollection>
#include <KGenericFactory>
#include <KIO/Job>
#include <KIO/NetAccess>

#include <QtOAuth/QtOAuth>

#include <qjson/parser.h>
#include "choqokdebug.h"

#include "accountmanager.h"
#include "mediamanager.h"
#include "passwordmanager.h"

#include "twitterapihelper/twitterapiaccount.h"
#include "twitterapihelper/twitterapimicroblog.h"

#include "mobypicturesettings.h"

K_PLUGIN_FACTORY ( MyPluginFactory, registerPlugin < Mobypicture > (); )
K_EXPORT_PLUGIN ( MyPluginFactory ( "choqok_mobypicture" ) )

Mobypicture::Mobypicture ( QObject* parent, const QList<QVariant>& )
        : Choqok::Uploader ( MyPluginFactory::componentData(), parent )
{

}

Mobypicture::~Mobypicture()
{

}

void Mobypicture::upload ( const QUrl &localUrl, const QByteArray& medium, const QByteArray& mediumType )
{
    MobypictureSettings::self()->readConfig();
    KIO::StoredTransferJob *job = 0;
    QByteArray apiKey = "85LUKv3w6luUF6Pa";
    if ( MobypictureSettings::oauth() ) {
        QString alias = MobypictureSettings::alias();
        if ( alias.isEmpty() ) {
            qCritical() << "No account to use";
            Q_EMIT uploadingFailed ( localUrl, i18n ( "There is no Twitter account configured to use." ) );
            return;
        }
        TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *> ( Choqok::AccountManager::self()->findAccount ( alias ) );
        if ( !acc )
            return;

        QUrl url ( "http://api.mobypicture.com/2.0/upload" );

        QMap<QString, QByteArray> formdata;
        formdata["key"] = apiKey;
        formdata["message"] = QString().toUtf8();

        QMap<QString, QByteArray> mediafile;
        mediafile["name"] = "media";
        mediafile["filename"] = localUrl.fileName().toUtf8();
        mediafile["mediumType"] = mediumType;
        mediafile["medium"] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append ( mediafile );

        QByteArray data = Choqok::MediaManager::createMultipartFormData ( formdata, listMediafiles );

        job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
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
    } else if ( MobypictureSettings::basic() ) {
        QUrl url ( "http://api.mobypicture.com" );
        QString login = MobypictureSettings::login();
        QString pass = Choqok::PasswordManager::self()->readPassword ( QString ( "mobypicture_%1" )
                       .arg ( MobypictureSettings::login() ) );
        QMap<QString, QByteArray> formdata;
        formdata["k"] = apiKey;
        formdata["u"] = login.toUtf8();
        formdata["p"] = pass.toUtf8();
        formdata["s"] = "none";
        formdata["format"] = "json";

        QMap<QString, QByteArray> mediafile;
        mediafile["name"] = "i";
        mediafile["filename"] = localUrl.fileName().toUtf8();
        mediafile["mediumType"] = mediumType;
        mediafile["medium"] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append ( mediafile );
        QByteArray data = Choqok::MediaManager::createMultipartFormData ( formdata, listMediafiles );

        job = KIO::storedHttpPost ( data, url, KIO::HideProgressInfo ) ;
        job->addMetaData ( "Authorization", "Basic " + QString ( login + ':' + pass ).toUtf8().toBase64() );
    }

    if ( !job ) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData ( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
    mUrlMap[job] = localUrl;
    connect ( job, SIGNAL ( result ( KJob* ) ),
              SLOT ( slotUpload ( KJob* ) ) );
    job->start();
}

void Mobypicture::slotUpload ( KJob* job )
{
    qCDebug(CHOQOK);
    QUrl localUrl = mUrlMap.take ( job );
    if ( job->error() ) {
        qCritical() << "Job Error: " << job->errorString();
        Q_EMIT uploadingFailed ( localUrl, job->errorString() );
        return;
    } else {
        qCDebug(CHOQOK) << qobject_cast<KIO::StoredTransferJob*> ( job )->data();
        QJson::Parser p;
        bool ok;
        QVariant res = p.parse ( qobject_cast<KIO::StoredTransferJob*> ( job )->data(), &ok );
        if ( ok ) {
            QVariantMap map = res.toMap();
            if ( MobypictureSettings::oauth() ) {
                if ( map.contains ( "errors" ) ) {
                    QVariantMap err = map.value ( "errors" ).toMap();
                    Q_EMIT uploadingFailed ( localUrl, err.value ( "message" ).toString() );
                } else if ( map.contains ( "media" ) ) {
                    QVariantMap media = map.value ( "media" ).toMap();
                    Q_EMIT mediumUploaded ( localUrl, media.value ( "mediaurl" ).toString() );
                }
            }
            if ( MobypictureSettings::basic() ) {
                if ( map.value ( "result" ) == "0" &&  map.contains ( "url" ) ) {
                    Q_EMIT mediumUploaded ( localUrl, map.value ( "url" ).toString() );
                } else {
                    Q_EMIT uploadingFailed ( localUrl, map.value ( "message" ).toString() );
                }
            }
        } else {
            qCDebug(CHOQOK) << "Parse error: " << qobject_cast<KIO::StoredTransferJob*> ( job )->data();
            Q_EMIT uploadingFailed ( localUrl, i18n ( "Malformed response" ) );
        }
    }
}
