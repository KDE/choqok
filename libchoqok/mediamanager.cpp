/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mediamanager.h"

#include <QApplication>
#include <QHash>
#include <QIcon>
#include <QMimeDatabase>

#include <KEmoticons>
#include <KEmoticonsTheme>
#include <KImageCache>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "libchoqokdebug.h"
#include "pluginmanager.h"
#include "uploader.h"

namespace Choqok
{

class MediaManager::Private
{
public:
    Private()
        : emoticons(KEmoticons().theme()), cache(QLatin1String("choqok-userimages"), 30000000), uploader(nullptr)
    {}
    KEmoticonsTheme emoticons;
    KImageCache cache;
    QHash<KJob *, QUrl> queue;
    QPixmap defaultImage;
    Uploader *uploader;
};

MediaManager::MediaManager()
    : QObject(qApp), d(new Private)
{
    d->defaultImage = QIcon::fromTheme(QLatin1String("image-loading")).pixmap(48);
}

MediaManager::~MediaManager()
{
    delete d;
    mSelf = nullptr;
}

MediaManager *MediaManager::mSelf = nullptr;

MediaManager *MediaManager::self()
{
    if (!mSelf) {
        mSelf = new MediaManager;
    }
    return mSelf;
}

QPixmap &MediaManager::defaultImage()
{
    return d->defaultImage;
}

QString MediaManager::parseEmoticons(const QString &text)
{
    return d->emoticons.parseEmoticons(text, KEmoticonsTheme::DefaultParse, QStringList() << QLatin1String("(e)"));
}

QPixmap MediaManager::fetchImage(const QUrl &remoteUrl, ReturnMode mode /*= Sync*/)
{
    QPixmap p;
    if (d->cache.findPixmap(remoteUrl.toDisplayString(), &p)) {
        Q_EMIT imageFetched(remoteUrl, p);
    } else if (mode == Async) {
        if (d->queue.values().contains(remoteUrl)) {
            ///The file is on the way, wait to download complete.
            return p;
        }
        KIO::StoredTransferJob *job = KIO::storedGet(remoteUrl, KIO::NoReload, KIO::HideProgressInfo) ;
        if (!job) {
            qCCritical(CHOQOK) << "Cannot create a FileCopyJob!";
            QString errMsg = i18n("Cannot create a KDE Job. Please check your installation.");
            Q_EMIT fetchError(remoteUrl, errMsg);
            return p;
        }
        d->queue.insert(job, remoteUrl);
        connect(job, &KIO::StoredTransferJob::result, this, &MediaManager::slotImageFetched);
        job->start();
    }
    return p;
}

void MediaManager::slotImageFetched(KJob *job)
{
    KIO::StoredTransferJob *baseJob = qobject_cast<KIO::StoredTransferJob *>(job);
    QUrl remote = d->queue.value(job);
    d->queue.remove(job);

    int responseCode = 0;
    if (baseJob->metaData().contains(QStringLiteral("responsecode"))) {
        responseCode = baseJob->queryMetaData(QStringLiteral("responsecode")).toInt();
    }

    if (job->error() || (responseCode > 399 && responseCode < 600)) {
        qCCritical(CHOQOK) << "Job error:" << job->error() << "\t" << job->errorString();
        qCCritical(CHOQOK) << "HTTP response code" << responseCode;
        QString errMsg = i18n("Cannot download image from %1.", job->errorString());
        Q_EMIT fetchError(remote, errMsg);
    } else {
        QPixmap p;
        if (p.loadFromData(baseJob->data())) {
            d->cache.insertPixmap(remote.toDisplayString(), p);
            Q_EMIT imageFetched(remote, p);
        } else {
            qCCritical(CHOQOK) << "Cannot parse reply from " << baseJob->url().toDisplayString();
            Q_EMIT fetchError(remote, i18n("The request failed. Cannot get image file."));
        }
    }
}

void MediaManager::clearImageCache()
{
    d->cache.clear();
}

QPixmap MediaManager::convertToGrayScale(const QPixmap &pic)
{
    QImage result = pic.toImage();
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            int pixel = result.pixel(x, y);
            int gray = qGray(pixel);
            int alpha = qAlpha(pixel);
            result.setPixel(x, y, qRgba(gray, gray, gray, alpha));
        }
    }
    return QPixmap::fromImage(result);
}

