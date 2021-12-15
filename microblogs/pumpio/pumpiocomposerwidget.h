/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOCOMPOSERWIDGET_H
#define PUMPIOCOMPOSERWIDGET_H

#include "composerwidget.h"

class PumpIOComposerWidget : public Choqok::UI::ComposerWidget
{
    Q_OBJECT
public:
    explicit PumpIOComposerWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~PumpIOComposerWidget();

public Q_SLOTS:
    void slotSetReply(const QString replyToId, const QString replyToUsername, const QString replyToObjectType);

protected Q_SLOTS:
    virtual void submitPost(const QString &text) override;
    virtual void slotPostSubmited(Choqok::Account *theAccount, Choqok::Post *post) override;

    void cancelAttach();
    void attachMedia();

private:
    class Private;
    Private *const d;

};

#endif // PUMPIOCOMPOSERWIDGET_H
