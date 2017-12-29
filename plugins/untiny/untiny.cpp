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

#include "untiny.h"

#include <QTimer>

#include <KIO/MimetypeJob>
#include <KPluginFactory>

#include "choqokuiglobal.h"
#include "postwidget.h"
#include "shortenmanager.h"

#include "untinysettings.h"

K_PLUGIN_FACTORY_WITH_JSON(UnTinyFactory, "choqok_untiny.json",
                           registerPlugin < UnTiny > ();)

UnTiny::UnTiny(QObject* parent, const QList< QVariant >& )
    : Choqok::Plugin(QLatin1String("choqok_untiny"), parent)
    , state(Stopped)
{
    connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
            this, &UnTiny::slotAddNewPostWidget);
}

UnTiny::~UnTiny()
{

}

void UnTiny::slotAddNewPostWidget(Choqok::UI::PostWidget* newWidget)
{
    postsQueue.enqueue(newWidget);
    if(state == Stopped){
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void UnTiny::startParsing()
{
    int i = 8;
    while( !postsQueue.isEmpty() && i>0 ){
        parse(postsQueue.dequeue());
        --i;
    }

    if(postsQueue.isEmpty())
        state = Stopped;
    else
        QTimer::singleShot(500, this, SLOT(startParsing()));
}

void UnTiny::parse(QPointer<Choqok::UI::PostWidget> postToParse)
{
    if(!postToParse)
        return;
    QStringList redirectList, pureList = postToParse->urls();
    QString content = postToParse->currentPost()->content;
    for (int i=0; i < pureList.count(); ++i) {
        if(pureList[i].length() > 30){
            continue;
        }
        if(!pureList[i].startsWith(QLatin1String("http"), Qt::CaseInsensitive)){
            pureList[i].prepend(QLatin1String("http://"));
        }
        redirectList << pureList[i];
    }
    for (const QString &url: redirectList) {
        KIO::MimetypeJob *job = KIO::mimetype( QUrl::fromUserInput(url), KIO::HideProgressInfo );
        if ( !job ) {
            qCritical() << "Cannot create a http header request!";
            break;
        }
        connect( job, &KIO::MimetypeJob::permanentRedirection, this, &UnTiny::slot301Redirected );
        mParsingList.insert(job, postToParse);
        job->start();
    }
}

void UnTiny::slot301Redirected(KIO::Job* job, QUrl fromUrl, QUrl toUrl)
{
    QPointer<Choqok::UI::PostWidget> postToParse = mParsingList.take(job);
    job->kill();
    if(postToParse){
        QString content = postToParse->content();
        QString fromUrlStr = fromUrl.url();
        content.replace(QRegExp(QStringLiteral("title='%1\'").arg(fromUrlStr)), QStringLiteral("title='%1\'").arg(toUrl.url()));
        content.replace(QRegExp(QStringLiteral("href='%1\'").arg(fromUrlStr)), QStringLiteral("href='%1\'").arg(toUrl.url()));
        postToParse->setContent(content);
        Choqok::ShortenManager::self()->emitNewUnshortenedUrl(postToParse, fromUrl, toUrl);
        if (toUrl.url().length() < 30 && fromUrl.host() == QLatin1String("t.co")){
            KIO::TransferJob *job = KIO::mimetype( toUrl, KIO::HideProgressInfo );
            if ( job ) {
                connect( job, &KIO::MimetypeJob::permanentRedirection, this, &UnTiny::slot301Redirected );
                mParsingList.insert(job, postToParse);
                job->start();
            }
        }
    }
}

#include "untiny.moc"
