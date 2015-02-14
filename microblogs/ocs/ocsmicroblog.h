/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef OCSMICROBLOG_H
#define OCSMICROBLOG_H
#include "microblog.h"

#include <attica/activity.h>

class OCSAccount;
namespace Attica {
class ProviderManager;
class BaseJob;
}

class OCSMicroblog : public Choqok::MicroBlog
{
Q_OBJECT
public:
    OCSMicroblog(QObject* parent, const QVariantList& args);
    virtual ~OCSMicroblog();

    virtual ChoqokEditAccountWidget* createEditAccountWidget(Choqok::Account* account, QWidget* parent);
    virtual void createPost(Choqok::Account* theAccount, Choqok::Post* post);
    virtual void abortCreatePost(Choqok::Account* theAccount, Choqok::Post* post = 0);
    virtual void fetchPost(Choqok::Account* theAccount, Choqok::Post* post);
    virtual void removePost(Choqok::Account* theAccount, Choqok::Post* post);
    virtual void saveTimeline(Choqok::Account* account, const QString& timelineName,
                              const QList< Choqok::UI::PostWidget* >& timeline);
    virtual QList< Choqok::Post* > loadTimeline(Choqok::Account* account, const QString& timelineName);
    virtual Choqok::Account* createNewAccount(const QString& alias);
    virtual void updateTimelines(Choqok::Account* theAccount);
    virtual Choqok::TimelineInfo* timelineInfo(const QString& timelineName);

    virtual QString profileUrl(Choqok::Account* account, const QString& username) const;

    Attica::ProviderManager* providerManager();

    bool isOperational();
    virtual void aboutToUnload();

Q_SIGNALS:
    void initialized();

protected Q_SLOTS:
    void slotTimelineLoaded(Attica::BaseJob*);
    void slotCreatePost(Attica::BaseJob*);
    void slotDefaultProvidersLoaded();

private:
    enum Task{Update};
    QList <Choqok::Post*> parseActivityList(const Attica::Activity::List &list);
    Attica::ProviderManager* mProviderManager;
    QMap<Attica::BaseJob*, OCSAccount*> mJobsAccount;
    QMap<Attica::BaseJob*, Choqok::Post*> mJobsPost;
    QMultiMap<Choqok::Account*, Task> scheduledTasks;
    bool mIsOperational;
};

#endif // OCSMICROBLOG_H
