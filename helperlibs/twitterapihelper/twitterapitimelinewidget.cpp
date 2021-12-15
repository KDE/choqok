/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapitimelinewidget.h"

#include "twitterapimicroblog.h"
#include "postwidget.h"

TwitterApiTimelineWidget::TwitterApiTimelineWidget(Choqok::Account *account, const QString &timelineName,
        QWidget *parent)
    : TimelineWidget(account, timelineName, parent)
{
    if (timelineName == QLatin1String("Favorite")) {
        TwitterApiMicroBlog *mBlog = qobject_cast<TwitterApiMicroBlog *>(account->microblog());
        connect(mBlog, &TwitterApiMicroBlog::favoriteRemoved, this,
                &TwitterApiTimelineWidget::removeUnFavoritedPost);
    }
}

TwitterApiTimelineWidget::~TwitterApiTimelineWidget()
{

}

void TwitterApiTimelineWidget::removeUnFavoritedPost(Choqok::Account *theAccount, const QString &postId)
{
    if (theAccount == currentAccount()) {
        if (posts().contains(postId)) {
            posts().value(postId)->close();
        }
    }
}

