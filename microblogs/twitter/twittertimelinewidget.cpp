/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twittertimelinewidget.h"

TwitterTimelineWidget::TwitterTimelineWidget(Choqok::Account *account, const QString &timelineName,
        QWidget *parent)
    : TwitterApiTimelineWidget(account, timelineName, parent)
{
    if (timelineName.startsWith(QLatin1Char('@'))) {
        setClosable();
    }
}

TwitterTimelineWidget::~TwitterTimelineWidget()
{

}

