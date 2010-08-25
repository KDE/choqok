/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <KGenericFactory>
#include <choqokuiglobal.h>
#include "postwidget.h"
#include <kio/jobclasses.h>
#include <KIO/Job>
#include <shortenmanager.h>
#include <untinysettings.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < UnTiny > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_untiny" ) )

UnTiny::UnTiny(QObject* parent, const QList< QVariant >& )
    :Choqok::Plugin(MyPluginFactory::componentData(), parent), state(Stopped)
{
    kDebug();
    connect( Choqok::UI::Global::SessionManager::self(),
            SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
             this,
            SLOT(slotAddNewPostWidget(Choqok::UI::PostWidget*)) );
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
//     kDebug();
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
    QString content = postToParse->currentPost().content;
    for( int i=0; i < pureList.count(); ++i) {
        if(pureList[i].length()>30)
            continue;
        if(!pureList[i].startsWith(QString("http"), Qt::CaseInsensitive)){
            pureList[i].prepend("http://");
        }
        redirectList << pureList[i];
    }
    if(UnTinySettings::useUntinyDotCom()) {
        foreach(const QString &url, redirectList) {
            QString untinyDotComUrl = QString("http://untiny.com/api/1.0/extract/?url=%1&format=text").arg(url);
            KIO::StoredTransferJob *job = KIO::storedGet( untinyDotComUrl, KIO::NoReload,
                                                    KIO::HideProgressInfo );
            if ( !job ) {
                kDebug() << "Cannot create a http header request!";
                break;
            }
            connect( job, SIGNAL( result( KJob*) ),
                    this, SLOT( slotUntinyDotComResult(KJob*) ) );
            mParsingList.insert(job, postToParse);
            mShortUrlsList.insert(job, url);
            job->start();
        }
    } else {
        foreach(const QString &url, redirectList) {
            KIO::TransferJob *job = KIO::mimetype( url, KIO::HideProgressInfo );
            if ( !job ) {
                kDebug() << "Cannot create a http header request!";
                break;
            }
            connect( job, SIGNAL( permanentRedirection( KIO::Job*, KUrl, KUrl ) ),
                    this, SLOT( slot301Redirected(KIO::Job*,KUrl,KUrl)) );
            mParsingList.insert(job, postToParse);
            job->start();
        }
    }
}

void UnTiny::slotUntinyDotComResult(KJob* job)
{
    if(!job)
        return;
    QString toUrl = qobject_cast<KIO::StoredTransferJob *>(job)->data();
    QPointer<Choqok::UI::PostWidget> postToParse = mParsingList.take(job);
    QString fromUrl = mShortUrlsList.take(job);
    if( postToParse && toUrl.startsWith(QString("http"), Qt::CaseInsensitive)){
//         kDebug()<<"Got redirect: "<<fromUrl<<toUrl;
        QString content = postToParse->content();
        content.replace(QRegExp("title='" + fromUrl + '\''), "title='" + toUrl + '\'');
        content.replace(QRegExp("href='" + fromUrl + '\''), "href='" + toUrl + '\'');
        if(postToParse) {
            postToParse->setContent(content);
            Choqok::ShortenManager::self()->emitNewUnshortenedUrl(postToParse, KUrl(fromUrl), KUrl(toUrl));
        }
    }

}

void UnTiny::slot301Redirected(KIO::Job* job, KUrl fromUrl, KUrl toUrl)
{
    QPointer<Choqok::UI::PostWidget> postToParse = mParsingList.take(job);
    job->kill();
    if(postToParse){
        QString content = postToParse->content();
//         kDebug()<<"Got redirect: "<<fromUrl<<toUrl;
        content.replace(QRegExp("title='" + fromUrl.url() + '\''), "title='" + toUrl.url() + '\'');
        content.replace(QRegExp("href='" + fromUrl.url() + '\''), "href='" + toUrl.url() + '\'');
        if(postToParse) {
            postToParse->setContent(content);
            Choqok::ShortenManager::self()->emitNewUnshortenedUrl(postToParse, fromUrl, toUrl);
        }
    }
}
