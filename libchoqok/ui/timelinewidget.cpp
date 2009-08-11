/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "timelinewidget.h"
#include "account.h"
#include <microblog.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <qlayoutitem.h>
#include "postwidget.h"
#include <KDebug>
#include "choqokbehaviorsettings.h"
#include <QApplication>

namespace Choqok
{

class TimelineWidget::Private
{
public:
    Private(Account *account, const QString &timelineName)
        :currentAccount(account), timelineName(timelineName)
    {}
    Account *currentAccount;
    QString timelineName;
    bool mStartUp;
};

TimelineWidget::TimelineWidget(Choqok::Account* account, const QString &timelineName, QWidget* parent /*= 0*/)
    : QWidget(parent), d(new Private(account, timelineName))
{
    setupUi();
    QList<Choqok::Post*> list = account->microblog()->loadTimeline(account->alias(), timelineName);
    connect(account->microblog(), SIGNAL(saveTimelines()), SLOT(saveTimeline()));
    addNewPosts(list, true);
}

TimelineWidget::~TimelineWidget()
{
    kDebug();
    delete d;
}

QString TimelineWidget::timelineName()
{
    return d->timelineName;
}

void TimelineWidget::setTimelineName(const QString &type)
{
    d->timelineName = type;
}

void TimelineWidget::setupUi()
{
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
    scrollAreaWidgetContents->setGeometry(QRect(0, 0, 254, 300));
    verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents);
    verticalLayout_2->setMargin(1);
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    mainLayout = new QVBoxLayout();
    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    mainLayout->addItem(verticalSpacer);
    mainLayout->setSpacing(3);
    mainLayout->setMargin(1);

    verticalLayout_2->addLayout(mainLayout);

    scrollArea->setWidget(scrollAreaWidgetContents);

    gridLayout->addWidget(scrollArea, 0, 0, 1, 1);

    QPalette p = palette();
}

void TimelineWidget::removeOldPosts()
{
    int count = posts.count() - BehaviorSettings::countOfPosts();
    kDebug()<<count;
    while( count > 0 && !posts.isEmpty() ){
        PostWidget *wd = posts.values().first();
        if(wd && wd->isRead()){
            posts.remove( posts.key(wd) );
            wd->close();
        }
        --count;
    }
}

void TimelineWidget::addNewPosts( QList< Choqok::Post* >& postList, bool setRead )
{
    kDebug()<<d->currentAccount->alias()<<' '<<d->timelineName<<' '<<postList.count();
    QList<Post*>::const_iterator it, endIt = postList.constEnd();
    int unread = 0;
    for(it = postList.constBegin(); it!= endIt; ++it){
        if(posts.keys().contains((*it)->postId))
            continue;
        PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, **it, this);
        pw->setRead(setRead);
        if(pw) {
            addPostWidgetToUi(pw);
            if( (*it)->author.userName != d->currentAccount->username() )
                ++unread;
        }
    }
    removeOldPosts();
    emit updateUnreadCount(unread);
}

void TimelineWidget::addPostWidgetToUi(PostWidget* widget)
{
    widget->initUi();
    widget->setObjectName(widget->currentPost().postId);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    connect( widget, SIGNAL(resendPost(const QString &)),
             this, SIGNAL(forwardResendPost(const QString &)));
    connect( widget, SIGNAL(reply(QString,QString)),
            this, SIGNAL(forwardReply(QString,QString)) );
    connect( widget, SIGNAL(postReaded()),
            this, SLOT(slotOnePostReaded()) );
    mainLayout->insertWidget(0, widget);
    posts.insert(widget->currentPost().postId, widget);

}

uint TimelineWidget::unreadCount()
{
    uint count = 0;
    foreach(PostWidget *pw, posts.values()){
        if(!pw->isRead())
            ++count;
    }
    return count;
}

void TimelineWidget::markAllAsRead()
{
    foreach(PostWidget *pw, posts.values()){
        pw->setRead();
    }
}

Account* TimelineWidget::currentAccount()
{
    return d->currentAccount;
}

void TimelineWidget::settingsChanged()
{
    foreach(PostWidget *pw, posts.values()){
        pw->setUiStyle();
    }
}

void TimelineWidget::slotOnePostReaded()
{
    kDebug();
    emit updateUnreadCount(-1);
}

void TimelineWidget::saveTimeline()
{
    if(currentAccount()->microblog())
        currentAccount()->microblog()->saveTimeline( currentAccount()->alias(), timelineName(), posts.values() );
}

}
