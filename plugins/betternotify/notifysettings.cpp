/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "notifysettings.h"

#include <QDesktopWidget>
#include <QMap>

#include <KConfigGroup>
#include <KSharedConfig>

#include "accountmanager.h"

class NotifySettings::Private
{
public:
    QMap<QString, QStringList> accounts;
    KConfigGroup *accountsConf;
    KConfigGroup *conf;
    int interval;
    QPoint position;
    QColor foregroundColor;
    QColor backgroundColor;
    QFont font;
};

NotifySettings::NotifySettings(QObject *parent)
    : QObject(parent), d(new Private)
{
    d->conf = new KConfigGroup(KSharedConfig::openConfig(), QLatin1String("BetterNotify Plugin"));
    d->accountsConf = new KConfigGroup(KSharedConfig::openConfig(), QLatin1String("BetterNotify Accounts Config"));
    load();
}

NotifySettings::~NotifySettings()
{
    save();
    delete d->accountsConf;
}

QMap< QString, QStringList > NotifySettings::accounts()
{
    return d->accounts;
}

void NotifySettings::setAccounts(QMap< QString, QStringList > accounts)
{
    d->accounts = accounts;
}

void NotifySettings::load()
{
    d->accounts.clear();
    d->accountsConf->sync();
    for (Choqok::Account *acc: Choqok::AccountManager::self()->accounts()) {
        d->accounts.insert(acc->alias(), d->accountsConf->readEntry(acc->alias(), QStringList()));
    }
    d->conf->sync();
    d->interval = d->conf->readEntry("Interval", 7);
    QRect screenRect(QDesktopWidget().screenGeometry());
    QPoint defaultPosition = QPoint(screenRect.center().x() - NOTIFICATION_WIDTH / 2, 30);
    d->position = d->conf->readEntry("NotifyPosition", defaultPosition);
    screenRect.adjust(0, 0, -1 * NOTIFICATION_WIDTH, -1 * NOTIFICATION_HEIGHT);
    if (!screenRect.contains(d->position)) {
        d->position = defaultPosition;
    }
    d->backgroundColor = d->conf->readEntry("NotifyBackground", QColor(0, 0, 0));
    d->foregroundColor = d->conf->readEntry("NotifyForeground", QColor(255, 255, 255));
    d->font = d->conf->readEntry("NotifyFont", QFont());
}

void NotifySettings::save()
{
    for (Choqok::Account *acc: Choqok::AccountManager::self()->accounts()) {
        d->accountsConf->writeEntry(acc->alias(), d->accounts.value(acc->alias()));
    }
    d->conf->writeEntry("Interval", d->interval);
    d->conf->writeEntry("NotifyPosition", d->position);
    d->conf->writeEntry("NotifyBackground", d->backgroundColor);
    d->conf->writeEntry("NotifyForeground", d->foregroundColor);
    d->conf->writeEntry("NotifyFont", d->font);
    d->accountsConf->sync();
}

int NotifySettings::notifyInterval() const
{
    return d->interval;
}

void NotifySettings::setNotifyInterval(int interval)
{
    d->interval = interval;
}

QPoint NotifySettings::position() const
{
    return d->position;
}

void NotifySettings::setPosition(const QPoint &position)
{
    d->position = position;
}

QColor NotifySettings::backgroundColor() const
{
    return d->backgroundColor;
}

void NotifySettings::setBackgroundColor(const QColor &color)
{
    d->backgroundColor = color;
}

QFont NotifySettings::font() const
{
    return d->font;
}

void NotifySettings::setFont(const QFont &font)
{
    d->font = font;
}

QColor NotifySettings::foregroundColor() const
{
    return d->foregroundColor;
}

void NotifySettings::setForegroundColor(const QColor &color)
{
    d->foregroundColor = color;
}
