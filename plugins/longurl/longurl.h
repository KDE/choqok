/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2014 Eugene Shalygin <eugene.shalygin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOK_LONGURL_H
#define CHOQOK_LONGURL_H

#include <QPointer>
#include <QQueue>
#include <QSharedPointer>
#include <QUrlQuery>

#include <KIO/Job>

#include "plugin.h"

class QUrl;

namespace Choqok
{
namespace UI
{
class PostWidget;
}
}

class LongUrl : public Choqok::Plugin
{
    typedef Choqok::Plugin base;
    Q_OBJECT
public:
    LongUrl(QObject *parent, const QList< QVariant > &args);
    ~LongUrl();

protected Q_SLOTS:
    void slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget);
    void startParsing();
    void dataReceived(KIO::Job *job, QByteArray data);
    void jobResult(KJob *job);
    virtual void aboutToUnload() override;
    void servicesDataReceived(KIO::Job *job, QByteArray data);
    void servicesJobResult(KJob *job);
private:
    enum ParserState { Running = 0, Stopped };
    ParserState state;

    typedef QPointer<Choqok::UI::PostWidget> PostWidgetPointer;

    void sheduleSupportedServicesFetch();
    bool isServiceSupported(const QString &host);
    void processJobResults(KJob *job);

    void parse(PostWidgetPointer postToParse);
    KJob *sheduleParsing(const QString &shortUrl);
    void suspendJobs();

    void replaceUrl(PostWidgetPointer post, const QUrl &fromUrl, const QUrl &toUrl);

    PostWidgetPointer takeJob(KJob *job)
    {
        return mParsingList.take(job);
    }

    void insertJob(KJob *job, PostWidgetPointer post)
    {
        mParsingList.insert(job, post);
    }

    QQueue< PostWidgetPointer > postsQueue;
    QMap<KJob *, PostWidgetPointer > mParsingList;
    QStringList supportedServices;
    typedef QMap<KJob *, QByteArray> DataMap;
    DataMap mData;
    typedef QMap<KJob *, QString> UrlsMap;
    UrlsMap mShortUrls;
    QSharedPointer<QByteArray> mServicesData;
    bool mServicesAreFetched;
};

#endif //CHOQOK_LONGURL_H
