/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2014 Andrea Scarpino <scarpino@kde.org>
    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOMICROBLOGWIDGET_H
#define PUMPIOMICROBLOGWIDGET_H

#include "microblogwidget.h"

class PumpIOMicroBlogWidget : public Choqok::UI::MicroBlogWidget
{
    Q_OBJECT
public:
    explicit PumpIOMicroBlogWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~PumpIOMicroBlogWidget();

    void initUi() override;

protected:
    Choqok::UI::TimelineWidget *addTimelineWidgetToUi(const QString &name) override;

};

#endif // PUMPIOMICROBLOGWIDGET_H
