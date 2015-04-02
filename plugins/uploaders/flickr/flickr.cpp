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

const static QString apiKey = "13f602e6e705834d8cdd5dd2ccb19651";
const static QString apiSecret = "98c89dbe39ae3bea";
const static QString apiKeSec = apiSecret + QString("api_key") + apiKey;

K_PLUGIN_FACTORY_WITH_JSON(FlickrFactory, "choqok_flickr.json",
                           registerPlugin < Flickr > ();)

Flickr::Flickr(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader("choqok_flickr", parent)
{
}

Flickr::~Flickr()
{
}

void Flickr::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    QUrl url("https://api.flickr.com/services/upload/");
    FlickrSettings::self()->load();
    QString token = Choqok::PasswordManager::self()->readPassword(QString("flickr_%1")
                    .arg(FlickrSettings::username()));
    QMap<QString, QByteArray> formdata;
    formdata["api_key"] = apiKey.toUtf8();
    formdata["auth_token"] = token.toUtf8();

    QString preSign;
    if (FlickrSettings::hidefromsearch()) {
        formdata["hidden"] = QByteArray("2");
        preSign.append("hidden2");
    } else {
        formdata["hidden"] = QByteArray("1");
        preSign.append("hidden1");
    }

    if (FlickrSettings::forprivate()) {

        if (FlickrSettings::forfamily()) {
            formdata["is_family"] = QByteArray("1");
            preSign.append("is_family1");
        }

        if (FlickrSettings::forfriends()) {
            formdata["is_friend"] = QByteArray("1");
            preSign.append("is_friend1");
        }
        formdata["is_public"] = QByteArray("0");
        preSign.append("is_public0");
    } else if (FlickrSettings::forpublic()) {
        formdata["is_public"] = QByteArray("1");
        preSign.append("is_public1");
    }

    if (FlickrSettings::safe()) {
        formdata["safety_level"] = QByteArray("1");
        preSign.append("safety_level1");
    }
    if (FlickrSettings::moderate()) {
        formdata["safety_level"] = QByteArray("2");
        preSign.append("safety_level2");
    }
    if (FlickrSettings::restricted()) {
        formdata["safety_level"] = QByteArray("3");
        preSign.append("safety_level3");
    }

    formdata["api_sig"] = createSign("auth_token" + token.toUtf8() + preSign.toUtf8());

    QMap<QString, QByteArray> mediafile;
    mediafile["name"] = "photo";
    mediafile["filename"] = localUrl.fileName().toUtf8();
    mediafile["mediumType"] = mediumType;
    mediafile["medium"] = medium;
    QList< QMap<QString, QByteArray> > listMediafiles;
    listMediafiles.append(mediafile);

    QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
    if (!job) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData("content-type", "Content-Type: multipart/form-data; boundary=AaB03x");
    mUrlMap[job] = localUrl;
    connect(job, SIGNAL(result(KJob*)),
            SLOT(slotUpload(KJob*)));
    job->start();
}

void Flickr::slotUpload(KJob *job)
{
    QUrl localUrl = mUrlMap.take(job);
    if (job->error()) {
        qCritical() << "Job Error: " << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        QDomDocument rep;
        QByteArray buffer = qobject_cast<KIO::StoredTransferJob *>(job)->data();
        //qDebug() << buffer;
        rep.setContent(buffer);
        QString photoId;
        QDomElement element = rep.documentElement();
        if (element.tagName() == "rsp") {
            QString res;
            res = element.attribute("stat" , "fail");
            QDomNode node = element.firstChild();
            while (!node.isNull()) {
                QDomElement elem = node.toElement();
                if (res == "ok") {

                    if (elem.tagName() == "photoid") {
                        photoId = elem.text();
                    }

                    QString remUrl;
                    if (FlickrSettings::shorturl()) {
                        remUrl = "https://flic.kr/p/" + base58encode(photoId.toULongLong());
                    } else {
                        remUrl = "https://flickr.com/photos/" + FlickrSettings::nsid() + '/' + photoId;
                    }

                    Q_EMIT mediumUploaded(localUrl, remUrl);
                    return;

                } else if (res == "fail") {
                    QString errMsg;
                    if (elem.tagName() == "err") {
                        errMsg = elem.text();
                        int errCode = elem.attribute("code" , "0").toInt();
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
                            errMsg = i18n("Unknown Error: %1. Please try again later", errCode);
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
    QString alphabet = "123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ";
    uint baseCount = alphabet.count();
    QString encoded;
    while (num >= baseCount) {
        encoded.prepend(alphabet.at(num % baseCount));
        num /= baseCount;
    }
    if (num) {
        encoded.prepend(alphabet.at(num));
    }
    return encoded;
}

QByteArray Flickr::createSign(QByteArray req)
{
    return QCryptographicHash::hash(apiKeSec.toUtf8().append(req), QCryptographicHash::Md5).toHex();
}

#include "flickr.moc"