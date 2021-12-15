/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GNUSOCIALAPICOMPOSERWIDGET_H
#define GNUSOCIALAPICOMPOSERWIDGET_H

#include "twitterapicomposerwidget.h"

class CHOQOK_HELPER_EXPORT GNUSocialApiComposerWidget : public TwitterApiComposerWidget
{
    Q_OBJECT
public:
    explicit GNUSocialApiComposerWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~GNUSocialApiComposerWidget();

protected Q_SLOTS:
    virtual void submitPost(const QString &text) override;
    void slotPostMediaSubmitted(Choqok::Account *theAccount, Choqok::Post *post);
    void selectMediumToAttach();
    void cancelAttachMedium();
    void slotRebuildEditor(Choqok::Account *theAccount);

private:
    class Private;
    Private *d;
};

#endif // GNUSOCIALAPICOMPOSERWIDGET_H
