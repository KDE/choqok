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

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include <QQueue>
#include <QPixmap>
#include <QVariant>

#include "plugin.h"

namespace Choqok {
    namespace UI {
        class PostWidget;
    }
}

class QPointer<Choqok::UI::PostWidget*>;

class ImagePreview : public Choqok::Plugin
{
    Q_OBJECT
public:
    ImagePreview( QObject* parent, const QList< QVariant >& args );
    ~ImagePreview();

protected Q_SLOTS:
    void slotAddNewPostWidget( Choqok::UI::PostWidget *newWidget );
    void startParsing();
    void slotImageFetched(const QString &remoteUrl,const QPixmap &pixmap);

private:
    enum ParserState{ Running = 0, Stopped };
    ParserState state;

    void parse( Choqok::UI::PostWidget *postToParse );
    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;
    QMap<QString, QPointer<Choqok::UI::PostWidget> > mParsingList;//remoteUrl, Post
    QMap<QString, QString> mBaseUrlMap;//remoteUrl, BaseUrl

    static const QRegExp mTwitpicRegExp;
    static const QRegExp mYFrogRegExp;
    static const QRegExp mTweetphotoRegExp;
    static const QRegExp mPlixiRegExp;
    static const QRegExp mImgLyRegExp;
    static const QRegExp mTwitgooRegExp;
    static const QRegExp mPumpIORegExp;
};

#endif // IMAGEPREVIEW_H
