/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "accountmanager.h"

#include <QUrl>
#include <QStandardPaths>
#include <QApplication>

#include <KConfig>
#include <KConfigGroup>
#include <KIO/DeleteJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWallet>

#include "libchoqokdebug.h"
#include "microblog.h"
#include "passwordmanager.h"
#include "pluginmanager.h"

namespace Choqok
{

static QList<Account *> sortAccountsByPriority(QList<Account *> &accounts)
{
    qCDebug(CHOQOK) << accounts.count();
    QList<Account *> result;
    for (Account *ac: accounts) {
        bool inserted = false;
        int i = 0;
        while (i < result.count()) {
            if (ac->priority() < result[i]->priority()) {
                result.insert(i, ac);
                inserted = true;
                break;
            }
            ++i;
        }
        if (!inserted) {
            result.insert(i, ac);
        }
    }
    return result;
}

class AccountManager::Private
{
public:
    Private()
        : conf(nullptr)
    {}
    QList<Account *> accounts;
    KSharedConfig::Ptr conf;
    QString lastError;
};

AccountManager::AccountManager()
    : QObject(qApp), d(new Private)
{
    qCDebug(CHOQOK);
    d->conf = KSharedConfig::openConfig();
}

AccountManager::~AccountManager()
{
    qCDebug(CHOQOK);
    mSelf = nullptr;
    d->conf->sync();
    delete d;
}

AccountManager *AccountManager::mSelf = nullptr;

AccountManager *AccountManager::self()
{
    if (!mSelf) {
        mSelf = new AccountManager;
    }
    return mSelf;
}

const QList< Account * > &AccountManager::accounts() const
{
    return d->accounts;
}

Account *AccountManager::findAccount(const QString &alias)
{
    qCDebug(CHOQOK) << "Finding:" << alias;
    for (Account *ac: d->accounts) {
        if (ac->alias().compare(alias) == 0) {
            return ac;
        }
    }

    d->lastError = i18n("There is no account with alias %1.", alias);
    return nullptr;
}

bool AccountManager::removeAccount(const QString &alias)
{
    qCDebug(CHOQOK) << "Removing" << alias;
    int count = d->accounts.count();
    d->conf->deleteGroup(QStringLiteral("Account_%1").arg(alias));
    d->conf->sync();
    for (int i = 0; i < count; ++i) {
        if (d->accounts[i]->alias() == alias) {
            Choqok::Account *a = d->accounts.takeAt(i);
            if (!a) {
                return false;
            }
            QStringList names = a->timelineNames();
            while (!names.isEmpty()) {
                const QString tmpFile = QStandardPaths::locate(QStandardPaths::DataLocation,
                                                               generatePostBackupFileName(a->alias(), names.takeFirst()));
                qCDebug(CHOQOK) << "Will remove" << tmpFile;
                const QUrl path = QUrl::fromLocalFile(tmpFile);

                if (path.isValid()) {
                    KIO::StatJob *job = KIO::stat(path, KIO::StatJob::SourceSide, 1);
                    KJobWidgets::setWindow(job, UI::Global::mainWindow());
                    job->exec();
                    KIO::DeleteJob *delJob = KIO::del(path);
                    KJobWidgets::setWindow(delJob, UI::Global::mainWindow());
                    delJob->exec();
                }
            }
            a->deleteLater();
            PasswordManager::self()->removePassword(alias);
            Q_EMIT accountRemoved(alias);
            return true;
        }
    }
    d->lastError = i18n("There is no account with alias %1.", alias);
    return false;
}

Account *AccountManager::registerAccount(Account *account)
{
    qCDebug(CHOQOK) << "Adding:" << account->alias();

    if (!account || d->accounts.contains(account) || account->alias().isEmpty()) {
        return nullptr;
    }

    // If this account already exists, do nothing
    for (Choqok::Account *curracc: d->accounts) {
        if (account->alias() == curracc->alias()) {
            d->lastError = i18n("An account with this alias already exists: a unique alias has to be specified.");
            qCDebug(CHOQOK) << "An account with this alias already exists: a unique alias has to be specified.";
            return nullptr;
        }
    }

    d->accounts.append(account);
    d->accounts = sortAccountsByPriority(d->accounts);

    Q_EMIT accountAdded(account);
    return account;
}

void AccountManager::loadAllAccounts()
{
    qCDebug(CHOQOK);
    for (Account *ac: d->accounts) {
        ac->deleteLater();
    }
    d->accounts.clear();
    const QStringList accountGroups = d->conf->groupList().filter(QRegExp(QLatin1String("^Account_")));
    qCDebug(CHOQOK) << accountGroups;
    for (const QString &grp: accountGroups) {
        qCDebug(CHOQOK) << grp;
        KConfigGroup cg(d->conf, grp);
//         KConfigGroup pluginConfig( d->conf, QLatin1String("Plugins") );

        QString blog = cg.readEntry("MicroBlog", QString());
        Choqok::MicroBlog *mBlog = nullptr;
        if (!blog.isEmpty() && cg.readEntry("Enabled", true)) {
            mBlog = qobject_cast<MicroBlog *>(PluginManager::self()->loadPlugin(blog));
        }
        if (mBlog) {
            const QString alias = cg.readEntry("Alias", QString());
            if (alias.isEmpty()) {
                continue;    ///Unknown alias
            }
            Account *acc = mBlog->createNewAccount(alias);
            if (acc) {
                d->accounts.append(acc);
            }
        }
    }
    qCDebug(CHOQOK) << d->accounts.count() << "accounts loaded.";
    d->accounts = sortAccountsByPriority(d->accounts);
    Q_EMIT allAccountsLoaded();
}

QString AccountManager::generatePostBackupFileName(const QString &alias, const QString &name)
{
    return QString(alias + QLatin1Char('_') + name + QLatin1String("_backuprc"));
}

QString AccountManager::lastError() const
{
    return d->lastError;
}

}

#include "moc_accountmanager.cpp"
