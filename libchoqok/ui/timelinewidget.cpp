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
#include "timelinewidget.h"
#include "account.h"
#include "microblog.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <qlayoutitem.h>
#include "postwidget.h"
#include <KDebug>
#include <QLabel>
#include <KPushButton>
#include <QPointer>
#include "notifymanager.h"
#include "choqokappearancesettings.h"
#include "choqokbehaviorsettings.h"
#include <QTextDocument>

namespace Choqok {
namespace UI {

class TimelineWidget::Private
{
public:
    Private(Account *account, const QString &timelineName)
        :currentAccount(account), timelineName(timelineName),
         btnMarkAllAsRead(0), unreadCount(0), placeholderLabel(0), info(0), isClosable(false)
    {
        if(account->microblog()->isValidTimeline(timelineName)) {
            info = account->microblog()->timelineInfo(timelineName);
        } else {//It's search timeline
            info = new Choqok::TimelineInfo;
            info->name = timelineName;
            info->description = i18nc("%1 is the name of a timeline", "Search results for %1", timelineName);
        }
    }
    Account *currentAccount;
    QString timelineName;
    bool mStartUp;
    QPointer<KPushButton> btnMarkAllAsRead;
    int unreadCount;
    QMap<ChoqokId, PostWidget *> posts;
    QMultiMap<QDateTime, PostWidget *>  sortedPostsList;
    QVBoxLayout *mainLayout;
    QHBoxLayout *titleBarLayout;
    QLabel *lblDesc;
    QLabel *placeholderLabel;
    QScrollArea *scrollArea;
    int order;            // 0: web, -1: natural
    Choqok::TimelineInfo *info;
    bool isClosable;
    KIcon timelineIcon;
};

TimelineWidget::TimelineWidget(Choqok::Account* account, const QString &timelineName, QWidget* parent /*= 0*/)
    : QWidget(parent), d(new Private(account, timelineName))
{
    setAttribute(Qt::WA_DeleteOnClose);
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
    
    if(!BehaviorSettings::markAllAsReadOnExit()) {
      addNewPosts(list);
    } else {
      QList<Post*>::const_iterator it, endIt = list.constEnd();
      for(it = list.constBegin(); it!= endIt; ++it){
          PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, *it, this);
          if(pw) {
              pw->setRead();
              addPostWidgetToUi(pw);
          }
      }
    }
}

QString TimelineWidget::timelineName()
{
    return d->timelineName;
}

QString TimelineWidget::timelineInfoName()
{
    return d->info->name;
}

QString TimelineWidget::timelineIconName()
{
    return d->info->icon;
}

void TimelineWidget::setTimelineIcon(const KIcon& icon)
{
    d->timelineIcon = icon;
}

KIcon& TimelineWidget::timelineIcon() const
{
    return d->timelineIcon;
}

void TimelineWidget::setTimelineName(const QString &type)
{
    d->timelineName = type;
}

void TimelineWidget::setupUi()
{
    d->lblDesc = new QLabel(this);
    TimelineInfo *info = currentAccount()->microblog()->timelineInfo(d->timelineName);
    if(info)
        d->lblDesc->setText(Qt::escape(info->description));
    d->lblDesc->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->lblDesc->setWordWrap(true);
    d->lblDesc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    QFont fnt = d->lblDesc->font();
    fnt.setBold(true);
    d->lblDesc->setFont(fnt);

    QVBoxLayout *gridLayout;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    gridLayout = new QVBoxLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setObjectName("gridLayout");
    d->scrollArea = new QScrollArea(this);
    d->scrollArea->setObjectName("scrollArea");
    d->scrollArea->setFrameShape(QFrame::NoFrame);
    d->scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
    scrollAreaWidgetContents->setGeometry(QRect(0, 0, 254, 300));
    verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents);
    verticalLayout_2->setMargin(1);
    d->mainLayout = new QVBoxLayout();
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    d->mainLayout->addItem(verticalSpacer);
    d->mainLayout->setSpacing(5);
    d->mainLayout->setMargin(1);

    d->titleBarLayout = new QHBoxLayout;
    d->titleBarLayout->addWidget(d->lblDesc);
    verticalLayout_2->addLayout(d->mainLayout);

    d->scrollArea->setWidget(scrollAreaWidgetContents);

    gridLayout->addLayout(d->titleBarLayout);
    gridLayout->addWidget(d->scrollArea);
    if (AppearanceSettings::useReverseOrder()) {
    d->order = -1;
    QTimer::singleShot(0, this, SLOT(scrollToBottom()));
    } else
    d->order = 0;
}

void TimelineWidget::removeOldPosts()
{
    int count = d->sortedPostsList.count() - BehaviorSettings::countOfPosts();
//     kDebug()<<count;
    while( count > 0 && !d->sortedPostsList.isEmpty() ){
        PostWidget *wd = d->sortedPostsList.values().first();
        if(wd && wd->isRead()){
            wd->close();
        }
        --count;
    }
}

void TimelineWidget::addPlaceholderMessage ( const QString& msg )
{
    if (d->posts.keys().length() == 0) {
        if (!d->placeholderLabel) {
            d->placeholderLabel = new QLabel(this);
            d->mainLayout->insertWidget(d->order, d->placeholderLabel);
        }
        d->placeholderLabel->setText(msg);
    }
}

