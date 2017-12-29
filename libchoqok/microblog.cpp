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

#include "microblog.h"

#include <QMenu>
#include <QTimer>

#include <KLocalizedString>

#include "account.h"
#include "accountmanager.h"
#include "choqokbehaviorsettings.h"
#include "composerwidget.h"
#include "libchoqokdebug.h"
#include "microblogwidget.h"
#include "postwidget.h"
#include "timelinewidget.h"

namespace Choqok
{
class MicroBlog::Private
{
public:
    QString serviceName;
    QString homepage;
    QStringList timelineTypes;
    QTimer *saveTimelinesTimer;
};

MicroBlog::MicroBlog(const QString &componentName, QObject *parent)
    : Plugin(componentName, parent), d(new Private)
{
    qCDebug(CHOQOK);
    d->saveTimelinesTimer = new QTimer(this);
    d->saveTimelinesTimer->setInterval(BehaviorSettings::notifyInterval() * 60000);
    connect(d->saveTimelinesTimer, &QTimer::timeout, this, &MicroBlog::saveTimelines);
    connect(BehaviorSettings::self(), &BehaviorSettings::configChanged, this,
            &MicroBlog::slotConfigChanged);
    d->saveTimelinesTimer->start();
}

MicroBlog::~MicroBlog()
{
    qCDebug(CHOQOK);
    delete d;
}

QMenu *MicroBlog::createActionsMenu(Account *, QWidget *parent)
{
    return new QMenu(parent);
}

QString MicroBlog::serviceName() const
{
    return d->serviceName;
}

QString MicroBlog::homepageUrl() const
{
    return d->homepage;
}

QString MicroBlog::errorString(ErrorType type)
{
    switch (type) {
    case ServerError:
        return i18n("The server returned an error");
        break;
    case CommunicationError:
        return i18n("Error on communication with server");
        break;
    case ParsingError:
        return i18n("Error on parsing results");
        break;
    case AuthenticationError:
        return i18n("Authentication error");
        break;
    case NotSupportedError:
        return i18n("The server does not support this feature");
        break;
    case OtherError:
        return i18n("Unknown error");
        break;
    };
    return QString();
}

void MicroBlog::setServiceName(const QString &serviceName)
{
    d->serviceName = serviceName;
}

void MicroBlog::setServiceHomepageUrl(const QString &homepage)
{
    d->homepage = homepage;
}

QStringList MicroBlog::timelineNames() const
{
    return d->timelineTypes;
}

void MicroBlog::setTimelineNames(const QStringList &types)
{
    d->timelineTypes = types;
}

void MicroBlog::addTimelineName(const QString &name)
{
    d->timelineTypes << name;
}

bool MicroBlog::isValidTimeline(const QString &timeline)
{
    return d->timelineTypes.contains(timeline);
}

void MicroBlog::slotConfigChanged()
{
    d->saveTimelinesTimer->setInterval(BehaviorSettings::notifyInterval() * 60000);
}

/// UI Objects:

Account *MicroBlog::createNewAccount(const QString &alias)
{
    Choqok::Account *acc = Choqok::AccountManager::self()->findAccount(alias);
    if (!acc) {
        return new Choqok::Account(this, alias);
    } else {
        return 0;
    }
}

UI::MicroBlogWidget *MicroBlog::createMicroBlogWidget(Account *account, QWidget *parent)
{
    return new UI::MicroBlogWidget(account, parent);
}

UI::ComposerWidget *MicroBlog::createComposerWidget(Account *account, QWidget *parent)
{
    return new UI::ComposerWidget(account, parent);
}

UI::TimelineWidget *MicroBlog::createTimelineWidget(Account *account, const QString &timelineName, QWidget *parent)
{
    return new UI::TimelineWidget(account, timelineName, parent);
}

UI::PostWidget *MicroBlog::createPostWidget(Account *account, Choqok::Post *post, QWidget *parent)
{
    return new UI::PostWidget(account, post, parent);
}

TimelineInfo *MicroBlog::timelineInfo(const QString &)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
    return 0;
}

void MicroBlog::abortAllJobs(Account *)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

void MicroBlog::abortCreatePost(Account *, Post *)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

void MicroBlog::createPost(Account *, Post *)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

void MicroBlog::fetchPost(Account *, Post *)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

void MicroBlog::removePost(Account *, Post *)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

void MicroBlog::updateTimelines(Account *)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

QList< Post * > MicroBlog::loadTimeline(Account *, const QString &)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
    return QList<Post *>();
}

void MicroBlog::saveTimeline(Account *, const QString &, const QList< UI::PostWidget * > &)
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
}

QUrl MicroBlog::postUrl(Account *, const QString &, const QString &) const
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
    return QUrl();
}

QUrl MicroBlog::profileUrl(Account *, const QString &) const
{
    qCWarning(CHOQOK) << "MicroBlog Plugin should implement this!";
    return QUrl();
}

}
