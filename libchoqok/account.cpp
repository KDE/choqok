/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "account.h"

#include <KSharedConfig>

#include "libchoqokdebug.h"
#include "microblog.h"
#include "passwordmanager.h"

namespace Choqok
{

class Account::Private
{
public:
    Private(Choqok::MicroBlog *parent, const QString &mAlias)
        : alias(mAlias), blog(parent)
    {
        configGroup = new KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Account_%1").arg(alias));
        username = configGroup->readEntry("Username", QString());
        priority = configGroup->readEntry("Priority", static_cast<uint>(0));
        readonly = configGroup->readEntry("ReadOnly", false);
        showInQuickPost = configGroup->readEntry("ShowInQuickPost", true);
        enable = configGroup->readEntry("Enable", true);
        postCharLimit = configGroup->readEntry("PostCharLimit", static_cast<uint>(140));
        password = PasswordManager::self()->readPassword(alias);
    }
    QString username;
    QString password;
    QString alias;
    MicroBlog *blog;
    KConfigGroup *configGroup;
    uint priority;
    bool readonly;
    bool enable;
    bool showInQuickPost;
    uint postCharLimit;
};

Account::Account(Choqok::MicroBlog *parent, const QString &alias)
    : QObject(parent), d(new Private(parent, alias))
{
    qCDebug(CHOQOK);
}

Account::~Account()
{
    qCDebug(CHOQOK) << alias();
//     writeConfig();
    delete d->configGroup;
}

void Account::writeConfig()
{
    d->configGroup->writeEntry("Alias", d->alias);
    d->configGroup->writeEntry("Username", d->username);
    d->configGroup->writeEntry("Priority", d->priority);
    d->configGroup->writeEntry("ReadOnly", d->readonly);
    d->configGroup->writeEntry("Enable", d->enable);
    d->configGroup->writeEntry("ShowInQuickPost", d->showInQuickPost);
    d->configGroup->writeEntry("MicroBlog", microblog()->pluginId());
    d->configGroup->writeEntry("PostCharLimit", d->postCharLimit);
    if (!password().isEmpty()) {
        PasswordManager::self()->writePassword(d->alias, password());
    }
    d->configGroup->sync();
    Q_EMIT modified(this);
}

QString Account::username() const
{
    return d->username;
}

void Account::setUsername(const QString &name)
{
    d->username = name;
}

QString Account::password() const
{
    return d->password;
}

void Account::setPassword(const QString &pass)
{
    d->password = pass;
}

QString Account::alias() const
{
    return d->alias;
}

void Account::setAlias(const QString &alias)
{
    d->alias = alias;
    d->configGroup->deleteGroup();
    delete d->configGroup;
    d->configGroup = new KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Account_%1").arg(d->alias));
    writeConfig();
}

bool Account::isReadOnly() const
{
    return d->readonly;
}
void Account::setReadOnly(bool readonly /*= true*/)
{
    d->readonly = readonly;
}

MicroBlog *Account::microblog() const
{
    return d->blog;
}

void Account::setPriority(uint priority)
{
    d->priority = priority;
//     d->configGroup->writeEntry( "Priority", d->priority );
}

uint Account::priority() const
{
    return d->priority;
}

bool Account::isEnabled() const
{
    return d->enable;
}

void Account::setEnabled(bool enabled)
{
    d->enable = enabled;
    Q_EMIT status(this, enabled);
}

uint Account::postCharLimit() const
{
    return d->postCharLimit;
}

void Account::setPostCharLimit(const uint limit)
{
    d->postCharLimit = limit;
}

bool Account::showInQuickPost() const
{
    return d->showInQuickPost;
}

void Account::setShowInQuickPost(bool show)
{
    d->showInQuickPost = show;
}

KConfigGroup *Account::configGroup() const
{
    return d->configGroup;
}

QStringList Account::timelineNames() const
{
    return d->blog->timelineNames();
}

}

