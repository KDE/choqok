/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIMICROBLOG_H
#define TWITTERAPIMICROBLOG_H

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QUrlQuery>

#include "microblog.h"
#include "twitterapisearch.h"

class TwitterApiSearchTimelineWidget;
class TwitterApiAccount;
class KJob;

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TWITTERAPIHELPER_EXPORT TwitterApiMicroBlog : public Choqok::MicroBlog
{
    Q_OBJECT
public:
    ~TwitterApiMicroBlog();

    virtual QMenu *createActionsMenu(Choqok::Account *theAccount,
                                     QWidget *parent = Choqok::UI::Global::mainWindow()) override;
    virtual QList< Choqok::Post * > loadTimeline(Choqok::Account *accountAlias, const QString &timelineName) override;
    virtual void saveTimeline(Choqok::Account *account, const QString &timelineName,
                              const QList< Choqok::UI::PostWidget * > &timeline) override;

    virtual Choqok::UI::ComposerWidget *createComposerWidget(Choqok::Account *account, QWidget *parent) override;
    /**
    \brief Create a new post

    @see postCreated()
    @see abortCreatePost()
    */
    virtual void createPost(Choqok::Account *theAccount, Choqok::Post *post) override;

    /**
    \brief Abort all requests!
    */
    virtual void abortAllJobs(Choqok::Account *theAccount) override;

    /**
    \brief Abort all of createPost requests!
    */
    virtual void abortCreatePost(Choqok::Account *theAccount, Choqok::Post *post = nullptr) override;

    /**
    \brief Fetch a post

    @see postFetched()
    */
    virtual void fetchPost(Choqok::Account *theAccount, Choqok::Post *post) override;

    /**
    \brief Remove a post

    @see postRemoved()
    */
    virtual void removePost(Choqok::Account *theAccount, Choqok::Post *post) override;

    /**
    \brief Repeat/Retweet a post using the API
    */
    virtual void repeatPost(Choqok::Account *theAccount, const QString &postId);

    /**
    Request to update all timelines of account!
    They will arrive in several signals! with timelineDataReceived() signal!

    @see timelineDataReceived()
    */
    virtual void updateTimelines(Choqok::Account *theAccount) override;

    /**
     add post with Id @p postId to @p theAccount favorite list
    */
    virtual void createFavorite(Choqok::Account *theAccount, const QString &postId);

    /**
     remove post with Id @p postId from @p theAccount favorite list
    */
    virtual void removeFavorite(Choqok::Account *theAccount, const QString &postId);

    /**
    Create friendship, or Follow/Subscribe to user with username or screen name @p username
    i.e. Follow / Subscribe
    */
    virtual void createFriendship(Choqok::Account *theAccount, const QString &username);

    /**
    Destroy friendship with user with username or screen name @p username
    i.e. Un Follow / UnSubscribe
    */
    virtual void destroyFriendship(Choqok::Account *theAccount, const QString &username);

    /**
    Block user with username or screen name @p username
    */
    virtual void blockUser(Choqok::Account *theAccount, const QString &username);

    /**
     * Report user as a spam with username or screen name @p username
     */
    virtual void reportUserAsSpam(Choqok::Account *theAccount, const QString &username);

    virtual void aboutToUnload() override;

    virtual void listFriendsUsername(TwitterApiAccount *theAccount, bool active = false);

    virtual void listFollowersUsername(TwitterApiAccount *theAccount, bool active = false);

    virtual Choqok::TimelineInfo *timelineInfo(const QString &timelineName) override;

    /**
    Return search backend to use for search.
    Should be implemented on sub classes
    */
    virtual TwitterApiSearch *searchBackend() = 0;

    virtual TwitterApiSearchTimelineWidget *createSearchTimelineWidget(Choqok::Account *theAccount,
            QString name, const SearchInfo &info,
            QWidget *parent);

    QDateTime dateFromString(const QString &date);

    /**
     * The text to add under repeated posts, to notice user about it.
     */
    virtual QString generateRepeatedByUserTooltip(const QString &username) = 0;

    /**
     * The question will show to confirm repeat post.
     */
    virtual QString repeatQuestion() = 0;

    virtual QByteArray authorizationHeader(TwitterApiAccount *theAccount,
                                           const QUrl &requestUrl, QNetworkAccessManager::Operation method,
                                           const QVariantMap &params = QVariantMap());

public Q_SLOTS:
    /**
    Launch a dialog to send direct message.
    There are 2 ways to use this function:
        1. Calling with theAccount option
        2. Get called by a signal from a QAction (Microblog menu)
    */
    virtual void showDirectMessageDialog(TwitterApiAccount *theAccount = nullptr,
                                         const QString &toUsername = QString());

    void showSearchDialog(TwitterApiAccount *theAccount = nullptr);

Q_SIGNALS:
    void favoriteCreated(Choqok::Account *theAccount, const QString &postId);
    void favoriteRemoved(Choqok::Account *theAccount, const QString &postId);
    void friendsUsernameListed(TwitterApiAccount *theAccount, const QStringList &friendsList);
    void followersUsernameListed(TwitterApiAccount *theAccount, const QStringList &friendsList);

    void friendshipCreated(Choqok::Account *theAccount, const QString &newFriendUsername);
    void friendshipDestroyed(Choqok::Account *theAccount, const QString &username);
    void userBlocked(Choqok::Account *theAccount, const QString &blockedUsername);

protected Q_SLOTS:
    virtual void slotCreatePost(KJob *job);
    virtual void slotFetchPost(KJob *job);
    virtual void slotRemovePost(KJob *job);
    virtual void slotCreateFavorite(KJob *job);
    virtual void slotRemoveFavorite(KJob *job);
    virtual void slotRequestTimeline(KJob *job);
    virtual void requestFriendsScreenName(TwitterApiAccount *theAccount, bool active);
    void slotRequestFriendsScreenNameActive(KJob *job);
    void slotRequestFriendsScreenNamePassive(KJob *job);
    virtual void requestFollowersScreenName(TwitterApiAccount *theAccount, bool active);
    void slotRequestFollowersScreenNameActive(KJob *job);
    void slotRequestFollowersScreenNamePassive(KJob *job);
    virtual void slotCreateFriendship(KJob *job);
    virtual void slotDestroyFriendship(KJob *job);
    virtual void slotBlockUser(KJob *job);
    virtual void slotReportUser(KJob *job);
    virtual void slotUpdateFriendsList();

protected:
    TwitterApiMicroBlog(const QString &componentName, QObject *parent = nullptr);
    /**
     Request update for @p timelineName timeline.
     timelineName should be a valid, previously created timeline.
    */
    virtual void requestTimeLine(Choqok::Account *theAccount, QString timelineName,
                                 QString sincePostId, int page = 1, QString maxId = QString());

    virtual void setTimelineInfos();

    void setRepeatedOfInfo(Choqok::Post *post, Choqok::Post *repeatedPost);

    virtual Choqok::Post *readPost(Choqok::Account *theAccount,
                                   const QVariantMap &var, Choqok::Post *post);
    virtual Choqok::Post *readPost(Choqok::Account *theAccount,
                                   const QByteArray &buffer, Choqok::Post *post);
    virtual QList<Choqok::Post *> readTimeline(Choqok::Account *theAccount, const QByteArray &buffer);
    virtual Choqok::Post *readDirectMessage(Choqok::Account *theAccount, const QByteArray &buffer);
    virtual Choqok::Post *readDirectMessage(Choqok::Account *theAccount, const QVariantMap &var);
    virtual QList<Choqok::Post *> readDirectMessages(Choqok::Account *theAccount, const QByteArray &buffer);
    virtual QStringList readFriendsScreenName(Choqok::Account *theAccount, const QByteArray &buffer);
    virtual QStringList readFollowersScreenName(Choqok::Account *theAccount, const QByteArray &buffer);
    virtual Choqok::User *readUserInfo(const QByteArray &buffer);
    virtual Choqok::User readUser(Choqok::Account *theAccount, const QVariantMap &map);
    /**
    Checks json returned from server for error, and return error string, Or an empty string if nothing found!
    */
    virtual QString checkForError(const QByteArray &buffer);
    void finishRequestFriendsScreenName(KJob *job, bool active);
    void finishRequestFollowersScreenName(KJob *job, bool active);
    void setQuotedPost(Choqok::Post* post, Choqok::Post* quotedPost);

    ///==========================================
    QHash<QString, QString> timelineApiPath;//TimelineType, path
    QMap<QString, Choqok::TimelineInfo *> mTimelineInfos; //timelineName, Info

    QMap<KJob *, QString> mFavoriteMap; //Job, postId
    QMap<KJob *, Choqok::Post *> mRemovePostMap;
    QMap<KJob *, Choqok::Post *> mCreatePostMap; //Job, post
    QMap<KJob *, Choqok::Post *> mFetchPostMap;
    QMap<KJob *, QString> mRequestTimelineMap; //Job, TimelineType
    QHash< Choqok::Account *, QMap<QString, QString> > mTimelineLatestId; //TimelineType, LatestId
    QMap<KJob *, Choqok::Account *> mJobsAccount;
    QMap<KJob *, QString> mFriendshipMap;
    QStringList friendsList;
    QStringList followersList;

private:
    class Private;
    Private *const d;
};

#endif
