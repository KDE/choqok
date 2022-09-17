/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitpic.h"

#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "accountmanager.h"
#include "mediamanager.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

#include "twitpicsettings.h"

K_PLUGIN_CLASS_WITH_JSON(Twitpic, "choqok_twitpic.json")

Twitpic::Twitpic(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader("choqok_twitpic", parent)
{
}

Twitpic::~Twitpic()
{
}

void Twitpic::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    TwitpicSettings::self()->load();
    QString alias = TwitpicSettings::alias();
    if (alias.isEmpty()) {
        qCritical() << "No account to use";
        Q_EMIT uploadingFailed(localUrl, i18n("There is no Twitter account configured to use."));
        return;
    }
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *>(Choqok::AccountManager::self()->findAccount(alias));
    if (!acc) {
        return;
    }
    ///Documentation: http://dev.twitpic.com/
    QUrl url("http://api.twitpic.com/2/upload.json");

    QMap<QString, QByteArray> formdata;
    formdata["key"] = "b66d1f2dc90b53ca1fcd75319cda0b72";

    QMap<QString, QByteArray> mediafile;
    mediafile["name"] = "media";
    mediafile["filename"] = localUrl.fileName().toUtf8();
    mediafile["mediumType"] = mediumType;
    mediafile["medium"] = medium;
    QList< QMap<QString, QByteArray> > listMediafiles;
    listMediafiles.append(mediafile);

    QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("X-Auth-Service-Provider: https://api.twitter.com/1/account/verify_credentials.json"));
    QOAuth::ParamMap params;
//     params.insert("realm","http://api.twitter.com/");
    QString requrl = "https://api.twitter.com/1/account/verify_credentials.json";
    QByteArray credentials = acc->oauthInterface()->createParametersString(requrl,
                             QOAuth::GET, acc->oauthToken(),
                             acc->oauthTokenSecret(),
                             QOAuth::HMAC_SHA1,
                             params, QOAuth::ParseForHeaderArguments);
    credentials.insert(6, "realm=\"http://api.twitter.com/\",") ;
    //qDebug()<<credentials
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("X-Verify-Credentials-Authorization: ") +
                     credentials);
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

void Twitpic::slotUpload(KJob *job)
{
    QUrl localUrl = mUrlMap.take(job);
    if (job->error()) {
        qCritical() << "Job Error:" << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        KIO::StoredTransferJob *stj = qobject_cast< KIO::StoredTransferJob * >(job);
        const QJsonDocument json = QJsonDocument::fromJson(stj->data());
        if (!json.isNull()) {
            const QVariantMap map = json.toVariant().toMap();
            if (map.contains("errors")) {
                QVariantMap err = map.value("errors").toList()[0].toMap();
                Q_EMIT uploadingFailed(localUrl, err.value("message").toString());
            } else {
                Q_EMIT mediumUploaded(localUrl, map.value("url").toString());
            }
        } else {
            qWarning() << "Parse error:" << stj->data();
            Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
        }
    }
}

#include "twitpic.moc"
