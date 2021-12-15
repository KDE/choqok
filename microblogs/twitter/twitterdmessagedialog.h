/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2015 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERDMESSAGEDIALOG_H
#define TWITTERDMESSAGEDIALOG_H

#include "twitterapidmessagedialog.h"

class KJob;

class TwitterDMessageDialog : public TwitterApiDMessageDialog
{
    Q_OBJECT
public:
    explicit TwitterDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~TwitterDMessageDialog();

private Q_SLOTS:
    void slotTextLimit(KJob *job);

private:
    void fetchTextLimit();

};

#endif // TWITTERDMESSAGEDIALOG_H
