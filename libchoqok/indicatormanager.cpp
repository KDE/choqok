/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "indicatormanager.h"

#include <QIcon>
#include <QTimer>

#include "account.h"
#include "accountmanager.h"
#include "choqokuiglobal.h"
#include "microblog.h"
#include "microblogwidget.h"
#include "choqokbehaviorsettings.h"

#define STR(x) #x
#define XSTR(x) STR(x)

namespace Choqok
{

MessageIndicatorManager::MessageIndicatorManager()
{

    iServer = QIndicate::Server::defaultInstance();
    iServer->setType("message.irc");
    QString desktopFile = QString("%1/%2.desktop")
                          .arg(XSTR(XDG_APPS_INSTALL_DIR))
                          .arg(QCoreApplication::applicationFilePath().section('/', -1));
    iServer->setDesktopFile(desktopFile);
    connect(iServer, SIGNAL(serverDisplay()), SLOT(slotShowMainWindow()));
    if (Choqok::BehaviorSettings::libindicate()) {
        iServer->show();
    }
    connect(Choqok::AccountManager::self(), SIGNAL(allAccountsLoaded()), SLOT(slotCanWorkWithAccs()));
    //QTimer::singleShot ( 500, this, SLOT (slotCanWorkWithAccs()) );

    connect(Choqok::BehaviorSettings::self(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
}

MessageIndicatorManager::~MessageIndicatorManager()
{
}

void MessageIndicatorManager::slotCanWorkWithAccs()
{
    QList<Choqok::UI::MicroBlogWidget *> lst = choqokMainWindow->microBlogsWidgetsList();
    if (lst.count() == Choqok::AccountManager::self()->accounts().count()) {
        for (Choqok::UI::MicroBlogWidget *microBlog, lst) {
            connect(microBlog, SIGNAL(updateUnreadCount(int,int)), SLOT(slotupdateUnreadCount(int,int)));
        }
    } else {
        QTimer::singleShot(500, this, SLOT(slotCanWorkWithAccs()));
    }
}

void MessageIndicatorManager::slotConfigChanged()
{
    if (!Choqok::BehaviorSettings::libindicate()) {
        iServer->hide();
    }
    if (Choqok::BehaviorSettings::libindicate()) {
        iServer->show();
    }
}
void MessageIndicatorManager::slotupdateUnreadCount(int change, int sum)
{
    Q_UNUSED(change);
    QString alias = qobject_cast<Choqok::UI::MicroBlogWidget *> (sender())->currentAccount()->alias();
    if (Choqok::BehaviorSettings::libindicate()) {
        newPostInc(sum, alias, QString());
    }
}

QImage MessageIndicatorManager::getIconByAlias(const QString &alias)
{
    Choqok::Account *acc = Choqok::AccountManager::self()->findAccount(alias);
    return QIcon::fromTheme(acc->microblog()->pluginIcon()).pixmap(QSize(16, 16), QIcon::Normal, QIcon::On).toImage();
}

void MessageIndicatorManager::newPostInc(int unread, const QString &alias, const QString &timeline)
{
    Q_UNUSED(timeline);

    if (!iList.contains(alias)) {
        QIndicate::Indicator *newIndicator = new QIndicate::Indicator(this);
        newIndicator->setNameProperty(alias);
        newIndicator->setIconProperty(getIconByAlias(alias));
        iList.insert(alias, newIndicator);
        connect(iList.value(alias), SIGNAL(display(QIndicate::Indicator*)), SLOT(slotDisplay(QIndicate::Indicator*)));
    }
    iList.value(alias)->setCountProperty(unread);
    iList.value(alias)->setDrawAttentionProperty(unread != 0);
    if (unread == 0) {
        iList.value(alias)->hide();
    } else {
        iList.value(alias)->show();
    }

}

void MessageIndicatorManager::slotDisplay(QIndicate::Indicator *indicator)
{
    QString alias = indicator->nameProperty();
    Choqok::Account *acc = Choqok::AccountManager::self()->findAccount(alias);
    choqokMainWindow->activateTab(acc->priority());
    slotShowMainWindow();
}

void MessageIndicatorManager::slotShowMainWindow()
{
    choqokMainWindow->activateChoqok();
}

MessageIndicatorManager *MessageIndicatorManager::mSelf = nullptr;

MessageIndicatorManager *MessageIndicatorManager::self()
{
    if (!mSelf) {
        mSelf = new MessageIndicatorManager;
    }
    return mSelf;
}

}

#include "moc_indicatormanager.cpp"
