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

#include "imagepreview.h"
#include <KGenericFactory>
#include <choqokuiglobal.h>
#include "postwidget.h"
#include <mediamanager.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < ImagePreview > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_imagepreview" ) )

const QRegExp ImagePreview::mTwitpicRegExp("(http://twitpic.com/[^\\s<>\"]+[^!,\\.\\s<>'\"\\]])");
const QRegExp ImagePreview::mYFrogRegExp("(http://yfrog.[^\\s<>\"]+[^!,\\.\\s<>'\\\"\\]])");

ImagePreview::ImagePreview(QObject* parent, const QList< QVariant >& )
    :Choqok::Plugin(MyPluginFactory::componentData(), parent), state(Stopped)
{
    kDebug();
    connect( Choqok::UI::Global::SessionManager::self(),
            SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*)),
             this,
            SLOT(slotAddNewPostWidget(Choqok::UI::PostWidget*)) );
}

ImagePreview::~ImagePreview()
{

}

void ImagePreview::slotAddNewPostWidget(Choqok::UI::PostWidget* newWidget)
{
    postsQueue.enqueue(newWidget);
    if(state == Stopped){
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void ImagePreview::startParsing()
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

void ImagePreview::parse(Choqok::UI::PostWidget* postToParse)
{
    if(!postToParse)
        return;
//     kDebug();
    int pos = 0;
    QStringList twitpicRedirectList;
    QStringList yfrogRedirectList;
    QString content = postToParse->currentPost().content;

    //Twitpic: http://www.twitpic.com/api.do
    while ((pos = mTwitpicRegExp.indexIn(content, pos)) != -1) {
        pos += mTwitpicRegExp.matchedLength();
        twitpicRedirectList << mTwitpicRegExp.cap(0);
        kDebug()<<mTwitpicRegExp.capturedTexts();
    }
    foreach(const QString &url, twitpicRedirectList) {
        QString twitpicUrl = QString( "http://twitpic.com/show/mini%1" ).arg(QString(url).remove("http://twitpic.com"));
        connect( Choqok::MediaManager::self(),
                 SIGNAL(imageFetched(QString,QPixmap)),
                 SLOT(slotImageFetched(QString,QPixmap)) );
        mParsingList.insert(twitpicUrl, postToParse);
        mBaseUrlMap.insert(twitpicUrl, url);
        Choqok::MediaManager::self()->fetchImage(twitpicUrl, Choqok::MediaManager::Async);
    }

    //YFrog: http://code.google.com/p/imageshackapi/wiki/YFROGurls
    //       http://code.google.com/p/imageshackapi/wiki/YFROGthumbnails
    pos = 0;
    while ((pos = mYFrogRegExp.indexIn(content, pos)) != -1) {
        pos += mYFrogRegExp.matchedLength();
        yfrogRedirectList << mYFrogRegExp.cap(0);
        kDebug()<<mYFrogRegExp.capturedTexts();
    }
    foreach(const QString &url, yfrogRedirectList){
//         if( url.endsWith('j') || url.endsWith('p') || url.endsWith('g') ) //To check if it's Image or not!
        connect( Choqok::MediaManager::self(),
                 SIGNAL(imageFetched(QString,QPixmap)),
                 SLOT(slotImageFetched(QString,QPixmap)) );
        QString yfrogThumbnailUrl = url + ".th.jpg";
        kDebug()<<"YFrog Thumbnail Url: "<<yfrogThumbnailUrl;
        mParsingList.insert(yfrogThumbnailUrl, postToParse);
        mBaseUrlMap.insert(yfrogThumbnailUrl, url);
        Choqok::MediaManager::self()->fetchImage(yfrogThumbnailUrl, Choqok::MediaManager::Async);
    }
}

void ImagePreview::slotImageFetched(const QString& remoteUrl, const QPixmap& pixmap)
{
    kDebug();
    Choqok::UI::PostWidget *postToParse = mParsingList.take(remoteUrl);
    QString baseUrl = mBaseUrlMap.take(remoteUrl);
    if(!postToParse)
        return;
    QString content = postToParse->content();
    QString imgUrl("img://image");
    postToParse->document()->addResource(QTextDocument::ImageResource, imgUrl, pixmap);
    content.replace(QRegExp('>'+baseUrl+'<'), "><img src='img://image' /><");
    postToParse->setContent(content);
}

