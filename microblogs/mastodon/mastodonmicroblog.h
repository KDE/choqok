/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017 Andrea Scarpino <scarpino@kde.org>

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

#ifndef MASTODONMICROBLOG_H
#define MASTODONMICROBLOG_H

#include <QNetworkAccessManager>

#include "microblog.h"

class QUrl;
class KJob;
class MastodonAccount;
class MastodonPost;

class MastodonMicroBlog : public Choqok::MicroBlog
{
    Q_OBJECT
public:
    explicit MastodonMicroBlog(QObject *parent, const QVariantList &args);
    virtual ~MastodonMicroBlog();

    virtual void aboutToUnload() override;

    virtual ChoqokEditAccountWidget *createEditAccountWidget(Choqok::Account *account, QWidget *parent) override;

    virtual Choqok::UI::ComposerWidget *createComposerWidget(Choqok::Account *account, QWidget *parent) override;

    virtual void createPost(Choqok::Account *theAccount, Choqok::Post *post) override;

    virtual Choqok::Account *createNewAccount(const QString &alias) override;

    virtual Choqok::UI::PostWidget *createPostWidget(Choqok::Account *account,
                                                    Choqok::Post *post,
                                                    QWidget *parent) override;

    virtual void fetchPost(Choqok::Account *theAccount, Choqok::Post *post) override;

    virtual QList<Choqok::Post * > loadTimeline(Choqok::Account *account,
                                                const QString &timelineName) override;

    virtual void removePost(Choqok::Account *theAccount, Choqok::Post *post) override;

    virtual QString generateRepeatedByUserTooltip(const QString &username) const;

    virtual QUrl profileUrl(Choqok::Account *account, const QString &username) const override;

    virtual void saveTimeline(Choqok::Account *account, const QString &timelineName,
                              const QList< Choqok::UI::PostWidget * > &timeline) override;

    virtual Choqok::TimelineInfo *timelineInfo(const QString &timelineName) override;

    virtual void updateTimelines(Choqok::Account *theAccount) override;

    void createReply(Choqok::Account *theAccount, MastodonPost *post);

    void toggleReblog(Choqok::Account *theAccount, Choqok::Post *post);

    void toggleFavorite(Choqok::Account *theAccount, Choqok::Post *post);

    void fetchFollowers(MastodonAccount *theAccount, bool active);

    void fetchFollowing(MastodonAccount *theAccount, bool active);

    static QString userNameFromAcct(const QString &acct);
    static QString hostFromAcct(const QString &acct);

Q_SIGNALS:
    void favorite(Choqok::Account *, Choqok::Post *);
    void followersUsernameListed(MastodonAccount *theAccount, const QStringList &friendsList);
    void followingUsernameListed(MastodonAccount *theAccount, const QStringList &friendsList);

public Q_SLOTS:
    virtual void showDirectMessageDialog(MastodonAccount *theAccount = nullptr,
                                         const QString &toUsername = QString());

    void slotRequestFollowersScreenNameActive(KJob *job);
    void slotRequestFollowersScreenNamePassive(KJob *job);

    void slotRequestFollowingScreenNameActive(KJob *job);
    void slotRequestFollowingScreenNamePassive(KJob *job);

protected Q_SLOTS:
    void slotCreatePost(KJob *job);
    void slotFavorite(KJob *job);
    void slotFetchPost(KJob *job);
    void slotReblog(KJob *job);
    void slotRemovePost(KJob *job);
    void slotUpdateTimeline(KJob *job);

protected:
    static const QString homeTimeline;
    static const QString publicTimeline;
    static const QString favouritesTimeline;

    QString authorizationMetaData(MastodonAccount *account) const;

    QString lastTimelineId(Choqok::Account *theAccount, const QString &timeline) const;

    Choqok::Post *readPost(const QVariantMap &var, Choqok::Post *post);

    QList<Choqok::Post * > readTimeline(const QByteArray &buffer);

    void setLastTimelineId(Choqok::Account *theAccount, const QString &timeline,
                           const QString &id);
    void setTimelinesInfo();

    void finishRequestFollowersScreenName(KJob *job, bool active);

    void finishRequestFollowingScreenName(KJob *job, bool active);

    QMap<KJob *, Choqok::Account * > m_accountJobs;
    QMap<KJob *, Choqok::Post * > m_createPostJobs;
    QMap<KJob *, Choqok::Post * > m_favoriteJobs;
    QMap<KJob *, Choqok::Post * > m_removePostJobs;
    QMap<KJob *, Choqok::Post * > m_shareJobs;
    QMap<QString, Choqok::TimelineInfo * > m_timelinesInfos;
    QHash<Choqok::Account *, QMap<QString, QString> > m_timelinesLatestIds;
    QMap<KJob *, Choqok::Account *> mJobsAccount;
    QHash<QString, QString> m_timelinesPaths;
    QMap<KJob *, QString> m_timelinesRequests;

private:
    class Private;
    Private *const d;

};

#endif // MASTODONMICROBLOG_H
