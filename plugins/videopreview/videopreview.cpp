/*
    This file is part of Choqok, the KDE micro-blogging client

    Based on the imagepreview extension
    Copyright (C) 2010-2012 Emanuele Bigiarini <pulmro@gmail.com>
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
#include "videopreview.h"

#include <QDomDocument>
#include <QDomElement>
#include <QEventLoop>

#include <KGenericFactory>
#include "choqokdebug.h"

#include "choqokuiglobal.h"
#include "postwidget.h"
#include "notifymanager.h"
#include "mediamanager.h"
#include "textbrowser.h"
#include "shortenmanager.h"


K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < VideoPreview > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_videopreview" ) )


const QRegExp VideoPreview::mYouTuRegExp("(http://youtu.[^\\s<>\"]+[^!,\\.\\s<>'\\\"\\]])");
const QRegExp VideoPreview::mYouTubeRegExp("(http://www.youtube.[^\\s<>\"]+[^!,\\.\\s<>'\\\"\\]])");
const QRegExp VideoPreview::mVimeoRegExp("(http://(.+)?vimeo.com/(.+)[&]?)");

const QRegExp VideoPreview::mYouTuCode("youtu.(.+)/(.+)[?&]?");


VideoPreview::VideoPreview(QObject* parent, const QList< QVariant >& )
        :Choqok::Plugin(MyPluginFactory::componentData(), parent), state(Stopped)
{
    qCDebug(CHOQOK);
    connect( Choqok::UI::Global::SessionManager::self(),
             SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
             this,
             SLOT(slotAddNewPostWidget(Choqok::UI::PostWidget*)) );
    connect( Choqok::ShortenManager::self(),
             SIGNAL(newUnshortenedUrl(Choqok::UI::PostWidget*,KUrl,KUrl)),
             this,
             SLOT(slotNewUnshortenedUrl(Choqok::UI::PostWidget*,KUrl,KUrl)) );
}

VideoPreview::~VideoPreview()
{

}

void VideoPreview::slotAddNewPostWidget(Choqok::UI::PostWidget* newWidget)
{
    postsQueue.enqueue(newWidget);
    if (state == Stopped) {
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void VideoPreview::slotNewUnshortenedUrl(Choqok::UI::PostWidget* widget, const KUrl &fromUrl, const KUrl &toUrl)
{
//     qCDebug(CHOQOK) << "I have to consider: " << fromUrl << " -> " << toUrl;
    Q_UNUSED(fromUrl)
    if (mYouTubeRegExp.indexIn(toUrl.prettyUrl()) != -1) {
        KUrl thisurl(mYouTubeRegExp.cap(0));
        QString thumbUrl = parseYoutube(thisurl.queryItemValue("v"), widget);
        connect(Choqok::MediaManager::self(), SIGNAL(imageFetched(QString,QPixmap)),
                SLOT(slotImageFetched(QString,QPixmap)));
        Choqok::MediaManager::self()->fetchImage(thumbUrl, Choqok::MediaManager::Async);
    }
    else if (mVimeoRegExp.indexIn(toUrl.prettyUrl()) != -1) {

        QString thumbUrl = parseVimeo(mVimeoRegExp.cap(3), widget);
        connect(Choqok::MediaManager::self(), SIGNAL(imageFetched(QString,QPixmap)),
                SLOT(slotImageFetched(QString,QPixmap)));
        Choqok::MediaManager::self()->fetchImage(thumbUrl, Choqok::MediaManager::Async);
    }

}


void VideoPreview::startParsing()
{
    int i = 8;
    while ( !postsQueue.isEmpty() && i>0 ) {
        parse(postsQueue.dequeue());
        --i;
    }

    if (postsQueue.isEmpty())
        state = Stopped;
    else
        QTimer::singleShot(500, this, SLOT(startParsing()));
}

void VideoPreview::parse(QPointer<Choqok::UI::PostWidget> postToParse)
{
    if (!postToParse)
        return;
    int pos = 0;
    int pos1 = 0;
    int pos2 = 0;
    int pos3 = 0;
    QStringList thumbList;

    QString content = postToParse->currentPost()->content;
//     qCDebug(CHOQOK) << content;

    while (((pos1 = mYouTuRegExp.indexIn(content, pos)) != -1) |
            ((pos2 = mYouTubeRegExp.indexIn(content, pos)) != -1) |
            ((pos3 = mVimeoRegExp.indexIn(content, pos)) != -1)) {

        if (pos1>=0) {
            pos = pos1 + mYouTuRegExp.matchedLength();
            if (mYouTuCode.indexIn(mYouTuRegExp.cap(0)) != -1) {
                thumbList << parseYoutube(mYouTuCode.cap(2), postToParse);
                qCDebug(CHOQOK) << "YouTu:) " << mYouTuCode.capturedTexts();
            }
        }
        else if (pos2>=0) {
            pos = pos2 + mYouTubeRegExp.matchedLength();
            KUrl thisurl(mYouTubeRegExp.cap(0));
            thumbList << parseYoutube(thisurl.queryItemValue("v"), postToParse);
            qCDebug(CHOQOK) << "YouTube:) " << mYouTubeRegExp.capturedTexts();
        }
        else if (pos3>=0) {
            pos = pos3 + mVimeoRegExp.matchedLength();
            thumbList << parseVimeo(mVimeoRegExp.cap(3), postToParse);
            qCDebug(CHOQOK) << "Vimeo:) " << mVimeoRegExp.capturedTexts();
        }
    }

    Q_FOREACH (const QString &thumb_url, thumbList) {

        qCDebug(CHOQOK) << thumb_url;
        connect( Choqok::MediaManager::self(),
                 SIGNAL(imageFetched(QString,QPixmap)),
                 SLOT(slotImageFetched(QString,QPixmap)) );

        Choqok::MediaManager::self()->fetchImage(thumb_url, Choqok::MediaManager::Async);
    }

}

QString VideoPreview::parseYoutube(QString videoid, QPointer< Choqok::UI::PostWidget > postToParse)
{
    QString youtubeUrl = QString( "http://gdata.youtube.com/feeds/api/videos/%1" ).arg(videoid);
//   qCDebug(CHOQOK) << youtubeUrl;
    QUrl th_url(youtubeUrl);
    KIO::TransferJob *job = KIO::get( th_url, KIO::NoReload, KIO::HideProgressInfo );
    QString title, description, thumb_url;
    QByteArray data;

    if ( job && KIO::NetAccess::synchronousRun(job, Choqok::UI::Global::mainWindow(), &data) ) {
        QDomDocument document;
        document.setContent ( data );
        QDomElement root = document.documentElement();
        if ( !root.isNull() ) {
            QDomElement node;
            node = root.firstChildElement("title");
            if (!node.isNull())
                title = QString(node.text());
            node = root.firstChildElement("media:group");
            node = node.firstChildElement("media:description");
            if (!node.isNull())
                description = QString(node.text());

            node = node.nextSiblingElement("media:thumbnail");
            if (!node.isNull())
                thumb_url = QString(node.attributeNode("url").value());
        }
        else {
            qCritical() << "Youtube XML response is NULL!";
        }

        description = description.left(70);


        qCDebug(CHOQOK) << "thumbnail url: "<< thumb_url;
        qCDebug(CHOQOK) << "video title: "<< title;

        mParsingList.insert(thumb_url, postToParse);
        mBaseUrlMap.insert(thumb_url, "http://www.youtube.com/watch?v="+videoid);
        mTitleVideoMap.insert(thumb_url, title);
        mDescriptionVideoMap.insert(thumb_url, description);
    }

    return thumb_url;
}

QString VideoPreview::parseVimeo(QString videoid, QPointer< Choqok::UI::PostWidget > postToParse)
{
    QString vimeoUrl = QString( "http://vimeo.com/api/v2/video/%1.xml" ).arg(videoid);
//   qCDebug(CHOQOK) << vimeoUrl;
    QUrl th_url(vimeoUrl);
    QEventLoop loop;
    KIO::TransferJob *job = KIO::get( th_url, KIO::NoReload, KIO::HideProgressInfo );
    QString title, description, thumb_url;
    QByteArray data;

    if ( job && KIO::NetAccess::synchronousRun(job, Choqok::UI::Global::mainWindow(), &data) ) {
        QDomDocument document;
        document.setContent ( data );
        QDomElement root = document.documentElement();
        if ( !root.isNull() ) {
            QDomElement videotag;
            videotag = root.firstChildElement("video");
            if ( !videotag.isNull()) {
                QDomElement node;
                node = videotag.firstChildElement("title");
                if ( !node.isNull())
                    title = QString(node.text());
                node = videotag.firstChildElement("description");
                if ( !node.isNull())
                    description = QString(node.text());
                node = videotag.firstChildElement("thumbnail_small");
                if ( !node.isNull())
                    thumb_url = QString(node.text());
            }
            else
                qCritical() << "Vimeo XML response is NULL";
        }
        description = description.left(70);

        qCDebug(CHOQOK) << "thumbnail url: "<< thumb_url;
        qCDebug(CHOQOK) << "video title: "<< title;

        mParsingList.insert(thumb_url, postToParse);
        mBaseUrlMap.insert(thumb_url, "http://vimeo.com/"+videoid);
        mTitleVideoMap.insert(thumb_url, title);
        mDescriptionVideoMap.insert(thumb_url, description);
    }

    return thumb_url;
}

void VideoPreview::slotImageFetched(const QString& remoteUrl, const QPixmap& pixmap)
{
//     qCDebug(CHOQOK);

    Choqok::UI::PostWidget *postToParse = mParsingList.take(remoteUrl);
    QString baseUrl = mBaseUrlMap.take(remoteUrl);
    QString title = mTitleVideoMap.take(remoteUrl);
    QString description = mDescriptionVideoMap.take(remoteUrl);

    if (!postToParse)
        return;
    QString content = postToParse->content();
    KUrl imgU(remoteUrl);
    imgU.setScheme("img");
    QString imgUrl = imgU.prettyUrl();
    postToParse->mainWidget()->document()->addResource(QTextDocument::ImageResource, imgUrl, pixmap);
    
    qCDebug(CHOQOK) << QRegExp('>'+baseUrl+'<').pattern();
    
    QString temp("<br/><table><tr><td rowspan=2><img align='left' height=64 src='" + imgUrl + "' /></td>");
    temp.append("<td><a href='" + baseUrl + "' title='" + baseUrl + "'><b>" + title + "</b></a></td></tr>");
    temp.append("<tr><font size=\"-1\">" + description + "</font></tr></table>");

    content.append(temp);
    postToParse->setContent(content);

}

