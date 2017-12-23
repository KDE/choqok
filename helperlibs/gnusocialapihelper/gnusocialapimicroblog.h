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
#ifndef GNUSOCIALAPIMICROBLOGPLUGIN_H
#define GNUSOCIALAPIMICROBLOGPLUGIN_H

#include <QPointer>

#include "twitterapimicroblog.h"

class GNUSocialApiSearch;
class KJob;

/**
This plugin is to GNU social service.

@Note GNU social was called StatusNet and Laconcia previously, So I just renamed it on UI :D

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_HELPER_EXPORT GNUSocialApiMicroBlog : public TwitterApiMicroBlog
{
    Q_OBJECT
public:
    GNUSocialApiMicroBlog(const QString &componentName, QObject *parent);
    ~GNUSocialApiMicroBlog();

    virtual Choqok::Account *createNewAccount(const QString &alias) override;
    virtual Choqok::UI::MicroBlogWidget *createMicroBlogWidget(Choqok::Account *account, QWidget *parent) override;
    virtual Choqok::UI::TimelineWidget *createTimelineWidget(Choqok::Account *account,
                                                             const QString &timelineName, QWidget *parent) override;
    virtual Choqok::UI::PostWidget *createPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent) override;
    virtual Choqok::UI::ComposerWidget *createComposerWidget(Choqok::Account *account, QWidget *parent) override;
    virtual QUrl profileUrl(Choqok::Account *account, const QString &username) const override;
    virtual QUrl postUrl(Choqok::Account *account, const QString &username, const QString &postId) const override;

    virtual TwitterApiSearch *searchBackend() override;

    void createPostWithAttachment(Choqok::Account *theAccount, Choqok::Post *post,
                                          const QString &mediumToAttach = QString());
    virtual QString generateRepeatedByUserTooltip(const QString &username) override;
    virtual QString repeatQuestion() override;

    void fetchConversation(Choqok::Account *theAccount, const QString &conversationId);

    virtual void requestFriendsScreenName(TwitterApiAccount *theAccount, bool active) override;

    virtual void showDirectMessageDialog(TwitterApiAccount *theAccount = 0, const QString &toUsername = QString()) override;

    static QString usernameFromProfileUrl(const QString &profileUrl);
    static QString hostFromProfileUrl(const QString &profileUrl);

Q_SIGNALS:
    void conversationFetched(Choqok::Account *theAccount, const QString &conversationId,
                             QList<Choqok::Post *> posts);

protected:
    using TwitterApiMicroBlog::readPost;
    virtual Choqok::Post *readPost(Choqok::Account *account, const QVariantMap &var, Choqok::Post *post) override;
    virtual void listFriendsUsername(TwitterApiAccount *theAccount, bool active = false) override;
    virtual QStringList readFriendsScreenName(Choqok::Account *theAccount, const QByteArray &buffer) override;

protected Q_SLOTS:
    void slotFetchConversation(KJob *job);
    void slotRequestFriendsScreenName(KJob *job);

private:
    void doRequestFriendsScreenName(TwitterApiAccount *theAccount, int page);

    QMap<KJob *, QString> mFetchConversationMap;
    QPointer<GNUSocialApiSearch> mSearchBackend;
    int friendsPage;
};

#endif
