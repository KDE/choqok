/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2014 Andrea Scarpino <scarpino@kde.org>
    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
            connect(mbw, &Choqok::UI::TimelineWidget::forwardReply,
                    pumpComposer, &PumpIOComposerWidget::slotSetReply);
        }
        slotUpdateUnreadCount(mbw->unreadCount(), mbw);
    } else {
        qCDebug(CHOQOK) << "Cannot Create a new TimelineWidget for timeline " << name;
        return nullptr;
    }

    if (timelinesTabWidget()->count() == 1) {
        timelinesTabWidget()->setTabBarHidden(true);
    } else {
        timelinesTabWidget()->setTabBarHidden(false);
    }

    return mbw;
}

#include "moc_pumpiomicroblogwidget.cpp"
