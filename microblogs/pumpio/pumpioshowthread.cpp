/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pumpioshowthread.h"

#include <KLocalizedString>

#include "pumpiodebug.h"
#include "pumpiomicroblog.h"
#include "pumpiopost.h"
#include "pumpiopostwidget.h"

class PumpIOShowThread::Private
{
public:
    Choqok::Account *account;
    QString postId;
};

PumpIOShowThread::PumpIOShowThread(Choqok::Account *account, Choqok::Post *post,
                                   QWidget *parent): QWidget(parent)
    , d(new Private)
{
    d->account = account;
    d->postId = post->postId;

    setupUi(this);

    setWindowTitle(i18nc("Thread of specified user", "Choqok: %1's thread", post->author.userName));

    connect(account->microblog(), &Choqok::MicroBlog::postFetched, this,
            &PumpIOShowThread::slotAddPost);

    PumpIOPost *p = dynamic_cast<PumpIOPost *>(post);
    if (p) {
        PumpIOPostWidget *widget = new PumpIOPostWidget(account, p, this);
        widget->initUi();
        widget->setRead();
        mainLayout->insertWidget(0, widget);
        connect(widget, &PumpIOPostWidget::reply, this, &PumpIOShowThread::forwardReply);

        PumpIOMicroBlog *microblog = qobject_cast<PumpIOMicroBlog * >(account->microblog());
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

void PumpIOShowThread::slotAddPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (theAccount == d->account && post->replyToPostId == d->postId) {
        PumpIOPostWidget *widget = new PumpIOPostWidget(theAccount, post, this);
        widget->initUi();
        widget->setRead();
        connect(widget, &PumpIOPostWidget::reply, this, &PumpIOShowThread::forwardReply);
        mainLayout->insertWidget(mainLayout->count() - 1, widget);
    }
}

#include "moc_pumpioshowthread.cpp"
