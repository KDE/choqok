/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
    connect(mBlog, SIGNAL(conversationFetched(Choqok::Account*,QString,QList<Choqok::Post*>)),
            this, SLOT(slotConversationFetched(Choqok::Account*,QString,QList<Choqok::Post*>)));
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

