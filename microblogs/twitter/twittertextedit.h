/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERTEXTEDIT_H
#define TWITTERTEXTEDIT_H

#include "twitterapitextedit.h"

class TwitterTextEdit : public TwitterApiTextEdit
{
    Q_OBJECT

public:
    explicit TwitterTextEdit(Choqok::Account *theAccount, QWidget *parent = nullptr);
    ~TwitterTextEdit();

protected Q_SLOTS:
    virtual void updateRemainingCharsCount() override;

private Q_SLOTS:
    void slotTCoMaximumLength(KJob *job);

private:
    void fetchTCoMaximumLength();

    class Private;
    Private *const d;
};

#endif // TWITTERTEXTEDIT_H
