/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010-2012 Emanuele Bigiarini <pulmro@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QQueue>
#include <QRegExp>
#include <QVariant>
#include <QUrl>
#include <QUrlQuery>

#include "plugin.h"

namespace Choqok
{
namespace UI
{
class PostWidget;
}
}

class VideoPreview : public Choqok::Plugin
{
    Q_OBJECT
public:
    VideoPreview(QObject *parent, const QList< QVariant > &args);
    ~VideoPreview();

protected Q_SLOTS:
    void slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget);
    void startParsing();
    void slotImageFetched(const QUrl &remoteUrl, const QPixmap &pixmap);
    void slotNewUnshortenedUrl(Choqok::UI::PostWidget *widget, const QUrl &fromUrl, const QUrl &toUrl);

private:
    enum ParserState { Running = 0, Stopped };
    ParserState state;

    void parse(QPointer< Choqok::UI::PostWidget > postToParse);
    QUrl parseYoutube(QString videoid , QPointer< Choqok::UI::PostWidget > postToParse);
    QUrl parseVimeo(QString videoid , QPointer< Choqok::UI::PostWidget > postToParse);

    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;
    QMap<QUrl, QPointer<Choqok::UI::PostWidget> > mParsingList;//remoteUrl, Post
    QMap<QUrl, QString> mBaseUrlMap;//remoteUrl, BaseUrl
    QMap<QUrl, QString> mTitleVideoMap;//remoteUrl, TitleVideo
    QMap<QUrl, QString> mDescriptionVideoMap;//remoteUrl, DescriptionVideo

    static const QRegExp mYouTubeRegExp;
    static const QRegExp mYouTuRegExp;
    static const QRegExp mVimeoRegExp;
    static const QRegExp mYouTuCode;

    //static const QRegExp mYFrogRegExp;
};

#endif //VIDEOPREVIEW_H
