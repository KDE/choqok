/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2015 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GNUSOCIALAPIDMESSAGEDIALOG_H
#define GNUSOCIALAPIDMESSAGEDIALOG_H

#include "twitterapidmessagedialog.h"

class CHOQOK_HELPER_EXPORT GNUSocialApiDMessageDialog : public TwitterApiDMessageDialog
{
    Q_OBJECT
public:
    explicit GNUSocialApiDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~GNUSocialApiDMessageDialog();

};

#endif // GNUSOCIALAPIDMESSAGEDIALOG_H
