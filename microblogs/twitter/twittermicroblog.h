/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    virtual QUrl postUrl(Choqok::Account *account, const QString &username, const QString &postId) const override;

    virtual QUrl profileUrl(Choqok::Account *account, const QString &username) const override;

    virtual void fetchPost(Choqok::Account *theAccount, Choqok::Post *post) override;

    virtual TwitterApiSearch *searchBackend() override;

    virtual QString generateRepeatedByUserTooltip(const QString &username) override;
    virtual QString repeatQuestion() override;
    virtual QMenu *createActionsMenu(Choqok::Account *theAccount, QWidget *parent = Choqok::UI::Global::mainWindow()) override;

    void fetchUserLists(TwitterAccount *theAccount, const QString &username);
    void addListTimeline(TwitterAccount *theAccount, const QString &username, const QString &listname);
    void setListTimelines(TwitterAccount *theAccount, const QStringList &lists);

    virtual Choqok::TimelineInfo *timelineInfo(const QString &timelineName) override;

    void createPostWithAttachment(Choqok::Account *theAccount, Choqok::Post *post, const QString &mediumToAttach = QString());

    void verifyCredentials(TwitterAccount *theAccount);

Q_SIGNALS:
    void userLists(Choqok::Account *theAccount, const QString &username, QList<Twitter::List> lists);

public Q_SLOTS:
    virtual void showDirectMessageDialog(TwitterApiAccount *theAccount = nullptr,
                                         const QString &toUsername = QString()) override;

protected Q_SLOTS:
    void showListDialog(TwitterApiAccount *theAccount = nullptr);
    void slotFetchUserLists(KJob *job);
    void slotFetchVerifyCredentials(KJob *job);

protected:
    virtual void requestTimeLine(Choqok::Account *theAccount, QString timelineName,
                                 QString sincePostId, int page = 1, QString maxId = QString()) override;
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
