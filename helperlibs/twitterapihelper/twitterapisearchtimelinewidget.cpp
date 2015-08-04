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

#include "twitterapisearchtimelinewidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>

#include <KLocalizedString>
#include <KSeparator>

#include "account.h"
#include "choqokbehaviorsettings.h"
#include "choqoktypes.h"
#include "postwidget.h"

#include "twitterapidebug.h"
#include "twitterapimicroblog.h"

class TwitterApiSearchTimelineWidget::Private
{
public:
    Private(const SearchInfo &info)
        : currentPage(1), searchInfo(info), loadingAnotherPage(false)
    {}
    QPointer<QPushButton> close;
    QPointer<QPushButton> next;
    QPointer<QPushButton> previous;
    QPointer<QLineEdit> pageNumber;
    QPointer<QCheckBox> autoUpdate;
    uint currentPage;
    SearchInfo searchInfo;
    QPointer<TwitterApiSearch> searchBackend;
    bool loadingAnotherPage;
};

TwitterApiSearchTimelineWidget::TwitterApiSearchTimelineWidget(Choqok::Account *account,
        const QString &timelineName,
        const SearchInfo &info,
        QWidget *parent)
    : TimelineWidget(account, timelineName, parent), d(new Private(info))
{
    setAttribute(Qt::WA_DeleteOnClose);
    d->searchBackend = qobject_cast<TwitterApiMicroBlog *>(currentAccount()->microblog())->searchBackend();
    connect(Choqok::UI::Global::mainWindow(), SIGNAL(updateTimelines()),
            this, SLOT(slotUpdateSearchResults()));
    addFooter();
    timelineDescription()->setText(i18nc("%1 is the name of a timeline", "Search results for %1", timelineName));
    setClosable();
}

TwitterApiSearchTimelineWidget::~TwitterApiSearchTimelineWidget()
{
    delete d;
}

void TwitterApiSearchTimelineWidget::saveTimeline()
{
    qCDebug(CHOQOK);
    //There's no implementation because we don't want to have it in Search Timelines :)
//     Choqok::UI::TimelineWidget::saveTimeline();
}

void TwitterApiSearchTimelineWidget::loadTimeline()
{
    qCDebug(CHOQOK);
    //There's no implementation because we don't want to have it in Search Timelines :)
//     Choqok::UI::TimelineWidget::loadTimeline();
}

void TwitterApiSearchTimelineWidget::addFooter()
{
    QHBoxLayout *footer = titleBarLayout();

    d->close = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-close")), QString(), this);
    d->close->setFixedSize(28, 28);
    d->close->setToolTip(i18n("Close Search"));

    if (d->searchInfo.isBrowsable) {
        d->previous = new QPushButton(this);
        d->previous->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
        d->previous->setMaximumSize(28, 28);
        d->previous->setToolTip(i18n("Previous"));

        d->next = new QPushButton(this);
        d->next->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
        d->next->setMaximumSize(28, 28);
        d->next->setToolTip(i18n("Next"));

        d->pageNumber = new QLineEdit(this);
        d->pageNumber->setValidator(new QIntValidator);
        d->pageNumber->setMaxLength(2);
        d->pageNumber->setMaximumWidth(40);
        d->pageNumber->setAlignment(Qt::AlignCenter);
        d->pageNumber->setToolTip(i18n("Page Number"));

        footer->addWidget(d->previous);
        footer->addWidget(d->pageNumber);
        footer->addWidget(d->next);
        footer->addWidget(new KSeparator(Qt::Vertical, this));
        connect(d->next, SIGNAL(clicked(bool)), SLOT(loadNextPage()));
        connect(d->previous, SIGNAL(clicked(bool)), SLOT(loadPreviousPage()));
        connect(d->pageNumber, SIGNAL(returnPressed()), SLOT(loadCustomPage()));
    }
    footer->addWidget(d->close);
    connect(d->close, SIGNAL(clicked(bool)), this, SIGNAL(closeMe()));

}

void TwitterApiSearchTimelineWidget::addNewPosts(QList< Choqok::Post * > &postList)
{
    if (d->loadingAnotherPage) {
        removeAllPosts();
        d->loadingAnotherPage = false;
    }
    /*bool markRead = false;
     if( posts().count() < 1 )
         markRead = true;*/
    int m = postList.count() - Choqok::BehaviorSettings::countOfPosts();
//     qCDebug(CHOQOK)<<m<<postList.count();
    while (m > 0) {
        postList.removeFirst();
        --m;
    }
    Choqok::UI::TimelineWidget::addNewPosts(postList);
//     if(markRead)
//         markAllAsRead();
    if (d->pageNumber) {
        d->pageNumber->setText(QString::number(d->currentPage));
    }
}

void TwitterApiSearchTimelineWidget::reloadList()
{
    loadCustomPage(QString::number(d->currentPage));
}

void TwitterApiSearchTimelineWidget::loadCustomPage()
{
    loadCustomPage(d->pageNumber->text());
}

void TwitterApiSearchTimelineWidget::loadCustomPage(const QString &pageNumber)
{
    int page = pageNumber.toUInt();
    if (page == 0) {
        page = 1;
    }
    d->loadingAnotherPage = true;
    d->currentPage = page;
    d->searchBackend->requestSearchResults(d->searchInfo, QString(), 0, page);
}

void TwitterApiSearchTimelineWidget::loadNextPage()
{
    loadCustomPage(QString::number(++d->currentPage));
}

void TwitterApiSearchTimelineWidget::loadPreviousPage()
{
    loadCustomPage(QString::number(--d->currentPage));
}

void TwitterApiSearchTimelineWidget::removeAllPosts()
{
    Q_FOREACH (Choqok::UI::PostWidget *wd, posts()) {
        wd->close();
    }
    posts().clear();
}

void TwitterApiSearchTimelineWidget::slotUpdateSearchResults()
{
    if (d->currentPage == 1) {
        QString lastId;
        if (!postWidgets().isEmpty()) {
            lastId = postWidgets().last()->currentPost()->postId;
        }
        d->searchBackend->requestSearchResults(d->searchInfo, lastId);
    }
}

SearchInfo TwitterApiSearchTimelineWidget::searchInfo() const
{
    return d->searchInfo;
}

