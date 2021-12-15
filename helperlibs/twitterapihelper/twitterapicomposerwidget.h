/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPICOMPOSERWIDGET_H
#define TWITTERAPICOMPOSERWIDGET_H

#include "twitterapihelper_export.h"
#include "composerwidget.h"

namespace Choqok
{
namespace UI
{
class PostWidget;
}
}

class TWITTERAPIHELPER_EXPORT TwitterApiComposerWidget : public Choqok::UI::ComposerWidget
{
    Q_OBJECT
public:
    explicit TwitterApiComposerWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~TwitterApiComposerWidget();

protected Q_SLOTS:
    virtual void slotNewPostReady(Choqok::UI::PostWidget *widget, Choqok::Account *theAccount);

private:
    class Private;
    Private *const d;
};

#endif // TWITTERAPICOMPOSERWIDGET_H
