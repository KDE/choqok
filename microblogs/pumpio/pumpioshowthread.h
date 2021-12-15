/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOSHOWTHREAD_H
#define PUMPIOSHOWTHREAD_H

#include <QWidget>

#include "account.h"
#include "choqoktypes.h"

#include "ui_pumpioshowthread.h"

class PumpIOShowThread : public QWidget, Ui::PumpIOShowThread
{
    Q_OBJECT
public:
    explicit PumpIOShowThread(Choqok::Account *account, Choqok::Post *post,
                              QWidget *parent = nullptr);
    virtual ~PumpIOShowThread();

Q_SIGNALS:
    void forwardReply(const QString replyToId, const QString replyToUsername,
                      const QString replyToObjectType);

protected Q_SLOTS:
    void slotAddPost(Choqok::Account *, Choqok::Post *);

private:
    class Private;
    Private *const d;

};

#endif // PUMPIOSHOWTHREAD_H
