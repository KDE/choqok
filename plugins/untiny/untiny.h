/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UNTINY_H
#define UNTINY_H

#include "plugin.h"

#include <QMap>
#include <QQueue>
#include <QUrl>
#include <QPointer>

namespace KIO {
class Job;
}

class KJob;
namespace Choqok {
namespace UI {
    class PostWidget;
}
}

class UnTiny : public Choqok::Plugin
{
    Q_OBJECT
public:
    UnTiny( QObject* parent, const QList< QVariant >& args );
    ~UnTiny();

protected Q_SLOTS:
    void slotAddNewPostWidget( Choqok::UI::PostWidget *newWidget );
    void slot301Redirected(KIO::Job*,QUrl,QUrl);
    void startParsing();

private:
    enum ParserState{ Running = 0, Stopped };
    ParserState state;

    void parse( QPointer< Choqok::UI::PostWidget > postToParse );
    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;
    QMap<KJob*, QPointer<Choqok::UI::PostWidget> > mParsingList;
    QMap<KJob*, QString> mShortUrlsList;
};

#endif //UNTINY_H
