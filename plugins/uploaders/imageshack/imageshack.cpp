/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "imageshack.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "mediamanager.h"

const static QString apiKey = QLatin1String("ZMWLXQBOfb570310607355f90c601148a3203f0f");

K_PLUGIN_CLASS_WITH_JSON(ImageShack, "choqok_imageshack.json")

ImageShack::ImageShack(QObject *parent, const QList<QVariant> &)
    : Choqok::Uploader(QLatin1String("choqok_imageshack"), parent)
{
}

ImageShack::~ImageShack()
{
}

void ImageShack::upload(const QUrl &localUrl, const QByteArray &medium, const QByteArray &mediumType)
{
    if (!mediumType.startsWith(QByteArray("image/"))) {
        Q_EMIT uploadingFailed(localUrl, i18n("Just supporting image uploading"));
        return;
    }
    QUrl url(QLatin1String("https://www.imageshack.us/upload_api.php"));
    QMap<QString, QByteArray> formdata;
    formdata[QLatin1String("key")] = apiKey.toLatin1();
    formdata[QLatin1String("rembar")] = "1";

    QMap<QString, QByteArray> mediafile;
    mediafile[QLatin1String("name")] = "fileupload";
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
    connect(job, &KIO::StoredTransferJob::result, this, &ImageShack::slotUpload);
    job->start();
}

void ImageShack::slotUpload(KJob *job)
{
    QUrl localUrl = mUrlMap.take(job);
    if (job->error()) {
        qCritical() << "Job Error:" << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
        QDomDocument doc;
        doc.setContent(stj->data());
        if (doc.firstChild().isNull()) {
            qWarning() << "Malformed response:" << stj->data();
            return;
        }
        QDomElement root = doc.documentElement();
        if (root.tagName() == QLatin1String("imginfo")) {
            QDomNode node = root.firstChild();
            while (!node.isNull()) {
                QDomElement elm = node.toElement();
                if (elm.tagName() == QLatin1String("links")) {
                    QDomNode node2 = node.firstChild();
                    while (!node2.isNull()) {
                        QDomElement elm2 = node2.toElement();
                        if (elm2.tagName() == QLatin1String("yfrog_link")) {
                            Q_EMIT mediumUploaded(localUrl, elm2.text());
                            return;
                        }
                        node2 = node2.nextSibling();
                    }
                }
                node = node.nextSibling();
            }
        } else {
            if (root.tagName() == QLatin1String("links")) {
                QDomNode node = root.firstChild();
                if (!node.isNull()) {
                    if (node.toElement().tagName() == QLatin1String("error")) {
                        Q_EMIT uploadingFailed(localUrl, node.toElement().text());
                        return;
                    }
                }
            }
        }
        Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
        qWarning() << "Response not detected:" << stj->data();
    }
}

#include "imageshack.moc"
