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

#include "microblogwidget.h"
#include "account.h"
#include <KDebug>
#include "timelinewidget.h"
#include <ktabwidget.h>
#include "composerwidget.h"
#include <QVBoxLayout>
#include <notifymanager.h>
#include <KMessageBox>
#include <choqokuiglobal.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <KPushButton>
#include <QLabel>
#include <KMenu>
#include <KDateTime>
#include <QKeyEvent>
#include "choqoktextedit.h"

namespace Choqok {
namespace UI {

class MicroBlogWidget::Private
{
public:
    Private(Account *acc)
    : account(acc), blog(acc->microblog()), composer(0)
    {
    }
    Account *account;
    MicroBlog *blog;
    ComposerWidget *composer;
    QMap<QString, TimelineWidget*> timelines;
    QMap<TimelineWidget*, int> timelineUnreadCount;
    KTabWidget *timelinesTabWidget;
    QLabel *latestUpdate;
    KPushButton *abortAll;
};

MicroBlogWidget::MicroBlogWidget( Account *account, QWidget* parent, Qt::WindowFlags f)
    :QWidget(parent, f), d(new Private(account))
{
    kDebug();
    setupUi();
    connect( this, SIGNAL(markAllAsRead()), SLOT(slotMarkAllAsRead()) );
    connect(d->blog, SIGNAL(timelineDataReceived(Choqok::Account*,QString,QList<Choqok::Post*>)),
            this, SLOT(newTimelineDataRecieved(Choqok::Account*,QString,QList<Choqok::Post*>)) );
    initTimelines();
    connect(d->blog, SIGNAL(error(Choqok::Account*,Choqok::MicroBlog::ErrorType,
                                  QString, Choqok::MicroBlog::ErrorLevel)),
            this, SLOT(error(Choqok::Account*,Choqok::MicroBlog::ErrorType,
                             QString, Choqok::MicroBlog::ErrorLevel)));
    connect(d->blog, SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,
                                    Choqok::MicroBlog::ErrorType,QString,  Choqok::MicroBlog::ErrorLevel)),
            this, SLOT(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                                    QString, Choqok::MicroBlog::ErrorLevel)));
}

Account * MicroBlogWidget::currentAccount() const
{
    return d->account;
}

void MicroBlogWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout( createToolbar() );
    if(d->composer)
        layout->addWidget(d->composer);
    d->timelinesTabWidget = new KTabWidget(this);
    layout->addWidget( d->timelinesTabWidget );
    this->layout()->setContentsMargins( 0, 0, 0, 0 );
}

void MicroBlogWidget::setComposerWidget(ComposerWidget *widget)
{
    if(d->composer)
        d->composer->deleteLater();
    d->composer = widget;
    d->composer->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum);
    qobject_cast<QVBoxLayout*>( this->layout() )->insertWidget(1, d->composer);
    foreach(TimelineWidget *mbw, d->timelines.values()) {
        connect(mbw, SIGNAL(forwardResendPost(QString)), d->composer, SLOT(setText(QString)));
        connect( mbw, SIGNAL(forwardReply(QString,QString)), d->composer, SLOT(setText(QString,QString)) );
    }
}

MicroBlogWidget::~MicroBlogWidget()
{
    kDebug();
    delete d;
}

void MicroBlogWidget::settingsChanged()
{
    kDebug();
    foreach(TimelineWidget *wd, d->timelines.values()){
        wd->settingsChanged();
    }
}

void MicroBlogWidget::updateTimelines()
{
    if(!d->account)
        kError()<<"NIST AMU JAN";
    kDebug()<<d->account->alias();
    d->account->microblog()->updateTimelines(currentAccount());
}

void MicroBlogWidget::removeOldPosts()
{
    foreach(TimelineWidget *wd, d->timelines.values()) {
        wd->removeOldPosts();
    }
}

void MicroBlogWidget::newTimelineDataRecieved( Choqok::Account* theAccount, const QString& type, QList< Choqok::Post* > data )
{
    if(theAccount != currentAccount())
        return;

    kDebug()<<d->account->alias()<<": "<<type;
    d->latestUpdate->setText(KDateTime::currentLocalTime().toString());
    if(d->timelines.contains(type)){
        d->timelines.value(type)->addNewPosts(data);
    } else {
        if(TimelineWidget *wd = addTimelineWidgetToUi(type) )
            wd->addNewPosts(data);
    }
}

void MicroBlogWidget::initTimelines()
{
    kDebug();
    foreach( QString timeline, d->blog->timelineTypes() ){
        addTimelineWidgetToUi(timeline);
    }
}

TimelineWidget* MicroBlogWidget::addTimelineWidgetToUi(const QString& name)
{
    TimelineWidget *mbw = d->blog->createTimelineWidget(d->account, name, this);
    if(mbw) {
        mbw->setObjectName(name);
        d->timelines.insert(name, mbw);
        d->timelineUnreadCount.insert(mbw, 0);
        d->timelinesTabWidget->addTab(mbw, name);
        connect(this, SIGNAL(markAllAsRead()), mbw, SLOT(markAllAsRead()));
        connect( mbw, SIGNAL(updateUnreadCount(int)),
                    this, SLOT(slotUpdateUnreadCount(int)) );
        if(d->composer) {
            connect( mbw, SIGNAL(forwardResendPost(QString)),
                     d->composer, SLOT(setText(QString)) );
            connect( mbw, SIGNAL(forwardReply(QString,QString)),
                     d->composer, SLOT(setText(QString,QString)) );
        }
        return mbw;
    } else {
        kError()<<"Cannot Create a new TimelineWidget for timeline "<<name;
        return 0L;
    }
    if(d->timelinesTabWidget->count() == 1)
        d->timelinesTabWidget->setTabBarHidden(true);
    else
        d->timelinesTabWidget->setTabBarHidden(false);
}

