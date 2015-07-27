/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#ifndef LACONICAMICROBLOGPLUGIN_H
#define LACONICAMICROBLOGPLUGIN_H

#include <QPointer>

#include "twitterapimicroblog.h"

class LaconicaSearch;
class ChoqokEditAccountWidget;
class KJob;

/**
This plugin is to GNU social service.

@Note GNU social was called StatusNet and Laconcia previously, So I just renamed it on UI :D

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class LaconicaMicroBlog : public TwitterApiMicroBlog
{
    Q_OBJECT
public:
    LaconicaMicroBlog(QObject *parent, const QVariantList &args);
    ~LaconicaMicroBlog();

    virtual Choqok::Account *createNewAccount(const QString &alias);
    virtual ChoqokEditAccountWidget *createEditAccountWidget(Choqok::Account *account, QWidget *parent);
    virtual Choqok::UI::MicroBlogWidget *createMicroBlogWidget(Choqok::Account *account, QWidget *parent);
    virtual Choqok::UI::TimelineWidget *createTimelineWidget(Choqok::Account *account,
            const QString &timelineName, QWidget *parent);
    virtual Choqok::UI::PostWidget *createPostWidget(Choqok::Account *account,
            Choqok::Post *post, QWidget *parent);
    virtual Choqok::UI::ComposerWidget *createComposerWidget(Choqok::Account *account, QWidget *parent);
    virtual QString profileUrl(Choqok::Account *account, const QString &username) const;
    virtual QString postUrl(Choqok::Account *account, const QString &username,
                            const QString &postId) const;

    virtual TwitterApiSearch *searchBackend();

    virtual void createPostWithAttachment(Choqok::Account *theAccount, Choqok::Post *post,
                                          const QString &mediumToAttach = QString());
    virtual QString generateRepeatedByUserTooltip(const QString &username);
    virtual QString repeatQuestion();

    virtual void fetchConversation(Choqok::Account *theAccount, const QString &conversationId);

    virtual void requestFriendsScreenName(TwitterApiAccount *theAccount, bool active);

    virtual void showDirectMessageDialog(TwitterApiAccount *theAccount = 0, const QString &toUsername = QString());

    static QString usernameFromProfileUrl(const QString &profileUrl);
    static QString hostFromProfileUrl(const QString &profileUrl);

Q_SIGNALS:
    void conversationFetched(Choqok::Account *theAccount, const QString &conversationId,
                             QList<Choqok::Post *> posts);

protected:
    using TwitterApiMicroBlog::readPost;
    virtual Choqok::Post *readPost(Choqok::Account *account, const QVariantMap &var, Choqok::Post *post);
    virtual void listFriendsUsername(TwitterApiAccount *theAccount, bool active = false);
    virtual QStringList readUsersScreenName(Choqok::Account *theAccount, const QByteArray &buffer);

protected Q_SLOTS:
    virtual void slotFetchConversation(KJob *job);
    void slotRequestFriendsScreenName(KJob *job);

private:
    void doRequestFriendsScreenName(TwitterApiAccount *theAccount, int page);

    QMap<KJob *, QString> mFetchConversationMap;
    QPointer<LaconicaSearch> mSearchBackend;
    int friendsPage;
};

#endif
