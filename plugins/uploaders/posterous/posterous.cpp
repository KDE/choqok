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

#include <KIO/Job>
#include <KIO/NetAccess>
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
    : Choqok::Uploader("choqok_posterous", parent)
{
}

Posterous::~Posterous()
{
}

QString Posterous::getAuthToken(const QUrl &localUrl)
{
    QUrl url("http://posterous.com/api/2/auth/token");
    QString login = PosterousSettings::login();
    QString pass = Choqok::PasswordManager::self()->readPassword(QString("posterous_%1").arg(PosterousSettings::login()));
    KIO::Job *job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    QByteArray data;
    QString token;
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: Basic ") + QString("%1:%2").arg(login).arg(pass).toUtf8().toBase64());
    if (KIO::NetAccess::synchronousRun(job, 0, &data)) {
        const QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isNull()) {
            QVariantMap map = json.toVariant().toMap();
            if (map.contains("api_token")) {
                QString tkn = map.value("api_token").toString();
                return tkn;
            } else {
                Q_EMIT uploadingFailed(localUrl, map.value("error").toString());
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
        QString pass = Choqok::PasswordManager::self()->readPassword(QString("posterous_%1").arg(PosterousSettings::login()));
        QString token = getAuthToken(localUrl);
        if (!token.isEmpty()) {
            QUrl url("http://posterous.com/api/2/users/me/sites/primary/posts");
            QMap<QString, QByteArray> formdata;
            formdata["post[title]"] = QByteArray();
            formdata["post[body]"] = QByteArray();
            formdata["autopost"] = "0";
            formdata["source"] = "Choqok";
            formdata["api_token"] = token.toUtf8();

            QMap<QString, QByteArray> mediafile;
            mediafile["name"] = "media";
            mediafile["filename"] = localUrl.fileName().toUtf8();
            mediafile["mediumType"] = mediumType;
            mediafile["medium"] = medium;
            QList< QMap<QString, QByteArray> > listMediafiles;
            listMediafiles.append(mediafile);

            QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);
            job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
            job->addMetaData(QStringLiteral("customHTTPHeader"),
                             QStringLiteral("Authorization: Basic ") +
                             QString("%1:%2").arg(login).arg(pass).toUtf8().toBase64());
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

        QUrl url("http://posterous.com/api2/upload.json");

        QMap<QString, QByteArray> formdata;
        formdata["source"] = "Choqok";
        formdata["sourceLink"] = "http://choqok.gnufolks.org/";

        QMap<QString, QByteArray> mediafile;
        mediafile["name"] = "media";
        mediafile["filename"] = localUrl.fileName().toUtf8();
        mediafile["mediumType"] = mediumType;
        mediafile["medium"] = medium;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);

        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        QOAuth::ParamMap params;
        QString requrl = "https://api.twitter.com/1/account/verify_credentials.json";
        QByteArray credentials = acc->oauthInterface()->createParametersString(requrl,
                                 QOAuth::GET, acc->oauthToken(),
                                 acc->oauthTokenSecret(),
                                 QOAuth::HMAC_SHA1,
                                 params, QOAuth::ParseForHeaderArguments);

        QString cHeader = "X-Verify-Credentials-Authorization: " +  QString(credentials) + "\r\n";
        cHeader.append("X-Auth-Service-Provider: https://api.twitter.com/1/account/verify_credentials.json");
        job->addMetaData("customHTTPHeader", cHeader);
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
            if (map.contains("error")) {
                Q_EMIT uploadingFailed(localUrl, map.value("error").toString());
            } else {
                if (PosterousSettings::oauth()) {
                    Q_EMIT mediumUploaded(localUrl, map.value("url").toString());
                }
                if (PosterousSettings::basic()) {
                    Q_EMIT mediumUploaded(localUrl, map.value("full_url").toString());
                }
            }
        } else {
            Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
            qWarning() << "Parse error:" << stj->data();
        }
    }
}

#include "posterous.moc"