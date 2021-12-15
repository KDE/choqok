/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MASTODONPOST_H
#define MASTODONPOST_H

#include <QStringList>

#include "choqoktypes.h"

class MastodonPost : public Choqok::Post
{
public:
    explicit MastodonPost();
    ~MastodonPost();

};

#endif //MASTODONPOST_H
