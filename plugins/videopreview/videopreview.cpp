/*
    This file is part of Choqok, the KDE micro-blogging client

    Based on the imagepreview extension
    SPDX-FileCopyrightText: 2010-2012 Emanuele Bigiarini <pulmro@gmail.com>
    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "videopreview.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QEventLoop>
#include <QTimer>

#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KPluginFactory>

#include "choqokuiglobal.h"
#include "postwidget.h"
#include "notifymanager.h"
#include "mediamanager.h"
#include "textbrowser.h"
#include "shortenmanager.h"

K_PLUGIN_FACTORY_WITH_JSON(VideoPreviewFactory, "choqok_videopreview.json",
                           registerPlugin < VideoPreview > ();)

const QRegExp VideoPreview::mYouTuRegExp(QLatin1String("(https?://youtu.[^\\s<>\"]+[^!,\\.\\s<>'\\\"\\]])"));
const QRegExp VideoPreview::mYouTubeRegExp(QLatin1String("(https?://www.youtube.[^\\s<>\"]+[^!,\\.\\s<>'\\\"\\]])"));
const QRegExp VideoPreview::mVimeoRegExp(QLatin1String("(https?://(.+)?vimeo.com/(.+)[&]?)"));

const QRegExp VideoPreview::mYouTuCode(QLatin1String("youtu.(.+)/(.+)[?&]?"));

VideoPreview::VideoPreview(QObject *parent, const QList< QVariant > &)
    : Choqok::Plugin(QLatin1String("choqok_videopreview"), parent)
    , state(Stopped)
{
    connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
            this, &VideoPreview::slotAddNewPostWidget);
    connect(Choqok::ShortenManager::self(), &Choqok::ShortenManager::newUnshortenedUrl,
            this, &VideoPreview::slotNewUnshortenedUrl);
}

VideoPreview::~VideoPreview()
{

}

void VideoPreview::slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget)
{
    postsQueue.enqueue(newWidget);
    if (state == Stopped) {
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void VideoPreview::slotNewUnshortenedUrl(Choqok::UI::PostWidget *widget, const QUrl &fromUrl, const QUrl &toUrl)
{
    Q_UNUSED(fromUrl)
    if (mYouTubeRegExp.indexIn(toUrl.toDisplayString()) != -1) {
        QUrl thisurl(mYouTubeRegExp.cap(0));
        QUrlQuery thisurlQuery(thisurl);
        QUrl thumbUrl = parseYoutube(thisurlQuery.queryItemValue(QLatin1String("v")), widget);
        connect(Choqok::MediaManager::self(), &Choqok::MediaManager::imageFetched,
                this, &VideoPreview::slotImageFetched);
        Choqok::MediaManager::self()->fetchImage(thumbUrl, Choqok::MediaManager::Async);
    } else if (mVimeoRegExp.indexIn(toUrl.toDisplayString()) != -1) {
        QUrl thumbUrl = parseVimeo(mVimeoRegExp.cap(3), widget);
        connect(Choqok::MediaManager::self(), &Choqok::MediaManager::imageFetched,
                this, &VideoPreview::slotImageFetched);
        Choqok::MediaManager::self()->fetchImage(thumbUrl, Choqok::MediaManager::Async);
    }

}

void VideoPreview::startParsing()
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

void VideoPreview::parse(QPointer<Choqok::UI::PostWidget> postToParse)
{
    if (!postToParse) {
        return;
    }
    int pos = 0;
    int pos1 = 0;
    int pos2 = 0;
    int pos3 = 0;
    QList<QUrl> thumbList;

    QString content = postToParse->currentPost()->content;

    while (((pos1 = mYouTuRegExp.indexIn(content, pos)) != -1) |
            ((pos2 = mYouTubeRegExp.indexIn(content, pos)) != -1) |
            ((pos3 = mVimeoRegExp.indexIn(content, pos)) != -1)) {

        if (pos1 >= 0) {
            pos = pos1 + mYouTuRegExp.matchedLength();
            if (mYouTuCode.indexIn(mYouTuRegExp.cap(0)) != -1) {
                thumbList << parseYoutube(mYouTuCode.cap(2), postToParse);
            }
        } else if (pos2 >= 0) {
            pos = pos2 + mYouTubeRegExp.matchedLength();
            QUrl thisurl(mYouTubeRegExp.cap(0));
            QUrlQuery thisurlQuery(thisurl);
            thumbList << parseYoutube(thisurlQuery.queryItemValue(QLatin1String("v")), postToParse);
        } else if (pos3 >= 0) {
            pos = pos3 + mVimeoRegExp.matchedLength();
            thumbList << parseVimeo(mVimeoRegExp.cap(3), postToParse);
        }
    }

    for (const QUrl &thumb_url: thumbList) {
        connect(Choqok::MediaManager::self(), &Choqok::MediaManager::imageFetched,
                this, &VideoPreview::slotImageFetched);

        Choqok::MediaManager::self()->fetchImage(thumb_url, Choqok::MediaManager::Async);
    }

}

QUrl VideoPreview::parseYoutube(QString videoid, QPointer< Choqok::UI::PostWidget > postToParse)
{
    QString youtubeUrl = QStringLiteral("https://gdata.youtube.com/feeds/api/videos/%1").arg(videoid);
    QUrl th_url(youtubeUrl);
    KIO::StoredTransferJob *job = KIO::storedGet(th_url, KIO::NoReload, KIO::HideProgressInfo);
    KJobWidgets::setWindow(job, Choqok::UI::Global::mainWindow());
    QString title, description;
    QUrl thumb_url;

    job->exec();
    if (!job->error()) {
        QDomDocument document;
        document.setContent(job->data());
        QDomElement root = document.documentElement();
        if (!root.isNull()) {
            QDomElement node;
            node = root.firstChildElement(QLatin1String("title"));
            if (!node.isNull()) {
                title = QString(node.text());
            }
            node = root.firstChildElement(QLatin1String("media:group"));
            node = node.firstChildElement(QLatin1String("media:description"));
            if (!node.isNull()) {
                description = QString(node.text());
            }

            node = node.nextSiblingElement(QLatin1String("media:thumbnail"));
            if (!node.isNull()) {
                thumb_url = QUrl::fromUserInput(node.attributeNode(QLatin1String("url")).value());
            }
        }

        description = description.left(70);

        mParsingList.insert(thumb_url, postToParse);
        mBaseUrlMap.insert(thumb_url, QLatin1String("https://www.youtube.com/watch?v=") + videoid);
        mTitleVideoMap.insert(thumb_url, title);
        mDescriptionVideoMap.insert(thumb_url, description);
    } else {
        qCritical() << "Youtube XML response is NULL!";
    }

    return thumb_url;
}

QUrl VideoPreview::parseVimeo(QString videoid, QPointer< Choqok::UI::PostWidget > postToParse)
{
    QString vimeoUrl = QStringLiteral("https://vimeo.com/api/v2/video/%1.xml").arg(videoid);
    QUrl th_url(vimeoUrl);
    QEventLoop loop;
    KIO::StoredTransferJob *job = KIO::storedGet(th_url, KIO::NoReload, KIO::HideProgressInfo);
    KJobWidgets::setWindow(job, Choqok::UI::Global::mainWindow());
    QString title, description;
    QUrl thumb_url;

    job->exec();
    if (!job->error()) {
        QDomDocument document;
        document.setContent(job->data());
        QDomElement root = document.documentElement();
        if (!root.isNull()) {
            QDomElement videotag;
            videotag = root.firstChildElement(QLatin1String("video"));
            if (!videotag.isNull()) {
                QDomElement node;
                node = videotag.firstChildElement(QLatin1String("title"));
                if (!node.isNull()) {
                    title = QString(node.text());
                }
                node = videotag.firstChildElement(QLatin1String("description"));
                if (!node.isNull()) {
                    description = QString(node.text());
                }
                node = videotag.firstChildElement(QLatin1String("thumbnail_small"));
                if (!node.isNull()) {
                    thumb_url = QUrl::fromUserInput(node.text());
                }
            }
        }
        description = description.left(70);

        mParsingList.insert(thumb_url, postToParse);
        mBaseUrlMap.insert(thumb_url, QLatin1String("https://vimeo.com/") + videoid);
        mTitleVideoMap.insert(thumb_url, title);
        mDescriptionVideoMap.insert(thumb_url, description);
    } else {
        qCritical() << "Vimeo XML response is NULL!";
    }

    return thumb_url;
}

void VideoPreview::slotImageFetched(const QUrl &remoteUrl, const QPixmap &pixmap)
{
    Choqok::UI::PostWidget *postToParse = mParsingList.take(remoteUrl);
    QString baseUrl = mBaseUrlMap.take(remoteUrl);
    QString title = mTitleVideoMap.take(remoteUrl);
    QString description = mDescriptionVideoMap.take(remoteUrl);

    if (!postToParse) {
        return;
    }
    QString content = postToParse->content();
    QUrl imgU(remoteUrl);
    imgU.setScheme(QLatin1String("img"));
    postToParse->mainWidget()->document()->addResource(QTextDocument::ImageResource, imgU, pixmap);

    QString temp(QLatin1String("<br/><table><tr><td rowspan=2><img align='left' height=64 src='")
                 + imgU.toDisplayString() + QLatin1String("' /></td>"));
    temp.append(QLatin1String("<td><a href='") + baseUrl + QLatin1String("' title='") + baseUrl + QLatin1String("'><b>") + title + QLatin1String("</b></a></td></tr>"));
    temp.append(QLatin1String("<tr><font size=\"-1\">") + description + QLatin1String("</font></tr></table>"));

    content.append(temp);
    postToParse->setContent(content);
}

#include "videopreview.moc"
