/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef OCSMICROBLOG_H
#define OCSMICROBLOG_H
#include "microblog.h"

#include <Attica/Activity>

class OCSAccount;
namespace Attica
{
class ProviderManager;
class BaseJob;
}

class OCSMicroblog : public Choqok::MicroBlog
{
    Q_OBJECT
public:
    OCSMicroblog(QObject *parent, const QVariantList &args);
    ~OCSMicroblog();

    virtual ChoqokEditAccountWidget *createEditAccountWidget(Choqok::Account *account, QWidget *parent) override;
    virtual void createPost(Choqok::Account *theAccount, Choqok::Post *post) override;
    virtual void abortCreatePost(Choqok::Account *theAccount, Choqok::Post *post = nullptr) override;
    virtual void fetchPost(Choqok::Account *theAccount, Choqok::Post *post) override;
    virtual void removePost(Choqok::Account *theAccount, Choqok::Post *post) override;
    virtual void saveTimeline(Choqok::Account *account, const QString &timelineName,
                              const QList< Choqok::UI::PostWidget * > &timeline) override;
    virtual QList< Choqok::Post * > loadTimeline(Choqok::Account *account, const QString &timelineName) override;
    virtual Choqok::Account *createNewAccount(const QString &alias) override;
    virtual void updateTimelines(Choqok::Account *theAccount) override;
    virtual Choqok::TimelineInfo *timelineInfo(const QString &timelineName) override;

    virtual QUrl profileUrl(Choqok::Account *account, const QString &username) const override;

    Attica::ProviderManager *providerManager();

    bool isOperational();
    virtual void aboutToUnload() override;

Q_SIGNALS:
    void initialized();

protected Q_SLOTS:
    void slotTimelineLoaded(Attica::BaseJob *);
    void slotCreatePost(Attica::BaseJob *);
    void slotDefaultProvidersLoaded();

private:
    enum Task {Update};
    QList <Choqok::Post *> parseActivityList(const Attica::Activity::List &list);
    Attica::ProviderManager *mProviderManager;
    QMap<Attica::BaseJob *, OCSAccount *> mJobsAccount;
    QMap<Attica::BaseJob *, Choqok::Post *> mJobsPost;
    QMultiMap<Choqok::Account *, Task> scheduledTasks;
    bool mIsOperational;
};

#endif // OCSMICROBLOG_H
