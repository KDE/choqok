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

#include <QtOAuth/QtOAuth>

#include "twitgoosettings.h"

K_PLUGIN_FACTORY_WITH_JSON(TwitgooFactory, "choqok_twitgoo.json",
                           registerPlugin < Twitgoo > ();)

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
    QOAuth::ParamMap params;
    QString requrl = QLatin1String("https://api.twitter.com/1/account/verify_credentials.json");
    QByteArray credentials = acc->oauthInterface()->createParametersString(requrl,
                             QOAuth::GET, acc->oauthToken(),
                             acc->oauthTokenSecret(),
                             QOAuth::HMAC_SHA1,
                             params, QOAuth::ParseForHeaderArguments);
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("X-Verify-Credentials-Authorization: ") + QLatin1String(credentials));
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

#include "twitgoo.moc"