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
#include <QLabel>
#include <KPushButton>

namespace Choqok {
namespace UI {

class TimelineWidget::Private
{
public:
    Private(Account *account, const QString &timelineName)
        :currentAccount(account), timelineName(timelineName),
         btnMarkAllAsRead(0), unreadCount(0)
    {}
    Account *currentAccount;
    QString timelineName;
    bool mStartUp;
    KPushButton *btnMarkAllAsRead;
    int unreadCount;
};

TimelineWidget::TimelineWidget(Choqok::Account* account, const QString &timelineName, QWidget* parent /*= 0*/)
    : QWidget(parent), d(new Private(account, timelineName))
{
    setupUi();
    loadTimeline();
}

TimelineWidget::~TimelineWidget()
{
    delete d;
}

void TimelineWidget::loadTimeline()
{
    QList<Choqok::Post*> list = currentAccount()->microblog()->loadTimeline(currentAccount(), timelineName());
    connect(currentAccount()->microblog(), SIGNAL(saveTimelines()), SLOT(saveTimeline()));

    QList<Post*>::const_iterator it, endIt = list.constEnd();
    for(it = list.constBegin(); it!= endIt; ++it){
        PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, **it, this);
        if(pw) {
            pw->setRead();
            addPostWidgetToUi(pw);
        }
    }
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
    QLabel *lblDesc = new QLabel(this);
    lblDesc->setText(currentAccount()->microblog()->timelineInfo(d->timelineName)->description);
    lblDesc->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    lblDesc->setWordWrap(true);
    QFont fnt = lblDesc->font();
    fnt.setBold(true);
    lblDesc->setFont(fnt);

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
    mainLayout = new QVBoxLayout();
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    mainLayout->addItem(verticalSpacer);
    mainLayout->setSpacing(3);
    mainLayout->setMargin(1);

    titleBarLayout = new QHBoxLayout;
    titleBarLayout->addWidget(lblDesc);
    verticalLayout_2->addLayout(mainLayout);

    scrollArea->setWidget(scrollAreaWidgetContents);

    gridLayout->addLayout(titleBarLayout);
    gridLayout->addWidget(scrollArea);

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

void TimelineWidget::addNewPosts( QList< Choqok::Post* >& postList)
{
    kDebug()<<d->currentAccount->alias()<<' '<<d->timelineName<<' '<<postList.count();
    QList<Post*>::const_iterator it, endIt = postList.constEnd();
    int unread = 0;
    for(it = postList.constBegin(); it!= endIt; ++it){
        if(posts.keys().contains((*it)->postId))
            continue;
        PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, **it, this);
        if(pw) {
            addPostWidgetToUi(pw);
            if( !pw->isRead() )
                ++unread;
        }
    }
    removeOldPosts();
    if(unread){
        d->unreadCount += unread;
        emit updateUnreadCount(unread);
        if(!d->btnMarkAllAsRead){
            d->btnMarkAllAsRead = new KPushButton(this);
            d->btnMarkAllAsRead->setIcon(KIcon("mail-mark-read"));
            d->btnMarkAllAsRead->setToolTip(i18n("Mark all as read"));
            d->btnMarkAllAsRead->setMaximumSize(16, 16);
            d->btnMarkAllAsRead->setIconSize(QSize(12,12));
            connect(d->btnMarkAllAsRead, SIGNAL(clicked(bool)), SLOT(markAllAsRead()));
            titleBarLayout->addWidget(d->btnMarkAllAsRead);
        }
    }
}

void TimelineWidget::addPostWidgetToUi(PostWidget* widget)
{
    widget->initUi();
    widget->setFocusProxy(this);
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

int TimelineWidget::unreadCount() const
{
    return d->unreadCount;
}

void TimelineWidget::markAllAsRead()
{
    if( d->unreadCount > 0 ) {
        foreach(PostWidget *pw, posts.values()){
            pw->setRead();
        }
        int unread = -d->unreadCount;
        d->unreadCount = 0;
        emit updateUnreadCount(unread);
        d->btnMarkAllAsRead->deleteLater();
        d->btnMarkAllAsRead = 0L;
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
    d->unreadCount--;
    emit updateUnreadCount(-1);
    if(d->unreadCount == 0){
        d->btnMarkAllAsRead->deleteLater();
        d->btnMarkAllAsRead = 0L;
    }
}

void TimelineWidget::saveTimeline()
{
    if(currentAccount()->microblog())
        currentAccount()->microblog()->saveTimeline( currentAccount(), timelineName(), posts.values() );
}

QList< PostWidget* > TimelineWidget::postWidgets()
{
    return posts.values();
}

}
}
#include "timelinewidget.moc"
