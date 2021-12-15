/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "flickr.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

#include "mediamanager.h"
#include "passwordmanager.h"

#include "flickrsettings.h"

const static QString apiKey = QLatin1String("13f602e6e705834d8cdd5dd2ccb19651");
const static QString apiSecret = QLatin1String("98c89dbe39ae3bea");
const static QString apiKeSec = apiSecret + QLatin1String("api_key") + apiKey;

K_PLUGIN_FACTORY_WITH_JSON(FlickrFactory, "choqok_flickr.json",
                           registerPlugin < Flickr > ();)

Flickr::Flickr(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader(QLatin1String("choqok_flickr"), parent)
{
}

Flickr::~Flickr()
{
}

void Flickr::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    QUrl url(QLatin1String("https://api.flickr.com/services/upload/"));
    FlickrSettings::self()->load();
    QString token = Choqok::PasswordManager::self()->readPassword(QStringLiteral("flickr_%1")
                    .arg(FlickrSettings::username()));
    QMap<QString, QByteArray> formdata;
    formdata[QLatin1String("api_key")] = apiKey.toUtf8();
    formdata[QLatin1String("auth_token")] = token.toUtf8();

    QString preSign;
    if (FlickrSettings::hidefromsearch()) {
        formdata[QLatin1String("hidden")] = QByteArray("2");
        preSign.append(QLatin1String("hidden2"));
    } else {
        formdata[QLatin1String("hidden")] = QByteArray("1");
        preSign.append(QLatin1String("hidden1"));
    }

    if (FlickrSettings::forprivate()) {

        if (FlickrSettings::forfamily()) {
            formdata[QLatin1String("is_family")] = QByteArray("1");
            preSign.append(QLatin1String("is_family1"));
        }

        if (FlickrSettings::forfriends()) {
            formdata[QLatin1String("is_friend")] = QByteArray("1");
            preSign.append(QLatin1String("is_friend1"));
        }
        formdata[QLatin1String("is_public")] = QByteArray("0");
        preSign.append(QLatin1String("is_public0"));
    } else if (FlickrSettings::forpublic()) {
        formdata[QLatin1String("is_public")] = QByteArray("1");
        preSign.append(QLatin1String("is_public1"));
    }

    if (FlickrSettings::safe()) {
        formdata[QLatin1String("safety_level")] = QByteArray("1");
        preSign.append(QLatin1String("safety_level1"));
    }
    if (FlickrSettings::moderate()) {
        formdata[QLatin1String("safety_level")] = QByteArray("2");
        preSign.append(QLatin1String("safety_level2"));
    }
    if (FlickrSettings::restricted()) {
        formdata[QLatin1String("safety_level")] = QByteArray("3");
        preSign.append(QLatin1String("safety_level3"));
    }

    formdata[QLatin1String("api_sig")] = createSign("auth_token" + token.toUtf8() + preSign.toUtf8());

    QMap<QString, QByteArray> mediafile;
    mediafile[QLatin1String("name")] = "photo";
    mediafile[QLatin1String("filename")] = localUrl.fileName().toUtf8();
    mediafile[QLatin1String("mediumType")] = mediumType;
    mediafile[QLatin1String("medium")] = medium;
    QList< QMap<QString, QByteArray> > listMediafiles;
    listMediafiles.append(mediafile);

    QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
    if (!job) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: multipart/form-data; boundary=AaB03x"));
    mUrlMap[job] = localUrl;
    connect(job, &KIO::StoredTransferJob::result, this, &Flickr::slotUpload);
    job->start();
}

void Flickr::slotUpload(KJob *job)
{
    QUrl localUrl = mUrlMap.take(job);
    if (job->error()) {
        qCritical() << "Job Error:" << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        QDomDocument rep;
        QByteArray buffer = qobject_cast<KIO::StoredTransferJob *>(job)->data();
        //qDebug() << buffer;
        rep.setContent(buffer);
        QString photoId;
        QDomElement element = rep.documentElement();
        if (element.tagName() == QLatin1String("rsp")) {
            QString res;
            res = element.attribute(QLatin1String("stat") , QLatin1String("fail"));
            QDomNode node = element.firstChild();
            while (!node.isNull()) {
                QDomElement elem = node.toElement();
                if (res == QLatin1String("ok")) {

                    if (elem.tagName() == QLatin1String("photoid")) {
                        photoId = elem.text();
                    }

                    QString remUrl;
                    if (FlickrSettings::shorturl()) {
                        remUrl = QLatin1String("https://flic.kr/p/") + base58encode(photoId.toULongLong());
                    } else {
                        remUrl = QLatin1String("https://flickr.com/photos/") + FlickrSettings::nsid() + QLatin1Char('/') + photoId;
                    }

                    Q_EMIT mediumUploaded(localUrl, remUrl);
                    return;

                } else if (res == QLatin1String("fail")) {
                    QString errMsg;
                    if (elem.tagName() == QLatin1String("err")) {
                        errMsg = elem.text();
                        int errCode = elem.attribute(QLatin1String("code") , QLatin1String("0")).toInt();
                        switch (errCode) {
                        case 2:
                            errMsg = i18n("The photo required argument was missing");
                            break;
                        case 3:
                            errMsg = i18n("The file was not correctly uploaded");
                            break;
                        case 4:
                            errMsg = i18n("The file was zero bytes in length");
                            break;
                        case 5:
                            errMsg = i18n("Filetype was not recognised");
                            break;
                        case 6:
                            errMsg = i18n("The calling user has reached their monthly bandwidth limit");
                            break;
                        case 96:
                        case 97:
                            errMsg = i18n("Signature problem. Please try again later");
                            break;
                        case 98:
                        case 99:
                            /*TODO:
                             Show auth dialog
                             */
                            errMsg = i18n("Login failed. Please re-authenticate Choqok");
                            break;
                        case 105:
                            errMsg = i18n("The requested service is temporarily unavailable. Try again later");
                            break;
                        default:
                            errMsg = i18n("Unknown Error:%1. Please try again later", errCode);
                            break;
                        }
                        Q_EMIT uploadingFailed(localUrl, errMsg);
                        return;
                    }
                } else {
                    Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
                    return;
                }
                node = node.nextSibling();
            }
        } else {
            Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
            return;
        }
    }
}

QString Flickr::base58encode(quint64 num)
{
    QString alphabet = QLatin1String("123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ");
    uint baseCount = alphabet.count();
    QString encoded;
    while (num >= baseCount) {
        encoded.prepend(alphabet.at(int(num % baseCount)));
        num /= baseCount;
    }
    if (num) {
        encoded.prepend(alphabet.at(int(num)));
    }
    return encoded;
}

QByteArray Flickr::createSign(QByteArray req)
{
    return QCryptographicHash::hash(apiKeSec.toUtf8().append(req), QCryptographicHash::Md5).toHex();
}

#include "flickr.moc"
