/*
    This file is part of Choqok, the KDE micro-blogging client


    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010-2012 Emanuele Bigiarini <pulmro@gmail.com>

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

#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QQueue>
#include <QRegExp>
#include <QVariant>
#include <QUrl>

#include "plugin.h"

namespace Choqok {
    namespace UI {
        class PostWidget;
    }
}

class VideoPreview : public Choqok::Plugin
{
    Q_OBJECT
public:
    VideoPreview( QObject* parent, const QList< QVariant >& args );
    ~VideoPreview();

protected Q_SLOTS:
    void slotAddNewPostWidget( Choqok::UI::PostWidget *newWidget );
    void startParsing();
    void slotImageFetched(const QString &remoteUrl,const QPixmap &pixmap);
    void slotNewUnshortenedUrl(Choqok::UI::PostWidget* widget, const QUrl& fromUrl, const QUrl& toUrl);

private:
    enum ParserState{ Running = 0, Stopped };
    ParserState state;

    void parse( QPointer< Choqok::UI::PostWidget > postToParse );
    QString parseYoutube( QString videoid , QPointer< Choqok::UI::PostWidget > postToParse );
    QString parseVimeo( QString videoid , QPointer< Choqok::UI::PostWidget > postToParse );

    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;
    QMap<QString, QPointer<Choqok::UI::PostWidget> > mParsingList;//remoteUrl, Post
    QMap<QString, QString> mBaseUrlMap;//remoteUrl, BaseUrl
    QMap<QString, QString> mTitleVideoMap;//remoteUrl, TitleVideo
    QMap<QString, QString> mDescriptionVideoMap;//remoteUrl, DescriptionVideo

    static const QRegExp mYouTubeRegExp;
    static const QRegExp mYouTuRegExp;
    static const QRegExp mVimeoRegExp;
    static const QRegExp mYouTuCode;

    //static const QRegExp mYFrogRegExp;
};

#endif //VIDEOPREVIEW_H
