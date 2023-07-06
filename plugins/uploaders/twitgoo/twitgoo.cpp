/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#include "twitgoo.h"

#include <QDebug>
#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

#include "accountmanager.h"
#include "mediamanager.h"
#include "passwordmanager.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

#include "twitgoosettings.h"

K_PLUGIN_CLASS_WITH_JSON(Twitgoo, "choqok_twitgoo.json")

Twitgoo::Twitgoo(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader(QLatin1String("choqok_twitgoo"), parent)
{
}

Twitgoo::~Twitgoo()
{
}

void Twitgoo::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    TwitgooSettings::self()->load();
    QString alias = TwitgooSettings::alias();
    if (alias.isEmpty()) {
        qCritical() << "No account to use";
        Q_EMIT uploadingFailed(localUrl, i18n("There is no Twitter account configured to use."));
        return;
    }
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *> (Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return;
    }

    QUrl url(QLatin1String("http://twitgoo.com/api/upload"));

    QMap<QString, QByteArray> formdata;
    formdata[QLatin1String("source")] = QCoreApplication::applicationName().toLatin1();
    formdata[QLatin1String("format")] = "json";

    QMap<QString, QByteArray> mediafile;
    mediafile[QLatin1String("name")] = "media";
    mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
    mediafile[QLatin1String("mediumType")] = mediumType;
    mediafile[QLatin1String("medium")] = medium;
    QList< QMap<QString, QByteArray> > listMediafiles;
    listMediafiles.append(mediafile);

    QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("X-Auth-Service-Provider: https://api.twitter.com/1/account/verify_credentials.json"));
    QUrl requrl(QLatin1String("https://api.twitter.com/1/account/verify_credentials.json"));
    QByteArray credentials = acc->oauthInterface()->authorizationHeader(requrl, QNetworkAccessManager::GetOperation);
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("X-Verify-Credentials-Authorization: ") + QLatin1String(credentials));
    if (!job) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData(QStringLiteral("content-type"),
                     QStringLiteral("Content-Type: multipart/form-data; boundary=AaB03x"));
    mUrlMap[job] = localUrl;
    connect(job, &KIO::StoredTransferJob::result, this, &Twitgoo::slotUpload);
    job->start();
}

void Twitgoo::slotUpload(KJob *job)
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
            QVariantMap map = json.toVariant().toMap();
            if (map.value(QLatin1String("status")) == QLatin1String("fail")) {
                QVariantMap err = map.value(QLatin1String("err")).toMap();
                Q_EMIT uploadingFailed(localUrl, err.value(QLatin1String("err_msg")).toString());
            } else if (map.value(QLatin1String("status")) == QLatin1String("ok")) {
                TwitgooSettings::self()->load();
                QString val = TwitgooSettings::directLink() ? QLatin1String("imageurl") : QLatin1String("mediaurl");
                Q_EMIT mediumUploaded(localUrl, map.value(val).toString());
            }
        } else {
            Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
            qWarning() << "Parse error:" << stj->data();
        }
    }
}

#include "moc_twitgoo.cpp"
#include "twitgoo.moc"
