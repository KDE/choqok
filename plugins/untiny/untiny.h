/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef UNTINY_H
#define UNTINY_H

#include <plugin.h>
#include <qqueue.h>
#include <KUrl>
#include <QPointer>

namespace KIO {
class Job;
}

class KJob;
namespace Choqok {
    class ShortenManager;
namespace UI {
    class PostWidget;
}
}

class KConfigGroup;

class UnTiny : public Choqok::Plugin
{
    Q_OBJECT
public:
    UnTiny( QObject* parent, const QList< QVariant >& args );
    ~UnTiny();

protected slots:
    void slotAddNewPostWidget( Choqok::UI::PostWidget *newWidget );
    void slot301Redirected(KIO::Job*,KUrl,KUrl);
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
