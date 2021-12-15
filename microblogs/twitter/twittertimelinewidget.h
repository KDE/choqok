/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERTIMELINEWIDGET_H
#define TWITTERTIMELINEWIDGET_H

#include "twitterapitimelinewidget.h"

class TwitterTimelineWidget : public TwitterApiTimelineWidget
{

public:
    TwitterTimelineWidget(Choqok::Account *account, const QString &timelineName, QWidget *parent = nullptr);
    ~TwitterTimelineWidget();
};

#endif // TWITTERTIMELINEWIDGET_H
