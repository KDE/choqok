/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "posterous.h"

#include <QDebug>
#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

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
    QString pass = Choqok::PasswordManager::self()->readPassword(QStringLiteral("posterous_%1").arg(PosterousSettings::login()));
    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData(QLatin1String("customHTTPHeader"),
                     QLatin1String("Authorization: Basic ") + QLatin1String(QStringLiteral("%1:%2").arg(login).arg(pass).toUtf8().toBase64()));
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
    KIO::StoredTransferJob *job = nullptr;
    if (PosterousSettings::basic()) {
        QString login = PosterousSettings::login();
        QString pass = Choqok::PasswordManager::self()->readPassword(QStringLiteral("posterous_%1").arg(PosterousSettings::login()));
        QString token = getAuthToken(localUrl);
        if (!token.isEmpty()) {
            QUrl url(QLatin1String("http://posterous.com/api/2/users/me/sites/primary/posts"));
            QMap<QString, QByteArray> formdata;
            formdata[QLatin1String("post[title]")] = QByteArray();
            formdata[QLatin1String("post[body]")] = QByteArray();
            formdata[QLatin1String("autopost")] = "0";
            formdata[QLatin1String("source")] = QCoreApplication::applicationName().toLatin1();
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
            job->addMetaData(QLatin1String("customHTTPHeader"),
                             QLatin1String("Authorization: Basic ") +
                             QLatin1String(QStringLiteral("%1:%2").arg(login).arg(pass).toUtf8().toBase64()));
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
        formdata[QLatin1String("source")] = QCoreApplication::applicationName().toLatin1();
        formdata[QLatin1String("sourceLink")] = "https://choqok.kde.org/";

        QMap<QString, QByteArray> mediafile;
        mediafile[QLatin1String("name")] = "media";
        mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
        mediafile[QLatin1String("mediumType")] = mediumType;
        mediafile[QLatin1String("medium")] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);

        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        QUrl requrl(QLatin1String("https://api.twitter.com/1/account/verify_credentials.json"));
        QByteArray credentials = acc->oauthInterface()->authorizationHeader(requrl, QNetworkAccessManager::GetOperation);

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
    connect(job, &KIO::StoredTransferJob::result, this, &Posterous::slotUpload);
    job->start();
}

void Posterous::slotUpload(KJob *job)
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
