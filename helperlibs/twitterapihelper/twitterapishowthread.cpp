/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapishowthread.h"

#include <QScrollArea>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "postwidget.h"
#include "twitterapiaccount.h"
#include "twitterapidebug.h"

class TwitterApiShowThread::Private
{
public:
    Private(Choqok::Account *currentAccount)
        : account(currentAccount)
    {}
    QVBoxLayout *mainLayout;
    Choqok::Account *account;
    QString desiredPostId;
};

TwitterApiShowThread::TwitterApiShowThread(Choqok::Account *account, Choqok::Post *finalPost,
        QWidget *parent)
    : QWidget(parent), d(new Private(account))
{
    qCDebug(CHOQOK);
    setupUi();
    setWindowTitle(i18n("Conversation"));
    connect(account->microblog(), &Choqok::MicroBlog::postFetched, this, &TwitterApiShowThread::slotAddNewPost);
    Choqok::UI::PostWidget *widget = d->account->microblog()->createPostWidget(d->account, finalPost, this);
    if (widget) {
        addPostWidgetToUi(widget);
        Choqok::Post *ps = new Choqok::Post;
        ps->postId = finalPost->replyToPostId;
        d->desiredPostId = finalPost->replyToPostId;
        d->account->microblog()->fetchPost(d->account, ps);
    }
}

TwitterApiShowThread::~TwitterApiShowThread()
{
    delete d;
}

void TwitterApiShowThread::setupUi()
{
    qCDebug(CHOQOK);
    QVBoxLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    gridLayout = new QVBoxLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QLatin1String("gridLayout"));
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QLatin1String("scrollArea"));
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QLatin1String("scrollAreaWidgetContents"));
    scrollAreaWidgetContents->setGeometry(QRect(0, 0, 254, 300));
    verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents);
    verticalLayout_2->setMargin(1);
    d->mainLayout = new QVBoxLayout();
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    d->mainLayout->addItem(verticalSpacer);
    d->mainLayout->setSpacing(3);
    d->mainLayout->setMargin(1);

    verticalLayout_2->addLayout(d->mainLayout);

    scrollArea->setWidget(scrollAreaWidgetContents);

    gridLayout->addWidget(scrollArea);
}

void TwitterApiShowThread::slotAddNewPost(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (theAccount == d->account && post->postId == d->desiredPostId) {
        Choqok::UI::PostWidget *widget = d->account->microblog()->createPostWidget(d->account, post, this);
        if (widget) {
            addPostWidgetToUi(widget);
            Choqok::Post *ps = new Choqok::Post;
            ps->postId = post->replyToPostId;
            d->desiredPostId = ps->postId;
            d->account->microblog()->fetchPost(d->account, ps);
        }
    }
}

void TwitterApiShowThread::addPostWidgetToUi(Choqok::UI::PostWidget *widget)
{
    qCDebug(CHOQOK);
    widget->initUi();
    widget->setRead();
    widget->setFocusProxy(this);
    widget->setObjectName(widget->currentPost()->postId);
    connect(widget, &Choqok::UI::PostWidget::resendPost, this, &TwitterApiShowThread::forwardResendPost);
    connect(widget, &Choqok::UI::PostWidget::resendPost, this, &TwitterApiShowThread::raiseMainWindow);
    connect(widget, &Choqok::UI::PostWidget::reply, this, &TwitterApiShowThread::raiseMainWindow);
    connect(widget, &Choqok::UI::PostWidget::reply, this, &TwitterApiShowThread::forwardReply);
//         connect( widget, SIGNAL(aboutClosing(QString,PostWidget*)),
//                 SLOT(postWidgetClosed(QString,PostWidget*)) );
    d->mainLayout->insertWidget(0, widget);
    //         d->posts.insert(widget->currentPost().postId, widget);
    Choqok::UI::Global::SessionManager::self()->emitNewPostWidgetAdded(widget, d->account);
}

void TwitterApiShowThread::raiseMainWindow()
{
    Choqok::UI::Global::mainWindow()->activateWindow();
}

#include "moc_twitterapishowthread.cpp"
