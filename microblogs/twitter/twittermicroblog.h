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
#ifndef TWITTERMICROBLOGPLUGIN_H
#define TWITTERMICROBLOGPLUGIN_H

#include <QPointer>

#include "twitterapimicroblog.h"

#include "twitterlist.h"

class TwitterAccount;
class TwitterSearch;
class ChoqokEditAccountWidget;
class KJob;

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TwitterMicroBlog : public TwitterApiMicroBlog
{
    Q_OBJECT
public:
    TwitterMicroBlog(QObject *parent, const QVariantList &args);
    ~TwitterMicroBlog();

    virtual Choqok::Account *createNewAccount(const QString &alias) override;
    virtual ChoqokEditAccountWidget *createEditAccountWidget(Choqok::Account *account, QWidget *parent) override;
    virtual Choqok::UI::MicroBlogWidget *createMicroBlogWidget(Choqok::Account *account, QWidget *parent) override;
    virtual Choqok::UI::TimelineWidget *createTimelineWidget(Choqok::Account *account,
                                                     const QString &timelineName, QWidget *parent) override;
    virtual Choqok::UI::PostWidget *createPostWidget(Choqok::Account *account,
                                             Choqok::Post *post, QWidget *parent) override;
    virtual Choqok::UI::ComposerWidget *createComposerWidget(Choqok::Account *account, QWidget *parent) override;

    virtual QString postUrl(Choqok::Account *account, const QString &username, const QString &postId) const override;

    virtual QUrl profileUrl(Choqok::Account *account, const Choqok::User &user) const override;

    virtual TwitterApiSearch *searchBackend() override;

    virtual QString generateRepeatedByUserTooltip(const QString &username) override;
    virtual QString repeatQuestion() override;
    virtual QMenu *createActionsMenu(Choqok::Account *theAccount, QWidget *parent = Choqok::UI::Global::mainWindow()) override;

    void fetchUserLists(TwitterAccount *theAccount, const QString &username);
    void addListTimeline(TwitterAccount *theAccount, const QString &username, const QString &listname);
    void setListTimelines(TwitterAccount *theAccount, const QStringList &lists);

    virtual Choqok::TimelineInfo *timelineInfo(const QString &timelineName) override;

    void createPostWithAttachment(Choqok::Account *theAccount, Choqok::Post *post, const QString &mediumToAttach = QString());
Q_SIGNALS:
    void userLists(Choqok::Account *theAccount, const QString &username, QList<Twitter::List> lists);

public Q_SLOTS:
    virtual void showDirectMessageDialog(TwitterApiAccount *theAccount = 0,
                                         const QString &toUsername = QString()) override;

protected Q_SLOTS:
    void showListDialog(TwitterApiAccount *theAccount = 0);
    void slotFetchUserLists(KJob *job);

protected:
    using TwitterApiMicroBlog::readDirectMessage;
    virtual Choqok::Post *readDirectMessage(Choqok::Account *theAccount, const QVariantMap &var) override;
    using TwitterApiMicroBlog::readPost;
    virtual Choqok::Post *readPost(Choqok::Account *account, const QVariantMap &var, Choqok::Post *post) override;
    void setTimelineInfos() override;

    QList<Twitter::List> readUserListsFromJson(Choqok::Account *theAccount, QByteArray buffer);
    Twitter::List readListFromJsonMap(Choqok::Account *theAccount, QVariantMap map);
    QMap<KJob *, QString> mFetchUsersListMap;

private:
    QPointer<TwitterSearch> mSearchBackend;
    QMap<QString, Choqok::TimelineInfo *> mListsInfo;
};

#endif
