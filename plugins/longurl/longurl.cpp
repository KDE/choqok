/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2014 Eugene Shalygin <eugene.shalygin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "longurl.h"

#include <QDebug>
#include <QJsonDocument>
#include <QSharedPointer>
#include <QTimer>

#include <KIO/TransferJob>
#include <KPluginFactory>

#include "postwidget.h"
#include "shortenmanager.h"

K_PLUGIN_FACTORY_WITH_JSON(LongUrlFactory, "choqok_longurl.json",
                           registerPlugin < LongUrl > ();)

const QString baseLongUrlDorComUrl = QLatin1String("http://api.longurl.org/v2/");

LongUrl::LongUrl(QObject *parent, const QList< QVariant > &)
    : Choqok::Plugin(QLatin1String("choqok_longurl"), parent)
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
    for (KJob *job: mParsingList.keys()) {
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
        if (!pureList[i].startsWith(QLatin1String("http"), Qt::CaseInsensitive)) {
            pureList[i].prepend(QLatin1String("http://"));
        }
        redirectList << pureList[i];
    }
    for (const QString &url: redirectList) {
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
        content.replace(QRegExp(QLatin1String("title='") + fromUrlStr + QLatin1Char('\'')), QLatin1String("title='") + toUrl.url() + QLatin1Char('\''));
        content.replace(QRegExp(QLatin1String("href='") + fromUrlStr + QLatin1Char('\'')), QLatin1String("href='") + toUrl.url() + QLatin1Char('\''));
        post->setContent(content);
        Choqok::ShortenManager::self()->emitNewUnshortenedUrl(post, fromUrl, toUrl);
    }
}

void LongUrl::sheduleSupportedServicesFetch()
{
    mServicesAreFetched = true;
    mServicesData = QSharedPointer<QByteArray>(new QByteArray());
    KIO::TransferJob *job = KIO::get(QUrl(baseLongUrlDorComUrl + QLatin1String("services?format=json")), KIO::NoReload, KIO::HideProgressInfo);
    connect(job, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(servicesDataReceived(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)), SLOT(servicesJobResult(KJob*)));
}

void LongUrl::servicesDataReceived(KIO::Job *job, QByteArray data)
{
    Q_UNUSED(job);
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
        QUrlQuery requestQuery;
        requestQuery.addQueryItem(QLatin1String("url"), url.url());
        requestQuery.addQueryItem(QLatin1String("format"), QLatin1String("json"));
        requestQuery.addQueryItem(QLatin1String("user-agent"), QLatin1String("Choqok"));
        request.setQuery(requestQuery);

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
    for (KJob *job: mParsingList.keys()) {
        job->suspend();
    }
}

#include "longurl.moc"
