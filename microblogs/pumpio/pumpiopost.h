/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOPOST_H
#define PUMPIOPOST_H

#include <QStringList>

#include "choqoktypes.h"

class PumpIOPost : public Choqok::Post
{
public:
    explicit PumpIOPost();
    virtual ~PumpIOPost();

    QUrl replies;
    QStringList shares;
    QStringList to;
    QStringList cc;
    QString replyToObjectType;
};

#endif //PUMPIOPOST_H
