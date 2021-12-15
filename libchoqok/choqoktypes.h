/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKTYPES_H
#define CHOQOKTYPES_H

#include <QDateTime>
#include <QUrl>

#include "choqok_export.h"

namespace Choqok
{

enum JobResult {
    Fail = 0,
    Success = 1
};

class CHOQOK_EXPORT User
{
public:
    User()
        : isProtected(false)
    {}
    User(const User& u) = default;
    User(User&& u) = default;
    virtual ~User() {}
    User& operator=(const User& u) = default;
    User& operator=(User&& u) = default;

    QString userId;
    QString realName;
    QString userName;
    QString location;
    QString description;
    QUrl profileImageUrl;
    QUrl homePageUrl;
    bool isProtected;
};

class CHOQOK_EXPORT QuotedPost
{
public:
    User user;
    QString postId;
    QString content;
};

class CHOQOK_EXPORT Post
{
public:
    Post()
        : isFavorited(false), isPrivate(false), isError(false), isRead(false), owners(0)
    {}
    Post(const Post& u) = default;
    Post(Post&& u) = default;
    virtual ~Post() {}
    Post& operator=(const Post& u) = default;
    Post& operator=(Post&& u) = default;
    
    QDateTime creationDateTime;
    QString postId;
    QUrl link;
    QString content;
    QString source;
    QString replyToPostId;
    User replyToUser;
    bool isFavorited;
    User author;
    QString type;
    bool isPrivate;
    bool isError;
    bool isRead;
    User repeatedFromUser;
    QString repeatedPostId;
    QDateTime repeatedDateTime;
    QString conversationId;
    QUrl media;          // first Image of Post, if available
    QuotedPost quotedPost;
    unsigned int owners; // number of associated PostWidgets
};
/**
Describe an specific timeline, Should use by @ref MicroBlog
*/
class CHOQOK_EXPORT TimelineInfo
{
public:
    QString name;
    QString description;
    QString icon;
};

}
#endif
