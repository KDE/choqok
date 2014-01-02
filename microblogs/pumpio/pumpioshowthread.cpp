/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013  Andrea Scarpino <scarpino@kde.org>

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

#include <KDebug>

#include "pumpiomicroblog.h"
#include "pumpiopost.h"
#include "pumpiopostwidget.h"

class PumpIOShowThread::Private
{
public:
    Choqok::Account* account;
    ChoqokId postId;
};

PumpIOShowThread::PumpIOShowThread(Choqok::Account* account, Choqok::Post* post,
                                   QWidget* parent): QWidget(parent)
                                   , d(new Private)
{
    d->account = account;
    d->postId = post->postId;

    setupUi(this);

    setWindowTitle("Choqok: " + post->author.userName + "'s thread");

    connect(account->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
            this, SLOT(slotAddPost(Choqok::Account*,Choqok::Post*)));

    PumpIOPost* p = dynamic_cast<PumpIOPost*>(post);
    if (p) {
        PumpIOPostWidget *widget = new PumpIOPostWidget(account, p, this);
        widget->initUi();
        widget->setRead();
        mainLayout->insertWidget(0, widget);

        PumpIOMicroBlog* microblog = dynamic_cast<PumpIOMicroBlog*>(account->microblog());
        if (microblog) {
            microblog->fetchReplies(account, p->replies);
        } else {
            kDebug() << "Microblog is not a PumpIOMicroBlog";
        }
    } else {
        kDebug() << "Post is not a PumpIOPost";
    }
}

PumpIOShowThread::~PumpIOShowThread()
{
    delete d;
}

void PumpIOShowThread::slotAddPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    if (theAccount == d->account && post->replyToPostId == d->postId) {
        PumpIOPostWidget *widget = new PumpIOPostWidget(theAccount, post, this);
        widget->initUi();
        widget->setRead();
        mainLayout->insertWidget(mainLayout->count() - 1, widget);
    }
}
