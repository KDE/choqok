/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2014 Eugene Shalygin <eugene.shalygin@gmail.com>

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

#include "longurl.h"

#include <QDebug>
#include <QJsonDocument>
#include <QSharedPointer>
#include <QTimer>

#include <KIO/JobUiDelegate>
#include <KPluginFactory>

#include "postwidget.h"
#include "shortenmanager.h"

K_PLUGIN_FACTORY_WITH_JSON(LongUrlFactory, "choqok_longurl.json",
                           registerPlugin < LongUrl > ();)

const QString baseLongUrlDorComUrl = QLatin1String("http://api.longurl.org/v2/");

LongUrl::LongUrl(QObject *parent, const QList< QVariant > &args)
    : Choqok::Plugin("choqok_longurl", parent)
    , state(Stopped), mServicesAreFetched(false)
{
    sheduleSupportedServicesFetch();
    connect(Choqok::UI::Global::SessionManager::self(),
            SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
            this,
            SLOT(slotAddNewPostWidget(Choqok::UI::PostWidget*)));
}

LongUrl::~LongUrl()
{
    suspendJobs();
    mData.clear();
    mShortUrls.clear();
    Q_FOREACH (KJob *job, mParsingList.keys()) {
        job->kill();
    }
    mParsingList.clear();
}

void LongUrl::parse(QPointer< Choqok::UI::PostWidget > postToParse)
{
    if (!postToParse) {
        return;
    }
    QStringList redirectList, pureList = postToParse->urls();
    QString content = postToParse->currentPost()->content;
    for (int i = 0; i < pureList.count(); ++i) {
        if (pureList[i].length() > 30) {
            continue;
        }
        if (!pureList[i].startsWith(QString("http"), Qt::CaseInsensitive)) {
            pureList[i].prepend("http://");
        }
        redirectList << pureList[i];
    }
    Q_FOREACH (const QString &url, redirectList) {
        KJob *job = sheduleParsing(url);
        if (job) {
            mParsingList.insert(job, postToParse);
            job->start();
        }
    }
}

void LongUrl::processJobResults(KJob *job)
{
    const QJsonDocument json = QJsonDocument::fromJson(mData[job]);
    if (json.isNull()) {
        return;
    }
    const QVariantMap m = json.toVariant().toMap();
    const QUrl longUrl = m.value(QLatin1String("long-url")).toUrl();
    replaceUrl(takeJob(job), QUrl(mShortUrls.take(job)), longUrl);
}

void LongUrl::startParsing()
{
    int i = 8;
    while (!postsQueue.isEmpty() && i > 0) {
        parse(postsQueue.dequeue());
        --i;
    }

    if (postsQueue.isEmpty()) {
        state = Stopped;
    } else {
        QTimer::singleShot(500, this, SLOT(startParsing()));
    }
}

void LongUrl::replaceUrl(LongUrl::PostWidgetPointer post, const QUrl &fromUrl, const QUrl &toUrl)
{
    if (post) {
        QString content = post->content();
        QString fromUrlStr = fromUrl.url();
        content.replace(QRegExp("title='" + fromUrlStr + '\''), "title='" + toUrl.url() + '\'');
        content.replace(QRegExp("href='" + fromUrlStr + '\''), "href='" + toUrl.url() + '\'');
        post->setContent(content);
        Choqok::ShortenManager::self()->emitNewUnshortenedUrl(post, fromUrl, toUrl);
    }
}

void LongUrl::sheduleSupportedServicesFetch()
{
    mServicesAreFetched = true;
    mServicesData = QSharedPointer<QByteArray>(new QByteArray());
    KIO::TransferJob *job = KIO::get(QUrl(baseLongUrlDorComUrl + "services?format=json"), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(servicesDataReceived(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)), SLOT(servicesJobResult(KJob*)));
}

void LongUrl::servicesDataReceived(KIO::Job *job, QByteArray data)
{
    mServicesData->append(data);
}

void LongUrl::servicesJobResult(KJob *job)
{
    if (!job->error()) {
        const QJsonDocument json = QJsonDocument::fromJson(*mServicesData);
        if (!json.isNull()) {
            supportedServices = json.toVariant().toMap().uniqueKeys();
        }
    } else {
        qCritical() << "Job Error:" << job->errorString();
    }
    mServicesAreFetched = false;
    mServicesData.clear();
}

bool LongUrl::isServiceSupported(const QString &host)
{
    return supportedServices.contains(host);
}

KJob *LongUrl::sheduleParsing(const QString &shortUrl)
{
    QUrl url(shortUrl);
    if (isServiceSupported(url.host())) {
        QUrl request = QUrl(baseLongUrlDorComUrl + QLatin1String("expand"));
        request.addQueryItem(QLatin1String("url"), url.url());
        request.addQueryItem(QLatin1String("format"), QLatin1String("json"));
        request.addQueryItem(QLatin1String("user-agent"), QLatin1String("Choqok"));
        KIO::TransferJob *job = KIO::get(request, KIO::NoReload, KIO::HideProgressInfo);
        mData.insert(job, QByteArray());
        mShortUrls.insert(job, shortUrl);
        connect(job, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(dataReceived(KIO::Job*,QByteArray)));
        connect(job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)));
        return job;
    }
    return 0;
}

void LongUrl::dataReceived(KIO::Job *job, QByteArray data)
{
    mData[job].append(data);
}

void LongUrl::jobResult(KJob *job)
{
    if (!job->error()) {
        processJobResults(job);
    }
    mData.remove(job);
    mShortUrls.remove(job);
    mParsingList.remove(job);
}

void LongUrl::slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget)
{
    postsQueue.enqueue(newWidget);
    if (state == Stopped && !mServicesAreFetched) {
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void LongUrl::aboutToUnload()
{
    suspendJobs();
    base::aboutToUnload();
}

void LongUrl::suspendJobs()
{
    Q_FOREACH (KJob *job, mParsingList.keys()) {
        job->suspend();
    }
}

#include "longurl.moc"