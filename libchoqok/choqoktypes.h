/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/

#ifndef CHOQOKTYPES_H
#define CHOQOKTYPES_H
#include <QDateTime>
#include <QVariant>
#include "choqok_export.h"

namespace Choqok
{

enum JobResult{
    Fail = 0,
    Success = 1
};

struct CHOQOK_EXPORT User {
public:
    QString userId;
    QString realName;
    QString userName;
    QString location;
    QString description;
    QString profileImageUrl;
    QString homePageUrl;
    bool isProtected;
    uint followersCount;
};

struct CHOQOK_EXPORT Post {
public:
    Post()
    :isFavorited(false), isPrivate(false), isError(false)
    {}
    QDateTime creationDateTime;
    QString postId;
    QString title;
    QString link;
    QString content;
    QString source;
    QString replyToPostId;
    QString replyToUserId;
    bool isFavorited;
    QString replyToUserName;
    User author;
    QString type;
    bool isPrivate;
    bool isError;
};
/*
enum FilterAction{
    Delete = 0,
    MoveTo = 1,
    CopyTo = 2
};


struct Filter {
public:
    QStringList applyToAccounts;
    QStringList applyToTimelines;
    FilterAction action;
    QString moveCopyTo;
};
*/

}
#endif