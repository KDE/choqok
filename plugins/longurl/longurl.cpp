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

#include "./longurl.h"
#include "postwidget.h"
#include <shortenmanager.h>

#include <KIO/JobUiDelegate>
#include <KIO/NetAccess>
#include <kio/jobclasses.h>
#include <KIO/Job>
#include <KGenericFactory>
#include <KTemporaryFile>

#include <qjson/parser.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < LongUrl > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_longurl" ) )

namespace {
const QString baseLongUrlDorComUrl = QLatin1String("http://api.longurl.org/v2/");
}

LongUrl::LongUrl(QObject* parent, const QList< QVariant >& args)
    : Choqok::Plugin(MyPluginFactory::componentData(), parent), state(Stopped)
{
    getSupportedServices();
    kDebug();
    connect( Choqok::UI::Global::SessionManager::self(),
             SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
             this,
             SLOT(slotAddNewPostWidget(Choqok::UI::PostWidget*)) );
}

LongUrl::~LongUrl()
{
    foreach(KJob* job, mParsingList.keys()) {
        job->suspend();
    }
    mData.clear();
    mShortUrls.clear();
    foreach(KJob* job, mParsingList.keys()) {
        job->kill();
    }
    mParsingList.clear();
}


void LongUrl::parse(QPointer< Choqok::UI::PostWidget > postToParse)
{
    if(!postToParse)
        return;
    QStringList redirectList, pureList = postToParse->urls();
    QString content = postToParse->currentPost()->content;
    for( int i=0; i < pureList.count(); ++i) {
        if(pureList[i].length()>30)
            continue;
        if(!pureList[i].startsWith(QString("http"), Qt::CaseInsensitive)) {
            pureList[i].prepend("http://");
        }
        redirectList << pureList[i];
    }
    foreach(const QString &url, redirectList) {
        KJob* job = sheduleParsing(url);
        if(job) {
            mParsingList.insert(job, postToParse);
            job->start();
        }
    }
}

void LongUrl::processJobResults(KJob* job)
{
    bool ok;
    QVariant v = QJson::Parser().parse(mData[job], &ok);
    if(!ok) {
        kDebug() << "Can not parse " << baseLongUrlDorComUrl << " responce";
        return;
    }
    QVariantMap m = v.toMap();
    QString longUrl = m.value(QLatin1String("long-url")).toString();
    replaceUrl(takeJob(job), KUrl(mShortUrls.take(job)), longUrl);
}

void LongUrl::startParsing()
{
    kDebug();
    int i = 8;
    while( !postsQueue.isEmpty() && i>0 ) {
        parse(postsQueue.dequeue());
        --i;
    }

    if(postsQueue.isEmpty())
        state = Stopped;
    else
        QTimer::singleShot(500, this, SLOT(startParsing()));
}

void LongUrl::replaceUrl(LongUrl::PostWidgetPointer post, const KUrl& fromUrl, const KUrl& toUrl)
{
    kDebug() << "Replacing URL: " << fromUrl << " --> " << toUrl;
    if(post) {
        QString content = post->content();
        QString fromUrlStr = fromUrl.url();
        content.replace(QRegExp("title='" + fromUrlStr + '\''), "title='" + toUrl.url() + '\'');
        content.replace(QRegExp("href='" + fromUrlStr + '\''), "href='" + toUrl.url() + '\'');
        post->setContent(content);
        Choqok::ShortenManager::self()->emitNewUnshortenedUrl(post, fromUrl, toUrl);
    }
}

void LongUrl::getSupportedServices()
{
    KTemporaryFile tmpFile;
    if (tmpFile.open()) {
        KIO::Job* getJob = KIO::file_copy(KUrl(baseLongUrlDorComUrl+"services?format=json"), KUrl(tmpFile.fileName()),
                                          -1, KIO::Overwrite | KIO::HideProgressInfo);
        if (KIO::NetAccess::synchronousRun(getJob, 0)) {
            tmpFile.close();
            QFile file(tmpFile.fileName());
            QVariantMap response = QJson::Parser().parse(&file).toMap();
            supportedServices = response.uniqueKeys();
        } else {
            getJob->ui()->showErrorMessage();
        }
    }
}

bool LongUrl::isServiceSupported(const QString& host)
{
    return supportedServices.contains(host);
}

KJob* LongUrl::sheduleParsing(const QString& shortUrl)
{
    KUrl url(shortUrl);
    if (isServiceSupported(url.host())) {
        KUrl request = KUrl(baseLongUrlDorComUrl+QLatin1String("expand"));
        request.addQueryItem(QLatin1String("url"), url.url());
        request.addQueryItem(QLatin1String("format"), QLatin1String("json"));
        request.addQueryItem(QLatin1String("user-agent"), QLatin1String("Choqok"));
        KIO::TransferJob* job = KIO::get(request, KIO::NoReload, KIO::HideProgressInfo);
        job->setAutoDelete(true);
        mData.insert(job, QByteArray());
        mShortUrls.insert(job, shortUrl);
        connect(job, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(dataReceived(KIO::Job*,QByteArray)));
        connect(job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)));
        return job;
    }
    return 0;
}

void LongUrl::dataReceived(KIO::Job* job, QByteArray data)
{
    mData[job].append(data);
}

void LongUrl::jobResult(KJob* job)
{
    if(!job->error()) {
        processJobResults(job);
    }
    mData.remove(job);
    mShortUrls.remove(job);
}

void LongUrl::slotAddNewPostWidget(Choqok::UI::PostWidget* newWidget)
{
    postsQueue.enqueue(newWidget);
    if(state == Stopped) {
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

