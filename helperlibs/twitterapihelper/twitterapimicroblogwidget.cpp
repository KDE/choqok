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

#include "twitterapimicroblogwidget.h"
#include "twitterapimicroblog.h"
#include "account.h"
#include "composerwidget.h"
#include <KTabWidget>
#include "twitterapisearchtimelinewidget.h"
#include <kicon.h>
#include <KDebug>
#include <QToolButton>
#include <klocalizedstring.h>
#include <KMessageBox>

class TwitterApiMicroBlogWidget::Private
{
public:
    Private(Choqok::Account *account)
        :btnCloseSearch(0)
    {
        kDebug();
        mBlog = qobject_cast<TwitterApiMicroBlog *>(account->microblog());
    }
    TwitterApiMicroBlog *mBlog;
    QToolButton *btnCloseSearch;
};

TwitterApiMicroBlogWidget::TwitterApiMicroBlogWidget(Choqok::Account* account, QWidget* parent)
    : MicroBlogWidget(account, parent), d(new Private(account))
{
    kDebug();
    connect( account, SIGNAL(modified(Choqok::Account*)),
             this, SLOT(slotAccountModified(Choqok::Account*)) );
    connect( d->mBlog->searchBackend(),
             SIGNAL(searchResultsReceived(SearchInfo,QList<Choqok::Post*>&)),
             SLOT(slotSearchResultsReceived(SearchInfo,QList<Choqok::Post*>&)) );
}

void TwitterApiMicroBlogWidget::initUi()
{
    kDebug();
    Choqok::UI::MicroBlogWidget::initUi();
    connect(timelinesTabWidget(), SIGNAL(currentChanged(int)), SLOT(slotCurrentTimelineChanged(int)) );
    d->btnCloseSearch = new QToolButton( this );
    d->btnCloseSearch->setIcon(KIcon("tab-close"));
    d->btnCloseSearch->setToolTip(i18nc("Close a search timeline", "Close Search"));
    timelinesTabWidget()->setCornerWidget(d->btnCloseSearch, Qt::TopRightCorner);
    connect(d->btnCloseSearch, SIGNAL(clicked(bool)), SLOT(slotCloseCurrentSearch()) );
    slotCurrentTimelineChanged(timelinesTabWidget()->currentIndex());
}

TwitterApiMicroBlogWidget::~TwitterApiMicroBlogWidget()
{
    delete d;
}

void TwitterApiMicroBlogWidget::slotSearchResultsReceived(const SearchInfo &info,
                                                          QList< Choqok::Post* >& postsList)
{
    kDebug();
    if( info.account == currentAccount() ){
        kDebug()<<postsList.count();
        QString name = QString("%1%2").arg(d->mBlog->searchBackend()->optionCode(info.option)).arg(info.query);
        if(mSearchTimelines.contains(name)){
            mSearchTimelines.value(name)->addNewPosts(postsList);
        }else{
            if( postsList.isEmpty() ){
                KMessageBox::sorry(this, i18n("The search result is empty."));
            } else {
                addSearchTimelineWidgetToUi( name, info )->addNewPosts(postsList);
            }
        }
    }
}

TwitterApiSearchTimelineWidget* TwitterApiMicroBlogWidget::addSearchTimelineWidgetToUi(const QString& name,
                                                                                       const SearchInfo &info)
{
    kDebug();
    TwitterApiSearchTimelineWidget *mbw = d->mBlog->createSearchTimelineWidget(currentAccount(), name,
                                                                               info, this);
    if(mbw) {
        mSearchTimelines.insert(name, mbw);
        timelinesTabWidget()->addTab(mbw, name);
        timelinesTabWidget()->setTabIcon(timelinesTabWidget()->indexOf(mbw), KIcon("edit-find"));
        connect( mbw, SIGNAL(updateUnreadCount(int)),
                    this, SLOT(slotUpdateUnreadCount(int)) );
        if(composer()) {
            connect( mbw, SIGNAL(forwardResendPost(QString)),
                     composer(), SLOT(setText(QString)) );
            connect( mbw, SIGNAL(forwardReply(QString,QString)),
                     composer(), SLOT(setText(QString,QString)) );
        }
        timelinesTabWidget()->setCurrentWidget(mbw);
    } else {
        kDebug()<<"Cannot Create a new TimelineWidget for timeline "<<name;
        return 0L;
    }
    if(timelinesTabWidget()->count() == 1)
        timelinesTabWidget()->setTabBarHidden(true);
    else
        timelinesTabWidget()->setTabBarHidden(false);
    return mbw;
}

void TwitterApiMicroBlogWidget::slotCurrentTimelineChanged(int index)
{
    TwitterApiSearchTimelineWidget *stw =
            qobject_cast<TwitterApiSearchTimelineWidget *>(timelinesTabWidget()->widget(index));
    if(stw)
        d->btnCloseSearch->setEnabled(true);
    else
        d->btnCloseSearch->setEnabled(false);
}

void TwitterApiMicroBlogWidget::slotCloseCurrentSearch()
{
    TwitterApiSearchTimelineWidget *stw =
            qobject_cast<TwitterApiSearchTimelineWidget *>(timelinesTabWidget()->currentWidget());
    if(stw) {
        stw->markAllAsRead();
        QString name = mSearchTimelines.key(stw);
        if(name.isEmpty())
            return;
        timelinesTabWidget()->removePage(timelinesTabWidget()->currentWidget());
        mSearchTimelines.value(name)->close();
        mSearchTimelines.remove(name);
    }
}

void TwitterApiMicroBlogWidget::markAllAsRead()
{
    Choqok::UI::MicroBlogWidget::markAllAsRead();
    foreach(TwitterApiSearchTimelineWidget *wd, mSearchTimelines) {
        wd->markAllAsRead();
        int tabIndex = timelinesTabWidget()->indexOf(wd);
        if(tabIndex == -1)
            continue;
        timelinesTabWidget()->setTabText( tabIndex, wd->timelineName() );
    }
}

void TwitterApiMicroBlogWidget::slotAccountModified(Choqok::Account* account)
{
    foreach(const QString &timeline, account->microblog()->timelineNames()){
        if( account->timelineNames().contains(timeline) ){
            if( !timelines().contains(timeline) ){
                addTimelineWidgetToUi(timeline);
            }
        } else if( timelines().contains(timeline) ) {
            Choqok::UI::TimelineWidget *tm = timelines().take(timeline);
            tm->deleteLater();
        }
    }
}

#include "twitterapimicroblogwidget.moc"
