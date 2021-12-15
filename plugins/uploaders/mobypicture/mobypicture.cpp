/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mobypicture.h"

#include <QDebug>
#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

#include "accountmanager.h"
#include "mediamanager.h"
#include "passwordmanager.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

#include "mobypicturesettings.h"

K_PLUGIN_FACTORY_WITH_JSON(MobypictureFactory, "choqok_mobypicture.json",
                           registerPlugin < Mobypicture > ();)

Mobypicture::Mobypicture(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader(QLatin1String("choqok_mobypicture"), parent)
{
}

Mobypicture::~Mobypicture()
{
}

void Mobypicture::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    MobypictureSettings::self()->load();
    KIO::StoredTransferJob *job = nullptr;
    QByteArray apiKey = "85LUKv3w6luUF6Pa";
    if (MobypictureSettings::oauth()) {
        QString alias = MobypictureSettings::alias();
        if (alias.isEmpty()) {
            qCritical() << "No account to use";
            Q_EMIT uploadingFailed(localUrl, i18n("There is no Twitter account configured to use."));
            return;
        }
        TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *> (Choqok::AccountManager::self()->findAccount(alias));
        if (!acc) {
            return;
        }

        QUrl url(QLatin1String("https://api.mobypicture.com/2.0/upload"));

        QMap<QString, QByteArray> formdata;
        formdata[QLatin1String("key")] = apiKey;
        formdata[QLatin1String("message")] = QString().toUtf8();

        QMap<QString, QByteArray> mediafile;
        mediafile[QLatin1String("name")] = "media";
        mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
        mediafile[QLatin1String("mediumType")] = mediumType;
        mediafile[QLatin1String("medium")] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);

        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        QUrl requrl(QLatin1String("https://api.twitter.com/1/account/verify_credentials.json"));
        QByteArray credentials = acc->oauthInterface()->authorizationHeader(requrl, QNetworkAccessManager::GetOperation);

        QString cHeader = QLatin1String("X-Verify-Credentials-Authorization: ") +  QLatin1String(credentials) + QLatin1String("\r\n");
        cHeader.append(QLatin1String("X-Auth-Service-Provider: https://api.twitter.com/1/account/verify_credentials.json"));
        job->addMetaData(QLatin1String("customHTTPHeader"), cHeader);
    } else if (MobypictureSettings::basic()) {
        QUrl url(QLatin1String("https://api.mobypicture.com"));
        QString login = MobypictureSettings::login();
        QString pass = Choqok::PasswordManager::self()->readPassword(QStringLiteral("mobypicture_%1")
                       .arg(MobypictureSettings::login()));
        QMap<QString, QByteArray> formdata;
        formdata[QLatin1String("k")] = apiKey;
        formdata[QLatin1String("u")] = login.toUtf8();
        formdata[QLatin1String("p")] = pass.toUtf8();
        formdata[QLatin1String("s")] = "none";
        formdata[QLatin1String("format")] = "json";

        QMap<QString, QByteArray> mediafile;
        mediafile[QLatin1String("name")] = "i";
        mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
        mediafile[QLatin1String("mediumType")] = mediumType;
        mediafile[QLatin1String("medium")] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);
        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        job->addMetaData(QLatin1String("Authorization"),
                         QLatin1String("Basic ") + QLatin1String(QStringLiteral("%1:%2").arg(login).arg(pass).toUtf8().toBase64()));
    }

    if (!job) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData(QLatin1String("content-type"),
                     QLatin1String("Content-Type: multipart/form-data; boundary=AaB03x"));
    mUrlMap[job] = localUrl;
    connect(job, &KIO::StoredTransferJob::result, this, &Mobypicture::slotUpload);
    job->start();
}

void Mobypicture::slotUpload(KJob *job)
{
    QUrl localUrl = mUrlMap.take(job);
    if (job->error()) {
        qCritical() << "Job Error:" << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
        //qDebug() << stj->data();
        const QJsonDocument json = QJsonDocument::fromJson(stj->data());
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();
            if (MobypictureSettings::oauth()) {
                if (map.contains(QLatin1String("errors"))) {
                    QVariantMap err = map.value(QLatin1String("errors")).toMap();
                    Q_EMIT uploadingFailed(localUrl, err.value(QLatin1String("message")).toString());
                } else if (map.contains(QLatin1String("media"))) {
                    QVariantMap media = map.value(QLatin1String("media")).toMap();
                    Q_EMIT mediumUploaded(localUrl, media.value(QLatin1String("mediaurl")).toString());
                }
            }
            if (MobypictureSettings::basic()) {
                if (map.value(QLatin1String("result")) == QLatin1String("0") &&  map.contains(QLatin1String("url"))) {
                    Q_EMIT mediumUploaded(localUrl, map.value(QLatin1String("url")).toString());
                } else {
                    Q_EMIT uploadingFailed(localUrl, map.value(QLatin1String("message")).toString());
                }
            }
        } else {
            Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
            qWarning() << "Parse error:" << stj->data();
        }
    }
}

#include "mobypicture.moc"