void MicroBlogWidget::slotUpdateUnreadCount(int change)
{
    kDebug()<<change;
    int sum = change;
    foreach(int n, d->timelineUnreadCount.values())
        sum += n;
    if(change != 0)
        emit updateUnreadCount(change, sum);

    TimelineWidget *wd = qobject_cast<TimelineWidget*>(sender());
    if(wd) {
        int cn = d->timelineUnreadCount[wd] = d->timelineUnreadCount[wd] + change;
        int tabIndex = d->timelinesTabWidget->indexOf(wd);
        if(tabIndex == -1)
            return;
        if(cn > 0)
            d->timelinesTabWidget->setTabText( tabIndex, wd->timelineName() +
                                                QString("(%1)").arg(d->timelineUnreadCount[wd]) );
        else
            d->timelinesTabWidget->setTabText( tabIndex, wd->timelineName() );
    }
}

void MicroBlogWidget::slotMarkAllAsRead()
{
    foreach(TimelineWidget *wd, d->timelines.values()) {
        d->timelineUnreadCount[wd] = 0;
        int tabIndex = d->timelinesTabWidget->indexOf(wd);
        if(tabIndex == -1)
            continue;
        d->timelinesTabWidget->setTabText( tabIndex, wd->timelineName() );
    }
}

ComposerWidget* MicroBlogWidget::composer()
{
    return d->composer;
}

QMap< QString, TimelineWidget* > MicroBlogWidget::timelines()
{
    return d->timelines;
}

KTabWidget* MicroBlogWidget::timelinesTabWidget()
{
    return d->timelinesTabWidget;
}

QMap< TimelineWidget*, int > MicroBlogWidget::timelineUnreadCount()
{
    return d->timelineUnreadCount;
}

void MicroBlogWidget::error(Choqok::Account* theAccount, MicroBlog::ErrorType errorType,
           QString errorMsg, MicroBlog::ErrorLevel level)
{
    if(theAccount == d->account){
        switch(level){
        case MicroBlog::Critical:
            KMessageBox::error( Choqok::UI::Global::mainWindow(), errorMsg, MicroBlog::errorString(errorType) );
            break;
        case MicroBlog::Normal:
            NotifyManager::error( errorMsg, MicroBlog::errorString(errorType) );
            break;
        default:
            if( Choqok::UI::Global::mainWindow()->statusBar() )
                Choqok::UI::Global::mainWindow()->statusBar()->showMessage(errorMsg);
            break;
        };
    }
}
void MicroBlogWidget::errorPost(Choqok::Account* theAccount, Choqok::Post*, MicroBlog::ErrorType errorType,
            QString errorMsg, MicroBlog::ErrorLevel level)
{
    if(theAccount == d->account){
        switch(level){
        case MicroBlog::Critical:
            KMessageBox::error( Choqok::UI::Global::mainWindow(), errorMsg, MicroBlog::errorString(errorType) );
            break;
        case MicroBlog::Normal:
            NotifyManager::error( errorMsg, MicroBlog::errorString(errorType) );
            break;
        default:
            if( Choqok::UI::Global::mainWindow()->statusBar() )
                Choqok::UI::Global::mainWindow()->statusBar()->showMessage(errorMsg);
            break;
        };
    }
}

QLayout * MicroBlogWidget::createToolbar()
{
    QHBoxLayout *toolbar = new QHBoxLayout;
    KPushButton *btnActions = new KPushButton(i18n("Actions"), this);

    QLabel *lblLatestUpdate = new QLabel( i18n("Latest update:"), this);
    lblLatestUpdate->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->latestUpdate = new QLabel( KDateTime::currentLocalTime().toString(), this);
    QFont fnt = lblLatestUpdate->font();
    fnt.setPointSize(fnt.pointSize() - 1);
    lblLatestUpdate->setFont(fnt);
    fnt.setBold(true);
    d->latestUpdate->setFont(fnt);

    d->abortAll = new KPushButton(this);
    d->abortAll->setIcon(KIcon("dialog-cancel"));
    d->abortAll->setMaximumWidth(25);
    d->abortAll->setToolTip(i18n("Abort all jobs"));
    connect( d->abortAll, SIGNAL(clicked(bool)), SLOT(slotAbortAllJobs()) );

    btnActions->setMenu(d->account->microblog()->createActionsMenu(d->account));
    toolbar->addWidget(btnActions);
    toolbar->addWidget(d->abortAll);
    toolbar->addSpacerItem(new QSpacerItem(1, 10, QSizePolicy::Expanding));
    toolbar->addWidget(lblLatestUpdate);
    toolbar->addWidget(d->latestUpdate);
    return toolbar;
}

void MicroBlogWidget::slotAbortAllJobs()
{
    currentAccount()->microblog()->abortAllJobs(currentAccount());
    composer()->abort();
}

void MicroBlogWidget::keyPressEvent(QKeyEvent* e)
{
    if(e->key() == Qt::Key_Escape)
        composer()->abort();
    QWidget::keyPressEvent(e);
}

void MicroBlogWidget::setFocus()
{
    if( composer() )
        composer()->editor()->setFocus( Qt::OtherFocusReason );
    else
        QWidget::setFocus();
}

}
}
#include "microblogwidget.moc"
