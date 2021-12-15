/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include <QQueue>
#include <QPixmap>
#include <QVariant>

#include "plugin.h"

namespace Choqok
{
namespace UI
{
class PostWidget;
}
}

class ImagePreview : public Choqok::Plugin
{
    Q_OBJECT
public:
    ImagePreview(QObject *parent, const QList< QVariant > &args);
    ~ImagePreview();

protected Q_SLOTS:
    void slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget);
    void startParsing();
    void slotImageFetched(const QUrl &remoteUrl, const QPixmap &pixmap);

private:
    enum ParserState { Running = 0, Stopped };
    ParserState state;

    void parse(Choqok::UI::PostWidget *postToParse);
    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;
    QMap<QUrl, QPointer<Choqok::UI::PostWidget> > mParsingList;//remoteUrl, Post
    QMap<QUrl, QString> mBaseUrlMap;//remoteUrl, BaseUrl

    static const QRegExp mImgLyRegExp;
    static const QRegExp mTwitgooRegExp;
    static const QRegExp mPumpIORegExp;
};

#endif // IMAGEPREVIEW_H