void MediaManager::uploadMedium(const QUrl &localUrl, const QString &pluginId)
{
    QString pId = pluginId;
    if (pId.isEmpty()) {
        pId = Choqok::BehaviorSettings::lastUsedUploaderPlugin();
    }
    if (pId.isEmpty()) {
        Q_EMIT mediumUploadFailed(localUrl, i18n("No pluginId specified, And last used plugin is null."));
        return;
    }
    if (!d->uploader) {
        Plugin *plugin = PluginManager::self()->loadPlugin(pId);
        d->uploader = qobject_cast<Uploader *>(plugin);
    } else if (d->uploader->pluginId() != pId) {
//         qCDebug(CHOQOK)<<"CREATING A NEW UPLOADER OBJECT";
        PluginManager::self()->unloadPlugin(d->uploader->pluginId());
        Plugin *plugin = PluginManager::self()->loadPlugin(pId);
        d->uploader = qobject_cast<Uploader *>(plugin);
    }
    if (!d->uploader) {
        return;
    }
    KIO::StoredTransferJob *picJob = KIO::storedGet(localUrl, KIO::Reload, KIO::HideProgressInfo);
    picJob->exec();
    if (picJob->error()) {
        qCritical() << "Job error:" << picJob->errorString();
        KMessageBox::detailedError(UI::Global::mainWindow(), i18n("Uploading medium failed: cannot read the medium file."),
                                   picJob->errorString());
        return;
    }
    const QByteArray picData = picJob->data();
    if (picData.count() == 0) {
        qCritical() << "Cannot read the media file, please check if it exists.";
        KMessageBox::error(UI::Global::mainWindow(), i18n("Uploading medium failed: cannot read the medium file."));
        return;
    }
    connect(d->uploader, &Uploader::mediumUploaded, this, &MediaManager::mediumUploaded);
    connect(d->uploader, &Uploader::uploadingFailed, this, &MediaManager::mediumUploadFailed);
    const QMimeDatabase db;
    d->uploader->upload(localUrl, picData, db.mimeTypeForUrl(localUrl).name().toLocal8Bit());
}

QByteArray MediaManager::createMultipartFormData(const QMap< QString, QByteArray > &formdata,
        const QList< QMap< QString, QByteArray > > &mediaFiles)
{
    QByteArray newLine("\r\n");
    QString formHeader(QLatin1String(newLine) + QLatin1String("Content-Disposition: form-data; name=\"%1\""));
    QByteArray header("--AaB03x");
    QByteArray footer("--AaB03x--");
    QString fileHeader(QLatin1String(newLine) + QLatin1String("Content-Disposition: file; name=\"%1\"; filename=\"%2\""));
    QByteArray data;

    data.append(header);

    if (!mediaFiles.isEmpty()) {
        QList< QMap< QString, QByteArray > >::const_iterator it1 = mediaFiles.constBegin();
        QList< QMap< QString, QByteArray > >::const_iterator endIt1 = mediaFiles.constEnd();
        for (; it1 != endIt1; ++it1) {
            data.append(fileHeader.arg(QLatin1String(it1->value(QLatin1String("name")).data())).arg(QLatin1String(it1->value(QLatin1String("filename")).data())).toUtf8());
            data.append(newLine + "Content-Type: " + it1->value(QLatin1String("mediumType")));
            data.append(newLine);
            data.append(newLine + it1->value(QLatin1String("medium")));
        }
    }

    for (const QString &key: formdata.keys()) {
        data.append(newLine);
        data.append(header);
        data.append(formHeader.arg(key).toLatin1());
        data.append(newLine);
        data.append(newLine + formdata.value(key));
    }
    data.append(newLine);
    data.append(footer);

    return data;
}

}

#include "moc_mediamanager.cpp"
