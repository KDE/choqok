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

#include "posterous.h"

#include <QDebug>
#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

#include <QtOAuth/QtOAuth>

#include "accountmanager.h"
#include "mediamanager.h"
#include "passwordmanager.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

#include "posteroussettings.h"

K_PLUGIN_FACTORY_WITH_JSON(PosterousFactory, "choqok_posterous.json",
                           registerPlugin < Posterous > ();)

Posterous::Posterous(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader(QLatin1String("choqok_posterous"), parent)
{
}

Posterous::~Posterous()
{
}

QString Posterous::getAuthToken(const QUrl &localUrl)
{
    QUrl url(QLatin1String("http://posterous.com/api/2/auth/token"));
    QString login = PosterousSettings::login();
    QString pass = Choqok::PasswordManager::self()->readPassword(QString::fromLatin1("posterous_%1").arg(PosterousSettings::login()));
    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    QString token;
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: Basic ") + QLatin1String(QString::fromLatin1("%1:%2").arg(login).arg(pass).toUtf8().toBase64()));
    job->exec();
    if (!job->error()) {
        const QByteArray data = job->data();
        const QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isNull()) {
            QVariantMap map = json.toVariant().toMap();
            if (map.contains(QLatin1String("api_token"))) {
                QString tkn = map.value(QLatin1String("api_token")).toString();
                return tkn;
            } else {
                Q_EMIT uploadingFailed(localUrl, map.value(QLatin1String("error")).toString());
                qWarning() << "Parse error:" << data;
            }
        }
    } else {
        qCritical() << "Job error:" << job->errorString();
    }

    return QString();
}

void Posterous::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    PosterousSettings::self()->load();
    KIO::StoredTransferJob *job = 0;
    if (PosterousSettings::basic()) {
        QString login = PosterousSettings::login();
        QString pass = Choqok::PasswordManager::self()->readPassword(QString::fromLatin1("posterous_%1").arg(PosterousSettings::login()));
        QString token = getAuthToken(localUrl);
        if (!token.isEmpty()) {
            QUrl url(QLatin1String("http://posterous.com/api/2/users/me/sites/primary/posts"));
            QMap<QString, QByteArray> formdata;
            formdata[QLatin1String("post[title]")] = QByteArray();
            formdata[QLatin1String("post[body]")] = QByteArray();
            formdata[QLatin1String("autopost")] = "0";
            formdata[QLatin1String("source")] = "Choqok";
            formdata[QLatin1String("api_token")] = token.toUtf8();

            QMap<QString, QByteArray> mediafile;
            mediafile[QLatin1String("name")] = "media";
            mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
            mediafile[QLatin1String("mediumType")] = mediumType;
            mediafile[QLatin1String("medium")] = medium;
            QList< QMap<QString, QByteArray> > listMediafiles;
            listMediafiles.append(mediafile);

            QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);
            job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
            job->addMetaData(QStringLiteral("customHTTPHeader"),
                             QStringLiteral("Authorization: Basic ") +
                             QLatin1String(QString::fromLatin1("%1:%2").arg(login).arg(pass).toUtf8().toBase64()));
        }
    } else if (PosterousSettings::oauth()) {
        QString alias = PosterousSettings::alias();
        if (alias.isEmpty()) {
            qCritical() << "No account to use";
            Q_EMIT uploadingFailed(localUrl, i18n("There is no Twitter account configured to use."));
            return;
        }
        TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *> (Choqok::AccountManager::self()->findAccount(alias));
        if (!acc) {
            return;
        }

        QUrl url(QLatin1String("http://posterous.com/api2/upload.json"));

        QMap<QString, QByteArray> formdata;
        formdata[QLatin1String("source")] = "Choqok";
        formdata[QLatin1String("sourceLink")] = "http://choqok.gnufolks.org/";

        QMap<QString, QByteArray> mediafile;
        mediafile[QLatin1String("name")] = "media";
        mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
        mediafile[QLatin1String("mediumType")] = mediumType;
        mediafile[QLatin1String("medium")] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);

        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        QOAuth::ParamMap params;
        QString requrl = QLatin1String("https://api.twitter.com/1/account/verify_credentials.json");
        QByteArray credentials = acc->oauthInterface()->createParametersString(requrl,
                                 QOAuth::GET, acc->oauthToken(),
                                 acc->oauthTokenSecret(),
                                 QOAuth::HMAC_SHA1,
                                 params, QOAuth::ParseForHeaderArguments);

        QString cHeader = QLatin1String("X-Verify-Credentials-Authorization: ") +  QLatin1String(credentials) + QLatin1String("\r\n");
        cHeader.append(QLatin1String("X-Auth-Service-Provider: https://api.twitter.com/1/account/verify_credentials.json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), cHeader);
    }
    if (!job) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("content-type"),
                     QStringLiteral("Content-Type: multipart/form-data; boundary=AaB03x"));
    mUrlMap[job] = localUrl;
    connect(job, SIGNAL(result(KJob*)),
            SLOT(slotUpload(KJob*)));
    job->start();
}

void Posterous::slotUpload(KJob *job)
{
    QUrl localUrl = mUrlMap.take(job);
    if (job->error()) {
        qCritical() << "Job Error: " << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
        //qDebug() << stj->data();
        const QJsonDocument json = QJsonDocument::fromJson(stj->data());
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();
            if (map.contains(QLatin1String("error"))) {
                Q_EMIT uploadingFailed(localUrl, map.value(QLatin1String("error")).toString());
            } else {
                if (PosterousSettings::oauth()) {
                    Q_EMIT mediumUploaded(localUrl, map.value(QLatin1String("url")).toString());
                }
                if (PosterousSettings::basic()) {
                    Q_EMIT mediumUploaded(localUrl, map.value(QLatin1String("full_url")).toString());
                }
            }
        } else {
            Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
            qWarning() << "Parse error:" << stj->data();
        }
    }
}

#include "posterous.moc"