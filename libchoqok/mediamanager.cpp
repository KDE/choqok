/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "mediamanager.h"

#include <QApplication>
#include <QHash>

#include "libchoqokdebug.h"
#include <KEmoticons>
#include <KEmoticonsTheme>
#include <QIcon>
#include <KImageCache>
#include <KIO/Job>
#include <KIO/JobClasses>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMimeType>

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "pluginmanager.h"
#include "uploader.h"

namespace Choqok
{

class MediaManager::Private
{
public:
    Private()
        : emoticons(KEmoticons().theme()), cache("choqok-userimages", 30000000), uploader(0)
    {}
    KEmoticonsTheme emoticons;
    KImageCache cache;
    QHash<KJob *, QString> queue;
    QPixmap defaultImage;
    Uploader *uploader;
};

MediaManager::MediaManager()
    : QObject(qApp), d(new Private)
{
    d->defaultImage = QIcon::fromTheme("image-loading").pixmap(48);
}

MediaManager::~MediaManager()
{
    delete d;
    mSelf = 0L;
}

MediaManager *MediaManager::mSelf = 0L;

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
    return d->emoticons.parseEmoticons(text, KEmoticonsTheme::DefaultParse, QStringList() << "(e)");
}

QPixmap MediaManager::fetchImage(const QString &remoteUrl, ReturnMode mode /*= Sync*/)
{
    QPixmap p;
    if (d->cache.findPixmap(remoteUrl, &p)) {
        Q_EMIT imageFetched(remoteUrl, p);
    } else if (mode == Async) {
        if (d->queue.values().contains(remoteUrl)) {
            ///The file is on the way, wait to download complete.
            return p;
        }
        QUrl srcUrl(remoteUrl);
        KIO::Job *job = KIO::storedGet(srcUrl, KIO::NoReload, KIO::HideProgressInfo) ;
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create a FileCopyJob!";
            QString errMsg = i18n("Cannot create a KDE Job. Please check your installation.");
            Q_EMIT fetchError(remoteUrl, errMsg);
            return p;
        }
        d->queue.insert(job, remoteUrl);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotImageFetched(KJob*)));
        job->start();
    }
    return p;
}

void MediaManager::slotImageFetched(KJob *job)
{
    KIO::StoredTransferJob *baseJob = qobject_cast<KIO::StoredTransferJob *>(job);
    QString remote = d->queue.value(job);
    d->queue.remove(job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job error: " << job->error() << "\t" << job->errorString();
        QString errMsg = i18n("Cannot download image from %1.",
                              job->errorString());
        Q_EMIT fetchError(remote, errMsg);
    } else {
        QPixmap p;
        if (!baseJob->data().startsWith(QByteArray("<?xml version=\"")) &&
                p.loadFromData(baseJob->data())) {
            d->cache.insertPixmap(remote, p);
            Q_EMIT imageFetched(remote, p);
        } else {
            qCDebug(CHOQOK) << "Parse Error: \nBase Url:" << baseJob->url() << "\ndata:" << baseJob->data();
            Q_EMIT fetchError(remote, i18n("The download failed. The returned file is corrupted."));
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
    } else if (d->uploader->pluginName() != pId) {
//         qCDebug(CHOQOK)<<"CREATING A NEW UPLOADER OBJECT";
        PluginManager::self()->unloadPlugin(d->uploader->pluginName());
        Plugin *plugin = PluginManager::self()->loadPlugin(pId);
        d->uploader = qobject_cast<Uploader *>(plugin);
    }
    if (!d->uploader) {
        return;
    }
    QByteArray picData;
    KIO::TransferJob *picJob = KIO::get(localUrl, KIO::Reload, KIO::HideProgressInfo);
    if (!KIO::NetAccess::synchronousRun(picJob, 0, &picData)) {
        qCritical() << "Job error: " << picJob->errorString();
        KMessageBox::detailedError(UI::Global::mainWindow(), i18n("Uploading medium failed: cannot read the medium file."),
                                   picJob->errorString());
        return;
    }
    if (picData.count() == 0) {
        qCritical() << "Cannot read the media file, please check if it exists.";
        KMessageBox::error(UI::Global::mainWindow(), i18n("Uploading medium failed: cannot read the medium file."));
        return;
    }
    QByteArray type = KMimeType::findByUrl(localUrl, 0, true)->name().toUtf8();
    connect(d->uploader, SIGNAL(mediumUploaded(QUrl,QString)),
            this, SIGNAL(mediumUploaded(QUrl,QString)));
    connect(d->uploader, SIGNAL(uploadingFailed(QUrl,QString)),
            this, SIGNAL(mediumUploadFailed(QUrl,QString)));
    d->uploader->upload(localUrl, picData, type);
}

QByteArray MediaManager::createMultipartFormData(const QMap< QString, QByteArray > &formdata,
        const QList< QMap< QString, QByteArray > > &mediaFiles)
{
    QByteArray newLine("\r\n");
    QString formHeader(newLine + "Content-Disposition: form-data; name=\"%1\"");
    QByteArray header("--AaB03x");
    QByteArray footer("--AaB03x--");
    QString fileHeader(newLine + "Content-Disposition: file; name=\"%1\"; filename=\"%2\"");
    QByteArray data;

    data.append(header);

    if (!mediaFiles.isEmpty()) {
        QList< QMap< QString, QByteArray > >::const_iterator it1 = mediaFiles.constBegin();
        QList< QMap< QString, QByteArray > >::const_iterator endIt1 = mediaFiles.constEnd();
        for (; it1 != endIt1; ++it1) {
            data.append(fileHeader.arg(it1->value("name").data()).arg(it1->value("filename").data()).toUtf8());
            data.append(newLine + "Content-Type: " + it1->value("mediumType"));
            data.append(newLine);
            data.append(newLine + it1->value("medium"));
        }
    }

    QMap< QString, QByteArray >::const_iterator it = formdata.constBegin();
    QMap< QString, QByteArray >::const_iterator endIt = formdata.constEnd();
    for (; it != endIt; ++it) {
        data.append(newLine);
        data.append(header);
        data.append(formHeader.arg(it.key()).toLatin1());
        data.append(newLine);
        data.append(newLine + it.value());
    }
    data.append(newLine);
    data.append(footer);

    return data;
}

}
