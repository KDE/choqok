/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPITIMELINEWIDGET_H
#define TWITTERAPITIMELINEWIDGET_H

#include "timelinewidget.h"

class CHOQOK_HELPER_EXPORT TwitterApiTimelineWidget : public Choqok::UI::TimelineWidget
{
    Q_OBJECT
public:
    TwitterApiTimelineWidget(Choqok::Account *account, const QString &timelineName, QWidget *parent = nullptr);
    virtual ~TwitterApiTimelineWidget();

protected Q_SLOTS:
    void removeUnFavoritedPost(Choqok::Account *theAccount, const QString &postId);
};

#endif // TWITTERAPITIMELINEWIDGET_H
