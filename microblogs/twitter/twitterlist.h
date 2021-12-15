/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERLIST_H
#define TWITTERLIST_H

#include "choqoktypes.h"

namespace Twitter
{

enum ListMode {Public, Private};

class List
{
public:
    QString listId;
    QString name;
    QString fullname;
    QString slug;
    QString description;
    int subscriberCount;
    int memberCount;
    QString uri;
    bool isFollowing;
    Twitter::ListMode mode;
    Choqok::User author;
};

}
#endif
