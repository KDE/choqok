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

#include "twitterapisearchtimelinewidget.h"
#include <QHBoxLayout>
#include <KPushButton>
#include <KRestrictedLine>
#include <QCheckBox>
#include <klocalizedstring.h>

class TwitterApiSearchTimelineWidget::Private
{
public:
    Private()
        :currentPage(1)
    {}
    KPushButton *reload;
    KPushButton *next;
    KPushButton *previous;
    KRestrictedLine *pageNumber;
    QCheckBox *autoUpdate;
    uint currentPage;
};

TwitterApiSearchTimelineWidget::TwitterApiSearchTimelineWidget(Choqok::Account* account, const QString& timelineName, QWidget* parent)
    : TimelineWidget(account, timelineName, parent), d(new Private)
{

}

TwitterApiSearchTimelineWidget::~TwitterApiSearchTimelineWidget()
{

}

void TwitterApiSearchTimelineWidget::saveTimeline()
{
    Choqok::UI::TimelineWidget::saveTimeline();
}

void TwitterApiSearchTimelineWidget::loadTimeline()
{
    Choqok::UI::TimelineWidget::loadTimeline();
}

void TwitterApiSearchTimelineWidget::setupUi()
{
    QHBoxLayout *footer = new QHBoxLayout;
    d->reload = new KPushButton(this);
    d->reload->setIcon(KIcon("view-refresh"));
    d->previous = new KPushButton(this);
    d->previous->setIcon(KIcon("go-previous"));
    d->next = new KPushButton(this);
    d->next->setIcon(KIcon("go-next"));
    d->pageNumber = new KRestrictedLine(this);
    d->pageNumber->setValidChars("1234567890");
    d->autoUpdate = new QCheckBox(i18n("Auto-update results"), this);

    footer->addWidget(d->reload);
    footer->addWidget(d->autoUpdate);
    footer->addWidget(d->previous);
    footer->addWidget(d->pageNumber);
    footer->addWidget(d->next);

    connect( d->reload, SIGNAL(clicked(bool)), SLOT(reloadList()) );
    connect( d->next, SIGNAL(clicked(bool)), SLOT(loadNextPage()) );
    connect( d->previous, SIGNAL(clicked(bool)), SLOT(loadPreviousPage()) );
    connect( d->pageNumber, SIGNAL(returnPressed(QString)), SLOT(loadCustomPage(QString)) );

    layout()->addItem(footer);
}

void TwitterApiSearchTimelineWidget::reloadList()
{
    loadCustomPage(QString::number(d->currentPage));
}

void TwitterApiSearchTimelineWidget::loadCustomPage(const QString& pageNumber)
{
    
}

void TwitterApiSearchTimelineWidget::loadNextPage()
{
    loadCustomPage(QString::number(++d->currentPage));
}

void TwitterApiSearchTimelineWidget::loadPreviousPage()
{
    loadCustomPage(QString::number(--d->currentPage));
}

#include "twitterapisearchtimelinewidget.moc"
