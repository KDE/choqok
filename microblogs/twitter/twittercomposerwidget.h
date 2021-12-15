/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERCOMPOSERWIDGET_H
#define TWITTERCOMPOSERWIDGET_H

#include "twitterapicomposerwidget.h"

class TwitterComposerWidget : public TwitterApiComposerWidget
{
    Q_OBJECT
public:
    explicit TwitterComposerWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~TwitterComposerWidget();

protected Q_SLOTS:
    virtual void submitPost(const QString &text) override;
    void slotPostMediaSubmitted(Choqok::Account *theAccount, Choqok::Post *post);
    void selectMediumToAttach();
    void cancelAttachMedium();

private:
    class Private;
    Private *d;
};

#endif // TWITTERCOMPOSERWIDGET_H
