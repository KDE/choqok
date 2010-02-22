/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twitterapishowthread.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include "twitterapiaccount.h"
#include <postwidget.h>
#include <KDebug>
#include <klocalizedstring.h>

class TwitterApiShowThread::Private{
public:
    Private(Choqok::Account *currentAccount)
        :account(currentAccount)
    {}
    QVBoxLayout *mainLayout;
    Choqok::Account *account;
    QString desiredPostId;
};

TwitterApiShowThread::TwitterApiShowThread(Choqok::Account* account, const Choqok::Post& finalPost,
                                           QWidget* parent)
                                           : QWidget(parent), d(new Private(account))
{
    kDebug();
    setupUi();
    setWindowTitle(i18n("Conversation"));
    connect( account->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
             this, SLOT(slotAddNewPost(Choqok::Account*,Choqok::Post*)) );
    Choqok::UI::PostWidget *widget = d->account->microblog()->createPostWidget(d->account, finalPost, this);
    if(widget) {
        addPostWidgetToUi(widget);
        Choqok::Post *ps = new Choqok::Post;
        ps->postId = finalPost.replyToPostId;
        d->desiredPostId = finalPost.replyToPostId;
        d->account->microblog()->fetchPost(d->account, ps);
    }
}


TwitterApiShowThread::~TwitterApiShowThread()
{

}

void TwitterApiShowThread::setupUi()
{
    kDebug();
    QVBoxLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    gridLayout = new QVBoxLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setObjectName("gridLayout");
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("scrollArea");
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
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

void TwitterApiShowThread::slotAddNewPost(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    if(theAccount == d->account && post->postId == d->desiredPostId) {
        Choqok::UI::PostWidget *widget = d->account->microblog()->createPostWidget(d->account, *post, this);
        if(widget) {
            addPostWidgetToUi(widget);
            Choqok::Post *ps = new Choqok::Post;
            ps->postId = post->replyToPostId;
            d->desiredPostId = ps->postId;
            d->account->microblog()->fetchPost(d->account, ps);
        }
    }
}

void TwitterApiShowThread::addPostWidgetToUi(Choqok::UI::PostWidget* widget)
{
    kDebug();
    widget->initUi();
    widget->setRead();
    widget->setFocusProxy(this);
    widget->setObjectName(widget->currentPost().postId);
    connect( widget, SIGNAL(resendPost(const QString &)),
             this, SIGNAL(forwardResendPost(const QString &)));
    connect( widget, SIGNAL(resendPost(QString)),
             this, SLOT(raiseMainWindow()) );
    connect( widget, SIGNAL(reply(QString, QString)),
             this, SLOT(raiseMainWindow()) );
    connect( widget, SIGNAL(reply(QString,QString)),
            this, SIGNAL(forwardReply(QString,QString)) );
//         connect( widget, SIGNAL(aboutClosing(ChoqokId,PostWidget*)),
//                 SLOT(postWidgetClosed(ChoqokId,PostWidget*)) );
    d->mainLayout->insertWidget(0, widget);
    //         d->posts.insert(widget->currentPost().postId, widget);
    Choqok::UI::Global::SessionManager::self()->emitNewPostWidgetAdded(widget);
}

void TwitterApiShowThread::raiseMainWindow()
{
    Choqok::UI::Global::mainWindow()->activateWindow();
}

#include "twitterapishowthread.moc"
