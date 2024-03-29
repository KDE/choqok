/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "timelinewidget.h"

#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>

#include "account.h"
#include "choqokappearancesettings.h"
#include "choqokbehaviorsettings.h"
#include "libchoqokdebug.h"
#include "microblog.h"
#include "postwidget.h"
#include "notifymanager.h"

namespace Choqok
{
namespace UI
{

class TimelineWidget::Private
{
public:
    Private(Account *account, const QString &timelineName)
        : currentAccount(account), timelineName(timelineName),
          btnMarkAllAsRead(nullptr), unreadCount(0), placeholderLabel(nullptr), info(nullptr), isClosable(false)
    {
        if (account->microblog()->isValidTimeline(timelineName)) {
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
    QPointer<QPushButton> btnMarkAllAsRead;
    int unreadCount;
    QMap<QString, PostWidget *> posts;
    QMultiMap<QDateTime, PostWidget *>  sortedPostsList;
    QVBoxLayout *mainLayout;
    QHBoxLayout *titleBarLayout;
    QLabel *lblDesc;
    QLabel *placeholderLabel;
    QScrollArea *scrollArea;
    int order;            // 0: web, -1: natural
    Choqok::TimelineInfo *info;
    bool isClosable;
    QIcon timelineIcon;
};

TimelineWidget::TimelineWidget(Choqok::Account *account, const QString &timelineName, QWidget *parent /*= 0*/)
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
    QList<Choqok::Post *> list = currentAccount()->microblog()->loadTimeline(currentAccount(), timelineName());
    connect(currentAccount()->microblog(), &MicroBlog::saveTimelines, this, &TimelineWidget::saveTimeline);

    if (!BehaviorSettings::markAllAsReadOnExit()) {
        addNewPosts(list);
    } else {
        for (Choqok::Post *p: list) {
            PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, p, this);
            if (pw) {
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

void TimelineWidget::setTimelineIcon(const QIcon &icon)
{
    d->timelineIcon = icon;
}

QIcon &TimelineWidget::timelineIcon() const
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
    if (info) {
        d->lblDesc->setText(info->description.toHtmlEscaped());
    }
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
    gridLayout->setObjectName(QLatin1String("gridLayout"));
    d->scrollArea = new QScrollArea(this);
    d->scrollArea->setObjectName(QLatin1String("scrollArea"));
    d->scrollArea->setFrameShape(QFrame::NoFrame);
    d->scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QLatin1String("scrollAreaWidgetContents"));
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
    } else {
        d->order = 0;
    }
}

void TimelineWidget::removeOldPosts()
{
    int count = d->sortedPostsList.count() - BehaviorSettings::countOfPosts();
//     qCDebug(CHOQOK)<<count;
    while (count > 0 && !d->sortedPostsList.isEmpty()) {
        PostWidget *wd = d->sortedPostsList.values().first();
        if (wd && wd->isRead()) {
            wd->close();
        }
        --count;
    }
}

void TimelineWidget::addPlaceholderMessage(const QString &msg)
{
    if ( d->posts.isEmpty() ) {
        if (!d->placeholderLabel) {
            d->placeholderLabel = new QLabel(this);
            d->mainLayout->insertWidget(d->order, d->placeholderLabel);
        }
        d->placeholderLabel->setText(msg);
    }
}

void TimelineWidget::addNewPosts(QList< Choqok::Post * > &postList)
{
    qCDebug(CHOQOK) << d->currentAccount->alias() << d->timelineName << postList.count();
    int unread = 0;
    for (Choqok::Post *p: postList) {
        if (d->posts.keys().contains(p->postId)) {
            continue;
        }
        PostWidget *pw = d->currentAccount->microblog()->createPostWidget(d->currentAccount, p, this);
        if (pw) {
            addPostWidgetToUi(pw);
            if (!pw->isRead()) {
                ++unread;
            }
        }
    }
    removeOldPosts();
    if (unread) {
        d->unreadCount += unread;
        Choqok::NotifyManager::newPostArrived(i18np("1 new post in %2 (%3)",
                                              "%1 new posts in %2 (%3)",
                                              unread, currentAccount()->alias(), d->timelineName));

        Q_EMIT updateUnreadCount(unread);
        showMarkAllAsReadButton();
    }
}

void TimelineWidget::showMarkAllAsReadButton()
{
    if (d->btnMarkAllAsRead) {
        delete d->btnMarkAllAsRead;
    }

    d->btnMarkAllAsRead = new QPushButton(this);
    d->btnMarkAllAsRead->setIcon(QIcon::fromTheme(QLatin1String("mail-mark-read")));
    d->btnMarkAllAsRead->setToolTip(i18n("Mark timeline as read"));
    d->btnMarkAllAsRead->setMaximumSize(14, 14);
    d->btnMarkAllAsRead->setIconSize(QSize(12, 12));
    connect(d->btnMarkAllAsRead, &QPushButton::clicked, this, &TimelineWidget::markAllAsRead);
    d->titleBarLayout->addWidget(d->btnMarkAllAsRead);
}

void TimelineWidget::addPostWidgetToUi(PostWidget *widget)
{
    widget->initUi();
    widget->setFocusProxy(this);
    widget->setObjectName(widget->currentPost()->postId);
    connect(widget, &PostWidget::resendPost, this, &TimelineWidget::forwardResendPost);
    connect(widget, &PostWidget::reply, this, &TimelineWidget::forwardReply);
    connect(widget, &PostWidget::postReaded, this, &TimelineWidget::slotOnePostReaded);
    connect(widget, &PostWidget::aboutClosing, this, &TimelineWidget::postWidgetClosed);
    d->mainLayout->insertWidget(d->order, widget);
    d->posts.insert(widget->currentPost()->postId, widget);
    d->sortedPostsList.insert(widget->currentPost()->creationDateTime, widget);
    Global::SessionManager::self()->emitNewPostWidgetAdded(widget, currentAccount(), timelineName());
    if (d->placeholderLabel) {
        d->mainLayout->removeWidget(d->placeholderLabel);
        delete d->placeholderLabel;
        d->placeholderLabel = nullptr;
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
    if (d->unreadCount > 0) {
        for (PostWidget *pw: d->sortedPostsList) {
            pw->setRead();
        }
        int unread = -d->unreadCount;
        d->unreadCount = 0;
        Q_EMIT updateUnreadCount(unread);
        d->btnMarkAllAsRead->deleteLater();
    }
}

void TimelineWidget::scrollToBottom()
{
    d->scrollArea->verticalScrollBar()->
    triggerAction(QAbstractSlider::SliderToMaximum);
}

Account *TimelineWidget::currentAccount()
{
    return d->currentAccount;
}

void TimelineWidget::settingsChanged()
{
    for (PostWidget *pw: d->sortedPostsList) {
        pw->setUiStyle();
    }
}

void TimelineWidget::slotOnePostReaded()
{
    d->unreadCount--;
    Q_EMIT updateUnreadCount(-1);
    if (d->unreadCount == 0) {
        d->btnMarkAllAsRead->deleteLater();
    }
}

void TimelineWidget::saveTimeline()
{
    if (currentAccount()->microblog()) {
        currentAccount()->microblog()->saveTimeline(currentAccount(), timelineName(), posts().values());
    }
}

QList< PostWidget * > TimelineWidget::postWidgets()
{
    return posts().values();
}

void TimelineWidget::postWidgetClosed(const QString &postId, PostWidget *post)
{
    d->posts.remove(postId);
    d->sortedPostsList.remove(post->currentPost()->creationDateTime, post);
}

QMap< QString, PostWidget * > &TimelineWidget::posts() const
{
    return d->posts;
}

QMultiMap< QDateTime, PostWidget * > &TimelineWidget::sortedPostsList() const
{
    return d->sortedPostsList;
}

QLabel *TimelineWidget::timelineDescription()
{
    return d->lblDesc;
}

QVBoxLayout *TimelineWidget::mainLayout()
{
    return d->mainLayout;
}

QHBoxLayout *TimelineWidget::titleBarLayout()
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

#include "moc_timelinewidget.cpp"