void TimelineWidget::addNewPosts( QList< Choqok::Post* >& postList)
{
    kDebug()<<d->currentAccount->alias()<<' '<<d->timelineName<<' '<<postList.count();
    QList<Post*>::const_iterator it, endIt = postList.constEnd();
    int unread = 0;
    for(it = postList.constBegin(); it!= endIt; ++it){
        if(d->posts.keys().contains((*it)->postId))
            continue;
        PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, *it, this);
        if(pw) {
            addPostWidgetToUi(pw);
            if( !pw->isRead() )
                ++unread;
        }
    }
    removeOldPosts();
    if(unread){
        d->unreadCount += unread;
        Choqok::NotifyManager::newPostArrived( i18np( "1 new post in %2(%3)",
                                                      "%1 new posts in %2(%3)",
                                                      unread, currentAccount()->alias(), d->timelineName ) );

        emit updateUnreadCount(unread);
        showMarkAllAsReadButton();
    }
}

void TimelineWidget::showMarkAllAsReadButton()
{
    if(!d->btnMarkAllAsRead){
        d->btnMarkAllAsRead = new KPushButton(this);
        d->btnMarkAllAsRead->setIcon(KIcon("mail-mark-read"));
        d->btnMarkAllAsRead->setToolTip(i18n("Mark timeline as read"));
        d->btnMarkAllAsRead->setMaximumSize(14, 14);
        d->btnMarkAllAsRead->setIconSize(QSize(12,12));
        connect(d->btnMarkAllAsRead, SIGNAL(clicked(bool)), SLOT(markAllAsRead()));
        d->titleBarLayout->addWidget(d->btnMarkAllAsRead);
    }
}

void TimelineWidget::addPostWidgetToUi(PostWidget* widget)
{
    widget->initUi();
    widget->setFocusProxy(this);
    widget->setObjectName(widget->currentPost()->postId);
    connect( widget, SIGNAL(resendPost(const QString &)),
             this, SIGNAL(forwardResendPost(const QString &)));
    connect( widget, SIGNAL(reply(QString,QString,QString)),
             this, SIGNAL(forwardReply(QString,QString,QString)) );
    connect( widget, SIGNAL(postReaded()),
            this, SLOT(slotOnePostReaded()) );
    connect( widget, SIGNAL(aboutClosing(ChoqokId,PostWidget*)),
             SLOT(postWidgetClosed(ChoqokId,PostWidget*)) );
    d->mainLayout->insertWidget(d->order, widget);
    d->posts.insert(widget->currentPost()->postId, widget);
    d->sortedPostsList.insert(widget->currentPost()->creationDateTime, widget);
    Global::SessionManager::self()->emitNewPostWidgetAdded(widget, currentAccount(), timelineName());
    if (d->placeholderLabel) {
        d->mainLayout->removeWidget(d->placeholderLabel);
        delete d->placeholderLabel;
        d->placeholderLabel = 0;
    }
}

int TimelineWidget::unreadCount() const
{
    return d->unreadCount;
}

void TimelineWidget::setUnreadCount(int unread)
{
    d->unreadCount = unread;
}

void TimelineWidget::markAllAsRead()
{
    if( d->unreadCount > 0 ) {
        foreach(PostWidget *pw, d->sortedPostsList){
            pw->setRead();
        }
        int unread = -d->unreadCount;
        d->unreadCount = 0;
        emit updateUnreadCount(unread);
        d->btnMarkAllAsRead->deleteLater();
    }
}

void TimelineWidget::scrollToBottom()
{
    d->scrollArea->verticalScrollBar()->
    triggerAction(QAbstractSlider::SliderToMaximum);
}

Account* TimelineWidget::currentAccount()
{
    return d->currentAccount;
}

void TimelineWidget::settingsChanged()
{
    foreach(PostWidget *pw, d->sortedPostsList){
        pw->setUiStyle();
    }
}

void TimelineWidget::slotOnePostReaded()
{
    d->unreadCount--;
    emit updateUnreadCount(-1);
    if(d->unreadCount == 0){
        d->btnMarkAllAsRead->deleteLater();
    }
}

void TimelineWidget::saveTimeline()
{
    if(currentAccount()->microblog())
        currentAccount()->microblog()->saveTimeline( currentAccount(), timelineName(), posts().values() );
}

QList< PostWidget* > TimelineWidget::postWidgets()
{
    return posts().values();
}

void TimelineWidget::postWidgetClosed(const ChoqokId& postId, PostWidget* post)
{
    d->posts.remove(postId);
    d->sortedPostsList.remove(post->currentPost()->creationDateTime, post);
}

QMap< ChoqokId, PostWidget* >& TimelineWidget::posts() const
{
    return d->posts;
}

QMultiMap< QDateTime, PostWidget* >& TimelineWidget::sortedPostsList() const
{
    return d->sortedPostsList;
}

QLabel* TimelineWidget::timelineDescription()
{
    return d->lblDesc;
}

QVBoxLayout* TimelineWidget::mainLayout()
{
    return d->mainLayout;
}

QHBoxLayout* TimelineWidget::titleBarLayout()
{
    return d->titleBarLayout;
}

bool TimelineWidget::isClosable() const
{
    return d->isClosable;
}

void TimelineWidget::setClosable(bool isClosable)
{
    d->isClosable = isClosable;
}

}
}
#include "timelinewidget.moc"
