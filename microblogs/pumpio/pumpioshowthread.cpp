/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013-2014 Andrea Scarpino <scarpino@kde.org>

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

#include "pumpioshowthread.h"

#include "choqokdebug.h"
#include <KLocale>

#include "pumpiomicroblog.h"
#include "pumpiopost.h"
#include "pumpiopostwidget.h"

class PumpIOShowThread::Private
{
public:
    Choqok::Account* account;
    QString postId;
};

PumpIOShowThread::PumpIOShowThread(Choqok::Account* account, Choqok::Post* post,
                                   QWidget* parent): QWidget(parent)
                                   , d(new Private)
{
    d->account = account;
    d->postId = post->postId;

    setupUi(this);

    setWindowTitle(i18nc("Thread of specified user", "Choqok: %1's thread", post->author.userName));

    connect(account->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
            this, SLOT(slotAddPost(Choqok::Account*,Choqok::Post*)));

    PumpIOPost* p = dynamic_cast<PumpIOPost*>(post);
    if (p) {
        PumpIOPostWidget *widget = new PumpIOPostWidget(account, p, this);
        widget->initUi();
        widget->setRead();
        mainLayout->insertWidget(0, widget);
        connect(widget, SIGNAL(reply(QString,QString,QString)),
                this, SIGNAL(forwardReply(QString, QString, QString)));

        PumpIOMicroBlog* microblog = qobject_cast<PumpIOMicroBlog* >(account->microblog());
        if (microblog) {
            microblog->fetchReplies(account, p->replies);
        } else {
            qCDebug(CHOQOK) << "Microblog is not a PumpIOMicroBlog";
        }
    } else {
        qCDebug(CHOQOK) << "Post is not a PumpIOPost";
    }
}

PumpIOShowThread::~PumpIOShowThread()
{
    delete d;
}

void PumpIOShowThread::slotAddPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    qCDebug(CHOQOK);
    if (theAccount == d->account && post->replyToPostId == d->postId) {
        PumpIOPostWidget *widget = new PumpIOPostWidget(theAccount, post, this);
        widget->initUi();
        widget->setRead();
        connect(widget, SIGNAL(reply(QString,QString,QString)),
                this, SIGNAL(forwardReply(QString,QString,QString)));
        mainLayout->insertWidget(mainLayout->count() - 1, widget);
    }
}
