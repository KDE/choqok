/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2014      Andrea Scarpino <scarpino@kde.org>
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

#include "pumpiomicroblogwidget.h"

#include "account.h"
#include "timelinewidget.h"

#include "pumpiocomposerwidget.h"
#include "pumpiodebug.h"

PumpIOMicroBlogWidget::PumpIOMicroBlogWidget(Choqok::Account *account, QWidget *parent)
    : MicroBlogWidget::MicroBlogWidget(account, parent)
{
}

PumpIOMicroBlogWidget::~PumpIOMicroBlogWidget()
{
}

void PumpIOMicroBlogWidget::initUi()
{
    Choqok::UI::MicroBlogWidget::initUi();
}

Choqok::UI::TimelineWidget *PumpIOMicroBlogWidget::addTimelineWidgetToUi(const QString &name)
{
    Choqok::UI::TimelineWidget *mbw = currentAccount()->microblog()->createTimelineWidget(currentAccount(), name, this);
    if (mbw) {
        Choqok::TimelineInfo *info = currentAccount()->microblog()->timelineInfo(name);
        timelines().insert(name, mbw);
        timelinesTabWidget()->addTab(mbw, info->name);
        timelinesTabWidget()->setTabIcon(timelinesTabWidget()->indexOf(mbw), QIcon::fromTheme(info->icon));
        connect(mbw, SIGNAL(updateUnreadCount(int)), this, SLOT(slotUpdateUnreadCount(int)));

        PumpIOComposerWidget *pumpComposer = qobject_cast<PumpIOComposerWidget * >(composer());
        if (pumpComposer) {
            connect(mbw, SIGNAL(forwardResendPost(QString)), pumpComposer,
                    SLOT(setText(QString)));
            connect(mbw, SIGNAL(forwardReply(QString,QString,QString)),
                    pumpComposer, SLOT(slotSetReply(QString,QString,QString)));
        }
        slotUpdateUnreadCount(mbw->unreadCount(), mbw);
    } else {
        qCDebug(CHOQOK) << "Cannot Create a new TimelineWidget for timeline " << name;
        return 0L;
    }

    if (timelinesTabWidget()->count() == 1) {
        timelinesTabWidget()->setTabBarHidden(true);
    } else {
        timelinesTabWidget()->setTabBarHidden(false);
    }

    return mbw;
}
