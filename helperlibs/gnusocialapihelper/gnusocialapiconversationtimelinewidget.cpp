/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gnusocialapiconversationtimelinewidget.h"

#include <QTimer>

#include <KLocalizedString>

#include "choqokappearancesettings.h"
#include "postwidget.h"

#include "gnusocialapimicroblog.h"

GNUSocialApiConversationTimelineWidget::GNUSocialApiConversationTimelineWidget(Choqok::Account *curAccount,
        const QString &convId, QWidget *parent)
    : TwitterApiTimelineWidget(curAccount, i18n("Conversation %1", convId), parent)
{
    setWindowTitle(i18n("Please wait..."));
    GNUSocialApiMicroBlog *mBlog = qobject_cast<GNUSocialApiMicroBlog *>(curAccount->microblog());
    resize(choqokMainWindow->width(), 500);
    move(choqokMainWindow->pos());
    conversationId = convId;
    connect(mBlog, &GNUSocialApiMicroBlog::conversationFetched,
            this, &GNUSocialApiConversationTimelineWidget::slotConversationFetched);
    mBlog->fetchConversation(curAccount, convId);
}

GNUSocialApiConversationTimelineWidget::~GNUSocialApiConversationTimelineWidget()
{

}

void GNUSocialApiConversationTimelineWidget::saveTimeline()
{
}

void GNUSocialApiConversationTimelineWidget::loadTimeline()
{
}

void GNUSocialApiConversationTimelineWidget::slotConversationFetched(Choqok::Account *theAccount,
        const QString &convId,
        QList< Choqok::Post * > posts)
{
    if (currentAccount() == theAccount && convId == this->conversationId) {
        setWindowTitle(i18n("Conversation"));
        addNewPosts(posts);
        for (Choqok::UI::PostWidget *post: postWidgets()) {
            post->setReadWithSignal();
        }
        QTimer::singleShot(0, this, SLOT(updateHeight()));
    }
}

void GNUSocialApiConversationTimelineWidget::updateHeight()
{
    int height = 25;
    for (Choqok::UI::PostWidget *wd: postWidgets()) {
        height += wd->height() + 5;
    }
    if (height > choqokMainWindow->height()) {
        height = choqokMainWindow->height();
    }
    resize(width(), height);
    if (!Choqok::AppearanceSettings::useReverseOrder()) {
        scrollToBottom();
    }
}

#include "moc_gnusocialapiconversationtimelinewidget.cpp"
