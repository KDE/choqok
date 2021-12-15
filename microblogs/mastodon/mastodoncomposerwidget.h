/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MASTODONCOMPOSERWIDGET_H
#define MASTODONCOMPOSERWIDGET_H

#include "composerwidget.h"

class MastodonComposerWidget : public Choqok::UI::ComposerWidget
{
    Q_OBJECT
public:
    explicit MastodonComposerWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~MastodonComposerWidget();

protected Q_SLOTS:
    virtual void submitPost(const QString &text) override;
    virtual void slotPostSubmited(Choqok::Account *theAccount, Choqok::Post *post) override;

    void cancelAttach();
    void attachMedia();

private:
    class Private;
    Private *const d;

};

#endif // MASTODONCOMPOSERWIDGET_H
